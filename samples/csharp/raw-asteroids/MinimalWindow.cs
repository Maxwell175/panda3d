using System;
using Interrogate;
using Panda3D.Core;

namespace Panda3D.RawAsteroids {
  class MinimalWindow {
    static void Main(string[] args) {
      try {
        Console.Error.WriteLine("[C#] Getting global engine...");
        GraphicsEngine engine = GraphicsEngine.GetGlobalPtr();
        Console.Error.WriteLine($"[C#] engine = {engine?.NativeHandle}");

        Console.Error.WriteLine("[C#] Getting default pipe...");
        GraphicsPipe pipe = GraphicsPipeSelection.GetGlobalPtr().MakeDefaultPipe();
        Console.Error.WriteLine($"[C#] pipe = {pipe?.NativeHandle}");
        if (pipe == null) throw new Exception("No graphics pipe");

        Console.Error.WriteLine("[C#] Creating FrameBufferProperties...");
        var fbProp = new FrameBufferProperties();
        fbProp.SetRgbColor(true);
        fbProp.SetColorBits(3 * 8);
        fbProp.SetDepthBits(24);
        fbProp.SetBackBuffers(1);
        Console.Error.WriteLine($"[C#] fbProp = {fbProp.NativeHandle}");

        Console.Error.WriteLine("[C#] Creating WindowProperties...");
        WindowProperties winProp = WindowProperties.GetDefault();
        winProp.SetSize(800, 600);
        Console.Error.WriteLine($"[C#] winProp = {winProp.NativeHandle}");

        Console.Error.WriteLine("[C#] Calling make_output...");
        IGraphicsOutput win = engine.MakeOutput(
          pipe, "window", 0, fbProp, winProp,
          (int)GraphicsPipeBufferCreationFlags.BfRequireWindow);
        Console.Error.WriteLine($"[C#] win = {win?.NativeHandle}");
        if (win == null) throw new Exception("make_output returned null");

        Console.Error.WriteLine("[C#] Setting clear color...");
        win.SetClearColorActive(true);
        var grey = new LVecBase4f(0.5f, 0.5f, 0.5f, 1.0f);
        win.SetClearColor(grey);

        Console.Error.WriteLine("[C#] Creating scene...");
        var render = new NodePath("render");
        var camera = new Camera("camera");
        camera.SetLens(new PerspectiveLens());
        var cam = render.AttachNewNode(camera);

        Console.Error.WriteLine("[C#] Loading model...");
        ILoader loader = Loader.GetGlobalPtr();
        Filename modelFile = "panda.egg";
        IPandaNode modelNode = loader.LoadSync(modelFile);
        if (modelNode == null) {
          Console.Error.WriteLine("[C#] Could not load panda.egg, continuing without model");
        } else {
          var model = render.AttachNewNode(modelNode);
          var pos = new LVecBase3f(0, 100, 0);
          model.SetPos(pos);
        }

        Console.Error.WriteLine("[C#] Creating display region...");
        IDisplayRegion dr = win.MakeDisplayRegion();
        dr.SetCamera(cam);

        Console.Error.WriteLine("[C#] Entering render loop...");
        GraphicsWindow gw = win.CastTo<GraphicsWindow>()
          ?? throw new Exception("Output is not a GraphicsWindow");
        while (gw != null && !gw.IsClosed()) {
          engine.RenderFrame();
        }
        Console.Error.WriteLine("[C#] Window closed, cleaning up...");
        engine.RemoveAllWindows();
      } catch (Exception ex) {
        Console.Error.WriteLine($"[C#] Exception: {ex}");
      }

    }
  }
}
