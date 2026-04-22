using System;
using Interrogate;
using Panda3D.Core;

namespace Panda3D.RawAsteroids {
  internal static class FrameworkBindings {
    internal static void SetText(TextNode node, string text) {
      node.SetText(text);
      node.ForceUpdate();
    }
  }
}
