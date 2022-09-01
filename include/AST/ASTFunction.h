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

#include "ASTTopDef.h"
#include "ASTVar.h"
#include "ASTLocalVar.h"
#include "ASTExprStmt.h"
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
    class ASTFunction : public ASTTopDef {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class FunctionParser;

        // Function return type
        ASTType *Type = nullptr;

        // Function Name
        const std::string Name;

        // Header contains parameters
        ASTParams *Params = nullptr;

        // Body is the main BlockStmt
        ASTBlock *Body = nullptr;

        // Contains all vars declared in this Block
        std::vector<ASTLocalVar *> LocalVars;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

        ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, const std::string Name,
                    VisibilityKind Visibility);

    public:

        ASTType *getType() const;

        const std::string getName() const;

        const ASTParams *getParams() const;

        const ASTBlock *getBody() const;

        const std::vector<ASTLocalVar *> &getLocalVars() const;

        CodeGenFunction *getCodeGen() const;

        void setCodeGen(CodeGenFunction *CGF);

        bool isVarArg();

        std::string str() const;
    };

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ASTReturn : public ASTExprStmt {

        friend class SemaBuilder;

        StmtKind Kind = StmtKind::STMT_RETURN;

        ASTReturn(const SourceLocation &Loc);

    public:

        StmtKind getKind() const override;

        std::string str() const override;
    };
}

#endif //FLY_FUNCTION_H
