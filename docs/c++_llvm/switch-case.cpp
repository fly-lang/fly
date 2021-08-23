int main() {
    int a = 1;
    bool b = false;

    switch (a) {
        case 1:
            b = true;
        case 2:
            b = false;
            break;
        case 3:
            b = false;
            break;
    }
}
