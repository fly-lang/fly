##===-- src/Runtime/macOS/CMakeLists.txt ----------------------------------===##

target_sources(Runtime PRIVATE Mem.c Io.c Proc.c Thread.c)

target_compile_options(Runtime PRIVATE
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
)
