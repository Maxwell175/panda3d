#nullable enable

using System;
using System.Runtime.InteropServices;

namespace Panda3D.Core;

/// <summary>
/// Blittable value-type view of one PNMImage pixel: three 16-bit channels
/// (matching the C++ <c>xel</c>). Used for zero-copy span access over a
/// PNMImage's pixel array.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Xel {
  public ushort R;
  public ushort G;
  public ushort B;

  public Xel(ushort r, ushort g, ushort b) { R = r; G = g; B = b; }
}

public partial class PNMImage {
  /// <summary>
  /// A zero-copy, mutable span over the entire pixel array (row-major,
  /// <c>GetXSize() * GetYSize()</c> pixels). Reads and writes go straight to the
  /// native buffer, avoiding per-pixel P/Invoke. The span is only valid until
  /// the image is resized/reallocated — do not hold it across such calls.
  /// </summary>
  public unsafe Span<Xel> GetPixelSpan() {
    void* p = (void*)GetArrayPointer();
    return p == null ? default : new Span<Xel>(p, GetXSize() * GetYSize());
  }

  /// <summary>Raw base address of the pixel array as an <see cref="IntPtr"/>.</summary>
  public IntPtr GetArrayIntPtr() => (IntPtr)(long)GetArrayPointer();

  /// <summary>
  /// A zero-copy, mutable span over the alpha array (<c>GetXSize() * GetYSize()</c>
  /// 16-bit values) when <see cref="HasAlpha"/>; an empty span otherwise. Only
  /// valid until the image is resized/reallocated.
  /// </summary>
  public unsafe Span<ushort> GetAlphaSpan() {
    if (!HasAlpha()) {
      return default;
    }
    void* p = (void*)GetAlphaArrayPointer();
    return p == null ? default : new Span<ushort>(p, GetXSize() * GetYSize());
  }
}
