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

public interface Reader {
    read(fly_buf buf, const ulong n, long out_read)
    close()
}

public interface Writer {
    write(fly_buf buf, const ulong n, long out_written)
    flush()
    close()
}

public class FileReader : Reader {
    int fd
    public read(fly_buf buf, const ulong n, long out_read) {}
    public close() {}
}

public class FileWriter : Writer {
    int fd
    public write(fly_buf buf, const ulong n, long out_written) {}
    public flush() {}
    public close() {}
}

public class BufReader : Reader {
    Reader inner
    long buf_ptr
    int buf_fill
    int buf_cap
    int pos
    public read(fly_buf buf, const ulong n, long out_read) {}
    public close() {}
}

public class BufWriter : Writer {
    Writer inner
    long buf_ptr
    int buf_fill
    int buf_cap
    public write(fly_buf buf, const ulong n, long out_written) {}
    public flush() {}
    public close() {}
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
public timeFormatSec(const long sec, const string pattern, string out)
public timeParse(const string s, const string pattern, fly_time out)
public timeParseToSec(const string s, const string pattern, long out)
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
public pathJoinN(const string parts, const int n, string out)
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
public pathGlobCount(const string pattern, int out)
public pathMatch(const string pattern, const string name, bool out)
public pathComp(const string path, fly_string_array out)
public pathSep(byte out)
public fsOpen(const string path, fly_file out)
public fsCreate(const string path, fly_file out)
public fsOpenOpts(const string path, const int flags, const int perm, fly_file out)
public fsClose(fly_file f)
public fsReader(fly_file f, Reader out)
public fsWriter(fly_file f, Writer out)
public fsRead(const string path, fly_buf out)
public fsWrite(const string path, fly_buf data, const int perm)
public fsAppend(const string path, fly_buf data, const int perm)
public fsSeekTo(fly_file f, const long offset, const int whence, long out)
public fsSeekPos(fly_file f, long out)
public fsStat(const string path, fly_stat out)
public fsLstat(const string path, fly_stat out)
public fsSize(const string path, ulong out)
public fsExists(const string path, bool out)
public fsSync(fly_file f)
public fsTruncate(const string path, const long size)
public fsChmod(const string path, const int mode)
public fsDelete(const string path)
public fsCopy(const string src, const string dst)
public fsMove(const string src, const string dst)
public fsRename(const string src, const string dst)
public fsDirCreate(const string path, const int perm)
public fsDirCreateAll(const string path, const int perm)
public fsDirDelete(const string path)
public fsDirDeleteAll(const string path)
public fsDirRead(const string path, fly_dir_entries out)
public fsSymlinkCreate(const string target, const string link)
public fsSymlinkRead(const string path, string out)
public fsTempFile(const string dir, const string pattern, string out_path, fly_file out_file)
public fsTempDir(const string dir, const string pattern, string out_path)
public fsWriteStr(const string path, const string content, const int perm)
public fsReadStr(const string path, string out)
public fsAppendStr(const string path, const string content, const int perm)
public timeNowSec(long out)
public durationFormatNsec(const long nsec, string out)
public envAllCount(int out)
public pathCompCount(const string path, int out)
public fsIsFile(const string path, bool out)
public fsIsDir(const string path, bool out)
public ioRead(Reader r, fly_buf buf, const ulong n, long out_read)
public ioReadAll(Reader r, fly_buf out)
public ioReadLine(Reader r, string out)
public ioReadLines(Reader r, fly_string_array out)
public ioClose(Reader r)
public ioWrite(Writer w, fly_buf buf, const ulong n, long out_written)
public ioWriteAll(Writer w, fly_buf buf)
public ioWriteString(Writer w, const string s)
public ioFlush(Writer w)
public ioReaderNew(Reader inner, const ulong cap, BufReader out)
public ioPeek(BufReader r, const ulong n, fly_buf out)
public ioBufReadLine(BufReader r, string out)
public ioFill(BufReader r)
public ioWriterNew(Writer inner, const ulong cap, BufWriter out)
public ioBufWrite(BufWriter w, fly_buf buf)
public ioBufFlush(BufWriter w)
public ioCopy(Reader src, Writer dst, long out_copied)
public ioCopyN(Reader src, Writer dst, const ulong n, long out_copied)
public ioPipe(Reader pipe_r, Writer pipe_w)
