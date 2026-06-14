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

public File open(const string path)
public File create(const string path)
public File openOpts(const string path, const int flags, const int perm)
public close(File f)
public fly.os.io.Buf read(const string path)
public write(const string path, fly.os.io.Buf data, const int perm)
public append(const string path, fly.os.io.Buf data, const int perm)
public long seekTo(File f, const long offset, const int whence)
public long seekPos(File f)
public Stat stat(const string path)
public Stat lstat(const string path)
public ulong size(const string path)
public bool exists(const string path)
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
public DirEntries dirRead(const string path)
public symlinkCreate(const string target, const string link)
public string symlinkRead(const string path)
public tempFile(const string dir, const string pattern)
public string tempDir(const string dir, const string pattern)
public writeStr(const string path, const string content, const int perm)
public string readStr(const string path)
public appendStr(const string path, const string content, const int perm)
public bool isFile(const string path)
public bool isDir(const string path)
