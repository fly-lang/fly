#include <cstdlib>
#include <cstring>

// Python-like array structure
struct PyArray {
    void* data;      // Pointer to data
    size_t size;     // Number of elements
    size_t capacity; // Allocated capacity
    size_t itemsize; // Size of each element
};

int main() {
    // Create array like: arr = [1, 2, 3] in Python
    PyArray arr;
    arr.size = 3;
    arr.capacity = 3;
    arr.itemsize = sizeof(int);
    arr.data = malloc(arr.capacity * arr.itemsize);

    // Set elements: arr[0] = 1, arr[1] = 2, arr[2] = 3
    int* data = (int*)arr.data;
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;

    // Access element: x = arr[1]
    int x = data[1];

    // Append element: arr.append(4)
    if (arr.size >= arr.capacity) {
        arr.capacity = arr.capacity * 2;
        arr.data = realloc(arr.data, arr.capacity * arr.itemsize);
        data = (int*)arr.data;
    }
    data[arr.size] = 4;
    arr.size++;

    // Free memory
    free(arr.data);

    return 0;
}
