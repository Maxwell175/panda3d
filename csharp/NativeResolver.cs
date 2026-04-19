using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Panda3D.Core {
    internal static class NativeResolver {
#pragma warning disable CA2255
        [ModuleInitializer]
#pragma warning restore CA2255
        internal static void Initialize() {
            NativeLibrary.SetDllImportResolver(typeof(NativeResolver).Assembly, Resolve);
        }

        private static IntPtr Resolve(string libraryName, Assembly assembly, DllImportSearchPath? searchPath) {
            // Search the assembly directory so that native NuGet assets (copied from
            // runtimes/<rid>/native/) and local dev build outputs are found without
            // requiring LD_LIBRARY_PATH or system-wide installation.
            if (NativeLibrary.TryLoad(libraryName, assembly, DllImportSearchPath.AssemblyDirectory, out IntPtr handle))
                return handle;
            return IntPtr.Zero;
        }
    }
}
