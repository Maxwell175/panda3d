/**
 * Macro benchmark for the threaded PipelineCycler.  Renders a large instanced
 * scene offscreen through tinydisplay with a tiny render target, so the cull
 * traversal (cycler reads) dominates over rasterization, and drives a workload
 * -- transform animation or procedural node generation -- either on the main
 * thread or on background worker threads, under a configurable render
 * threading-model.  Reports fps, frame-time percentiles, and workload
 * throughput so the same source can be run across threading scenarios and on a
 * pre-EBR build for comparison.
 *
 * Exit: 0 = ran, 77 = no graphics pipe (skip), 1 = arg/setup error.
 */

#include "pandaFramework.h"
#include "load_prc_file.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "transformState.h"
#include "geom.h"
#include "geomNode.h"
#include "geomTriangles.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "trueClock.h"
#include "animControlCollection.h"
#include "auto_bind.h"
#include "loader.h"
#include "partBundleNode.h"
#include "character.h"
#include "epochHolder.h"
#include "epochManager.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <deque>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace {

struct Options {
  double duration = 5.0;
  std::string threading_model;          // "" / "Cull" / "cull/draw"
  std::string display;                  // "" (tinydisplay aux) / pandagl / ...
  int objects = 20000;                  // initial animated scene size
  int groups = 8;
  std::string workload = "animate";     // "animate" | "generate"
  int workers = 0;                      // 0 = workload on main thread
  bool shared = false;                  // animate: workers share the object pool
  int anim_per_frame = 2000;            // main-thread animate batch
  int gen_batch = 200;                  // nodes per generate step
  int gen_window = 4000;                // per-generator live-node cap
  int readers = 0;                      // read-only traverser threads
  bool always_cow = false;              // force pipeline-always-cow
  bool complex = false;                 // build a deep, varied static scene
  int depth = 7;                        // complex: hierarchy depth
  int branch = 4;                       // complex: children per internal node
  int crowd = 0;                        // animated characters (0 = off)
  // Extensions spelled out: an extensionless path needs
  // default-model-extension, which the inline prc does not set.
  std::string crowd_model =             // skinned model with clips beside it
      "samples/roaming-ralph/models/ralph.egg.pz";
  std::string crowd_anim =
      "samples/roaming-ralph/models/ralph-run.egg.pz";
  float crowd_radius = 60.0f;           // scatter disc, in units
  bool crowd_frame_blend = false;       // interpolate between baked frames
  int anim_threads = 0;                 // fan character updates out over N threads
};

std::atomic<bool> g_stop{false};
std::atomic<long long> g_anim_ops{0};
std::atomic<long long> g_gen_nodes{0};
std::atomic<long long> g_read_ops{0};

// One shared single-triangle Geom, instanced into every object.  Real geometry
// (rather than a synthetic set_bounds()) gives every node a genuine, cached
// bounding volume computed the same way Panda computes it in production, and
// keeps the draw thread doing real (if tiny) work -- so both the cull and draw
// pipeline stages exercise the cycler.  Built once before any worker starts.
PT(Geom) g_geom;

void build_shared_geom() {
  PT(GeomVertexData) vdata =
    new GeomVertexData("tri", GeomVertexFormat::get_v3(), Geom::UH_static);
  vdata->set_num_rows(3);
  GeomVertexWriter vw(vdata, "vertex");
  vw.add_data3(0.0f, 0.0f, 0.0f);
  vw.add_data3(1.0f, 0.0f, 0.0f);
  vw.add_data3(0.0f, 0.0f, 1.0f);
  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
  tris->add_vertices(0, 1, 2);
  tris->close_primitive();
  g_geom = new Geom(vdata);
  g_geom->add_primitive(tris);
}

NodePath make_object(NodePath parent, std::mt19937 &rng) {
  std::uniform_real_distribution<float> pos(-50.0f, 50.0f);
  PT(GeomNode) gn = new GeomNode("obj");
  gn->add_geom(g_geom);
  NodePath obj = parent.attach_new_node(gn);
  obj.set_pos(pos(rng), pos(rng) + 200.0f, pos(rng));
  return obj;
}

