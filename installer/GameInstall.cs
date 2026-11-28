using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace FFPR.Setup
{
    // A content-addressed record per game. The pending record is written
    // before the first changed file. Its removal commits the entire operation.
    internal sealed class Ownership
    {
        public Dictionary<string, string> Original { get; set; }
        public Dictionary<string, string> Current { get; set; }
    }

    internal sealed class Snapshot
    {
        public Dictionary<string, string> Before { get; set; }
        public Dictionary<string, string> After { get; set; }
        public Ownership Previous { get; set; }
    }

    internal sealed class InstallState
    {
        public Ownership Active { get; set; }
        public Snapshot Rollback { get; set; }
    }

    internal sealed class PendingChange
    {
        public InstallState OldState { get; set; }
        public Snapshot Files { get; set; }
    }

    internal sealed class GameInstall
    {
        private const string WorkName = ".ffpr-setup";
        private const string Proxy = "hid.dll";
        private const int RecordLimit = 128 * 1024;
        private static readonly HashSet<string> Allowed = new HashSet<string>(Payload.Names, StringComparer.Ordinal);
        private readonly GameTarget game;
        private readonly string work;

        internal GameInstall(GameTarget game)
        {
            this.game = game;
            work = SafeFiles.In(game.Root, WorkName);
            SafeFiles.CheckDirectory(work);
        }

        private string StatePath { get { return SafeFiles.In(work, "state.json"); } }
        private string PendingPath { get { return SafeFiles.In(work, "pending.json"); } }
        internal bool RecoveryPending { get { return SafeFiles.Exists(PendingPath); } }
        internal Version InstalledVersion()
        {
            game.Validate();
            return SafeFiles.ModVersion(Target(Proxy));
        }

        internal string Install(Version version, Dictionary<string, byte[]> payload)
        {
            // The check path is read-only, including for an existing manual install.
            Version installed = InstalledVersion();
            if (!RecoveryPending && installed != null && installed >= version)
                return installed == version ? "Already up to date." : "Installed version is newer. No changes.";
            return WithLock(state =>
            {
                installed = InstalledVersion();
                if (installed != null && installed >= version)
                    return installed == version ? "Already up to date." : "Installed version is newer. No changes.";
                if (!Allowed.SetEquals(payload.Keys)) throw new InvalidDataException("Incomplete install payload.");
                if (state.Active != null) RequireMatch(state.Active.Current);
                var active = state.Active == null
                    ? new Ownership { Original = Map(), Current = Map() }
                    : new Ownership { Original = Map(state.Active.Original), Current = Map(state.Active.Current) };
                var change = EmptySnapshot(state.Active);
                foreach (string name in Payload.Names)
                {
                    string old = SafeFiles.HashFile(Target(name));
                    byte[] next = payload[name];
                    string hash = SafeFiles.Hash(next);
                    if (hash == old) continue; // Equal pre-existing files remain owned by their original installer.
                    if ((name == "SDL3.dll" || name == "SDL3_LICENSE.txt" || name == "MinHook_LICENSE.txt") &&
                        old != null && !active.Current.ContainsKey(name))
                        throw new IOException("An existing " + name + " differs from the release. Setup left it in place.");
                    SaveExisting(name, old);
                    SaveBlob(hash, next);
                    change.Before.Add(name, old);
                    change.After.Add(name, hash);
                    if (!active.Original.ContainsKey(name)) active.Original.Add(name, name == Proxy ? null : old);
                    active.Current[name] = hash;
                }
                return Commit(state, change, new InstallState { Active = active, Rollback = change },
                              "Installed version " + version + ".");
            });
        }

        internal string Rollback()
        {
            if (!RecoveryPending && !SafeFiles.Exists(StatePath)) return "There is no saved change to roll back.";
            return WithLock(state =>
            {
                if (state.Rollback == null) return "There is no saved change to roll back.";
                Snapshot previous = state.Rollback;
                var restore = new Snapshot { Before = previous.After, After = previous.Before, Previous = state.Active };
                return Commit(state, restore, new InstallState { Active = previous.Previous }, "Rolled back the last change.");
            }, true);
        }

        internal string Uninstall()
        {
            if (!RecoveryPending && InstalledVersion() == null) return "The screen reader is not installed.";
            return WithLock(state =>
            {
                if (InstalledVersion() == null) return "The screen reader is not installed.";
                var change = EmptySnapshot(state.Active);
                var contents = state.Active != null
                    ? state.Active.Current.Where(file => !IsShared(file.Key)).ToDictionary(file => file.Key, file => file.Value)
                    : new Dictionary<string, string> { { Proxy, SafeFiles.HashFile(Target(Proxy)) } };
                RequireMatch(contents);
                foreach (var file in contents)
                {
                    string restore = state.Active == null ? null : state.Active.Original[file.Key];
                    SaveExisting(file.Key, file.Value);
                    change.Before.Add(file.Key, file.Value);
                    change.After.Add(file.Key, restore);
                }
                return Commit(state, change, new InstallState { Rollback = change }, "Uninstalled the screen reader.");
            });
        }

        private string WithLock(Func<InstallState, string> run, bool recoverOnly = false)
        {
            game.RequireClosed();
            Directory.CreateDirectory(work);
            string lockPath = SafeFiles.In(work, "lock");
            SafeFiles.Exists(lockPath);
            using (var held = new FileStream(lockPath, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None))
            {
                game.RequireClosed();
                bool pending = RecoveryPending;
                if (pending) Recover();
                InstallState state = ReadState();
                Prune(state);
                if (pending && recoverOnly) return "Recovered the interrupted change.";
                return run(state);
            }
        }

        private string Commit(InstallState previous, Snapshot change, InstallState next, string result)
        {
            RequireMatch(change.Before);
            VerifyBlobs(change.Before);
            VerifyBlobs(change.After);
            game.RequireClosed();
            WriteRecord(PendingPath, new PendingChange { OldState = previous, Files = change });
            try
            {
                Apply(change.Before, change.After);
                RequireMatch(change.After);
                WriteRecord(StatePath, next);
                File.Delete(PendingPath);
            }
            catch (Exception failure)
            {
                try { Recover(); }
                catch (Exception recovery)
                {
                    throw new IOException("Setup could not finish or restore all files. Close the game and choose Rollback. " +
                                          recovery.Message, new AggregateException(failure, recovery));
                }
                throw new IOException("The changes were rolled back. " + failure.Message, failure);
            }
            Prune(next);
            return result;
        }

        private void Recover()
        {
            game.RequireClosed();
            var pending = ReadRecord<PendingChange>(PendingPath);
            if (pending == null) return;
            ValidateState(pending.OldState);
            ValidateSnapshot(pending.Files);
            foreach (var file in pending.Files.Before)
            {
                string actual = SafeFiles.HashFile(Target(file.Key));
                if (actual != file.Value && actual != pending.Files.After[file.Key])
                    throw new IOException("A file was changed outside setup during recovery: " + file.Key);
            }
            VerifyBlobs(pending.Files.Before);
            Apply(pending.Files.After, pending.Files.Before);
            RequireMatch(pending.Files.Before);
            WriteRecord(StatePath, pending.OldState);
            File.Delete(PendingPath);
        }

        private void Apply(Dictionary<string, string> expected, Dictionary<string, string> map)
        {
            foreach (var file in map.OrderBy(item => item.Key != Proxy ? 1 : item.Value == null ? 0 : 2))
            {
                string path = Target(file.Key);
                game.RequireClosed();
                string actual = SafeFiles.HashFile(path);
                if (actual == file.Value) continue;
                if (actual != expected[file.Key])
                    throw new IOException("A file was changed outside setup: " + file.Key);
                SafeFiles.WriteAtomic(work, path, file.Value == null ? null : ReadBlob(file.Value));
            }
        }

        private string Target(string name) { return SafeFiles.In(game.Root, name); }
        private string Blob(string hash) { return SafeFiles.In(work, "files/" + hash); }
        // SDL can also serve unrelated game mods after setup adds it.
        private static bool IsShared(string name) { return name == "SDL3.dll" || name == "SDL3_LICENSE.txt"; }

        private void SaveExisting(string name, string hash)
        {
            if (hash == null) return;
            byte[] bytes = SafeFiles.Read(Target(name), SafeFiles.MaxFile);
            if (SafeFiles.Hash(bytes) != hash) throw new IOException("File changed while making its backup: " + name);
            SaveBlob(hash, bytes);
        }

        private void SaveBlob(string hash, byte[] data)
        {
            string path = Blob(hash);
            string old = SafeFiles.HashFile(path);
            if (old == hash) return;
            if (old != null) throw new IOException("A setup backup is damaged.");
            SafeFiles.WriteAtomic(work, path, data);
        }

        private byte[] ReadBlob(string hash)
        {
            byte[] bytes = SafeFiles.Read(Blob(hash), SafeFiles.MaxFile);
            if (SafeFiles.Hash(bytes) != hash) throw new IOException("A setup backup is damaged.");
            return bytes;
        }

        private void RequireMatch(Dictionary<string, string> map)
        {
            foreach (var file in map)
                if (SafeFiles.HashFile(Target(file.Key)) != file.Value)
                    throw new IOException("A file was changed outside setup: " + file.Key);
        }

        private void VerifyBlobs(Dictionary<string, string> map)
        {
            foreach (string hash in map.Values.Where(value => value != null).Distinct())
                if (SafeFiles.HashFile(Blob(hash)) != hash) throw new IOException("A setup backup is missing or damaged.");
        }

        private void Prune(InstallState state)
        {
            // Keep the last successful change and original files needed by
            // Uninstall. Cleanup is optional if another program holds a backup.
            try
            {
                var keep = new HashSet<string>(StringComparer.Ordinal);
                Action<Ownership> retain = owner =>
                {
                    if (owner != null) { keep.UnionWith(owner.Original.Values); keep.UnionWith(owner.Current.Values); }
                };
                retain(state.Active);
                if (state.Rollback != null)
                {
                    retain(state.Rollback.Previous);
                    keep.UnionWith(state.Rollback.Before.Values);
                    keep.UnionWith(state.Rollback.After.Values);
                }
                string folder = Path.GetDirectoryName(Blob(new string('0', 64)));
                if (Directory.Exists(folder))
                    foreach (string file in Directory.EnumerateFiles(folder).Take(256))
                        if (ReleaseSource.Sha.IsMatch(Path.GetFileName(file)) && !keep.Contains(Path.GetFileName(file)) &&
                            SafeFiles.Exists(file)) File.Delete(file);
                foreach (string file in Directory.EnumerateFiles(work, "stage-*").Take(64))
                    if (System.Text.RegularExpressions.Regex.IsMatch(Path.GetFileName(file), @"\Astage-[a-f0-9]{32}\z") &&
                        SafeFiles.Exists(file)) File.Delete(file);
            }
            catch (IOException) { }
            catch (UnauthorizedAccessException) { }
        }

        private InstallState ReadState()
        {
            InstallState state = ReadRecord<InstallState>(StatePath) ?? new InstallState();
            ValidateState(state);
            return state;
        }

        private static T ReadRecord<T>(string path) where T : class
        {
            if (!SafeFiles.Exists(path)) return null;
            var result = ReleaseSource.Serializer(RecordLimit).Deserialize<T>(ReleaseSource.Utf8.GetString(SafeFiles.Read(path, RecordLimit)));
            if (result == null) throw new InvalidDataException("Setup's recovery record is invalid.");
            return result;
        }

        private void WriteRecord(string path, object record)
        {
            SafeFiles.WriteAtomic(work, path, ReleaseSource.Utf8.GetBytes(ReleaseSource.Serializer(RecordLimit).Serialize(record)));
        }

        private static void ValidateState(InstallState state)
        {
            if (state == null) throw new InvalidDataException("Setup's state record is missing.");
            ValidateOwnership(state.Active);
            if (state.Rollback != null) ValidateSnapshot(state.Rollback);
        }

        private static void ValidateSnapshot(Snapshot snapshot)
        {
            if (snapshot == null) throw new InvalidDataException("Setup's backup record is missing.");
            ValidatePair(snapshot.Before, snapshot.After);
            ValidateOwnership(snapshot.Previous);
        }

        private static void ValidateOwnership(Ownership owner)
        {
            if (owner == null) return;
            ValidatePair(owner.Original, owner.Current);
            string proxy;
            if (!owner.Current.TryGetValue(Proxy, out proxy) || proxy == null || owner.Original[Proxy] != null ||
                owner.Current.Values.Any(hash => hash == null))
                throw new InvalidDataException("Setup's ownership record is invalid.");
        }

        private static void ValidatePair(Dictionary<string, string> first, Dictionary<string, string> second)
        {
            if (first == null || second == null || !new HashSet<string>(first.Keys, StringComparer.Ordinal).SetEquals(second.Keys) ||
                first.Keys.Any(name => !Allowed.Contains(name)) ||
                first.Values.Concat(second.Values).Any(hash => hash != null && !ReleaseSource.Sha.IsMatch(hash)))
                throw new InvalidDataException("Setup's backup record has unexpected paths or checksums.");
        }

        private static Snapshot EmptySnapshot(Ownership owner)
        {
            return new Snapshot { Before = Map(), After = Map(), Previous = owner };
        }

        private static Dictionary<string, string> Map(Dictionary<string, string> from = null)
        {
            return from == null ? new Dictionary<string, string>(StringComparer.Ordinal) :
                new Dictionary<string, string>(from, StringComparer.Ordinal);
        }
    }
}
