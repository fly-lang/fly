namespace fly.os.io

public struct Buf {
    long ptr
    ulong size
    ulong cap
}

public interface Reader {
    read(Buf buf, const ulong n)
    close()
}

public interface Writer {
    write(Buf buf, const ulong n)
    flush()
    close()
}

public class FileReader : Reader {
    int fd
    public read(Buf buf, const ulong n) {}
    public close() {}
}

public class FileWriter : Writer {
    int fd
    public write(Buf buf, const ulong n) {}
    public flush() {}
    public close() {}
}

public class BufReader : Reader {
    long buf_ptr
    int buf_fill
    int buf_cap
    int pos
    public read(Buf buf, const ulong n) {}
    public close() {}
}

public class BufWriter : Writer {
    long buf_ptr
    int buf_fill
    int buf_cap
    public write(Buf buf, const ulong n) {}
    public flush() {}
    public close() {}
}

public Reader fileReader(const int fd)
public Writer fileWriter(const int fd)
public Buf readAll(Reader r)
public string readLine(Reader r)
public fly.os.env.StringArray readLines(Reader r)
public bool writeAll(Writer w, Buf buf)
public bool writeString(Writer w, const string s)
public BufReader readerNew(Reader inner, const ulong cap)
public Buf peek(BufReader r, const ulong n)
public string bufReadLine(BufReader r)
public fill(BufReader r)
public BufWriter writerNew(Writer inner, const ulong cap)
public bufWrite(BufWriter w, Buf buf)
public bufFlush(BufWriter w)
public long copy(Reader src, Writer dst)
public long copyN(Reader src, Writer dst, const ulong n)
public long pipe(Reader pipe_r, Writer pipe_w)

public void printErr(const string s) {
    Writer w = fileWriter(2)
    writeString(w, s)
}

public void printErrLn(const string s) {
    Writer w = fileWriter(2)
    writeString(w, s)
    writeString(w, "\n")
}

public void print(const string s) {
    Writer w = fileWriter(1)
    writeString(w, s)
}

public void printLn(const string s) {
    Writer w = fileWriter(1)
    writeString(w, s)
    writeString(w, "\n")
}
