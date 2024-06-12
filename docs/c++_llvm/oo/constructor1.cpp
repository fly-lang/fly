class Test {
    int a;
public:
    Test(int a) {
        this->a = a;
    }

    int getA() {
        return a;
    }
};

int main() {
    Test *t = new Test(1);
    int a = t->getA();
    a = t->getA();
    delete t;
    return a;
}
