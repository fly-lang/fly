struct Error {
    unsigned char type = 0;
    int number = 0;
    void *details = nullptr;
};

void testInt(Error &error) {
    error.type = 1;
    error.number = 1;
}

void testString(Error &error) {
    error.type = 2;
    char *message = "hello";
    error.details = message;
}

int main() {
    Error err = Error();
    testInt(err);
    testString(err);
    return err.number != 0;
}