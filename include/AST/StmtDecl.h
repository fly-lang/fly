//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Stmt.h - Statement declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_STMTDECL_H
#define FLY_STMTDECL_H

#include "FuncDecl.h"
#include "llvm/ADT/StringMap.h"
#include "vector"

namespace fly {

    class ReturnDecl;
    class FuncRefDecl;

    class StmtDecl : public Decl {

        friend class Parser;

        DeclKind Kind = DeclKind::D_STMT;

        std::vector<Decl*> Instructions;

        llvm::StringMap<VarDecl*> Vars;

        llvm::StringMap<FuncRefDecl *> Invokes;

        ReturnDecl* Return;

        bool addVar(VarRefDecl *Var);

        bool addVar(VarDecl *Var);

        bool addInvoke(FuncRefDecl *Invoke);

    public:

        explicit StmtDecl(const SourceLocation &Loc) : Decl(Loc) {

        }

        StmtDecl(const SourceLocation &Loc, llvm::StringMap<VarDecl*> &Map);

        StmtDecl(const SourceLocation &Loc, std::vector<VarDecl*> &Vector);

        DeclKind getKind() const override;

        const std::vector<Decl *> &getIstructions() const {
            return Instructions;
        }

        const llvm::StringMap<VarDecl *> &getVars() const {
            return Vars;
        }

        ReturnDecl *getReturn() const {
            return Return;
        }
    };

    class CondStmt : public StmtDecl {

        CondStmt(const SourceLocation &Loc) : StmtDecl(Loc) {

        }
    };

    class LoopStmt : public StmtDecl {

        LoopStmt(const SourceLocation &Loc) : StmtDecl(Loc) {

        }
    };
}


#endif //FLY_STMTDECL_H
