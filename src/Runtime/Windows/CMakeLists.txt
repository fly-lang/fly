##===-- src/Runtime/Windows/CMakeLists.txt --------------------------------===##

target_sources(Runtime PRIVATE Mem.c Io.c Proc.c Thread.c)

if(MSVC)
    target_compile_options(Runtime PRIVATE /GS-)
else()
    target_compile_options(Runtime PRIVATE -fno-stack-protector -fno-builtin)
endif()
