using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net;
using System.Net.Cache;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Web.Script.Serialization;

namespace FFPR.Setup
{
    internal sealed class Release
    {
        internal readonly string Tag, Digest;
        internal readonly Version Version;
        internal readonly Uri Url;
        internal readonly int Size;

        internal Release(string tag, Version version, Uri url, int size, string digest)
        {
            Tag = tag; Version = version; Url = url; Size = size; Digest = digest;
        }
    }

    internal static class ReleaseSource
    {
        internal const string Repository = "bladestorm360/FFPRCore-Screen-Reader";
        private const int MetadataLimit = 1024 * 1024;
        private const string BundlePrefix = "Final Fantasy Pixel Master Core Screen Reader V";
        internal static readonly UTF8Encoding Utf8 = new UTF8Encoding(false, true);
        internal static readonly Regex Sha = new Regex(@"\A[0-9a-f]{64}\z");

        internal static JavaScriptSerializer Serializer(int size)
        {
            return new JavaScriptSerializer { MaxJsonLength = size, RecursionLimit = 24 };
        }

        internal static Release Check()
        {
            return Parse(Utf8.GetString(Download(new Uri("https://api.github.com/repos/" + Repository + "/releases/latest"),
                                                MetadataLimit, false)));
        }

        internal static Release Parse(string json)
        {
            var values = Serializer(MetadataLimit).DeserializeObject(json) as Dictionary<string, object>;
            if (values == null || !Object.Equals(Value(values, "draft"), false) ||
                !Object.Equals(Value(values, "prerelease"), false))
                throw new InvalidDataException("GitHub did not return a published stable release.");
            string tag = Value(values, "tag_name") as string ?? "";
            Version version;
            if (!Regex.IsMatch(tag, @"\Av[0-9]{1,5}(?:\.[0-9]{1,5}){1,3}\z") ||
                !Version.TryParse(tag.Substring(1), out version))
                throw new InvalidDataException("GitHub returned an unsupported release version.");
            version = new Version(version.Major, version.Minor, Math.Max(0, version.Build), Math.Max(0, version.Revision));
            string expected = (BundlePrefix + tag.Substring(1)).Replace(' ', '.') + ".zip";
            var url = new Uri("https://github.com/" + Repository + "/releases/download/" + tag + "/" + expected);
            Release result = null;
            var assets = Value(values, "assets") as IEnumerable<object>;
            foreach (object entry in assets ?? new object[0])
            {
                var asset = entry as Dictionary<string, object>;
                if (asset == null || !Object.Equals(Value(asset, "name"), expected)) continue;
                string digest = (Value(asset, "digest") as string ?? "").ToLowerInvariant();
                object sizeValue = Value(asset, "size");
                int size = sizeValue is int ? (int)sizeValue : 0;
                if (result != null || size <= 0 || size > SafeFiles.MaxFile ||
                    !Object.Equals(Value(asset, "state"), "uploaded") ||
                    !Object.Equals(Value(asset, "browser_download_url"), url.AbsoluteUri) ||
                    !digest.StartsWith("sha256:", StringComparison.Ordinal) || !Sha.IsMatch(digest.Substring(7)))
                    throw new InvalidDataException("The GitHub release ZIP information is invalid.");
                result = new Release(tag, version, url, size, digest.Substring(7));
            }
            return result ?? new Release(tag, version, null, 0, null);
        }

        private static object Value(Dictionary<string, object> data, string key)
        {
            object value;
            return data.TryGetValue(key, out value) ? value : null;
        }

        internal static Uri Redirect(string location)
        {
            Uri url;
            if (!Uri.TryCreate(location, UriKind.Absolute, out url) || url.Scheme != Uri.UriSchemeHttps ||
                !url.IsDefaultPort || url.UserInfo.Length != 0 || url.Fragment.Length != 0 ||
                (url.Host != "release-assets.githubusercontent.com" && url.Host != "objects.githubusercontent.com"))
                throw new InvalidDataException("GitHub redirected the download to an unexpected host.");
            return url;
        }

