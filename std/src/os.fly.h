namespace fly.os

public struct fly_time {
    long sec
    long nsec
}

public struct fly_duration {
    long nsec
}

public struct fly_file {
    int fd
    byte flags
}

public struct fly_stat {
    ulong size
    ulong mtime_sec
    ulong mtime_nsec
    uint mode
    int is_file
    int is_dir
    int is_symlink
}

public struct fly_buf {
    long ptr
    ulong size
    ulong cap
}

public struct fly_reader {
    long ctx
    long fn_read
    long fn_close
}

public struct fly_writer {
    long ctx
    long fn_write
    long fn_flush
    long fn_close
}

public struct fly_buf_reader {
    fly_reader inner
    fly_buf buf
    ulong pos
}

public struct fly_buf_writer {
    fly_writer inner
    fly_buf buf
}

public struct fly_dir_entry {
    string name
    fly_stat stat
}

public struct fly_dir_entries {
    long items
    ulong len
    ulong cap
}

public struct fly_string_array {
    long items
    int count
}

public timeNow(fly_time out)
public timeMonotonic(fly_time out)
public timeSleep(fly_duration d)
public timeSince(fly_time t, fly_duration out)
public timeDiff(fly_time a, fly_time b, fly_duration out)
public timeAdd(fly_time t, fly_duration d, fly_time out)
public timeCompare(fly_time a, fly_time b, int out)
public timeUnix(fly_time t, long out)
public timeUnixNano(fly_time t, long out)
public timeFromUnix(const long sec, fly_time out)
public timeFromUnixNano(const long nsec, fly_time out)
public timeFormat(fly_time t, const string pattern, string out)
public timeParse(const string s, const string pattern, fly_time out)
public timeDurationSecs(fly_duration d, long out)
public timeDurationMillis(fly_duration d, long out)
public timeDurationMicros(fly_duration d, long out)
public timeDurationFormat(fly_duration d, string out)
public envGet(const string key, string out)
public envSet(const string key, const string value)
public envDelete(const string key)
public envAll(fly_string_array out)
public envExpand(const string s, string out)
public envCwdGet(string out)
public envCwdSet(const string path)
public envArgsGet(fly_string_array out)
public envArgsCount(int out)
public envHostname(string out)
public envOsname(string out)
public envExit(const int code)
public pathJoin(const string base, const string comp, string out)
public pathJoinN(const string parts, const ulong n, string out)
public pathAbsolute(const string path, string out)
public pathBasename(const string path, string out)
public pathDirname(const string path, string out)
public pathExt(const string path, string out)
public pathStem(const string path, string out)
public pathSplit(const string path, string out_dir, string out_base)
public pathSplitExt(const string path, string out_stem, string out_ext)
public pathIsAbsolute(const string path, bool out)
public pathIsRelative(const string path, bool out)
public pathNormalize(const string path, string out)
public pathNormalizeInPlace(string path)
public pathRel(const string base, const string target, string out)
public pathIsFile(const string path, bool out)
public pathIsDir(const string path, bool out)
public pathIsSym(const string path, bool out)
public pathGlob(const string pattern, fly_string_array out)
public pathMatch(const string pattern, const string name, bool out)
public pathComp(const string path, fly_string_array out)
public pathSep(byte out)
public fsOpen(const string path, fly_file out)
public fsCreate(const string path, fly_file out)
public fsOpenOpts(const string path, const byte flags, const uint perm, fly_file out)
public fsClose(fly_file f)
public fsReader(fly_file f, fly_reader out)
public fsWriter(fly_file f, fly_writer out)
public fsRead(const string path, fly_buf out)
public fsWrite(const string path, fly_buf data, const uint perm)
public fsAppend(const string path, fly_buf data, const uint perm)
public fsSeekTo(fly_file f, const long offset, const int whence, long out)
public fsSeekPos(fly_file f, long out)
public fsStat(const string path, fly_stat out)
public fsLstat(const string path, fly_stat out)
public fsSize(const string path, ulong out)
public fsExists(const string path, bool out)
public fsSync(fly_file f)
public fsTruncate(const string path, const ulong size)
public fsChmod(const string path, const uint mode)
public fsDelete(const string path)
public fsCopy(const string src, const string dst)
public fsMove(const string src, const string dst)
public fsRename(const string src, const string dst)
public fsDirCreate(const string path, const uint perm)
public fsDirCreateAll(const string path, const uint perm)
public fsDirDelete(const string path)
public fsDirDeleteAll(const string path)
public fsDirRead(const string path, fly_dir_entries out)
public fsSymlinkCreate(const string target, const string link)
public fsSymlinkRead(const string path, string out)
public fsTempFile(const string dir, const string pattern, string out_path, fly_file out_file)
public fsTempDir(const string dir, const string pattern, string out_path)
public ioRead(fly_reader r, fly_buf buf, const ulong n, ulong out_read)
public ioReadAll(fly_reader r, fly_buf out)
public ioReadLine(fly_reader r, string out)
public ioReadLines(fly_reader r, fly_string_array out)
public ioClose(fly_reader r)
public ioWrite(fly_writer w, fly_buf buf, const ulong n, ulong out_written)
public ioWriteAll(fly_writer w, fly_buf buf)
public ioWriteString(fly_writer w, const string s)
public ioFlush(fly_writer w)
public ioReaderNew(fly_reader inner, const ulong cap, fly_buf_reader out)
public ioPeek(fly_buf_reader r, const ulong n, fly_buf out)
public ioBufReadLine(fly_buf_reader r, string out)
public ioFill(fly_buf_reader r)
public ioWriterNew(fly_writer inner, const ulong cap, fly_buf_writer out)
public ioBufWrite(fly_buf_writer w, fly_buf buf)
public ioBufFlush(fly_buf_writer w)
public ioCopy(fly_reader src, fly_writer dst, long out_copied)
public ioCopyN(fly_reader src, fly_writer dst, const ulong n, long out_copied)
public ioPipe(fly_reader pipe_r, fly_writer pipe_w)
