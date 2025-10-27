//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaEnumEntry.h - SemaClassType
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ENUM_ENTRY_H
#define FLY_SEMA_ENUM_ENTRY_H

#include "SemaVar.h"
#include "CodeGen/CodeGenEnumEntry.h"
#include <cstddef>

namespace fly {

    class ASTVar;
    class SemaComment;
	class CodeGenEnumEntry;

    class SemaEnumEntry : public SemaVar {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        size_t Index;

		CodeGenEnumEntry *CodeGen;

        SemaComment *Comment;

        explicit SemaEnumEntry(ASTVar *AST);

    public:

        size_t getIndex() const;

        SemaComment *getComment() const;

    	CodeGenEnumEntry *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase * CGC) override;
    };

}  // end namespace fly

#endif // FLY_SSEMA_ENUM_ENTRY_H