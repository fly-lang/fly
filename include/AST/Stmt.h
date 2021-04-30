//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/Stmt.h - Statement declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#ifndef FLY_STMT_H
#define FLY_STMT_H

#include "FunctionDecl.h"
#include "llvm/ADT/StringMap.h"
#include "vector"

namespace fly {

    class ReturnDecl;

    class Stmt : public DeclBase {

        friend class Parser;

        DeclKind Kind = DeclKind::D_STMT;

        std::vector<DeclBase*> Instructions;

        llvm::StringMap<VarDecl*> Vars;

        ReturnDecl* Return;

    public:
        explicit Stmt(const SourceLocation &Loc) : DeclBase(Loc) {

        }

        Stmt(const SourceLocation &Loc, llvm::StringMap<VarDecl*> &Map) : DeclBase(Loc) {
            for (auto &Entry : Map) {
                Vars.insert(std::pair<StringRef, VarDecl*>(Entry.getKey(), Entry.getValue()));
            }
        }

        Stmt(const SourceLocation &Loc, std::vector<VarDecl*> &Vector) : DeclBase(Loc) {
            if (!Vector.empty()) {
                for (VarDecl *Var : Vector) {
                    Vars.insert(std::pair<StringRef, VarDecl *>(Var->getName(), Var));
                }
            }
        }

        DeclKind getKind() {
            return Kind;
        }

        const std::vector<DeclBase *> &getIstructions() const {
            return Instructions;
        }

        const llvm::StringMap<VarDecl *> &getVars() const {
            return Vars;
        }

        ReturnDecl *getReturn() const {
            return Return;
        }
    };

    class CondStmt : public Stmt {

        CondStmt(const SourceLocation &Loc) : Stmt(Loc) {

        }
    };

    class LoopStmt : public Stmt {

        LoopStmt(const SourceLocation &Loc) : Stmt(Loc) {

        }
    };
}


#endif //FLY_STMT_H
