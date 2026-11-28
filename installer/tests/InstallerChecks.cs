using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;

namespace FFPR.Setup
{
    internal static class InstallerChecks
    {
        private static int checks;

        private static int Main(string[] args)
        {
            try
            {
                SetupRuntime.Initialize();
                if (args.Length != 1 && args.Length != 4)
                    throw new ArgumentException("Supply a new fixture directory and optionally a ZIP, tag, and SHA-256.");
                string scratch = Path.GetFullPath(args[0]);
                if (Directory.Exists(scratch)) throw new IOException("Choose a new fixture directory.");
                Directory.CreateDirectory(scratch);
                Release release;
                Dictionary<string, byte[]> payload;
                if (args.Length == 4)
                {
                    byte[] zip = SafeFiles.Read(args[1], SafeFiles.MaxFile);
                    release = TestRelease(args[2], zip.Length, args[3]);
                    payload = ReleaseSource.Open(release, zip);
                }
                else
                {
                    release = ReleaseSource.Check();
                    payload = ReleaseSource.Fetch(release);
                }
                Console.WriteLine("Verified the release ZIP, manifest and runtime pins for " + release.Tag + ".");
                ReleaseChecks(release.Tag);
                InstallChecks(scratch, release, payload);
                Console.WriteLine("PASS: " + checks + " installer assertions. No games launched.");
                return 0;
            }
            catch (Exception error) { Console.Error.WriteLine(error); return 1; }
        }

        private static void Check(bool value, string label)
        {
            if (!value) throw new InvalidOperationException(label);
            ++checks;
        }

        private static void Reject(Action operation, string label)
        {
            bool rejected = false;
            try { operation(); }
            catch (InvalidDataException) { rejected = true; }
            catch (IOException) { rejected = true; }
            Check(rejected, label);
        }

        private static Release TestRelease(string tag, int size, string digest)
        {
            string name = "Final.Fantasy.Pixel.Master.Core.Screen.Reader.V" + tag.Substring(1) + ".zip";
            string url = "https://github.com/" + ReleaseSource.Repository + "/releases/download/" + tag + "/" + name;
            string data = "{\"draft\":false,\"prerelease\":false,\"tag_name\":\"" + tag + "\",\"assets\":[{\"name\":\"" +
                name + "\",\"size\":" + size + ",\"state\":\"uploaded\",\"digest\":\"sha256:" + digest +
                "\",\"browser_download_url\":\"" + url + "\"}]}";
            return ReleaseSource.Parse(data);
        }

        private static void ReleaseChecks(string tag)
        {
            Reject(() => ReleaseSource.Parse("{\"draft\":true,\"prerelease\":false}"), "draft rejected");
            Reject(() => ReleaseSource.Parse("{\"draft\":false,\"prerelease\":true}"), "prerelease rejected");
            Reject(() => TestRelease(tag, 12, "bad"), "invalid digest rejected");
            Check(ReleaseSource.Redirect("https://release-assets.githubusercontent.com/object").IsDefaultPort,
                  "GitHub asset redirect accepted");
            Reject(() => ReleaseSource.Redirect("https://release-assets.githubusercontent.com.evil.test/object"), "foreign redirect rejected");
            Reject(() => ReleaseSource.Redirect("http://release-assets.githubusercontent.com/object"), "HTTP redirect rejected");
            Reject(() => ReleaseSource.Redirect("https://release-assets.githubusercontent.com:8443/object"), "nonstandard redirect port rejected");
            using (var stream = new MemoryStream())
            {
                using (var zip = new ZipArchive(stream, ZipArchiveMode.Create, true))
                    zip.CreateEntry("../hid.dll");
                byte[] bytes = stream.ToArray();
                Reject(() => ReleaseSource.Open(TestRelease(tag, bytes.Length, new string('0', 64)), bytes), "ZIP digest mismatch rejected");
                Reject(() => ReleaseSource.Open(TestRelease(tag, bytes.Length, SafeFiles.Hash(bytes)), bytes), "unsafe ZIP path rejected");
            }
        }

