#nullable enable

using System;

namespace Panda3D.Core;

public partial class CollisionPolygon {
  /// <summary>
  /// Rebuilds the polygon from an arbitrary list of coplanar vertices (three or
  /// more). Convenience over the native <see cref="SetupPoints(pvector_LPoint3)"/>
  /// so points can be passed inline.
  /// </summary>
  public void SetupPoints(params LPoint3f[] points) {
    ArgumentNullException.ThrowIfNull(points);
    var vec = new pvector_LPoint3();
    foreach (LPoint3f p in points) {
      vec.Add(p);
    }
    SetupPoints(vec);
  }
}
