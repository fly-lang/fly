class Base {
public:
	int c;
	int d;
	virtual void f() {}
};

class Base2 {
public:
	int c;
	int d;
};

class Test : Base, Base2 {
public:
    int e;
    void f() override {}
};

int main() {
    Test *t = new Test();
    delete t;
    return 1;
}