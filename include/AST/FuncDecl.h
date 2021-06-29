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
#include <unordered_set>
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
    class CodeGenFunction;
    class CodeGenCall;
    class CodeGenVar;

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

        const TopDeclKind Kind;
        const TypeBase *Type;
        const llvm::StringRef Name;
        bool Constant;
        std::vector<VarRef *> VarRefs;

        // Calls to be resolved from NameSpace Calls
        std::vector<FuncCall *> Calls;
        FuncHeader *Header;
        BlockStmt *Body;
        CodeGenFunction *CodeGen = NULL;

    public:
        FuncDecl(ASTNode *Node, const SourceLocation &Loc, const TypeBase *RetType, const llvm::StringRef &Name);

        TopDeclKind getKind() const override;

        const TypeBase *getType() const;

        const llvm::StringRef &getName() const;

        bool isConstant() const;

        const FuncHeader *getHeader() const;

        BlockStmt *getBody();

        const std::vector<FuncCall *> &getCalls() const;

        const std::vector<VarRef *> &getVarRefs() const;

        CodeGenFunction *getCodeGen() const;

        void setCodeGen(CodeGenFunction *CGF);

        FuncParam *addParam(const SourceLocation &Loc, TypeBase *Type, const StringRef &Name);

        bool addCall(FuncCall *Call);

        void setVarArg(FuncParam* VarArg);

        bool Finalize();
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
        GroupExpr* Group;
        const TypeBase *Ty;

    public:
        ReturnStmt(const SourceLocation &Loc, BlockStmt *Block, class GroupExpr *Group);

        StmtKind getKind() const override;

        GroupExpr *getExpr() const;
    };

    class FuncCallArg {
        Expr *Value;
        TypeBase *Ty;

    public:

        FuncCallArg(Expr *Value, TypeBase *Ty);

        Expr *getValue() const;

        TypeBase *getType() const;
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
        const llvm::StringRef NameSpace;
        const llvm::StringRef Name;
        std::vector<FuncCallArg *> Args;
        FuncDecl *Func = NULL;
        CodeGenCall *CGC = NULL;

    public:
//        FuncCall(const SourceLocation &Loc, const llvm::StringRef &Name);
        FuncCall(const SourceLocation &Loc, const llvm::StringRef &NameSpace, const llvm::StringRef &Name);

        const SourceLocation &getLocation() const;

        const StringRef &getNameSpace() const;

        const StringRef &getName() const;

        const std::vector<FuncCallArg *> getArgs() const;

        FuncCallArg *addArg(Expr * Arg, TypeBase *Ty = NULL);

        FuncDecl *getDecl() const;

        void setDecl(FuncDecl *func);

        CodeGenCall *getCodeGen() const;

        void setCodeGen(CodeGenCall *CGC);

        static FuncCall *CreateCall(FuncDecl *FDecl);

    };

    class FuncCallHash : std::hash<FuncCall *> {
    public:
        // id is returned as hash function
        size_t operator()(const FuncCall *Call) const {
            const size_t &h1 = (std::hash<std::string>()(Call->getName().str()));
            const size_t &h2 = (std::hash<std::string>()(Call->getNameSpace().str()));
            return h1 ^ h2;

        }
    };

    struct FuncCallComp : std::equal_to<FuncCall *> {
    public:
        bool operator()(const FuncCall *C1, const FuncCall *C2) const {

            bool Result = C1->getName().equals(C2->getName()) &&
                    C1->getNameSpace().equals(C2->getNameSpace()) &&
                    C1->getArgs().size() == C2->getArgs().size();
            if (Result) {
                for (int i = 0; i < C1->getArgs().size(); i++) {
                    Result &= C1->getArgs()[i]->getType() == C2->getArgs()[i]->getType();
                }
            }
            return Result;
        }
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
