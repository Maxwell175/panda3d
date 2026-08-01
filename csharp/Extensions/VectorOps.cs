#nullable enable

using System;

namespace Panda3D.Core;

// Component-wise Floor / Ceil / Round, mirroring the Python __floor__/__ceil__/
// __round__ vector extensions. Pure component math over X/Y/Z(/W), so these are
// C# partial-class methods rather than a native round-trip. Round uses
// round-half-to-even (banker's rounding), matching Python's round(). LPoint*,
// LVector*, LColor* derive from these bases and inherit the methods (returning
// the base vector type). Only float/double vectors have them (no-op for ints).

public partial class LVecBase2f {
  public LVecBase2f Floor() => new(MathF.Floor(X), MathF.Floor(Y));
  public LVecBase2f Ceil() => new(MathF.Ceiling(X), MathF.Ceiling(Y));
  public LVecBase2f Round() => new(MathF.Round(X), MathF.Round(Y));
}

public partial class LVecBase2d {
  public LVecBase2d Floor() => new(Math.Floor(X), Math.Floor(Y));
  public LVecBase2d Ceil() => new(Math.Ceiling(X), Math.Ceiling(Y));
  public LVecBase2d Round() => new(Math.Round(X), Math.Round(Y));
}

public partial class LVecBase3f {
  public LVecBase3f Floor() => new(MathF.Floor(X), MathF.Floor(Y), MathF.Floor(Z));
  public LVecBase3f Ceil() => new(MathF.Ceiling(X), MathF.Ceiling(Y), MathF.Ceiling(Z));
  public LVecBase3f Round() => new(MathF.Round(X), MathF.Round(Y), MathF.Round(Z));
}

public partial class LVecBase3d {
  public LVecBase3d Floor() => new(Math.Floor(X), Math.Floor(Y), Math.Floor(Z));
  public LVecBase3d Ceil() => new(Math.Ceiling(X), Math.Ceiling(Y), Math.Ceiling(Z));
  public LVecBase3d Round() => new(Math.Round(X), Math.Round(Y), Math.Round(Z));
}

public partial class LVecBase4f {
  public LVecBase4f Floor() => new(MathF.Floor(X), MathF.Floor(Y), MathF.Floor(Z), MathF.Floor(W));
  public LVecBase4f Ceil() => new(MathF.Ceiling(X), MathF.Ceiling(Y), MathF.Ceiling(Z), MathF.Ceiling(W));
  public LVecBase4f Round() => new(MathF.Round(X), MathF.Round(Y), MathF.Round(Z), MathF.Round(W));
}

public partial class LVecBase4d {
  public LVecBase4d Floor() => new(Math.Floor(X), Math.Floor(Y), Math.Floor(Z), Math.Floor(W));
  public LVecBase4d Ceil() => new(Math.Ceiling(X), Math.Ceiling(Y), Math.Ceiling(Z), Math.Ceiling(W));
  public LVecBase4d Round() => new(Math.Round(X), Math.Round(Y), Math.Round(Z), Math.Round(W));
}
