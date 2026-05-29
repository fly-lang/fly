//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaEnumEntry.h - enum entry semantic analysis
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ENUM_ENTRY_H
#define FLY_SEMA_ENUM_ENTRY_H

#include "CodeGen/CodeGenEnumEntry.h"
#include "SemaExpr.h"

namespace fly {

    class ASTVar;
    class SemaComment;
	class CodeGenEnumEntry;
	class ASTEnumEntry;

    class SemaEnumEntry : public SemaExpr {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

    	ASTEnumEntry &AST;

        size_t Index;

        SemaComment *Comment = nullptr;

    	CodeGenEnumEntry *CodeGen = nullptr;

        explicit SemaEnumEntry(ASTEnumEntry &AST, SemaEnumType *Type);

    public:

        ~SemaEnumEntry() override;

        size_t getIndex() const;

        SemaComment *getComment() const;

    	CodeGenEnumEntry *getCodeGen() const override;

    	void setCodeGen(CodeGenEnumEntry *CodeGen);

    	std::string str() const override;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SEMA_ENUM_ENTRY_H
