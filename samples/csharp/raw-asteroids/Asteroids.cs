using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using Interrogate;
using Panda3D.Core;


namespace Panda3D.RawAsteroids {
  internal static class AsteroidsGame {
    private const float SpritePos = 55.0f;
    private const float ScreenX = 20.0f;
    private const float ScreenY = 15.0f;
    private const float TurnRate = 360.0f;
    private const float Acceleration = 10.0f;
    private const float MaxVel = 6.0f;
    private const float MaxVelSq = MaxVel * MaxVel;
    private const float BulletLife = 2.0f;
    private const float BulletRepeat = 0.2f;
    private const float BulletSpeed = 10.0f;
    private const float AstInitVel = 1.0f;
    private const float AstInitScale = 3.0f;
    private const float AstVelScale = 2.2f;
    private const float AstSizeScale = 0.6f;
    private const float AstMinScale = 1.1f;
    private const float RespawnDelay = 2.0f;
    private const float DegToRad = MathF.PI / 180.0f;

    private static readonly string AssetsRoot =
        Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "asteroids"))
        .Replace('\\', '/');
    private static readonly string ModelsRoot = AssetsRoot + "/models";
    private static readonly string TexturesRoot = AssetsRoot + "/textures";

    private sealed class VelocityState {
      public float X;
      public float Z;
    }

    private sealed class BulletState {
      public NodePath Node = null!;
      public float VX;
      public float VZ;
      public double ExpiresAt;
    }

    private sealed class AsteroidState {
      public NodePath Node = null!;
      public ITexture Texture = null!;
      public float VX;
      public float VZ;
    }

    private static readonly VelocityState ShipVelocity = new();
    private static readonly List<BulletState> Bullets = new();
    private static readonly List<AsteroidState> Asteroids = new();

    private static GraphicsEngine engine = null!;
    private static GraphicsWindow window = null!;
    private static ILoader loader = null!;
    private static ClockObject clock = null!;
    private static Randomizer random = null!;

    private static NodePath render = null!;
    private static NodePath cameraRoot = null!;
    private static NodePath aspect2d = null!;
    private static NodePath planeModel = null!;
    private static NodePath background = null!;
    private static NodePath ship = null!;
    private static NodePath statusLabel = null!;
    private static TextNode statusText = null!;

    private static ITexture backgroundTexture = null!;
    private static ITexture shipTexture = null!;
    private static ITexture bulletTexture = null!;
    private static readonly List<ITexture> asteroidTextures = new();

    private static NodePath dataRoot = null!;
    private static IMouseWatcher mouseWatcher = null!;
    private static int leftButton;
    private static int rightButton;
    private static int thrustButton;
    private static int fireButton;
    private static int escapeButton;

    private static bool alive;
    private static double gameTime;
    private static double nextBulletTime;
    private static float respawnTimer;

    public static void Main(string[] args) {
      try {
        Initialize();
        Run();
      } catch (Exception ex) {
        Console.Error.WriteLine(ex);
      } finally {
        if (engine != null) {
          engine.RemoveAllWindows();
        }
      }
    }

    private static void Initialize() {
      engine = GraphicsEngine.GetGlobalPtr();
      GraphicsPipe pipe = GraphicsPipeSelection.GetGlobalPtr().MakeDefaultPipe();
      if (pipe == null) {
        throw new InvalidOperationException("No graphics pipe available.");
      }

      var fbProp = new FrameBufferProperties();
      fbProp.SetRgbColor(true);
      fbProp.SetColorBits(24);
      fbProp.SetDepthBits(24);
      fbProp.SetBackBuffers(1);

      WindowProperties winProp = WindowProperties.GetDefault();
      winProp.SetSize(800, 600);
      winProp.SetTitle("Panda3D C# Asteroids");

      IGraphicsOutput output = engine.MakeOutput(
        pipe, "window", 0, fbProp, winProp,
        (int)GraphicsPipeBufferCreationFlags.BfRequireWindow);
      if (output == null) {
        throw new InvalidOperationException("MakeOutput returned null.");
      }

      window = output.CastTo<GraphicsWindow>()
        ?? throw new InvalidOperationException("Output is not a GraphicsWindow.");

      output.SetClearColorActive(true);

      var clear = new LVecBase4f(0.0f, 0.0f, 0.0f, 1.0f);
      output.SetClearColor(clear);

      render = new NodePath("render");
      var cameraNode = new Camera("camera");
      var lens = new PerspectiveLens();
      lens.SetAspectRatio(800.0f / 600.0f);
      cameraNode.SetLens(lens);
      cameraNode.SetScene(render);
      cameraRoot = render.AttachNewNode(cameraNode);

      IDisplayRegion displayRegion = output.MakeDisplayRegion();
      displayRegion.SetCamera(cameraRoot);
      new FrameRateMeter("a").SetupWindow(window);

      float aspectRatio = 800.0f / 600.0f;
      aspect2d = new NodePath("aspect2d");
      aspect2d.SetDepthTest(false);
      aspect2d.SetDepthWrite(false);
      aspect2d.SetTransparency(TransparencyAttribMode.MAlpha);
      aspect2d.SetBin("unsorted", 0);
      var cam2d = new Camera("camera2d");
      var lens2d = new OrthographicLens();
      lens2d.SetFilmSize(2.0f * aspectRatio, 2.0f);
      lens2d.SetNearFar(-1000.0f, 1000.0f);
      cam2d.SetLens(lens2d);
      cam2d.SetScene(aspect2d);
      NodePath cam2dNp = aspect2d.AttachNewNode(cam2d);
      IDisplayRegion dr2d = output.MakeDisplayRegion();
      dr2d.SetSort(10);
      dr2d.SetCamera(cam2dNp);
      dr2d.SetClearColorActive(false);
      dr2d.SetClearDepthActive(true);

      loader = Loader.GetGlobalPtr();
      clock = ClockObject.GetGlobalClock();
      random = new Randomizer();

      LoadAssets();
      CreateScene();
      InitializeInput();
      SpawnAsteroids();
      UpdateStatus("Ready");

      alive = true;
      gameTime = 0.0;
      nextBulletTime = 0.0;
      respawnTimer = 0.0f;
      clock.Reset();
    }

    private static void LoadAssets() {
      planeModel = LoadModel(ModelsRoot + "/plane.egg");
      backgroundTexture = LoadTexture(TexturesRoot + "/stars.jpg");
      shipTexture = LoadTexture(TexturesRoot + "/ship.png");
      bulletTexture = LoadTexture(TexturesRoot + "/bullet.png");
      asteroidTextures.Add(LoadTexture(TexturesRoot + "/asteroid1.png"));
      asteroidTextures.Add(LoadTexture(TexturesRoot + "/asteroid2.png"));
      asteroidTextures.Add(LoadTexture(TexturesRoot + "/asteroid3.png"));
    }

    private static void CreateScene() {
      background = LoadObject(backgroundTexture, 146.0f, 0.0f, 0.0f, 200.0f, false);
      ship = LoadObject(shipTexture, 1.0f, 0.0f, 0.0f, SpritePos, true);
      SetVelocity(ship, 0.0f, 0.0f);
      ship.SetR(0.0f);

      float ar = 800.0f / 600.0f;
      Create2dLabel("title", "Panda3D: Tutorial - Tasks", ar - 0.1f, -1.0f + 0.1f, 0.07f, TextPropertiesAlignment.ARight);
      Create2dLabel("esc", "ESC: Quit", -ar + 0.07f, 1.0f - 0.1f, 0.05f, TextPropertiesAlignment.ALeft);
      Create2dLabel("left", "[Left Arrow]: Turn Left (CCW)", -ar + 0.07f, 1.0f - 0.16f, 0.05f, TextPropertiesAlignment.ALeft);
      Create2dLabel("right", "[Right Arrow]: Turn Right (CW)", -ar + 0.07f, 1.0f - 0.22f, 0.05f, TextPropertiesAlignment.ALeft);
      Create2dLabel("up", "[Up Arrow]: Accelerate", -ar + 0.07f, 1.0f - 0.28f, 0.05f, TextPropertiesAlignment.ALeft);
      Create2dLabel("space", "[Space Bar]: Fire", -ar + 0.07f, 1.0f - 0.34f, 0.05f, TextPropertiesAlignment.ALeft);
      string mode = RuntimeFeature.IsDynamicCodeCompiled ? "Shared" : "NativeAOT Static";
      Create2dLabel("engine", $"Panda3D C# \u2022 {mode}", -ar + 0.07f, 1.0f - 0.42f, 0.04f, TextPropertiesAlignment.ALeft);

      statusText = new TextNode("status");
      FrameworkBindings.SetText(statusText, string.Empty);
      statusText.SetAlign(TextPropertiesAlignment.ACenter);
      var color = new LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f);
      statusText.SetTextColor(color);
      var shadow = new LVecBase4f(0.0f, 0.0f, 0.0f, 0.5f);
      statusText.SetShadowColor(shadow);
      var offset = new LVecBase2f(0.04f, 0.04f);
      statusText.SetShadow(offset);
      statusText.SetBin("fixed");
      statusText.SetDrawOrder(0);
      statusLabel = aspect2d.AttachNewNode(statusText);
      SetPos(statusLabel, 0.0f, 0.0f, -0.9f);
      SetScale(statusLabel, 0.07f);
    }

    private static void InitializeInput() {
      dataRoot = new NodePath("data");

      var mouseNode = new MouseAndKeyboard(window, 0, "mouse");
      NodePath mouse = dataRoot.AttachNewNode(mouseNode);

      mouseWatcher = new MouseWatcher("watcher");
      mouse.AttachNewNode(mouseWatcher);

      var reg = ButtonRegistry.Ptr();
      leftButton = reg.FindButton("arrow_left");
      rightButton = reg.FindButton("arrow_right");
      thrustButton = reg.FindButton("arrow_up");
      fireButton = reg.FindButton("space");
      escapeButton = reg.FindButton("escape");
    }

    private static void Run() {
      int frame = 0;
      using var dgTrav = new DataGraphTraverser();
      while (!window.IsClosed()) {
        dgTrav.Traverse(dataRoot.Node());
        engine.RenderFrame();

        float dt = Math.Min((float)clock.GetDt(), 0.05f);
        gameTime += dt;

        if (IsDown(escapeButton)) {
          break;
        }

        Update(dt);
        frame++;
      }
    }

    private static void Update(float dt) {
      if (!alive) {
        respawnTimer -= dt;
        if (respawnTimer <= 0.0f) {
          RespawnShip();
        } else {
          UpdateStatus($"Respawning in {respawnTimer:0.0}s");
        }
        return;
      }

      UpdateShip(dt);

      for (int i = 0; i < Asteroids.Count; ++i) {
        UpdatePos(Asteroids[i].Node, Asteroids[i].VX, Asteroids[i].VZ, dt);
      }

      for (int i = Bullets.Count - 1; i >= 0; --i) {
        BulletState bullet = Bullets[i];
        UpdatePos(bullet.Node, bullet.VX, bullet.VZ, dt);
        if (bullet.ExpiresAt <= gameTime) {
          bullet.Node.RemoveNode();
          Bullets.RemoveAt(i);
        }
      }

      CheckBulletCollisions();
      CheckShipCollision();

      if (alive && Asteroids.Count == 0) {
        SpawnAsteroids();
      }

      if (alive) {
        UpdateStatus($"Asteroids: {Asteroids.Count}   Bullets: {Bullets.Count}");
      }
    }

    private static void UpdateShip(float dt) {
      float heading = ship.GetR();
      if (IsDown(rightButton)) {
        heading += dt * TurnRate;
        ship.SetR(heading % 360.0f);
      } else if (IsDown(leftButton)) {
        heading -= dt * TurnRate;
        ship.SetR(heading % 360.0f);
      }

      heading = ship.GetR();
      if (IsDown(thrustButton)) {
        float headingRad = heading * DegToRad;
        float vx = ShipVelocity.X + MathF.Sin(headingRad) * Acceleration * dt;
        float vz = ShipVelocity.Z + MathF.Cos(headingRad) * Acceleration * dt;
        float speedSq = vx * vx + vz * vz;
        if (speedSq > MaxVelSq) {
          float scale = MaxVel / MathF.Sqrt(speedSq);
          vx *= scale;
          vz *= scale;
        }
        SetVelocity(ship, vx, vz);
      }

      UpdatePos(ship, ShipVelocity.X, ShipVelocity.Z, dt);

      if (IsDown(fireButton) && gameTime > nextBulletTime) {
        Fire(gameTime);
        nextBulletTime = gameTime + BulletRepeat;
      }
    }

    private static void Fire(double time) {
      float direction = ship.GetR() * DegToRad;
      NodePath bulletNode = LoadObject(bulletTexture, 0.2f, ship.GetX(), ship.GetZ(), SpritePos, true);
      Bullets.Add(new BulletState {
        Node = bulletNode,
        VX = ShipVelocity.X + MathF.Sin(direction) * BulletSpeed,
        VZ = ShipVelocity.Z + MathF.Cos(direction) * BulletSpeed,
        ExpiresAt = time + BulletLife
      });
    }

    private static void UpdatePos(NodePath obj, float vx, float vz, float dt) {
      float x = obj.GetX() + vx * dt;
      float z = obj.GetZ() + vz * dt;
      float radius = obj.GetSx() * 0.5f;

      if (x - radius > ScreenX)       x = -ScreenX;
      else if (x + radius < -ScreenX)  x = ScreenX;
      if (z - radius > ScreenY)        z = -ScreenY;
      else if (z + radius < -ScreenY)  z = ScreenY;

      SetPos(obj, x, obj.GetY(), z);
    }

    private static void CheckBulletCollisions() {
      for (int bi = Bullets.Count - 1; bi >= 0; --bi) {
        if (bi >= Bullets.Count) continue;

        BulletState bullet = Bullets[bi];
        float bx = bullet.Node.GetX();
        float bz = bullet.Node.GetZ();
        float bSize = bullet.Node.GetSx();

        for (int ai = Asteroids.Count - 1; ai >= 0; --ai) {
          AsteroidState asteroid = Asteroids[ai];
          float ax = asteroid.Node.GetX();
          float az = asteroid.Node.GetZ();
          float aSize = asteroid.Node.GetSx();

          float dx = bx - ax;
          float dz = bz - az;
          float r = (bSize + aSize) * 0.5f;
          if (dx * dx + dz * dz < r * r) {
            bullet.Node.RemoveNode();
            Bullets.RemoveAt(bi);
            AsteroidHit(ai);
            break;
          }
        }
      }
    }

    private static void CheckShipCollision() {
      float shipSize = ship.GetSx();
      float shipX = ship.GetX();
      float shipZ = ship.GetZ();
      for (int i = 0; i < Asteroids.Count; ++i) {
        AsteroidState asteroid = Asteroids[i];
        float ax = asteroid.Node.GetX();
        float az = asteroid.Node.GetZ();
        float aSize = asteroid.Node.GetSx();

        float dx = shipX - ax;
        float dz = shipZ - az;
        float r = (shipSize + aSize) * 0.5f;
        if (dx * dx + dz * dz < r * r) {
          ShipDestroyed();
          return;
        }
      }
    }

    private static void ShipDestroyed() {
      alive = false;
      respawnTimer = RespawnDelay;
      ship.Hide();
      SetVelocity(ship, 0.0f, 0.0f);
      ship.SetR(0.0f);

      for (int i = 0; i < Asteroids.Count; ++i)
        Asteroids[i].Node.RemoveNode();
      Asteroids.Clear();

      for (int i = 0; i < Bullets.Count; ++i)
        Bullets[i].Node.RemoveNode();
      Bullets.Clear();

      UpdateStatus("Ship destroyed");
    }

    private static void RespawnShip() {
      ship.SetR(0.0f);
      SetPos(ship, 0.0f, SpritePos, 0.0f);
      SetVelocity(ship, 0.0f, 0.0f);
      ship.Show();
      alive = true;
      respawnTimer = 0.0f;
      SpawnAsteroids();
      UpdateStatus("Respawned");
    }

    private static void SpawnAsteroids() {
      for (int i = 0; i < Asteroids.Count; ++i)
        Asteroids[i].Node.RemoveNode();
      Asteroids.Clear();

      for (int i = 0; i < 10; ++i) {
        ITexture texture = asteroidTextures[random.RandomInt(asteroidTextures.Count)];
        NodePath asteroidNode = LoadObject(texture, AstInitScale, 0.0f, 0.0f, SpritePos, true);

        int xChoice = random.RandomInt(((int)ScreenX + 1) * 2 - 9) - (int)ScreenX;
        if (xChoice >= -4) xChoice += 9;
        asteroidNode.SetX(xChoice);

        int yChoice = random.RandomInt(((int)ScreenY + 1) * 2 - 9) - (int)ScreenY;
        if (yChoice >= -4) yChoice += 9;
        asteroidNode.SetZ(yChoice);

        float heading = (float)random.RandomReal(MathF.PI * 2.0f);
        Asteroids.Add(new AsteroidState {
          Node = asteroidNode,
          Texture = texture,
          VX = MathF.Sin(heading) * AstInitVel,
          VZ = MathF.Cos(heading) * AstInitVel
        });
      }
    }

    private static void AsteroidHit(int index) {
      AsteroidState asteroid = Asteroids[index];
      float oldScale;
      var scale = asteroid.Node.GetScale();
        oldScale = scale.X;

      if (oldScale <= AstMinScale) {
        asteroid.Node.RemoveNode();
        Asteroids.RemoveAt(index);
        return;
      }

      float newScale = oldScale * AstSizeScale;
      SetScale(asteroid.Node, newScale);

      float speed = MathF.Sqrt(asteroid.VX * asteroid.VX + asteroid.VZ * asteroid.VZ) * AstVelScale;
      float dirX = -asteroid.VZ;
      float dirZ = asteroid.VX;
      float dirLen = MathF.Sqrt(dirX * dirX + dirZ * dirZ);
      if (dirLen <= 0.0001f) { dirX = 1.0f; dirZ = 0.0f; dirLen = 1.0f; }

      asteroid.VX = dirX / dirLen * speed;
      asteroid.VZ = dirZ / dirLen * speed;

      NodePath newAst = LoadObject(asteroid.Texture, newScale, 0.0f, 0.0f, SpritePos, true);
      SetPos(newAst, asteroid.Node.GetX(), asteroid.Node.GetY(), asteroid.Node.GetZ());

      Asteroids.Add(new AsteroidState {
        Node = newAst,
        Texture = asteroid.Texture,
        VX = -asteroid.VX,
        VZ = -asteroid.VZ
      });
    }

    private static NodePath LoadObject(ITexture texture, float scale, float x, float z, float depth, bool transparency) {
      NodePath obj = planeModel.CopyTo(cameraRoot);
      SetPos(obj, x, depth, z);
      SetScale(obj, scale);
      obj.SetBin("unsorted", 0);
      obj.SetDepthTest(false);
      if (transparency) {
        obj.SetTransparency(TransparencyAttribMode.MAlpha);
      }
      obj.SetTexture(texture, 1);
      return obj;
    }

    private static NodePath LoadModel(string path) {
      Filename file = path;
      IPandaNode node = loader.LoadSync(file);
      if (node == null) {
        throw new InvalidOperationException("Could not load model: " + path);
      }
      return new NodePath(node);
    }

    private static ITexture LoadTexture(string path) {
      Filename file = path;
      ITexture texture = TexturePool.LoadTexture(file);
      if (texture == null) {
        throw new InvalidOperationException("Could not load texture: " + path);
      }
      texture.SetWrapU(SamplerStateWrapMode.WmClamp);
      texture.SetWrapV(SamplerStateWrapMode.WmClamp);
      return texture;
    }

    private static NodePath Create2dLabel(string name, string text, float x, float z, float scale, TextPropertiesAlignment align) {
      var textNode = new TextNode(name);
      FrameworkBindings.SetText(textNode, text);
      textNode.SetAlign(align);
      var color = new LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f);
      textNode.SetTextColor(color);
      var shadow = new LVecBase4f(0.0f, 0.0f, 0.0f, 0.5f);
      textNode.SetShadowColor(shadow);
      var offset = new LVecBase2f(0.04f, 0.04f);
      textNode.SetShadow(offset);

      textNode.SetBin("fixed");
      textNode.SetDrawOrder(0);
      IPandaNode baked = textNode.Generate();
      NodePath path = aspect2d.AttachNewNode(baked);
      var pos = new LPoint3f(x, 0.0f, z);
      path.SetPos(pos);
      var s = new LVecBase3f(scale, scale, scale);
      path.SetScale(s);
      return path;
    }

    private static void UpdateStatus(string text) {
      FrameworkBindings.SetText(statusText, text);
    }

    private static void SetVelocity(NodePath obj, float vx, float vz) {
      if (ReferenceEquals(obj, ship)) {
        ShipVelocity.X = vx;
        ShipVelocity.Z = vz;
      }
    }

    private static bool IsDown(int button) {
      if (mouseWatcher == null || button == 0) return false;
      return mouseWatcher.IsButtonDown(button);
    }

    private static void SetPos(NodePath node, float x, float y, float z) {
      using var pos = new LPoint3f(x, y, z);
      node.SetPos(pos);
    }

    private static void SetScale(NodePath node, float scale) {
      using var value = new LVecBase3f(scale, scale, scale);
      node.SetScale(value);
    }
  }
}
