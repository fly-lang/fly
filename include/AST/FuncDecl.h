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
#include "VarDecl.h"
#include "Stmt.h"
#include "llvm/ADT/StringMap.h"
#include <vector>

namespace fly {

    class GroupExpr;
    class Expr;
    class TypeBase;
    class VarRef;
    class FuncHeader;
    class BlockStmt;
    class FuncCall;
    class FuncParam;
    class GlobalVarDecl;
    class CodeGenFunction;
    class CodeGenVar;
    class CodeGenCall;

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

        // Kind of TopDecl identified by enum
        const TopDeclKind Kind;

        // Function return type
        TypeBase *Type;

        // Function Name
        const llvm::StringRef Name;

        // this function return a constant value
        bool Constant;

        // Header contains parameters
        FuncHeader *Header;

        // Body is the main BlockStmt
        BlockStmt *Body;

        // Contains all Calls to be resolved with Function
        std::vector<FuncCall *> UnRefCalls;

        // Contains all VarRef to be resolved with GlobalVar
        std::vector<VarRef *> UnRefGlobalVars;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = NULL;

    public:
        FuncDecl(ASTNode *Node, const SourceLocation &Loc, TypeBase *RetType, const llvm::StringRef &Name);

        TopDeclKind getKind() const override;

        TypeBase *getType() const;

        const llvm::StringRef &getName() const;

        bool isConstant() const;

        const FuncHeader *getHeader() const;

        BlockStmt *getBody();

        const std::vector<FuncCall *> &getUnRefCalls() const;

        CodeGenFunction *getCodeGen() const;

        void setCodeGen(CodeGenFunction *CGF);

        FuncParam *addParam(const SourceLocation &Loc, TypeBase *Type, const StringRef &Name);

        bool isVarArg();

        void setVarArg(FuncParam* VarArg);

        bool addUnRefCall(FuncCall *Call);

        void addUnRefGlobalVar(VarRef *Var);

        bool ResolveCall(FuncCall *ResolvedCall, FuncCall *Call);

        bool Finalize();
    };

    class FuncDeclHash : std::hash<FuncDecl *> {
    public:
        // id is returned as hash function
        size_t operator()(const FuncDecl *Decl) const;
    };

    struct FuncDeclComp : std::equal_to<FuncDecl *> {
    public:
        bool operator()(const FuncDecl *C1, const FuncDecl *C2) const;
    };

    /**
     * Function Parameter
     */
    class FuncParam : public VarDecl {

        const SourceLocation Location;

        CodeGenVar *CodeGen;

    public:
        FuncParam(const SourceLocation &Loc, TypeBase *Type, const llvm::StringRef &Name);

        CodeGenVar *getCodeGen() const;

        void setCodeGen(CodeGenVar *CG);
    };

    /**
     * All Parameters of a Function for Definition
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class FuncHeader {

        friend class FunctionParser;
        friend class FuncDecl;

        std::vector<FuncParam *> Params;
        FuncParam* VarArg = NULL;

    public:
        const std::vector<FuncParam *> &getParams() const;

        const FuncParam* getVarArg() const;
    };

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ReturnStmt: public Stmt {

        StmtKind Kind = StmtKind::STMT_RETURN;
        Expr* Exp;
        const TypeBase *Ty;

    public:
        ReturnStmt(const SourceLocation &Loc, BlockStmt *Block, Expr *Exp);

        StmtKind getKind() const override;

        Expr *getExpr() const;
    };

    class FuncArg {
        Expr *Value;
        TypeBase *Ty;

    public:

        FuncArg(Expr *Value, TypeBase *Ty);

        Expr *getValue() const;

        TypeBase *getType() const;

        void setType(TypeBase *T);
    };

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class FuncCall {

        friend class Parser;
        friend class FunctionParser;

        const SourceLocation Loc;
        llvm::StringRef NameSpace;
        const llvm::StringRef Name;
        std::vector<FuncArg *> Args;
        FuncDecl *Decl = NULL;
        CodeGenCall *CGC = NULL;

    public:
        FuncCall(const SourceLocation &Loc, const llvm::StringRef &NameSpace, const llvm::StringRef &Name);

        const SourceLocation &getLocation() const;

        const StringRef &getNameSpace() const;

        void setNameSpace(const llvm::StringRef &NameSpace);

        const StringRef &getName() const;

        const std::vector<FuncArg *> getArgs() const;

        FuncArg *addArg(FuncArg *Arg);

        FuncDecl *getDecl() const;

        void setDecl(FuncDecl *FDecl);

        CodeGenCall *getCodeGen() const;

        void setCodeGen(CodeGenCall *CGC);

        static FuncCall *CreateCall(FuncDecl *FDecl);

    };

    class FuncCallHash : std::hash<FuncCall *> {
    public:
        // id is returned as hash function
        size_t operator()(const FuncCall *Call) const;
    };

    struct FuncCallComp : std::equal_to<FuncCall *> {
    public:
        bool operator()(const FuncCall *C1, const FuncCall *C2) const;
    };

    /**
     * Declaration of Reference to a Function
     * Ex.
     *  func()
     */
    class FuncCallStmt : public Stmt {

        FuncCall *Call;

    public:
        FuncCallStmt(const SourceLocation &Loc, BlockStmt *Block, FuncCall *Call);

        StmtKind getKind() const override;

        FuncCall *getCall() const;
    };
}

#endif //FLY_FUNCTION_H
