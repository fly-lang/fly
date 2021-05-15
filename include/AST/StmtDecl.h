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

    enum StmtKind {
        D_STMT_NORMAL,
        D_STMT_IF,
        D_STMT_ELSIF,
        D_STMT_ELSE,
        D_STMT_SWITCH,
        D_STMT_CASE,
        D_STMT_DEFAULT,
        D_STMT_FOR
    };

    class StmtDecl : public Decl {

        friend class Parser;
        friend class FunctionParser;
        friend class IfStmtDecl;
        friend class ElsifStmtDecl;
        friend class ElseStmtDecl;

        DeclKind Kind = DeclKind::D_STMT;
        enum StmtKind StmtKind = StmtKind::D_STMT_NORMAL;

        const StmtDecl *Parent;

        std::vector<Decl*> Content;

        llvm::StringMap<VarDecl*> Vars;

        llvm::StringMap<FuncRefDecl *> Invokes;

        ReturnDecl* Return;

        bool addVar(VarRefDecl *Var);

        bool addVar(VarDecl *Var);

        bool addInvoke(FuncRefDecl *Invoke);

    public:

        StmtDecl(const SourceLocation &Loc, StmtDecl *Parent);

        DeclKind getKind() const override;

        virtual enum StmtKind getStmtKind() const {
            return StmtKind;
        };

        const std::vector<Decl *> &getContent() const;

        bool isEmpty() const;

        const llvm::StringMap<VarDecl *> &getVars() const;

        ReturnDecl *getReturn() const;
    };

    class CondStmtDecl : public StmtDecl {

    public:
        CondStmtDecl(const SourceLocation &Loc, StmtDecl *Parent);
    };

    class LoopStmtDecl : public StmtDecl {

    public:
        LoopStmtDecl(const SourceLocation &Loc, StmtDecl *Parent);
    };

    class BreakDecl : public Decl {

        DeclKind Kind = DeclKind::D_BREAK;

    public:
        BreakDecl(const SourceLocation &Loc);

        DeclKind getKind() const override;
    };

    class ContinueDecl : public Decl {

        DeclKind Kind = DeclKind::D_CONTINUE;

    public:
        ContinueDecl(const SourceLocation &Loc);

        DeclKind getKind() const override;
    };
}


#endif //FLY_STMTDECL_H
