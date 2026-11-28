using System;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Text.RegularExpressions;

namespace FFPR.Setup
{
    internal static class SafeFiles
    {
        internal const int MaxFile = 64 * 1024 * 1024;

        internal static bool IsSegment(string value)
        {
            return !String.IsNullOrWhiteSpace(value) && !value.EndsWith(".") && !value.EndsWith(" ") &&
                value.IndexOfAny(Path.GetInvalidFileNameChars()) < 0 &&
                !Regex.IsMatch(value, @"\A(?:CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\.|\z)", RegexOptions.IgnoreCase);
        }

        internal static string In(string root, string relative)
        {
            string path = root;
            foreach (string part in relative.Split('/'))
            {
                if (!IsSegment(part)) throw new IOException("Invalid installation path.");
                path = Path.Combine(path, part);
            }
            CheckDirectory(Path.GetDirectoryName(path));
            return path;
        }

        internal static void CheckDirectory(string path)
        {
            for (var folder = new DirectoryInfo(path); folder != null; folder = folder.Parent)
            {
                FileAttributes? value = Attributes(folder.FullName);
                if (value.HasValue && ((value.Value & FileAttributes.Directory) == 0 ||
                                       (value.Value & FileAttributes.ReparsePoint) != 0))
                    throw new IOException("The installation path contains a linked folder or a file: " + folder.FullName);
            }
        }

        internal static bool Exists(string path)
        {
            FileAttributes? value = Attributes(path);
            if (!value.HasValue) return false;
            if ((value.Value & (FileAttributes.Directory | FileAttributes.ReparsePoint)) != 0)
                throw new IOException("Setup expected an ordinary file: " + path);
            return true;
        }

        private static FileAttributes? Attributes(string path)
        {
            try { return File.GetAttributes(path); }
            catch (FileNotFoundException) { return null; }
            catch (DirectoryNotFoundException) { return null; }
        }

        internal static byte[] Read(string path, int limit)
        {
            if (!Exists(path)) throw new FileNotFoundException("An expected file is missing.", path);
            using (var input = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
                return ReadBounded(input, limit);
        }

        internal static byte[] ReadBounded(Stream stream, int limit)
        {
            using (var output = new MemoryStream())
            {
                byte[] buffer = new byte[65536];
                int count;
                while ((count = stream.Read(buffer, 0, buffer.Length)) != 0)
                {
                    if (output.Length + count > limit) throw new InvalidDataException("A file exceeded its size limit.");
                    output.Write(buffer, 0, count);
                }
                return output.ToArray();
            }
        }

        internal static string Hash(byte[] content)
        {
            using (var sha = SHA256.Create()) return Hex(sha.ComputeHash(content));
        }

        internal static string HashFile(string path)
        {
            if (!Exists(path)) return null;
            using (var input = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            using (var sha = SHA256.Create())
            {
                if (input.Length > MaxFile) throw new IOException("A file exceeded its size limit: " + path);
                return Hex(sha.ComputeHash(input));
            }
        }

        private static string Hex(byte[] bytes) { return BitConverter.ToString(bytes).Replace("-", "").ToLowerInvariant(); }

        internal static void WriteAtomic(string work, string destination, byte[] bytes)
        {
            CheckDirectory(Path.GetDirectoryName(destination));
            if (bytes == null)
            {
                if (Exists(destination)) File.Delete(destination);
                return;
            }
            Directory.CreateDirectory(Path.GetDirectoryName(destination));
            string stage = In(work, "stage-" + Guid.NewGuid().ToString("N"));
            try
            {
                using (var stream = new FileStream(stage, FileMode.CreateNew, FileAccess.Write, FileShare.None))
                {
                    stream.Write(bytes, 0, bytes.Length);
                    stream.Flush(true);
                }
                if (Exists(destination)) File.Replace(stage, destination, null);
                else File.Move(stage, destination);
            }
            finally { if (Exists(stage)) File.Delete(stage); }
        }

        internal static Version ModVersion(string path)
        {
            if (!Exists(path)) return null;
            var info = FileVersionInfo.GetVersionInfo(path);
            if (info.InternalName != "ffpr_screen_reader" ||
                info.ProductName != "Final Fantasy Pixel Master Core Screen Reader" ||
                info.CompanyName != "FFPR Screen Reader Project (bladestorm360)")
                throw new IOException("The existing hid.dll does not identify itself as the FFPR Screen Reader.");
            return new Version(info.FileMajorPart, info.FileMinorPart, info.FileBuildPart, info.FilePrivatePart);
        }
    }
}
