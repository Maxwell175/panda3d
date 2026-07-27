#nullable enable

using System;
using System.Runtime.CompilerServices;
using Interrogate;

namespace Panda3D.Core {
    /// <summary>
    /// Holds the current thread inside a Panda EBR epoch for the scope's
    /// lifetime (via <see cref="Thread.BeginEpoch"/> / <see cref="Thread.EndEpoch"/>).
    /// Native releases are already framed automatically; use this to batch a
    /// section of off-main-thread scene work under one epoch. Stack-scoped only;
    /// do not copy. Nests cheaply.
    /// </summary>
    public struct EpochScope : IDisposable {
        // False for default(EpochScope) and cleared on Dispose, so a stray or
        // repeated Dispose never unbalances end_epoch().
        private bool _active;

        public EpochScope() {
            Thread.BeginEpoch();
            _active = true;
        }

        public void Dispose() {
            if (_active) {
                _active = false;
                Thread.EndEpoch();
            }
        }
    }

    // Wires Interrogate.Core's neutral release hooks to Panda's epoch framing so
    // every native release, including finalizer-thread ones, runs inside an epoch.
    internal static class EpochIntegration {
#pragma warning disable CA2255
        [ModuleInitializer]
#pragma warning restore CA2255
        internal static void Install() {
            NativeObject.BeforeRelease = Thread.BeginEpoch;
            NativeObject.AfterRelease = Thread.EndEpoch;
        }
    }
}
