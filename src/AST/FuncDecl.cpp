//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/FuncDecl.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/FuncDecl.h"

using namespace fly;

FuncDecl::FuncDecl(const SourceLocation &Loc, const TypeBase *RetType, const llvm::StringRef &Name) :
        TopDecl(Loc), Type(RetType), Name(Name), Header(new ParamsFuncDecl),
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

const ParamsFuncDecl *FuncDecl::getHeader() const {
    return Header;
}

const TypeBase *FuncDecl::getType() const {
    return Type;
}

CodeGenFunction *FuncDecl::getCodeGen() const {
    return CodeGen;
}

void FuncDecl::setCodeGen(CodeGenFunction *codeGen) {
    CodeGen = codeGen;
}

VarDeclStmt *FuncDecl::addParam(const SourceLocation &Loc, TypeBase *Type, const StringRef &Name) {
    VarDeclStmt *VDecl = new VarDeclStmt(Loc, Body, Type, Name);
    Header->Params.push_back(VDecl);
    return VDecl;
}

void FuncDecl::setVarArg(VarDeclStmt* VarArg) {
    Header->VarArg = VarArg;
}

//const llvm::StringMap<VarRef *> &FuncDecl::getVarRef() const {
//    return VarRef;
//}

//const llvm::StringMap<FunctionRef *> &FuncDecl::getFuncRef() const {
//    return FuncRef;
//}

//const llvm::StringMap<ClassRef *> &FuncDecl::getClassRef() const {
//    return ClassRef;
//}

const std::vector<VarDeclStmt *> &ParamsFuncDecl::getParams() const {
    return Params;
}

const VarDeclStmt *ParamsFuncDecl::getVarArg() const {
    return VarArg;
}

const std::vector<Expr *> &ParamsFuncCall::getArgs() const {
    return Args;
}

const VarRef *ParamsFuncCall::getVarArg() const {
    return VarArg;
}

ReturnStmt::ReturnStmt(const SourceLocation &Loc, BlockStmt *CurrentStmt, class GroupExpr *Group) : Stmt(Loc, CurrentStmt),
        Ty(CurrentStmt->getContainer()->getType()), Group(Group) {}

GroupExpr *ReturnStmt::getExpr() const {
    return Group;
}

StmtKind ReturnStmt::getKind() const {
    return Kind;
}

FuncCall::FuncCall(const SourceLocation &Loc, const llvm::StringRef &Name) : Name(Name), Params(new ParamsFuncCall) {

}

FuncCall::FuncCall(const SourceLocation &Loc, FuncDecl *D) : Name(D->getName()), Func(D) {

}

const llvm::StringRef &FuncCall::getName() const {
    return Name;
}

const ParamsFuncCall *FuncCall::getParams() const {
    return Params;
}

FuncDecl *FuncCall::getDecl() const {
    return Func;
}

FuncCallStmt::FuncCallStmt(const SourceLocation &Loc, BlockStmt *CurrentStmt, const llvm::StringRef &Name) :
    FuncCall(Loc, Name), Stmt(Loc, CurrentStmt) {

}

FuncCallStmt::FuncCallStmt(const SourceLocation &Loc, BlockStmt *CurrentStmt, FuncDecl *D) : FuncCall(Loc, D),
    Stmt(Loc, CurrentStmt) {

}

StmtKind FuncCallStmt::getKind() const {
    return STMT_FUNC_CALL;
}
