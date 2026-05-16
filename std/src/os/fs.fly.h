namespace fly.os.fs

public struct File {
    int fd
    byte flags
}

public struct Stat {
    ulong size
    ulong mtime_sec
    ulong mtime_nsec
    uint mode
    int is_file
    int is_dir
    int is_symlink
}

public struct DirEntry {
    string name
    Stat stat
}

public struct DirEntries {
    long items
    ulong len
    ulong cap
}

public open(const string path, File out)
public create(const string path, File out)
public openOpts(const string path, const int flags, const int perm, File out)
public close(File f)
public read(const string path, fly.os.io.Buf out)
public write(const string path, fly.os.io.Buf data, const int perm)
public append(const string path, fly.os.io.Buf data, const int perm)
public seekTo(File f, const long offset, const int whence, long out)
public seekPos(File f, long out)
public stat(const string path, Stat out)
public lstat(const string path, Stat out)
public size(const string path, ulong out)
public exists(const string path, bool out)
public sync(File f)
public truncate(const string path, const long size)
public chmod(const string path, const int mode)
public delete(const string path)
public copy(const string src, const string dst)
public move(const string src, const string dst)
public rename(const string src, const string dst)
public dirCreate(const string path, const int perm)
public dirCreateAll(const string path, const int perm)
public dirDelete(const string path)
public dirDeleteAll(const string path)
public dirRead(const string path, DirEntries out)
public symlinkCreate(const string target, const string link)
public symlinkRead(const string path, string out)
public tempFile(const string dir, const string pattern, string out_path, File out_file)
public tempDir(const string dir, const string pattern, string out_path)
public writeStr(const string path, const string content, const int perm)
public readStr(const string path, string out)
public appendStr(const string path, const string content, const int perm)
public isFile(const string path, bool out)
public isDir(const string path, bool out)
