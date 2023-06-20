class Test {

public:
    int a = 0;
};

int main() {
    Test *t = new Test();
    int a = t->a;
    return a;
}
