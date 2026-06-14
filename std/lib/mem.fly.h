namespace fly.mem

public struct Ptr {
    ulong addr
}

public Ptr toPtr(const long addr)
public copy(const long dst, const long src, const ulong size)
public move(const long dst, const long src, const ulong size)
public zero(const long dst, const ulong size)
public long alloc(const ulong size)
public free(const long ptr, const ulong size)
public bool isNull(const long ptr)
public long realloc(const long ptr, const ulong newSize)
public int compare(const long a, const long b, const ulong size)
public byte readByte(const long ptr, const int offset)
public short readShort(const long ptr, const int offset)
public int readInt(const long ptr, const int offset)
public long readLong(const long ptr, const int offset)
public writeByte(const long ptr, const int offset, const byte val)
public writeShort(const long ptr, const int offset, const short val)
public writeInt(const long ptr, const int offset, const int val)
public writeLong(const long ptr, const int offset, const long val)
