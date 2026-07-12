using System;
using System.IO;
using System.Runtime.InteropServices;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace Panda3D.Tools.Tasks
{
    /// <summary>
    /// Base MSBuild task for a Panda3D command-line tool shipped in the
    /// <c>Panda3D.Tools</c> package. Resolves the host-platform executable from the
    /// package's <c>tools/&lt;rid&gt;/</c> bundle (which sits next to this task
    /// assembly) and runs it through <see cref="ToolTask"/> so output is surfaced in
    /// the MSBuild log and a non-zero exit code fails the build.
    /// <para>
    /// The bundled executables find their Panda shared libraries via an
    /// <c>$ORIGIN</c>/<c>@loader_path</c> rpath, so no environment plumbing is needed.
    /// <see cref="ToolsDir"/> may be set explicitly (e.g. to <c>$(Panda3DToolsDir)</c>);
    /// otherwise it is discovered from this assembly's location and the host RID.
    /// </para>
    /// </summary>
    public abstract class PandaToolTask : ToolTask
    {
        /// <summary>Bundle directory with the host-platform tool exes + libs. Optional; auto-resolved if unset.</summary>
        public string ToolsDir { get; set; }

        /// <summary>Extra options passed verbatim, before the output/input arguments (e.g. <c>-tbnall</c>).</summary>
        public string Options { get; set; }

        /// <summary>Input file(s).</summary>
        public ITaskItem[] Inputs { get; set; }

        /// <summary>
        /// Output file. Passed as <c>-o &lt;file&gt;</c> for tools that take it (egg/image
        /// converters), or as a trailing positional argument for the others.
        /// </summary>
        public string Output { get; set; }

        /// <summary>Working directory for the tool. Defaults to the project directory.</summary>
        public string WorkingDirectory { get; set; }

        /// <summary>The bare tool name, e.g. <c>egg2bam</c>. Set by each concrete task.</summary>
        protected abstract string ToolBaseName { get; }

        /// <summary>
        /// Whether the tool takes its output as <c>-o &lt;file&gt;</c> (most egg/image
        /// converters) rather than a trailing positional argument.
        /// </summary>
        protected virtual bool UsesOutputFlag => true;

        protected override string ToolName =>
            RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ? ToolBaseName + ".exe" : ToolBaseName;

        protected override string GenerateFullPathToTool() =>
            Path.Combine(ResolveToolsDir(), ToolName);

        protected override string GetWorkingDirectory() =>
            string.IsNullOrEmpty(WorkingDirectory) ? base.GetWorkingDirectory() : WorkingDirectory;

        protected override string GenerateCommandLineCommands()
        {
            var b = new CommandLineBuilder();
            if (!string.IsNullOrEmpty(Options))
                b.AppendTextUnquoted(" " + Options);
            if (UsesOutputFlag && !string.IsNullOrEmpty(Output))
            {
                b.AppendSwitch("-o");
                b.AppendFileNameIfNotNull(Output);
            }
            if (Inputs != null)
                foreach (var input in Inputs)
                    b.AppendFileNameIfNotNull(input.ItemSpec);
            if (!UsesOutputFlag && !string.IsNullOrEmpty(Output))
                b.AppendFileNameIfNotNull(Output);
            return b.ToString();
        }

        string ResolveToolsDir()
        {
            if (!string.IsNullOrEmpty(ToolsDir))
                return ToolsDir;
            // package layout: <pkg>/tasks/netstandard2.0/<this dll>  and  <pkg>/tools/<rid>/
            string dllDir = Path.GetDirectoryName(typeof(PandaToolTask).Assembly.Location);
            return Path.GetFullPath(Path.Combine(dllDir, "..", "..", "tools", HostRid()));
        }

        static string HostRid()
        {
            string arch = RuntimeInformation.ProcessArchitecture == Architecture.Arm64 ? "arm64" : "x64";
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) return "win-x64";
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX)) return "osx-" + arch;
            return "linux-x64";
        }
    }
}
