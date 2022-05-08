//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFunc.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FUNCTION_H
#define FLY_FUNCTION_H

#include "ASTTopDecl.h"
#include "ASTVar.h"
#include "ASTLocalVar.h"
#include "ASTStmt.h"
#include "llvm/ADT/StringMap.h"
#include <unordered_set>
#include <vector>

namespace fly {

    class ASTGroupExpr;
    class ASTParams;
    class ASTExpr;
    class ASTType;
    class ASTVarRef;
    class ASTBlock;
    class ASTFunctionCall;
    class ASTGlobalVar;
    class CodeGenFunction;
    class CodeGenVar;
    class CodeGenLocalVar;
    class CodeGenCall;

    /**
     * The Function Declaration and definition
     * Ex.
     *   int func() {
     *     return 1
     *   }
     */
    class ASTFunction : public ASTTopDecl {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class FunctionParser;

        // Function return type
        ASTType *ReturnType;

        // Function Name
        const std::string Name;

        // Header contains parameters
        ASTParams *Params;

        // Body is the main BlockStmt
        ASTBlock *Body;

        // Contains all vars declared in this Block
        std::vector<ASTLocalVar *> LocalVars;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

    public:
        ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, const std::string &Name,
                    VisibilityKind Visibility);

        ASTType *getType() const;

        const std::string &getName() const;

        const ASTParams *getParams() const;

        const ASTBlock *getBody() const;

        const std::vector<ASTLocalVar *> &getLocalVars() const;

        CodeGenFunction *getCodeGen() const;

        void setCodeGen(CodeGenFunction *CGF);

        bool isVarArg();

        std::string str() const;

        bool operator==(const ASTFunction &F) const;
    };

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ASTReturn : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_RETURN;
        ASTExpr *Expr;

    public:
        ASTReturn(const SourceLocation &Loc, ASTExpr *Expr);

        StmtKind getKind() const override;

        ASTExpr *getExpr() const;

        std::string str() const override;
    };
}

namespace std {
    using namespace fly;

    template <>
    struct hash<ASTFunction *> {
        // id is returned as hash function
        size_t operator()(ASTFunction *F) const noexcept;
    };

    template <>
    struct equal_to<ASTFunction *> {
        bool operator()(const ASTFunction *F1, const ASTFunction *F2) const;
    };
}


#endif //FLY_FUNCTION_H
