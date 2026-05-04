##===-- runtime/Linux/CMakeLists.txt ----------------------------------===##

target_sources(FlyRuntime PRIVATE Mem.c Io.c Proc.c Thread.c Unwind.c)

target_compile_options(FlyRuntime PRIVATE
    -ffreestanding
    -nostdinc
    -fno-stack-protector
    -fno-builtin
)