        private static void InstallChecks(string root, Release release, Dictionary<string, byte[]> payload)
        {
            string steam = Path.Combine(root, "Steam");
            string extra = Path.Combine(root, "Other library");
            string apps = Path.Combine(steam, "steamapps");
            Directory.CreateDirectory(apps);
            File.WriteAllText(Path.Combine(apps, "libraryfolders.vdf"),
                              "\"libraryfolders\" { \"1\" { \"path\" \"" + extra.Replace("\\", "\\\\") + "\" } }");
            for (int index = 0; index < GameCatalog.Games.Length; ++index)
                MakeGame(index == 5 ? extra : steam, GameCatalog.Games[index]);
            var games = GameCatalog.Find(new[] { steam });
            Check(games.Count == 6 && games[5].Root.StartsWith(extra, StringComparison.Ordinal), "six AppIDs across two Steam libraries");
            string bad = Path.Combine(apps, "appmanifest_" + GameCatalog.Games[0].AppId + ".acf");
            string good = File.ReadAllText(bad);
            File.WriteAllText(bad, good.Replace(GameCatalog.Games[0].Title + " PR", ".."));
            Check(GameCatalog.Find(new[] { steam }).Count == 5, "escaping install directory rejected");
            File.WriteAllText(bad, good);
            // Keep the real game-process check in the transaction path with
            // fixture-only executable names, independent of Steam play.
            games = games.Select(target =>
            {
                var game = new Game(target.Game.AppId, "FFPR Setup Fixture " + target.Game.AppId);
                File.WriteAllText(Path.Combine(target.Root, game.Exe), "Offline fixture, never executed.");
                return new GameTarget(game, target.Root);
            }).ToList();

            var messages = new List<string>();
            int downloads = 0, requests = 0;
            Action<SetupAction, GameTarget[]> run = (action, list) => SetupActions.Run(action, list, messages.Add,
                () => { ++requests; return release; }, _ => { ++downloads; return payload; });
            GameTarget first = games[0], second = games[1], third = games[2], conflict = games[3], manual = games[4];
            string originalSettings = Path.Combine(first.Root, "ffpr_screen_reader.ini");
            string unrelated = Path.Combine(first.Root, "Unrelated.dll");
            File.WriteAllText(originalSettings, "Settings intact.");
            File.WriteAllText(unrelated, "Unrelated mod.");
            string borrowed = Path.Combine(second.Root, "SDL3.dll");
            File.WriteAllBytes(borrowed, payload["SDL3.dll"]);
            string foreign = Path.Combine(conflict.Root, "hid.dll");
            File.WriteAllText(foreign, "Foreign DLL.");
            run(SetupAction.Check, games.ToArray());
            Check(requests == 1 && downloads == 0 && games.All(game => !Directory.Exists(Path.Combine(game.Root, ".ffpr-setup"))),
                  "check is metadata-only and changes no game");
            run(SetupAction.Install, new[] { first, second, conflict });
            Check(downloads == 1 && requests == 2, "one download for multiple selected games");
            Check(new GameInstall(first).InstalledVersion() == release.Version &&
                  new GameInstall(second).InstalledVersion() == release.Version &&
                  new GameInstall(third).InstalledVersion() == null &&
                  File.ReadAllText(foreign) == "Foreign DLL.", "only selected compatible games installed");
            Check(Payload.Names.All(name => SafeFiles.HashFile(Path.Combine(first.Root, name)) == SafeFiles.Hash(payload[name])),
                  "all installed release files match");
            string state = Path.Combine(first.Root, ".ffpr-setup", "state.json");
            string installedState = File.ReadAllText(state);
            File.WriteAllBytes(Path.Combine(manual.Root, "hid.dll"), payload["hid.dll"]);
            run(SetupAction.Install, new[] { first, second, manual });
            Check(downloads == 1 && File.ReadAllText(state) == installedState &&
                  !Directory.Exists(Path.Combine(manual.Root, ".ffpr-setup")), "equal release is a true no-op for both setup and manual installs");
            run(SetupAction.Rollback, new[] { second });
            Check(downloads == 1 && requests == 3 && new GameInstall(second).InstalledVersion() == null &&
                  File.Exists(borrowed), "offline rollback of first install restores the borrowed dependency");
            run(SetupAction.Uninstall, new[] { first });
            Check(new GameInstall(first).InstalledVersion() == null && File.Exists(Path.Combine(first.Root, "SDL3.dll")) &&
                  File.ReadAllText(originalSettings) == "Settings intact." && File.ReadAllText(unrelated) == "Unrelated mod.",
                  "uninstall disables only the screen reader, preserving shared SDL, settings and other mods");
            run(SetupAction.Rollback, new[] { first });
            Check(new GameInstall(first).InstalledVersion() == release.Version, "rollback can undo uninstall");

            byte[] earlier = Stamp(payload["hid.dll"], 0, 9);
            File.WriteAllBytes(Path.Combine(third.Root, "hid.dll"), earlier);
            File.WriteAllText(Path.Combine(third.Root, "SDL3.dll"), "Unrelated SDL");
            Check(new GameInstall(third).InstalledVersion() == new Version(0, 9, 0, 0), "Windows metadata recognizes an older manual install");
            run(SetupAction.Install, new[] { third });
            Check(new GameInstall(third).InstalledVersion() == new Version(0, 9, 0, 0) &&
                  File.ReadAllText(Path.Combine(third.Root, "SDL3.dll")) == "Unrelated SDL",
                  "conflicting shared SDL is never replaced");
            File.Delete(Path.Combine(third.Root, "SDL3.dll"));
            run(SetupAction.Install, new[] { third });
            Check(new GameInstall(third).InstalledVersion() == release.Version, "manual install upgrades");
            run(SetupAction.Rollback, new[] { third });
            Check(SafeFiles.HashFile(Path.Combine(third.Root, "hid.dll")) == SafeFiles.Hash(earlier), "rollback restores exact older proxy");

            PendingRecovery(first, payload);
            run(SetupAction.Uninstall, new[] { manual });
            Check(new GameInstall(manual).InstalledVersion() == null, "manual uninstall: " + messages.Last());
            byte[] newer = Stamp(payload["hid.dll"], 99, 0);
            File.WriteAllBytes(Path.Combine(conflict.Root, "hid.dll"), newer);
            int priorDownloads = downloads;
            run(SetupAction.Install, new[] { conflict });
            Check(downloads == priorDownloads && SafeFiles.HashFile(foreign) == SafeFiles.Hash(newer), "newer release is never downgraded");
            Check(!Directory.Exists(Path.Combine(conflict.Root, ".ffpr-setup")), "read-only failures and skips create no install record");
        }