// Build a deep, varied static subtree: every internal node carries a
// non-identity transform (set_hpr) and a RenderState (set_color), so the cull
// traversal performs real net-transform (Eigen) and net-state composition at
// each node -- representative of a complex scene's steady-state render cost,
// without any per-frame scene mutation.  Leaves carry the shared geom.
void build_complex(NodePath parent, int depth, int branch, std::mt19937 &rng,
                   std::vector<NodePath> &leaves) {
  std::uniform_real_distribution<float> pos(-8.0f, 8.0f);
  std::uniform_real_distribution<float> ang(0.0f, 360.0f);
  std::uniform_real_distribution<float> col(0.2f, 1.0f);
  if (depth <= 0) {
    PT(GeomNode) gn = new GeomNode("leaf");
    gn->add_geom(g_geom);
    NodePath np = parent.attach_new_node(gn);
    np.set_pos(pos(rng), pos(rng), pos(rng));
    leaves.push_back(np);
    return;
  }
  NodePath node = parent.attach_new_node("n");
  node.set_pos(pos(rng), pos(rng), pos(rng));
  node.set_hpr(ang(rng), ang(rng), ang(rng));
  node.set_color(col(rng), col(rng), col(rng), 1.0f);
  for (int i = 0; i < branch; ++i) {
    build_complex(node, depth - 1, branch, rng, leaves);
  }
}

// A crowd of independently-animating skinned characters.
//
// Where --complex measures a large static tree -- many nodes, almost nothing
// dirty per frame -- a crowd is the opposite: every character's joints and
// skinned vertex data change every frame, so the count of DIRTY cyclers scales
// with the crowd, and with it the per-stage copy-on-write and the retire queue.
//
// Each character is a real copy (copy_to, not an instance); instancing would
// share one Character and measure a cache rather than a crowd.  Returns false
// if the model or clip could not be loaded, so the caller skips rather than
// reporting an empty crowd as a fast one.
bool build_crowd(NodePath parent, const Options &o, std::mt19937 &rng,
                 std::vector<std::unique_ptr<AnimControlCollection>> &controls,
                 std::vector<NodePath> &chars) {
  Loader *loader = Loader::get_global_ptr();

  PT(PandaNode) model = loader->load_sync(Filename(o.crowd_model));
  if (model == nullptr) {
    fprintf(stderr, "skip: cannot load crowd model '%s' "
                    "(run from the panda3d source root, or pass --crowd-model)\n",
            o.crowd_model.c_str());
    return false;
  }
  NodePath model_np(model);

  PT(PandaNode) anim = loader->load_sync(Filename(o.crowd_anim));
  if (anim == nullptr) {
    fprintf(stderr, "skip: cannot load crowd animation '%s'\n", o.crowd_anim.c_str());
    return false;
  }

  std::uniform_real_distribution<float> ang(0.0f, 360.0f);
  std::uniform_real_distribution<float> unit(0.0f, 1.0f);

  for (int i = 0; i < o.crowd; ++i) {
    NodePath ch = parent.attach_new_node("crowd-member");

    // A fresh copy of both the model and the clip: binding needs an AnimBundle
    // that is not already bound to another character's PartBundle.
    NodePath body = model_np.copy_to(ch);
    NodePath clip = NodePath(anim).copy_to(body);

    auto c = std::make_unique<AnimControlCollection>();
    auto_bind(ch.node(), *c, ~0);
    if (c->get_num_anims() == 0) {
      fprintf(stderr, "skip: crowd model bound no animations "
                      "(is '%s' a clip for '%s'?)\n",
              o.crowd_anim.c_str(), o.crowd_model.c_str());
      return false;
    }

    // Off by default.  With it on the pose is a fresh interpolation every
    // rendered frame, so the skinning cache never hits -- a fixed per-frame
    // workload, which is the right basis for comparing builds.
    for (int p = 0; p < c->get_num_anims(); ++p) {
      c->get_anim(p)->get_part()->set_frame_blend_flag(o.crowd_frame_blend);
    }

    // Staggered, so the crowd is not one character drawn N times: each member
    // dirties its cyclers on its own schedule.
    c->loop_all(true);
    for (int p = 0; p < c->get_num_anims(); ++p) {
      AnimControl *ac = c->get_anim(p);
      ac->pose(unit(rng) * std::max(1, ac->get_num_frames()));
      ac->set_play_rate(0.8 + 0.4 * unit(rng));
    }
    c->loop_all(true);

    // Uniform over the disc in front of the camera, so most of the crowd
    // survives the frustum test; animation hangs off Character::cull_callback,
    // so an off-frustum character costs nothing.
    double a = ang(rng) * (3.14159265358979 / 180.0);
    double r = std::sqrt(unit(rng)) * o.crowd_radius;
    ch.set_pos((float)(std::cos(a) * r), (float)(std::sin(a) * r) + o.crowd_radius * 1.6f,
               0.0f);
    ch.set_h(ang(rng));

    controls.push_back(std::move(c));
    chars.push_back(ch);
  }
  return true;
}

