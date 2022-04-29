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
    class ASTExpr;
    class ASTType;
    class ASTVarRef;
    class ASTFuncHeader;
    class ASTBlock;
    class ASTFuncCall;
    class ASTFuncParam;
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
    class ASTFunc : public ASTTopDecl {

        friend class ASTNode;
        friend class ASTNameSpace;
        friend class Parser;
        friend class FunctionParser;
        friend class CodeGenTest;

        // Function return type
        ASTType *ReturnType;

        // Function Name
        const std::string Name;

        // this function return a constant value
        bool Constant;

        // Header contains parameters
        ASTFuncHeader *Header;

        // Body is the main BlockStmt
        ASTBlock *Body;

        // Contains all vars declared in this Block
        std::vector<ASTLocalVar *> LocalVars;

        // Populated during codegen phase
        CodeGenFunction *CodeGen = nullptr;

    public:
        ASTFunc(ASTNode *Node, const SourceLocation &Loc, ASTType *ReturnType, const std::string Name);

        ASTType *getType() const;

        const std::string &getName() const;

        bool isConstant() const;

        const ASTFuncHeader *getHeader() const;

        ASTBlock *getBody();

        const std::vector<ASTLocalVar *> &getLocalVars() const;

        void addLocalVar(ASTLocalVar *LocalVar);

        CodeGenFunction *getCodeGen() const;

        void setCodeGen(CodeGenFunction *CGF);

        ASTFuncParam *addParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name);

        bool isVarArg();

        void setVarArg(ASTFuncParam* VarArg);

        std::string str() const;

        bool operator==(const ASTFunc& F) const;
    };

    /**
     * Function Parameter
     */
    class ASTFuncParam : public ASTVar {

        const SourceLocation Location;

        ASTValueExpr *Expr = nullptr;

        CodeGenLocalVar *CodeGen;

    public:
        ASTFuncParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name);

        ASTExpr *getExpr() const override;

        void setExpr(ASTExpr *E) override;

        CodeGenLocalVar *getCodeGen() const override;

        void setCodeGen(CodeGenLocalVar *CG);

        std::string str() const override;
    };

    /**
     * All Parameters of a Function for Definition
     * Ex.
     *   func(int param1, float param2, bool param3, ...)
     */
    class ASTFuncHeader {

        friend class FunctionParser;
        friend class ASTFunc;

        std::vector<ASTFuncParam *> Params;
        ASTFuncParam* VarArg = nullptr;

    public:
        const std::vector<ASTFuncParam *> &getParams() const;

        const ASTFuncParam* getVarArg() const;
    };

    /**
     * The Return Declaration into a Function
     * Ex.
     *   return true
     */
    class ASTReturn : public ASTStmt {

        StmtKind Kind = StmtKind::STMT_RETURN;
        ASTExpr* Expr;

    public:
        ASTReturn(const SourceLocation &Loc, ASTBlock *Block, ASTExpr *Expr);

        StmtKind getKind() const override;

        ASTExpr *getExpr() const;

        std::string str() const override;
    };

    class ASTCallArg {
        ASTExpr *Value;
        ASTType *Type;

    public:

        ASTCallArg(ASTExpr *Value, ASTType *Type);

        ASTExpr *getValue() const;

        ASTType *getType() const;

        void setType(ASTType *T);

        std::string str() const;
    };

    /**
     * A Reference to a Function in a Declaration
     * Ex.
     *  int a = sqrt(4)
     */
    class ASTFuncCall {

        friend class Parser;
        friend class FunctionParser;

        const SourceLocation Loc;
        std::string NameSpace;
        const std::string Name;
        std::vector<ASTCallArg *> Args;
        ASTFunc *Decl = nullptr;
        CodeGenCall *CGC = nullptr;

    public:
        ASTFuncCall(const SourceLocation &Loc, const std::string &NameSpace, const std::string &Name);

        const SourceLocation &getLocation() const;

        const std::string &getNameSpace() const;

        void setNameSpace(const std::string &NameSpace);

        const std::string &getName() const;

        const std::vector<ASTCallArg *> getArgs() const;

        ASTCallArg *addArg(ASTCallArg *Arg);

        ASTFunc *getDecl() const;

        void setDecl(ASTFunc *FDecl);

        CodeGenCall *getCodeGen() const;

        void setCodeGen(CodeGenCall *CGC);

        static ASTFuncCall *CreateCall(ASTFunc *FDecl);

        bool isUsable(ASTFuncCall *Call);

        std::string str() const;
    };
}

namespace std {
    using namespace fly;

    template <>
    struct hash<ASTFunc *> {
        // id is returned as hash function
        size_t operator()(ASTFunc *Decl) const noexcept;
    };

    template <>
    struct equal_to<ASTFunc *> {
        bool operator()(const ASTFunc *C1, const ASTFunc *C2) const;
    };
}


#endif //FLY_FUNCTION_H
