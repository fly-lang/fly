//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGenValue.h - Code Generator of Value
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_CODEGEN_VALUE_H
#define FLY_CODEGEN_VALUE_H

#include "CodeGenExpr.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Instructions.h>

namespace llvm {
    class Type;
    class ConstantInt;
}

namespace fly {

    class CodeGenModule;
    class SemaVar;
    class SemaEnumList;

    class CodeGenArrayValue : public CodeGenExpr {

    	// Array value information (for storing array elements)
    	std::vector<llvm::Value *> Values;

    	llvm::Type *ElementType = nullptr;

    public:

    	CodeGenArrayValue(CodeGenModule *CGM);

    	std::vector<llvm::Value *> getValues() const;

    	llvm::Type *getElementType() const;

    	void GenExpr(SemaArrayValue *Sema);

    	void GenExpr(SemaEnumList *Sema);

    };
}

#endif // FLY_CODEGEN_VALUE_H
