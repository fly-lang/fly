void ref(int &a) {
    a = 2;
}

int main() {
    int a = 1;
    ref(a);
    return 0;
}
