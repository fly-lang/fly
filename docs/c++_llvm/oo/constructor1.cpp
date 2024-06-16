class Test {
    int a;
public:
    float b;
    Test(int a) {
        this->a = a;
        this->b = 33;
    }

    int getA() {
        return a;
    }
};

int main() {
    Test *t = new Test(1);
    int a = t->getA();
    a = t->getA();
    float b = t->b;
    delete t;
    return a;
}
