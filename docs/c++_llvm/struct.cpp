struct testVars {
    int a;
    public: int b = 2;
    private: const int c = 3;
};

struct testFuncs {
    int a() { return 1; }
    public: int b() { return 1; }
    private: const int c() { return 1; }
};

int main() {
//    testVars t;
    testFuncs t;
    int a = t.a();
//    int b = t.a;
//    int c = t.a;
    return a;
}
