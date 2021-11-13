# Documentation
You can find here the Project Documentation.

### Wiki
This is the main repository of all project docs.
You can also view documentation from [Github Wiki](https://github.com/fly-lang/fly/wiki) webpages.

If you cannot see the wiki directory (it is configured as github submodule):

`git pull --recurse-submodules`

### C++/LLVM
Contains some example of C++ code compiled in LLVM IR, useful for better understand code generators into CodeGen.
You can use following commands to obtain different outputs.

#### Compile .c into .ll
clang -emit-llvm -S file.c

#### Compile .cpp/.c into .ll
clang++ -emit-llvm -S file.cpp

#### Compile .c into .bc
clang -emit-llvm -c file.c

#### Compile .cpp/.c into .bc
clang++ -emit-llvm -c file.cpp

#### Transform .ll into .bc
llvm-as file.ll -o file.bc

#### Transform .bc into .ll
llvm-dis file.bc -o file.ll

#### Transform .bc to .o
llc file.bc -filetype=obj -o out.o

#### Convert to assembly
llc file.bc –o file.s

#### Link all files
ld.lld file1.o file2.o -o out
ld.lld file1.o file2.o -relocation-model=pic -o out
