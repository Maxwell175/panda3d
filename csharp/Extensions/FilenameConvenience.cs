#nullable enable

namespace Panda3D.Core;

public partial class Filename {
  /// <summary>
  /// Implicitly converts a path string to a <see cref="Filename"/> via
  /// <see cref="FromOsSpecific(string)"/>. The string is treated as the native path format
  /// for the current OS; clean forward-slash resource paths (e.g. "models/ralph.egg") pass
  /// through unchanged, so this is safe for both. Lets you pass a string anywhere a Filename
  /// is expected instead of calling FromOsSpecific by hand.
  /// </summary>
  public static implicit operator Filename(string osSpecificPath) => FromOsSpecific(osSpecificPath);

  /// <summary>The full path in Panda's internal (forward-slash) form.</summary>
  public override string ToString() => GetFullpath();
}
