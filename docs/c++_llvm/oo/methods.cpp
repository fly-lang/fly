struct Test {
    int a() { return 1; }
    protected: int b() { return 1; }
    private: const int c() { return 1; }
};

int main() {
    Test t;
    int a = t.a();
    return a;
}
