struct Test {
    int a = 1;
    int b;
};

Test getTest() {
    Test t;
    return t;
}

int main() {
    Test *t = new Test();
    int a = t->a;
    t->b = 2;
    return getTest().a;
}