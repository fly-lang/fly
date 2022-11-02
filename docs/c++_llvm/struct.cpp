struct test {
    int a;
public: int b = 2;
private: const int c = 3;
};

int main() {
    test t;
    int a = t.a;
    int b = t.a;
    int c = t.a;
    return t.a;
}
