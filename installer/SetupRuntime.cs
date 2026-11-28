using System;
using System.ComponentModel;
using System.Runtime.InteropServices;

namespace FFPR.Setup
{
    internal static class SetupRuntime
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        [DefaultDllImportSearchPaths(DllImportSearchPath.System32)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetDefaultDllDirectories(uint flags);

        internal static void Initialize()
        {
            const uint system32 = 0x00000800;
            if (!SetDefaultDllDirectories(system32))
                throw new Win32Exception(Marshal.GetLastWin32Error(), "Setup could not set its Windows DLL search directories.");
            AppContext.SetSwitch("Switch.System.IO.UseLegacyPathHandling", false);
            AppContext.SetSwitch("Switch.System.IO.BlockLongPaths", false);
            AppContext.SetSwitch("Switch.System.Net.DontEnableSystemDefaultTlsVersions", false);
            AppContext.SetSwitch("Switch.System.Net.DontEnableSchUseStrongCrypto", false);
        }
    }
}
