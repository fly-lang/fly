##===-- runtime/macOS/CMakeLists.txt ----------------------------------===##

target_sources(FlyRuntime PRIVATE Mem.c Io.c Proc.c Thread.c Fs.c Time.c Env.c)

target_compile_options(FlyRuntime PRIVATE
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
)