        private static byte[] Download(Uri url, int limit, bool asset)
        {
            using (var deadline = new CancellationTokenSource(TimeSpan.FromSeconds(65)))
            {
                for (int hop = 0; hop <= 2; ++hop)
                {
                    var request = (HttpWebRequest)WebRequest.Create(url);
                    request.AllowAutoRedirect = false;
                    request.Timeout = request.ReadWriteTimeout = 10000;
                    request.CachePolicy = new RequestCachePolicy(RequestCacheLevel.NoCacheNoStore);
                    request.UserAgent = "FFPR-Screen-Reader-Setup";
                    request.Accept = asset ? "application/octet-stream" : "application/vnd.github+json";
                    if (!asset) request.Headers["X-GitHub-Api-Version"] = "2022-11-28";
                    try
                    {
                        using (deadline.Token.Register(request.Abort))
                        using (var response = (HttpWebResponse)request.GetResponse())
                        {
                            int code = (int)response.StatusCode;
                            if (asset && (code == 301 || code == 302 || code == 303 || code == 307 || code == 308))
                            {
                                url = Redirect(response.Headers["Location"]);
                                continue;
                            }
                            if (response.StatusCode != HttpStatusCode.OK || response.ContentLength > limit)
                                throw new InvalidDataException("GitHub returned an unexpected download response.");
                            using (Stream stream = response.GetResponseStream()) return SafeFiles.ReadBounded(stream, limit);
                        }
                    }
                    catch (WebException error)
                    {
                        if (error.Response != null) error.Response.Dispose();
                        throw new IOException("Could not download from GitHub (" + error.Status + ").", error);
                    }
                }
            }
            throw new InvalidDataException("GitHub sent too many download redirects.");
        }

        internal static Dictionary<string, byte[]> Fetch(Release release)
        {
            if (release.Url == null) throw new IOException("The latest release has no compatible complete ZIP.");
            return Open(release, Download(release.Url, release.Size, true));
        }

