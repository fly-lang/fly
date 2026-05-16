namespace fly.os.io

public struct Buf {
    long ptr
    ulong size
    ulong cap
}

public interface Reader {
    read(Buf buf, const ulong n, long out_read)
    close()
}

public interface Writer {
    write(Buf buf, const ulong n, long out_written)
    flush()
    close()
}

public class FileReader : Reader {
    int fd
    public read(Buf buf, const ulong n, long out_read) {}
    public close() {}
}

public class FileWriter : Writer {
    int fd
    public write(Buf buf, const ulong n, long out_written) {}
    public flush() {}
    public close() {}
}

public class BufReader : Reader {
    Reader inner
    long buf_ptr
    int buf_fill
    int buf_cap
    int pos
    public read(Buf buf, const ulong n, long out_read) {}
    public close() {}
}

public class BufWriter : Writer {
    Writer inner
    long buf_ptr
    int buf_fill
    int buf_cap
    public write(Buf buf, const ulong n, long out_written) {}
    public flush() {}
    public close() {}
}

public fileReader(const int fd, Reader out)
public fileWriter(const int fd, Writer out)
public readAll(Reader r, Buf out)
public readLine(Reader r, string out)
public readLines(Reader r, fly.os.env.StringArray out)
public writeAll(Writer w, Buf buf)
public writeString(Writer w, const string s)
public readerNew(Reader inner, const ulong cap, BufReader out)
public peek(BufReader r, const ulong n, Buf out)
public bufReadLine(BufReader r, string out)
public fill(BufReader r)
public writerNew(Writer inner, const ulong cap, BufWriter out)
public bufWrite(BufWriter w, Buf buf)
public bufFlush(BufWriter w)
public copy(Reader src, Writer dst, long out_copied)
public copyN(Reader src, Writer dst, const ulong n, long out_copied)
public pipe(Reader pipe_r, Writer pipe_w)
