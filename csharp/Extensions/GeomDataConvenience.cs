#nullable enable

using System;

namespace Panda3D.Core;

// byte[] / ReadOnlySpan<byte> intake for raw vertex-buffer writes, mirroring the
// Python buffer-protocol copy_data_from. The native method takes a raw pointer;
// these pin the managed bytes and forward. The existing IntPtr and
// handle-to-handle overloads are left as-is.

public partial class GeomVertexArrayDataHandle {
  /// <summary>
  /// Copies raw bytes into this array's data buffer. The length must match the
  /// array's current data size (see the native copy_data_from contract).
  /// </summary>
  public unsafe void CopyDataFrom(ReadOnlySpan<byte> source) {
    fixed (byte* p = source) {
      copy_data_from((IntPtr)p, (uint)source.Length);
    }
  }

  /// <summary>Copies raw bytes into this array's data buffer.</summary>
  public void CopyDataFrom(byte[] source) {
    ArgumentNullException.ThrowIfNull(source);
    CopyDataFrom(new ReadOnlySpan<byte>(source));
  }
}