void animate_worker(std::vector<NodePath> objs, int seed) {
  std::mt19937 rng((unsigned)seed * 2654435761u + 1u);
  std::uniform_int_distribution<size_t> pick(0, objs.empty() ? 0 : objs.size() - 1);
  std::uniform_real_distribution<float> pos(-50.0f, 50.0f);
  long long n = 0;
  while (!g_stop.load(std::memory_order_relaxed) && !objs.empty()) {
    NodePath &o = objs[pick(rng)];
    o.set_pos(pos(rng), pos(rng) + 200.0f, pos(rng));
    ++n;
  }
  g_anim_ops.fetch_add(n, std::memory_order_relaxed);
}

// Procedural generation: stream subtrees in under a private root, removing the
// oldest once the live window is exceeded (models async load/unload).
void generate_worker(NodePath gen_root, int seed, int batch, int window) {
  std::mt19937 rng((unsigned)seed * 40503u + 7u);
  std::deque<NodePath> live;
  long long n = 0;
  while (!g_stop.load(std::memory_order_relaxed)) {
    for (int i = 0; i < batch; ++i) {
      live.push_back(make_object(gen_root, rng));
      ++n;
      if ((int)live.size() > window) {
        live.front().remove_node();
        live.pop_front();
      }
    }
  }
  g_gen_nodes.fetch_add(n, std::memory_order_relaxed);
}

void reader_worker(std::vector<NodePath> objs, int seed) {
  long long n = 0;
  while (!g_stop.load(std::memory_order_relaxed)) {
    for (const NodePath &o : objs) {
      const TransformState *t = o.node()->get_transform();
      n += (t != nullptr);
    }
  }
  g_read_ops.fetch_add(n, std::memory_order_relaxed);
}

