class Test {
public:
    static int a;
    static const int b = 3;
};

int main() {
    Test::a = 1;
    return Test::b;
}
