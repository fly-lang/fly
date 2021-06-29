//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/FuncDecl.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/FuncDecl.h"
#include "AST/BlockStmt.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"

using namespace fly;

FuncDecl::FuncDecl(ASTNode *Node, const SourceLocation &Loc, const TypeBase *RetType, const llvm::StringRef &Name) :
    Kind(TopDeclKind::DECL_FUNCTION), TopDecl(Node, Loc), Type(RetType), Name(Name), Header(new FuncHeader),
                   Body(new BlockStmt(Loc, this, NULL)) {}

TopDeclKind FuncDecl::getKind() const {
return Kind;
}

const llvm::StringRef &FuncDecl::getName() const {
    return Name;
}

bool FuncDecl::isConstant() const {
    return Constant;
}

BlockStmt *FuncDecl::getBody() {
    return Body;
}

const std::vector<FuncCall *> &FuncDecl::getCalls() const {
    return Calls;
}

const std::vector<VarRef *> &FuncDecl::getVarRefs() const {
    return VarRefs;
}

const FuncHeader *FuncDecl::getHeader() const {
    return Header;
}

const TypeBase *FuncDecl::getType() const {
    return Type;
}

CodeGenFunction *FuncDecl::getCodeGen() const {
    return CodeGen;
}

void FuncDecl::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

FuncParam *FuncDecl::addParam(const SourceLocation &Loc, TypeBase *Type, const StringRef &Name) {
    FuncParam *VDecl = new FuncParam(Loc, Type, Name);
    Header->Params.push_back(VDecl);
    return VDecl;
}

bool FuncDecl::addCall(FuncCall *Call) {
    Calls.push_back(Call);
    return true;
}

void FuncDecl::setVarArg(FuncParam* VarArg) {
    Header->VarArg = VarArg;
}

bool FuncDecl::Finalize() {

    // Resolve VarRef with VarDecl in the namespace
    for (auto *VarRef : VarRefs) {
        // TODO Resolve GlobalVar
        // TODO Resolve VarDeclStmt
    }

    // Resolve Calls with FuncDecl in the namespace
    for (auto *Call : Calls) {
        // Search into NameSpace
        const ASTNameSpace *NS = Node->findNameSpace(Call->getNameSpace());
        assert(NS && "Namespace not found"); // FIXME Error Message
        auto It = NS->getCalls().find(Call);
        assert(It != NS->getCalls().end() && "Unresolved Call"); // FIXME Error Message
        Call->setDecl(((FuncCall *)*It)->getDecl());
    }
    return true;
}

FuncParam::FuncParam(const SourceLocation &Loc, TypeBase *Type, const llvm::StringRef &Name) :
        VarDecl(Type, Name, false), Location(Loc) {

}

CodeGenVar *FuncParam::getCodeGen() const {
    return CodeGen;
}

void FuncParam::setCodeGen(CodeGenVar *CG) {
    CodeGen = CG;
}

const std::vector<FuncParam *> &FuncHeader::getParams() const {
    return Params;
}

const FuncParam *FuncHeader::getVarArg() const {
    return VarArg;
}

ReturnStmt::ReturnStmt(const SourceLocation &Loc, BlockStmt *Block, class GroupExpr *Group) : Stmt(Loc, Block),
                                                                                              Ty(Block->getTop()->getType()), Group(Group) {}

GroupExpr *ReturnStmt::getExpr() const {
    return Group;
}

StmtKind ReturnStmt::getKind() const {
    return Kind;
}

//FuncCall::FuncCall(const SourceLocation &Loc, const StringRef &Name) : Loc(Loc), S(S), NameSpace(""),
//    Name(Name) {
//
//}

FuncCall::FuncCall(const SourceLocation &Loc, const StringRef &NameSpace, const StringRef &Name) :
    Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const SourceLocation &FuncCall::getLocation() const {
    return Loc;
}

const llvm::StringRef &FuncCall::getName() const {
    return Name;
}

const std::vector<FuncCallArg*> FuncCall::getArgs() const {
    return Args;
}

FuncDecl *FuncCall::getDecl() const {
    return Func;
}

void FuncCall::setDecl(FuncDecl *F) {
    Func = F;
}

CodeGenCall *FuncCall::getCodeGen() const {
    return CGC;
}

void FuncCall::setCodeGen(CodeGenCall *CGC) {
    CGC = CGC;
}

FuncCallArg *FuncCall::addArg(Expr *Ex, TypeBase *Ty) {
    FuncCallArg *Arg = new FuncCallArg(Ex, Ty);
    Args.push_back(Arg);
    return Arg;
}

const StringRef &FuncCall::getNameSpace() const {
    return NameSpace;
}

FuncCall *FuncCall::CreateCall(FuncDecl *FDecl) {
    FuncCall *FCall = new FuncCall(SourceLocation(), FDecl->getNameSpace().getNameSpace(), FDecl->getName());
    FCall->setDecl(FDecl);
    for (auto &Param : FDecl->getHeader()->getParams()) {
        FCall->addArg(NULL, Param->getType());
    }
    return FCall;
}

FuncCallStmt::FuncCallStmt(const SourceLocation &Loc, BlockStmt *Block, FuncCall *Call) :
    Stmt(Loc, Block), Call(Call) {

}

StmtKind FuncCallStmt::getKind() const {
    return STMT_FUNC_CALL;
}

FuncCall *FuncCallStmt::getCall() const {
    return Call;
}

FuncCallArg::FuncCallArg(Expr *Value, TypeBase *Ty) : Value(Value), Ty(Ty) {

}

Expr *FuncCallArg::getValue() const {
    return Value;
}

TypeBase *FuncCallArg::getType() const {
    return Ty;
}