        internal static Dictionary<string, byte[]> Open(Release release, byte[] archive)
        {
            if (archive.Length != release.Size || archive.Length > SafeFiles.MaxFile ||
                SafeFiles.Hash(archive) != release.Digest)
                throw new InvalidDataException("The release ZIP does not match the published size and SHA-256.");
            string root = BundlePrefix + release.Tag.Substring(1) + "/";
            var expected = new HashSet<string>(Payload.Names, StringComparer.Ordinal);
            expected.UnionWith(new[] { "ReadMe.md", "ReadMe.txt", "SHA256SUMS.txt" });
            var directories = new HashSet<string>(StringComparer.Ordinal) { root };
            foreach (string path in expected)
                for (int i = path.IndexOf('/'); i >= 0; i = path.IndexOf('/', i + 1))
                    directories.Add(root + path.Substring(0, i + 1));
            var files = new Dictionary<string, byte[]>(StringComparer.Ordinal);
            var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            using (var zip = new ZipArchive(new MemoryStream(archive, false), ZipArchiveMode.Read))
            {
                if (zip.Entries.Count > expected.Count + directories.Count)
                    throw new InvalidDataException("Unexpected number of release ZIP entries.");
                int budget = SafeFiles.MaxFile;
                foreach (ZipArchiveEntry entry in zip.Entries)
                {
                    if (!seen.Add(entry.FullName)) throw new InvalidDataException("Duplicate release ZIP entry.");
                    if (directories.Contains(entry.FullName) && entry.Length == 0) continue;
                    if (!entry.FullName.StartsWith(root, StringComparison.Ordinal))
                        throw new InvalidDataException("Unexpected release ZIP folder.");
                    string name = entry.FullName.Substring(root.Length);
                    if (!expected.Contains(name) || entry.Length <= 0 || entry.Length > budget ||
                        (name == "SHA256SUMS.txt" && entry.Length > 16384))
                        throw new InvalidDataException("Unexpected release ZIP file or length.");
                    using (Stream file = entry.Open())
                    {
                        byte[] data = SafeFiles.ReadBounded(file, (int)entry.Length);
                        if (data.Length != entry.Length) throw new InvalidDataException("Truncated release ZIP file.");
                        files.Add(name, data);
                    }
                    budget -= (int)entry.Length;
                }
            }
            if (files.Count != expected.Count) throw new InvalidDataException("The release ZIP is incomplete.");

            var hashes = new Dictionary<string, string>(StringComparer.Ordinal);
            foreach (string line in Utf8.GetString(files["SHA256SUMS.txt"]).Split('\n'))
            {
                if (String.IsNullOrWhiteSpace(line)) continue;
                Match match = Regex.Match(line.TrimEnd('\r'), @"\A([0-9a-f]{64})  (.+)\z");
                if (!match.Success || match.Groups[2].Value == "SHA256SUMS.txt" ||
                    !files.ContainsKey(match.Groups[2].Value) || hashes.ContainsKey(match.Groups[2].Value))
                    throw new InvalidDataException("The release checksum list is invalid.");
                hashes.Add(match.Groups[2].Value, match.Groups[1].Value);
            }
            if (hashes.Count != expected.Count - 1) throw new InvalidDataException("The release checksum list is incomplete.");
            foreach (var hash in hashes)
                if (SafeFiles.Hash(files[hash.Key]) != hash.Value)
                    throw new InvalidDataException("The release file checksum did not match: " + hash.Key);
            foreach (var pinned in Payload.Pinned)
                if (hashes[pinned.Key] != pinned.Value)
                    throw new InvalidDataException("This setup requires a compatible pinned runtime: " + pinned.Key);
            foreach (var entry in files.Where(item => item.Key.EndsWith(".dll", StringComparison.Ordinal)))
                if (!Payload.IsX64Dll(entry.Value)) throw new InvalidDataException("Expected a Windows x64 DLL: " + entry.Key);
            return Payload.Names.ToDictionary(name => name, name => files[name], StringComparer.Ordinal);
        }
    }

    internal static class Payload
    {
        internal static readonly Dictionary<string, string> Pinned = Pins();
        internal static readonly string[] Names = new[] {
            "hid.dll", "SDL3.dll", "MinHook_LICENSE.txt", "SDL3_LICENSE.txt", ".ffpr-deps/prism/SOURCE.txt"
        }.Concat(Pinned.Keys.OrderBy(name => name, StringComparer.Ordinal)).ToArray();

        private static Dictionary<string, string> Pins()
        {
            var result = new Dictionary<string, string>(StringComparer.Ordinal);
            foreach (string source in new[] { "prism", "openal" })
            {
                using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(source + ".lock.json"))
                {
                    var root = ReleaseSource.Serializer(16384).Deserialize<Dictionary<string, object>>(
                        ReleaseSource.Utf8.GetString(SafeFiles.ReadBounded(stream, 16384)));
                    var files = (Dictionary<string, object>)root["files"];
                    foreach (var file in files)
                    {
                        string hash = source == "prism" ? (string)((Dictionary<string, object>)file.Value)["sha256"] : (string)file.Value;
                        string folder = source == "prism" ? "prism" : "openal-soft";
                        result.Add(".ffpr-deps/" + folder + "/" + file.Key, hash);
                    }
                }
            }
            return result;
        }

        internal static bool IsX64Dll(byte[] bytes)
        {
            if (bytes.Length < 64 || bytes[0] != 'M' || bytes[1] != 'Z') return false;
            int pe = BitConverter.ToInt32(bytes, 60);
            return pe >= 64 && pe <= bytes.Length - 26 && BitConverter.ToInt32(bytes, pe) == 0x4550 &&
                BitConverter.ToUInt16(bytes, pe + 4) == 0x8664 &&
                (BitConverter.ToUInt16(bytes, pe + 22) & 0x2000) != 0 &&
                BitConverter.ToUInt16(bytes, pe + 24) == 0x20b;
        }
    }
}
