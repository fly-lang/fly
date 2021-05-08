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

FuncDecl::FuncDecl(const SourceLocation &Loc, const TypeBase *Type, const StringRef &Name) :
        Decl(Loc), Type(Type), Name(Name), Params(new ParamsFunc) {}

DeclKind FuncDecl::getKind() const {
return Kind;
}

const StringRef &FuncDecl::getName() const {
    return Name;
}

bool FuncDecl::isConstant() const {
    return Constant;
}

const StmtDecl *FuncDecl::getBody() const {
    return Body;
}

const ParamsFunc *FuncDecl::getParams() const {
    return Params;
}

const TypeBase *FuncDecl::getType() const {
    return Type;
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

const std::vector<VarDecl *> &ParamsFunc::getVars() const {
    return Vars;
}

const VarDecl *ParamsFunc::getVarArg() const {
    return VarArg;
}

const std::vector<Expr *> &ParamsFuncRef::getArgs() const {
    return Args;
}

const VarRef *ParamsFuncRef::getVarArg() const {
    return VarArg;
}

ReturnDecl::ReturnDecl(SourceLocation &Loc, class GroupExpr *Group) : Decl(Loc), Group(Group) {}

GroupExpr *ReturnDecl::getExpr() const {
    return Group;
}

DeclKind ReturnDecl::getKind() const {
    return Kind;
}

FuncRef::FuncRef(const SourceLocation &Loc, const StringRef &Name) : Refer(Loc), Name(Name), Params(new ParamsFuncRef) {

}

FuncRef::FuncRef(const SourceLocation &Loc, FuncDecl *D) : Refer(Loc), Name(D->getName()), D(D) {

}

const StringRef &FuncRef::getName() const {
    return Name;
}

const ParamsFuncRef *FuncRef::getParams() const {
    return Params;
}

FuncDecl *FuncRef::getDecl() const {
    return D;
}

FuncRefDecl::FuncRefDecl(const SourceLocation &Loc, const StringRef &Name) : FuncRef(Loc, Name), Decl(Loc) {

}

FuncRefDecl::FuncRefDecl(const SourceLocation &Loc, FuncDecl *D) : FuncRef(Loc, D), Decl(Loc) {

}

DeclKind FuncRefDecl::getKind() const {
    return R_FUNCTION;
}
