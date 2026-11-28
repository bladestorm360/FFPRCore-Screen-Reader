using System;
using System.Collections.Generic;
using System.IO;

namespace FFPR.Setup
{
    internal enum SetupAction { Check, Install, Rollback, Uninstall }

    internal static class SetupActions
    {
        internal static void Run(SetupAction action, IList<GameTarget> games, Action<string> report,
                                 Func<Release> check, Func<Release, Dictionary<string, byte[]>> fetch)
        {
            if (games.Count == 0) { report("Select at least one game."); return; }
            if (action == SetupAction.Rollback || action == SetupAction.Uninstall)
            {
                foreach (GameTarget game in games)
                    ForGame(game, report, () =>
                    {
                        var installer = new GameInstall(game);
                        return action == SetupAction.Rollback ? installer.Rollback() : installer.Uninstall();
                    });
                return;
            }

            report("Checking GitHub for the latest stable release.");
            Release release = check();
            report("Latest stable release: " + release.Tag + ".");
            var needed = new List<GameTarget>();
            foreach (GameTarget game in games)
                ForGame(game, report, () =>
                {
                    var installer = new GameInstall(game);
                    if (installer.RecoveryPending) return "An install was interrupted. Select Rollback to recover.";
                    Version installed = installer.InstalledVersion();
                    if (installed != null && installed >= release.Version)
                        return installed == release.Version ? "Already up to date." : "Installed version is newer. No changes.";
                    if (release.Url == null) return "The latest release has no compatible complete ZIP.";
                    if (action == SetupAction.Install)
                    {
                        game.RequireClosed();
                        needed.Add(game);
                    }
                    return installed == null ? "Ready for a fresh installation." : "An update is available from " + installed + ".";
                });

            if (needed.Count == 0) return;
            report("Downloading and verifying " + release.Tag + " from GitHub.");
            Dictionary<string, byte[]> payload = fetch(release);
            VerifyProxyVersion(payload["hid.dll"], release.Version);
            foreach (GameTarget game in needed)
                ForGame(game, report, () => new GameInstall(game).Install(release.Version, payload));
        }

        private static void ForGame(GameTarget game, Action<string> report, Func<string> action)
        {
            try { report(game + " — " + action()); }
            catch (InvalidDataException error) { report(game + " — " + error.Message); }
            catch (IOException error) { report(game + " — " + error.Message); }
            catch (UnauthorizedAccessException)
            {
                report(game + " — Windows denied access to the game folder. Check its write permissions.");
            }
            catch (ArgumentException error) { report(game + " — Invalid setup data. " + error.Message); }
            catch (InvalidOperationException error) { report(game + " — " + error.Message); }
        }

        private static void VerifyProxyVersion(byte[] bytes, Version expected)
        {
            string path = Path.Combine(Path.GetTempPath(), "ffpr-version-" + Guid.NewGuid().ToString("N") + ".dll");
            using (var file = new FileStream(path, FileMode.CreateNew, FileAccess.Write, FileShare.None))
            {
                try { file.Write(bytes, 0, bytes.Length); }
                catch { file.Dispose(); File.Delete(path); throw; }
            }
            try
            {
                if (SafeFiles.ModVersion(path) != expected)
                    throw new InvalidDataException("The DLL version does not match the release tag.");
            }
            finally { File.Delete(path); }
        }
    }
}
