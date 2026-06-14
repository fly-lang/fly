namespace fly.bridge

public struct Args {
}

public class CLang {
    string lib
    public call(string sym, const Args args) {}
    public call(string sym, const Args args, int out) {}
    public call(string sym, const Args args, long out) {}
    public call(string sym, const Args args, float out) {}
    public call(string sym, const Args args, double out) {}
    public call(string sym, const Args args, bool out) {}
    public call(string sym, const Args args, fly.mem.Ptr out) {}
}

