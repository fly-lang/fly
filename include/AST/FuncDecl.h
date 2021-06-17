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

#include "TopDecl.h"
#include "ClassDecl.h"
#include "Expr.h"
#include "TypeBase.h"
#include "BlockStmt.h"
#include "llvm/ADT/StringMap.h"
#include <vector>

namespace fly {

    class ParamsFuncDecl;
    class BlockStmt;
    class FuncCall;
    class CodeGenFunction;

    /**
     * The Function Declaration and definition
     * Ex.
     *   int func() {
     *     return 1
     *   }
     */
    class FuncDecl : public TopDecl {

        friend class ASTNode;
        friend class Parser;
        friend class FunctionParser;
        friend class CodeGenTest;

        TopDeclKind Kind = TopDeclKind::DECL_FUNCTION;
        const TypeBase *Type;
        const llvm::StringRef Name;
        bool Constant;
//        llvm::StringMap<VarRef *> VarRef;
//        llvm::StringMap<FunctionRef *> FuncRef;
//        llvm::StringMap<ClassRef *> ClassRef;
        ParamsFuncDecl *Header;
        BlockStmt *Body;
        CodeGenFunction *CodeGen = NULL;

    public:
        FuncDecl(const SourceLocation &Loc, const TypeBase *RetType, const llvm::StringRef &Name);

        TopDeclKind getKind() const override;

        const TypeBase *getType() const;

        const llvm::StringRef &getName() const;

        bool isConstant() const;

        const ParamsFuncDecl *getHeader() const;

        BlockStmt *getBody();

//        const llvm::StringMap<FunctionRef *> &getFuncRef() const;

//        const llvm::StringMap<ClassRef *> &getClassRef() const;

        CodeGenFunction *getCodeGen() const;

        void setCodeGen(CodeGenFunction *codeGen);

        VarDeclStmt *addParam(const SourceLocation &Loc, TypeBase *Type, const StringRef &Name);

        void setVarArg(VarDeclStmt* VarArg);

    };

    /**
     * All Parameters of a Function for Definition
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class ParamsFuncDecl {

        friend class FunctionParser;
        friend class FuncDecl;

        std::vector<VarDeclStmt *> Params;
        VarDeclStmt* VarArg = NULL;

    public:
        const std::vector<VarDeclStmt *> &getParams() const;

        const VarDeclStmt* getVarArg() const;
    };

    /**
     * All Parameters of a Function
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class ParamsFuncCall {

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
    class ReturnStmt: public Stmt {

        StmtKind Kind = StmtKind::STMT_RETURN;
        GroupExpr* Group;
        const TypeBase *Ty;

    public:
        ReturnStmt(const SourceLocation &Loc, BlockStmt *CurrentStmt, class GroupExpr *Group);

        StmtKind getKind() const override;

        GroupExpr *getExpr() const;
    };

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class FuncCall {

        friend class Parser;
        friend class FunctionParser;

        const llvm::StringRef Name;
        ParamsFuncCall *Params = NULL;
        FuncDecl *Func = NULL;

    public:
        FuncCall(const SourceLocation &Loc, const llvm::StringRef &Name);
        FuncCall(const SourceLocation &Loc, FuncDecl *Decl);

        const StringRef &getName() const;

        const ParamsFuncCall *getParams() const;

        FuncDecl *getDecl() const;
    };

    /**
     * Declaration of Reference to a Function
     * Ex.
     *  func()
     */
    class FuncCallStmt : public FuncCall, public Stmt {

    public:
        FuncCallStmt(const SourceLocation &Loc, BlockStmt *CurrentStmt, const llvm::StringRef &Name);
        FuncCallStmt(const SourceLocation &Loc, BlockStmt *CurrentStmt, FuncDecl *Decl);

        StmtKind getKind() const override;
    };
}

#endif //FLY_FUNCTION_H
