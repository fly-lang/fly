enum class Test {
    A, B, C
};

int main() {
    Test t = Test::A;
    if (t == Test::B)
        return 1;
    return 0;
}