int parse(int argc, char **argv, Options &o) {
  for (int i = 1; i < argc; ++i) {
    auto next = [&](int &dst) { if (i + 1 < argc) dst = atoi(argv[++i]); };
    if (!strcmp(argv[i], "--duration") && i + 1 < argc) o.duration = atof(argv[++i]);
    else if (!strcmp(argv[i], "--threading-model") && i + 1 < argc) o.threading_model = argv[++i];
    else if (!strcmp(argv[i], "--display") && i + 1 < argc) o.display = argv[++i];
    else if (!strcmp(argv[i], "--objects")) next(o.objects);
    else if (!strcmp(argv[i], "--groups")) next(o.groups);
    else if (!strcmp(argv[i], "--workload") && i + 1 < argc) o.workload = argv[++i];
    else if (!strcmp(argv[i], "--workers")) next(o.workers);
    else if (!strcmp(argv[i], "--shared")) o.shared = true;
    else if (!strcmp(argv[i], "--anim-per-frame")) next(o.anim_per_frame);
    else if (!strcmp(argv[i], "--gen-batch")) next(o.gen_batch);
    else if (!strcmp(argv[i], "--gen-window")) next(o.gen_window);
    else if (!strcmp(argv[i], "--readers")) next(o.readers);
    else if (!strcmp(argv[i], "--always-cow")) o.always_cow = true;
    else if (!strcmp(argv[i], "--complex")) o.complex = true;
    else if (!strcmp(argv[i], "--depth")) next(o.depth);
    else if (!strcmp(argv[i], "--branch")) next(o.branch);
    else if (!strcmp(argv[i], "--crowd")) next(o.crowd);
    else if (!strcmp(argv[i], "--crowd-model") && i + 1 < argc) o.crowd_model = argv[++i];
    else if (!strcmp(argv[i], "--crowd-anim") && i + 1 < argc) o.crowd_anim = argv[++i];
    else if (!strcmp(argv[i], "--crowd-radius") && i + 1 < argc) o.crowd_radius = (float)atof(argv[++i]);
    else if (!strcmp(argv[i], "--crowd-frame-blend")) o.crowd_frame_blend = true;
    else if (!strcmp(argv[i], "--anim-threads")) next(o.anim_threads);
    else { fprintf(stderr, "unknown arg: %s\n", argv[i]); return 1; }
  }
  return 0;
}

}  // namespace

