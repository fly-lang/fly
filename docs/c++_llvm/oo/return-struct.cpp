struct Test {
    int a;
};

Test *get() {
    return new Test;
}

int main() {
    Test *t = get();
    return 0;
}
