struct test {
    int a;
public: int b = 2;
private: const int c = 3;
};

int main() {
    test t;
    t.a = 1;
    t.b = 4;
}
