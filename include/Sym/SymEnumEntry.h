//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_ENUM_ENTRY_H
#define FLY_SYM_ENUM_ENTRY_H

#include <cstdint>

#include "SymVar.h"
#include "CodeGen/CodeGenEnumEntry.h"

namespace fly {

    class ASTVar;
    class SymComment;
	class CodeGenEnumEntry;

    class SymEnumEntry : SymVar {

        friend class SymBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        uint32_t Index;

		CodeGenEnumEntry *CodeGen;

        SymComment *Comment;

        explicit SymEnumEntry(ASTVar *AST);

    public:

        uint32_t getIndex() const;

        SymComment *getComment() const;

    	CodeGenEnumEntry *getCodeGen() const override;

        void setCodeGen(CodeGenEnumEntry * CodeGen);
    };

}  // end namespace fly

#endif // FLY_SYM_ENUM_ENTRY_H