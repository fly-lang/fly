//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunction.h - AST Function Base header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_FUNCTIONBASE_H
#define FLY_AST_FUNCTIONBASE_H

#include "ASTBase.h"

namespace fly {

    class ASTScope;
    class ASTVar;
    class ASTComment;
    class ASTTypeRef;
class ASTBlockStmt;
class SymFunctionBase;

    class ASTFunction : public ASTBase {

        friend class ASTBuilder;
        friend class SymBuilder;
        friend class SemaResolver;
        friend class ParserFunction;
        friend class ParserClass;

        llvm::StringRef Name;

        // Function return type
        ASTTypeRef *ReturnTypeRef = nullptr;

        llvm::SmallVector<ASTScope *, 8> Scopes;

        llvm::SmallVector<ASTVar *, 8> Params;

        llvm::SmallVector<ASTVar *, 8> LocalVars;

        // Body is the main BlockStmt
        ASTBlockStmt *Body = nullptr;

        ASTVar *ErrorHandler = nullptr;

    protected:

        ASTFunction(const SourceLocation &Loc, ASTTypeRef *ReturnType,llvm::SmallVector<ASTScope *, 8> &Scopes,
            llvm::StringRef Name, llvm::SmallVector<ASTVar *, 8> &Params);

    public:

        llvm::StringRef getName() const;

        ASTTypeRef *getReturnTypeRef() const;

        llvm::SmallVector<ASTScope *, 8> getScopes() const;

        llvm::SmallVector<ASTVar *, 8> getParams() const;

        llvm::SmallVector<ASTVar *, 8> getLocalVars() const;

        ASTBlockStmt *getBody() const;

        ASTVar *getErrorHandler();

        bool isVarArg();

        std::string str() const override;
    };
}

#endif //FLY_AST_FUNCTIONBASE_H
