//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenEnum.h - enum type code generation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_ENUM_H
#define FLY_CODEGEN_ENUM_H

#include "CodeGenType.h"
#include "llvm/ADT/StringMap.h"

namespace llvm {
    class Type;
    class GlobalVariable;
}

namespace fly {

    class CodeGenModule;
    class SemaEnumType;
    class CodeGenEnumEntry;

    /**
     * CodeGenEnum - Code generator for enum types.
     *
     * Enums are represented as i32 integers in LLVM IR.
     * Each enum entry is a constant integer value based on its index.
     */
    class CodeGenEnum : public CodeGenType {

        SemaEnumType *Sema = nullptr;

        llvm::StringMap<CodeGenEnumEntry *> Entries;

        // Lazily-built per-module table of entry name strings, indexed by the
        // entry value (1-based; slot 0 is the undefined/default entry). Used to
        // lower `.name` on an enum-typed variable. Private linkage — each module
        // that needs it gets its own copy, avoiding cross-module link concerns.
        llvm::GlobalVariable *NamesTable = nullptr;

    public:
        CodeGenEnum(CodeGenModule *CGM, SemaEnumType *Sema, bool isExternal = false);

        SemaEnumType *getSema() const;

        const llvm::StringMap<CodeGenEnumEntry *> &getEntries() const;

        // Build (or return the cached) names table for `.name` on a variable.
        llvm::GlobalVariable *getNamesTable();
    };

}

#endif //FLY_CODEGEN_ENUM_H

