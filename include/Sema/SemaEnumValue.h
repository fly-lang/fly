//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaEnumValue.h - SemaClassType
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ENUM_ENTRY_H
#define FLY_SEMA_ENUM_ENTRY_H

#include "CodeGen/CodeGenEnumValue.h"
#include "SemaValue.h"

namespace fly {

    class ASTVar;
    class SemaComment;
	class CodeGenEnumValue;

    class SemaEnumValue : public SemaValue {

        friend class SemaBuilder;
        friend class Resolver;
        friend class SemaValidator;

        size_t Index;

        SemaComment *Comment = nullptr;

        explicit SemaEnumValue(ASTEnumValue &AST, SemaEnumType *Type);

    public:

        ~SemaEnumValue() override;

        size_t getIndex() const;

        SemaComment *getComment() const;

    	CodeGenEnumValue *getCodeGen() const override;

    	void setCodeGen(CodeGenExpr *CGC) override;

        void accept(SemaVisitor& Visitor) override;
    };

}  // end namespace fly

#endif // FLY_SSEMA_ENUM_ENTRY_H