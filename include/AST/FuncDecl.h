//===--------------------------------------------------------------------------------------------------------------===//
// include/AST/FunctionDecl.h - Function declaration
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_FUNCTION_H
#define FLY_FUNCTION_H

#include "VarDecl.h"
#include "ClassDecl.h"
#include "Refer.h"
#include "Expr.h"
#include "TypeBase.h"
#include "StmtDecl.h"
#include "llvm/ADT/StringMap.h"
#include "vector"

namespace fly {

    class ParamsFunc;
    class StmtDecl;
    class FuncRef;

    /**
     * The Function Declaration and definition
     * Ex.
     *   int func() {
     *     return 1
     *   }
     */
    class FuncDecl : public Decl, public TopDecl {

        friend class ASTNode;
        friend class Parser;
        friend class FunctionParser;

        DeclKind Kind = DeclKind::D_FUNCTION;
        const TypeBase *Type;
        const StringRef Name;
        bool Constant;
//        llvm::StringMap<VarRef *> VarRef;
//        llvm::StringMap<FunctionRef *> FuncRef;
//        llvm::StringMap<ClassRef *> ClassRef;
        ParamsFunc *Params = NULL;
        StmtDecl *Body = NULL;

    public:
        FuncDecl(const SourceLocation &Loc, const TypeBase *Type, const StringRef &Name);

        DeclKind getKind() const override;

        const TypeBase *getType() const;

        const StringRef &getName() const;

        bool isConstant() const;

        const ParamsFunc *getParams() const;

        const StmtDecl *getBody() const;

        const llvm::StringMap<VarRef *> &getVarRef() const;

//        const llvm::StringMap<FunctionRef *> &getFuncRef() const;

//        const llvm::StringMap<ClassRef *> &getClassRef() const;

    };

    /**
     * All Parameters of a Function for Definition
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class ParamsFunc {

        friend class FunctionParser;

        std::vector<VarDecl*> Vars;
        VarDecl* VarArg;

    public:
        const std::vector<VarDecl *> &getVars() const;

        const VarDecl* getVarArg() const;
    };

    /**
     * All Parameters of a Function
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class ParamsFuncRef {

        friend class FunctionParser;

        std::vector<Expr*> Args;
        VarRef* VarArg;

    public:
        const std::vector<Expr *> &getArgs() const;

        const VarRef* getVarArg() const;
    };

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ReturnDecl: public Decl {

        DeclKind Kind = DeclKind::D_RETURN;
        GroupExpr* Group;

    public:
        ReturnDecl(SourceLocation &Loc, class GroupExpr *Group);

        DeclKind getKind() const override;

        GroupExpr *getExpr() const;
    };

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class FuncRef : public Refer {

        friend class Parser;
        friend class FunctionParser;

        const StringRef Name;
        ParamsFuncRef *Params = NULL;
        FuncDecl *D = NULL;

    public:
        FuncRef(const SourceLocation &Loc, const StringRef &Name);
        FuncRef(const SourceLocation &Loc, FuncDecl *Decl);

        const StringRef &getName() const;

        const ParamsFuncRef *getParams() const;

        FuncDecl *getDecl() const override;
    };

    /**
     * Declaration of Reference to a Function
     * Ex.
     *  func()
     */
    class FuncRefDecl : public FuncRef, public Decl {

    public:
        FuncRefDecl(const SourceLocation &Loc, const StringRef &Name);
        FuncRefDecl(const SourceLocation &Loc, FuncDecl *Decl);

        DeclKind getKind() const override;
    };
}

#endif //FLY_FUNCTION_H
