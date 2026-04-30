//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenEnum.cpp - Code Generator Enum
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenEnum.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include "Sema/SemaEnumType.h"
#include "Sema/SemaEnumEntry.h"

using namespace fly;

CodeGenEnum::CodeGenEnum(CodeGenModule *CGM, SemaEnumType *Sema, bool isExternal)
        : CodeGenType(CGM), Sema(Sema) {
    // Enums are represented as i32 integers
    T = CodeGen::Int32Ty;

    // Generate CodeGen for each enum entry
    for (auto &Entry : Sema->getEntries()) {
        SemaEnumEntry *SemaEntry = Entry.getValue();
        CodeGenEnumEntry *CGE = new CodeGenEnumEntry(CGM, SemaEntry);
        SemaEntry->setCodeGen(CGE);
        Entries.insert(std::make_pair(Entry.getKey(), CGE));
    }
}

SemaEnumType *CodeGenEnum::getSema() const {
    return Sema;
}

const llvm::StringMap<CodeGenEnumEntry *> &CodeGenEnum::getEntries() const {
    return Entries;
}


