struct Test {
    int a = 1;
    int b;
};

int main() {
    Test *t = new Test();
    int a = t->a;
    t->b = 2;
    return 0;
}
