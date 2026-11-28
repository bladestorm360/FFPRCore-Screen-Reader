using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using Microsoft.Win32;

namespace FFPR.Setup
{
    internal sealed class Game
    {
        internal readonly int AppId;
        internal readonly string Title;
        internal string Exe { get { return Title + ".exe"; } }

        internal Game(int id, string title) { AppId = id; Title = title; }
    }

    internal sealed class GameTarget
    {
        internal readonly Game Game;
        internal readonly string Root;

        internal GameTarget(Game game, string root) { Game = game; Root = root; }
        public override string ToString() { return Game.Title + " — " + Root; }

        internal void Validate()
        {
            SafeFiles.CheckDirectory(Root);
            if (!SafeFiles.Exists(SafeFiles.In(Root, Game.Exe)))
                throw new IOException("The selected game executable is missing.");
        }

        internal void RequireClosed()
        {
            Validate();
            Process[] matches = Process.GetProcessesByName(Path.GetFileNameWithoutExtension(Game.Exe));
            try
            {
                if (matches.Length != 0) throw new IOException("Close " + Game.Title + " before making changes.");
            }
            finally { foreach (Process match in matches) match.Dispose(); }
        }
    }

    internal static class GameCatalog
    {
        internal static readonly Game[] Games = {
            new Game(1173770, "FINAL FANTASY"), new Game(1173780, "FINAL FANTASY II"),
            new Game(1173790, "FINAL FANTASY III"), new Game(1173800, "FINAL FANTASY IV"),
            new Game(1173810, "FINAL FANTASY V"), new Game(1173820, "FINAL FANTASY VI")
        };

        internal static List<GameTarget> Find(IEnumerable<string> steamRoots)
        {
            var libraries = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (string value in steamRoots)
            {
                string root = FullLocalPath(value);
                if (root == null || !libraries.Add(root)) continue;
                string list = ReadOptional(Path.Combine(root, "steamapps", "libraryfolders.vdf"));
                if (list == null) continue;
                foreach (Match field in Regex.Matches(list, "\"(?:path|[0-9]+)\"\\s*\"([^\"\\r\\n]+)\"",
                                                     RegexOptions.IgnoreCase))
                {
                    string candidate = FullLocalPath(field.Groups[1].Value.Replace("\\\\", "\\"));
                    if (candidate != null && libraries.Count < 128) libraries.Add(candidate);
                }
            }

            var targets = new List<GameTarget>();
            foreach (Game game in Games)
                foreach (string library in libraries)
                {
                    try
                    {
                        string apps = Path.Combine(library, "steamapps");
                        string manifest = ReadOptional(Path.Combine(apps, "appmanifest_" + game.AppId + ".acf"));
                        if (manifest == null || Field(manifest, "appid") != game.AppId.ToString()) continue;
                        string folder = Field(manifest, "installdir");
                        if (!SafeFiles.IsSegment(folder)) continue;
                        var target = new GameTarget(game, Path.Combine(apps, "common", folder));
                        target.Validate();
                        targets.Add(target);
                    }
                    catch (IOException) { }
                    catch (UnauthorizedAccessException) { }
                }
            return targets;
        }

        internal static IEnumerable<string> SteamRoots()
        {
            var roots = new List<string>();
            foreach (RegistryKey hive in new[] { Registry.CurrentUser, Registry.LocalMachine })
                foreach (string name in new[] { @"Software\Valve\Steam", @"Software\WOW6432Node\Valve\Steam" })
                    try
                    {
                        using (RegistryKey key = hive.OpenSubKey(name, false))
                            if (key != null)
                                foreach (string field in new[] { "SteamPath", "InstallPath" })
                                    roots.Add(key.GetValue(field) as string);
                    }
                    catch (UnauthorizedAccessException) { }
                    catch (System.Security.SecurityException) { }
            foreach (string variable in new[] { "ProgramFiles(x86)", "ProgramFiles" })
            {
                string root = Environment.GetEnvironmentVariable(variable);
                if (!String.IsNullOrEmpty(root)) roots.Add(Path.Combine(root, "Steam"));
            }
            return roots;
        }

        private static string Field(string text, string name)
        {
            Match match = Regex.Match(text, "\"" + name + "\"\\s*\"([^\"\\r\\n]+)\"", RegexOptions.IgnoreCase);
            return match.Success ? match.Groups[1].Value : null;
        }

        private static string FullLocalPath(string path)
        {
            if (String.IsNullOrWhiteSpace(path) || !Regex.IsMatch(path, @"\A[A-Za-z]:[/\\]")) return null;
            try
            {
                string full = Path.GetFullPath(path);
                return full == Path.GetPathRoot(full) ? full : full.TrimEnd(Path.DirectorySeparatorChar);
            }
            catch (ArgumentException) { return null; }
            catch (NotSupportedException) { return null; }
            catch (PathTooLongException) { return null; }
        }

        private static string ReadOptional(string path)
        {
            try
            {
                SafeFiles.CheckDirectory(Path.GetDirectoryName(path));
                if (!SafeFiles.Exists(path)) return null;
                return new System.Text.UTF8Encoding(false, true).GetString(SafeFiles.Read(path, 262144)).TrimStart('\uFEFF');
            }
            catch (IOException) { return null; }
            catch (UnauthorizedAccessException) { return null; }
            catch (System.Text.DecoderFallbackException) { return null; }
        }
    }
}