int main(int argc, char **argv) {
  Options o;
  if (parse(argc, argv, o) != 0) return 1;

  std::string prc =
      "window-type offscreen\n";
  if (!o.display.empty()) {
    prc += "load-display " + o.display + "\n";
  }
  prc +=
      "aux-display p3tinydisplay\n"
      "win-size 16 16\n"
      // This prc is inline and reads no etc/*.prc, so the working directory
      // and the egg loader both have to be named explicitly.
      "model-path .\n"
      "load-file-type egg pandaegg\n"
      "notify-level-pgraph fatal\n"
      "notify-level-display fatal\n"
      "notify-level-loader fatal\n"
      "notify-level-egg2pg fatal\n";
  if (!o.threading_model.empty()) {
    prc += "threading-model " + o.threading_model + "\n";
  }
  if (o.always_cow) {
    prc += "pipeline-always-cow true\n";
  }
  load_prc_file_data("scenegraph_bench", prc);

  PandaFramework framework;
  framework.open_framework(argc, argv);
  WindowFramework *window = framework.open_window();
  if (window == nullptr) {
    fprintf(stderr, "skip: no graphics pipe / no window backend available\n");
    return 77;
  }
  NodePath render = window->get_render();

  build_shared_geom();

  std::mt19937 rng(12345);

  // Build the initial scene: `groups` subtrees of instanced objects.  For the
  // generate workload this is just a small static backdrop; for animate it is
  // the pool the workers move.
  std::vector<std::vector<NodePath>> group_objs(std::max(1, o.groups));
  std::vector<NodePath> all_objs;
  std::vector<std::unique_ptr<AnimControlCollection>> crowd_controls;
  std::vector<NodePath> crowd_chars;
  std::vector<PT(Character)> crowd_characters;
  int init_objects;
  if (o.crowd > 0) {
    // A crowd stands alone: the point is that the per-frame cost is dirty
    // cyclers, so mixing in the instanced-triangle pool would only add clean
    // ones and dilute what is being measured.
    NodePath croot = render.attach_new_node("crowd-root");
    if (!build_crowd(croot, o, rng, crowd_controls, crowd_chars)) {
      return 77;
    }
    all_objs = crowd_chars;
    init_objects = (int)crowd_chars.size();
    // For --anim-threads.  Character::update() is what cull_callback calls and
    // no-ops once it has run for this frame's time, so updating here moves the
    // work off cull rather than duplicating it.
    for (NodePath &ch : crowd_chars) {
      NodePath cnp = ch.find("**/+Character");
      if (!cnp.is_empty()) {
        crowd_characters.push_back(DCAST(Character, cnp.node()));
      }
    }
    if (o.anim_threads > 0 && crowd_characters.empty()) {
      fprintf(stderr, "skip: --anim-threads but no Character nodes found\n");
      return 77;
    }
  } else if (o.complex) {
    // One deep, varied static tree (transforms + states at every node).  Leaves
    // populate all_objs.  Intended for steady-state render measurement with
    // --workers 0 --anim-per-frame 0 (no per-frame scene mutation).  Anchor it
    // in front of the camera so most of it stays in the frustum and the cull
    // actually traverses (and reads) the nodes, rather than skipping subtrees.
    NodePath croot = render.attach_new_node("complex-root");
    croot.set_pos(0.0f, 120.0f, 0.0f);
    build_complex(croot, o.depth, o.branch, rng, all_objs);
    init_objects = (int)all_objs.size();
  } else {
    init_objects = (o.workload == "generate") ? std::min(o.objects, 2000) : o.objects;
    for (int g = 0; g < (int)group_objs.size(); ++g) {
      NodePath grp = render.attach_new_node("group");
      int count = init_objects / (int)group_objs.size();
      for (int i = 0; i < count; ++i) {
        NodePath obj = make_object(grp, rng);
        group_objs[g].push_back(obj);
        all_objs.push_back(obj);
      }
    }
  }

  // Roots for procedural generation (one per generator worker, or one for main).
  std::vector<NodePath> gen_roots;
  int n_gen = (o.workload == "generate") ? std::max(1, o.workers) : 0;
  for (int i = 0; i < n_gen; ++i) {
    gen_roots.push_back(render.attach_new_node("gen-root"));
  }

  if (o.crowd > 0) {
    fprintf(stderr,
      "[bench] workload=crowd characters=%d clips=%d frame-blend=%s "
      "threading-model=\"%s\" readers=%d duration=%.1fs\n",
      init_objects, crowd_controls.empty() ? 0 : crowd_controls[0]->get_num_anims(),
      o.crowd_frame_blend ? "on" : "off",
      o.threading_model.c_str(), o.readers, o.duration);
  } else {
    fprintf(stderr,
      "[bench] workload=%s workers=%d threading-model=\"%s\" objects=%d groups=%d "
      "readers=%d duration=%.1fs\n",
      o.workload.c_str(), o.workers, o.threading_model.c_str(),
      init_objects, o.groups, o.readers, o.duration);
  }

  // Spawn workload + reader threads.
  std::vector<std::thread> threads;
  if (o.workload == "animate" && o.workers > 0) {
    for (int w = 0; w < o.workers; ++w) {
      std::vector<NodePath> mine = o.shared ? all_objs : group_objs[w % group_objs.size()];
      threads.emplace_back(animate_worker, std::move(mine), w + 1);
    }
  } else if (o.workload == "generate") {
    for (int w = 0; w < o.workers; ++w) {
      threads.emplace_back(generate_worker, gen_roots[w], w + 1, o.gen_batch, o.gen_window);
    }
  }
  for (int r = 0; r < o.readers; ++r) {
    threads.emplace_back(reader_worker, all_objs, 1000 + r);
  }

  // Main loop: render frames for the duration, doing the workload inline when
  // there are no worker threads.  Record per-frame times for percentiles.
  Thread *current_thread = Thread::get_current_thread();
  std::vector<double> frame_ms;
  frame_ms.reserve(100000);
  std::deque<NodePath> main_gen_live;
  std::uniform_int_distribution<size_t> pick(0, all_objs.empty() ? 0 : all_objs.size() - 1);
  std::uniform_real_distribution<float> pos(-50.0f, 50.0f);

  // Warm up: the first frames pay one-time setup (geom munge, state compose,
  // bounds init) that would otherwise swamp the steady-state measurement.
  for (int i = 0; i < 30; ++i) {
    framework.do_frame(current_thread);
  }

  // Character-update fan-out: per-character animation is independent, so it
  // should scale with cores.  A persistent pool with a barrier per frame, so
  // what is measured is the fan-out and not thread creation.
  std::vector<std::thread> anim_pool;
  std::mutex anim_mx;
  std::condition_variable anim_cv, anim_done_cv;
  unsigned anim_epoch = 0, anim_finished = 0;
  bool anim_quit = false;
  if (o.anim_threads > 0) {
    int nthreads = o.anim_threads;
    for (int t = 0; t < nthreads; ++t) {
      anim_pool.emplace_back([&, t, nthreads]() {
        unsigned seen = 0;
        for (;;) {
          std::unique_lock<std::mutex> lk(anim_mx);
          anim_cv.wait(lk, [&]{ return anim_quit || anim_epoch != seen; });
          if (anim_quit) return;
          seen = anim_epoch;
          lk.unlock();
          {
            // Each batch is its own epoch frame: a parked thread holding a
            // published slot would pin the reclaim floor process-wide.
            EpochHolder epoch;
            for (size_t i = t; i < crowd_characters.size(); i += nthreads) {
              crowd_characters[i]->update();
            }
          }
          lk.lock();
          if (++anim_finished == (unsigned)nthreads) {
            anim_done_cv.notify_one();
          }
        }
      });
    }
  }
  auto run_anim_pass = [&]() {
    if (o.anim_threads <= 0) return;
    {
      std::lock_guard<std::mutex> lk(anim_mx);
      anim_finished = 0;
      ++anim_epoch;
    }
    anim_cv.notify_all();
    std::unique_lock<std::mutex> lk(anim_mx);
    anim_done_cv.wait(lk, [&]{ return anim_finished == (unsigned)o.anim_threads; });
  };

  TrueClock *clock = TrueClock::get_global_ptr();
  double t_start = clock->get_long_time();
  while (clock->get_long_time() - t_start < o.duration) {
    // A crowd is its own workload; a set_pos batch on top would measure both.
    if (o.workers == 0 && o.crowd == 0) {
      if (o.workload == "animate") {
        for (int i = 0; i < o.anim_per_frame && !all_objs.empty(); ++i) {
          all_objs[pick(rng)].set_pos(pos(rng), pos(rng) + 200.0f, pos(rng));
        }
        g_anim_ops.fetch_add(o.anim_per_frame, std::memory_order_relaxed);
      } else {  // generate on the main thread
        for (int i = 0; i < o.gen_batch; ++i) {
          main_gen_live.push_back(make_object(gen_roots[0], rng));
          if ((int)main_gen_live.size() > o.gen_window) {
            main_gen_live.front().remove_node();
            main_gen_live.pop_front();
          }
        }
        g_gen_nodes.fetch_add(o.gen_batch, std::memory_order_relaxed);
      }
    }
    double f0 = clock->get_long_time();
    run_anim_pass();
    framework.do_frame(current_thread);
    frame_ms.push_back((clock->get_long_time() - f0) * 1000.0);
  }
  double elapsed = clock->get_long_time() - t_start;

  g_stop.store(true);
  for (auto &t : threads) t.join();
  if (!anim_pool.empty()) {
    { std::lock_guard<std::mutex> lk(anim_mx); anim_quit = true; }
    anim_cv.notify_all();
    for (auto &t : anim_pool) t.join();
  }

  // Report.
  std::sort(frame_ms.begin(), frame_ms.end());
  auto pct = [&](double p) {
    if (frame_ms.empty()) return 0.0;
    size_t idx = (size_t)(p * (frame_ms.size() - 1));
    return frame_ms[idx];
  };
  size_t frames = frame_ms.size();
  double fps = frames / elapsed;
  long long anim = g_anim_ops.load(), gen = g_gen_nodes.load(), reads = g_read_ops.load();

  printf("%-9s %5d %-9s %8zu %8.1f %8.3f %8.3f %12.0f %12.0f %12.0f\n",
         o.crowd > 0 ? "crowd" : o.workload.c_str(), o.workers,
         o.threading_model.empty() ? "single" : o.threading_model.c_str(),
         frames, fps, pct(0.50), pct(0.99),
         anim / elapsed, gen / elapsed, reads / elapsed);
  fflush(stdout);

  std::_Exit(0);  // skip framework teardown (can touch threading internals post-run)
}
