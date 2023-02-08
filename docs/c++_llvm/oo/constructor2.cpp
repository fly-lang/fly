class Test {

public:
    int a;
};

int main() {
    Test *t = new Test();
    return t->a;
}