        private static void PendingRecovery(GameTarget game, Dictionary<string, byte[]> payload)
        {
            string root = Path.Combine(game.Root, ".ffpr-setup");
            string pendingFile = Path.Combine(root, "pending.json");
            string proxy = Path.Combine(game.Root, "hid.dll");
            byte[] changed = Stamp(payload["hid.dll"], 99, 0);
            string hash = SafeFiles.Hash(payload["hid.dll"]);
            var json = ReleaseSource.Serializer(128 * 1024);
            var oldState = json.Deserialize<InstallState>(File.ReadAllText(Path.Combine(root, "state.json")));
            var pending = new PendingChange {
                OldState = oldState, Files = new Snapshot {
                    Before = new Dictionary<string, string> { { "hid.dll", hash } },
                    After = new Dictionary<string, string> { { "hid.dll", SafeFiles.Hash(changed) } },
                    Previous = oldState.Active
                }
            };
            File.WriteAllText(pendingFile, json.Serialize(pending), ReleaseSource.Utf8);
            File.WriteAllText(proxy, "Unrelated mid-operation rewrite");
            Reject(() => new GameInstall(game).Rollback(), "recovery preserves an unrelated file change");
            Check(File.Exists(pendingFile) && File.ReadAllText(proxy) == "Unrelated mid-operation rewrite",
                  "failed recovery leaves the pending record and unrelated bytes");
            File.WriteAllBytes(proxy, changed);
            new GameInstall(game).Rollback();
            Check(!File.Exists(pendingFile) && SafeFiles.HashFile(proxy) == hash, "interrupted update restores its verified before-state");
        }

        private static void MakeGame(string library, Game game)
        {
            string apps = Path.Combine(library, "steamapps");
            string folder = game.Title + " PR";
            string root = Path.Combine(apps, "common", folder);
            Directory.CreateDirectory(root);
            File.WriteAllText(Path.Combine(root, game.Exe), "Offline fixture, never executed.");
            File.WriteAllText(Path.Combine(apps, "appmanifest_" + game.AppId + ".acf"),
                "\"AppState\" { \"appid\" \"" + game.AppId + "\" \"installdir\" \"" + folder + "\" }");
        }

        private static byte[] Stamp(byte[] bytes, int major, int minor)
        {
            var copy = (byte[])bytes.Clone();
            for (int i = 0; i <= copy.Length - 24; ++i)
            {
                if (BitConverter.ToUInt32(copy, i) != 0xfeef04bd || BitConverter.ToUInt32(copy, i + 4) != 0x00010000)
                    continue;
                Buffer.BlockCopy(BitConverter.GetBytes((major << 16) | minor), 0, copy, i + 8, 4);
                Buffer.BlockCopy(BitConverter.GetBytes((major << 16) | minor), 0, copy, i + 16, 4);
                return copy;
            }
            throw new InvalidDataException("Fixture could not find the Windows version stamp.");
        }
    }
}
