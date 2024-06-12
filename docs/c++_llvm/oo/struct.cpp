struct Test {
    int a;
    int b;
};

int main() {
    Test *t = new Test();
    t->a = 1;
    t->b = 2;
    delete t;
    return 1;
}