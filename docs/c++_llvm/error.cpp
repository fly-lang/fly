struct Error {
    unsigned long number;
    char *message;
    void *details;
};

void test(Error &error) {
    error.number = 1;
    error.message = "hello";
}

int main() {
    Error err = Error();
    test(err);
    return err.number != 0;
}