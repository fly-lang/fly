struct Test {
    int a;
    int b;
    const int c = 3;
};

int main() {
    Test t;
    int a = t.a;
    return 0;
}
