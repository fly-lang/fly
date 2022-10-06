struct test {
    int a;
public: int b = 2;
private: const int c = 3;
};

int main() {
    test t;
    int a = 1;
    t.a = a;
    t.a = 2;
//    t.a = 2;
//    t.b = 4;
    return t.a;
}
