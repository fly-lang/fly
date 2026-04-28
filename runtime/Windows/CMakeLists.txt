##===-- runtime/Windows/CMakeLists.txt --------------------------------===##

target_sources(FlyRuntime PRIVATE Mem.c Io.c Proc.c Thread.c)

if(MSVC)
    target_compile_options(FlyRuntime PRIVATE /GS-)
else()
    target_compile_options(FlyRuntime PRIVATE -fno-stack-protector -fno-builtin)
endif()
