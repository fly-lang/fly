//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunc.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_ASTPARAMS_H
#define FLY_ASTPARAMS_H

#include "ASTLocalVar.h"

namespace fly {

    class ASTFunctionBase;
    class ASTValue;

    /**
     * Function Parameter
     */
    class ASTParam : public ASTLocalVar {

        friend class SemaBuilder;
        friend class SemaResolver;

        ASTFunctionBase *Function; // Need this property to access directly to ASTFunction because Parent is always null

        // LocalVar Code Generator
        CodeGenVarBase *CodeGen = nullptr;

        bool Constant = false;

        ASTVarKind VarKind;

        ASTType *Type = nullptr;

        llvm::StringRef Name;

        ASTValue *DefaultValue;

        ASTParam(ASTFunctionBase *Function, const SourceLocation &Loc, ASTType *Type, llvm::StringRef Name, bool Constant);

    public:

        ASTFunctionBase *getFunction();

        ASTVarKind getVarKind() override;

        ASTType *getType() const override;

        llvm::StringRef getName() const override;

        ASTValue *getDefaultValue() const;

        void setDefaultValue(ASTValue *Value);

        CodeGenVarBase *getCodeGen() const override;

        void setCodeGen(CodeGenVarBase *CG);

        std::string print() const override;

        std::string str() const override;
    };

    /**
     * All Parameters of a Function for Definition
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class ASTParams : public Debuggable {

        friend class ASTFunctionBase;

        std::vector<ASTParam *> List;
        ASTParam* Ellipsis = nullptr;

    public:
        uint64_t getSize() const;

        ASTParam *at(unsigned long Index) const;

        const bool isEmpty() const;

        const std::vector<ASTParam *> &getList() const;

        const ASTParam* getEllipsis() const;

        std::string str() const override;
    };
}

#endif //FLY_ASTPARAMS_H
