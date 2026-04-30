#include <malloc.h>
#include "stdio.h"

struct Test {
    int a;
    int b;
};

int mainFunc() {
    // Allocate memory for one instance of struct Test
    struct Test *ptr = (struct Test *)malloc(sizeof(struct Test));

    // Initialize the struct members
    ptr->a = 10;
    ptr->b = 20;

    // Free the allocated memory
    free(ptr);

    return 0;
}