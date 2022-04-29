//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunc.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunc.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTStmt.h"
#include "AST/ASTBlock.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"
#include <string>

using namespace fly;

ASTFunc::ASTFunc(ASTNode *Node, const SourceLocation &Loc, ASTType *ReturnType, const std::string Name) :
        ASTTopDecl(Loc, Node, TopDeclKind::DECL_FUNCTION), ReturnType(ReturnType), Name(Name), Header(new ASTFuncHeader),
        Body(new ASTBlock(Loc, this, nullptr)) {

}

const std::string &ASTFunc::getName() const {
    return Name;
}

bool ASTFunc::isConstant() const {
    return Constant;
}

ASTBlock *ASTFunc::getBody() {
    return Body;
}

const ASTFuncHeader *ASTFunc::getHeader() const {
    return Header;
}

ASTType *ASTFunc::getType() const {
    return ReturnType;
}

const std::vector<ASTLocalVar *> &ASTFunc::getLocalVars() const {
    return LocalVars;
}

void ASTFunc::addLocalVar(ASTLocalVar *LocalVar) {
    LocalVars.push_back(LocalVar);
}

CodeGenFunction *ASTFunc::getCodeGen() const {
    return CodeGen;
}

void ASTFunc::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

ASTFuncParam *ASTFunc::addParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name) {
    ASTFuncParam *VDecl = new ASTFuncParam(Loc, Type, Name);
    Header->Params.push_back(VDecl);
    return VDecl;
}

bool ASTFunc::isVarArg() {
    return Header->VarArg != nullptr;
}

void ASTFunc::setVarArg(ASTFuncParam* VarArg) {
    Header->VarArg = VarArg;
}

std::string ASTFunc::str() const {
    std::string Str = "{ Name=" + Name +
            ", Params=[";
    if(!Header->getParams().empty()) {
        for (ASTFuncParam *Param: Header->getParams()) {
            Str += Param->str() + ", ";
        }
        Str = Str.substr(0, Str.length()-2);
    }
    Str += "], ReturnType=" + ReturnType->str();
    return Str;
}

bool ASTFunc::operator==(const ASTFunc &F) const {
    bool Result = this->getName() == F.getName() &&
            this->getNameSpace()->getName() == F.getNameSpace()->getName() &&
            this->getHeader()->getParams().size() == F.getHeader()->getParams().size();
    if (Result) {
        for (int i = 0; i < this->getHeader()->getParams().size(); i++) {
            if (! this->getHeader()->getParams()[i]->getType()->equals(F.getHeader()->getParams()[i]->getType())) {
                return false;
            }
        }
    }
    return Result;
}

size_t std::hash<ASTFunc *>::operator()(ASTFunc *Decl) const noexcept {
    size_t Hash = (std::hash<std::string>()(Decl->getName()));
    Hash ^= (std::hash<std::string>()(Decl->getNameSpace()->getName()));
    for (auto &Param : Decl->getHeader()->getParams()) {
        Hash ^= (std::hash<std::string>()(Param->getType()->str()));
    }
    return Hash;
}

bool std::equal_to<ASTFunc *>::operator()(const ASTFunc *C1, const ASTFunc *C2) const {
    bool Result = C1->getName() == C2->getName() &&
            C1->getNameSpace()->getName() == C2->getNameSpace()->getName() &&
                  C1->getHeader()->getParams().size() == C2->getHeader()->getParams().size();
    if (Result) {
        for (int i = 0; i < C1->getHeader()->getParams().size(); i++) {
            if (! C1->getHeader()->getParams()[i]->getType()->equals(C2->getHeader()->getParams()[i]->getType())) {
                return false;
            }
        }
    }
    return Result;
}

ASTFuncParam::ASTFuncParam(const SourceLocation &Loc, ASTType *Type, const std::string &Name) :
        ASTVar(Type, Name), Location(Loc) {

}

ASTExpr *ASTFuncParam::getExpr() const {
    return Expr;
}

void ASTFuncParam::setExpr(ASTExpr *E) {
    assert(E->getKind() == EXPR_VALUE && "Invalid Value for Param");
    Expr = (ASTValueExpr *)E;
}

CodeGenLocalVar *ASTFuncParam::getCodeGen() const {
    return CodeGen;
}

void ASTFuncParam::setCodeGen(CodeGenLocalVar *CG) {
    CodeGen = CG;
}

std::string ASTFuncParam::str() const {
    return "{ " + ASTVar::str() +
            ", Expr=" + (Expr ? Expr->str() : "{}") +
            " }";
}

const std::vector<ASTFuncParam *> &ASTFuncHeader::getParams() const {
    return Params;
}

const ASTFuncParam *ASTFuncHeader::getVarArg() const {
    return VarArg;
}

ASTReturn::ASTReturn(const SourceLocation &Loc, ASTBlock *Block, ASTExpr *Expr) : ASTStmt(Loc, Block),
                                                                                  Expr(Expr) {}

ASTExpr *ASTReturn::getExpr() const {
    return Expr;
}

StmtKind ASTReturn::getKind() const {
    return Kind;
}

std::string ASTReturn::str() const {
    return "{ Kind=" + std::to_string(Kind) +
           ", Expr=" + Expr->str() +
           " }";
}

ASTFuncCall::ASTFuncCall(const SourceLocation &Loc, const std::string &NameSpace, const std::string &Name) :
    Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const SourceLocation &ASTFuncCall::getLocation() const {
    return Loc;
}

const std::string &ASTFuncCall::getName() const {
    return Name;
}

const std::vector<ASTCallArg*> ASTFuncCall::getArgs() const {
    return Args;
}

ASTFunc *ASTFuncCall::getDecl() const {
    return Decl;
}

void ASTFuncCall::setDecl(ASTFunc *FDecl) {
    Decl = FDecl;
}

CodeGenCall *ASTFuncCall::getCodeGen() const {
    return CGC;
}

void ASTFuncCall::setCodeGen(CodeGenCall *CGC) {
    CGC = CGC;
}

ASTCallArg *ASTFuncCall::addArg(ASTCallArg *Arg) {
    Args.push_back(Arg);
    return Arg;
}

const std::string &ASTFuncCall::getNameSpace() const {
    return NameSpace;
}

void ASTFuncCall::setNameSpace(const std::string &NS) {
    NameSpace = NS;
}

ASTFuncCall *ASTFuncCall::CreateCall(ASTFunc *FDecl) {
    ASTFuncCall *FCall = new ASTFuncCall(SourceLocation(), FDecl->getNameSpace()->getName(), FDecl->getName());
    FCall->setDecl(FDecl);
    for (auto &Param : FDecl->getHeader()->getParams()) {
        FCall->addArg(new ASTCallArg(nullptr, Param->getType()));
    }
    return FCall;
}

bool ASTFuncCall::isUsable(ASTFuncCall *Call) {
    const auto &Params = Decl->getHeader()->getParams();
    const auto &Args = Call->getArgs();

    // Check Number of Args on First
    if (Decl->isVarArg()) {
        if (Params.size() > Args.size()) {
            return false;
        }
    } else {
        if (Params.size() != Args.size()) {
            return false;
        }
    }

    // Check Type
    for (int i = 0; i < Params.size(); i++) {
        bool isLast = i+1 == Params.size();

        //Check VarArgs by compare each Arg Type with last Param Type
        if (isLast && Decl->isVarArg()) {
            for (int n = i; n < Args.size(); n++) {
                // Check Equal Type
                if (Params[i]->getType()->getKind() == Args[n]->getType()->getKind()) {
                    return false;
                }
            }
        } else {
            ASTCallArg *Arg = Args[i];
            if (Arg->getType() == nullptr) {
                ASTType *Ty = ASTResolver::ResolveExprType(Arg->getValue());
                Arg->setType(Ty);
            }

            // Check Equal Type
            if (!Params[i]->getType()->equals(Arg->getType())) {
                return false;
            }
        }
    }
    return true;
}

std::string ASTFuncCall::str() const {
    std::string Str = "{ NameSpace=" + NameSpace +
           ", Name=" + Name +
           ", Args=[";
    if (!Args.empty()) {
        for (ASTCallArg *Arg : Args) {
            Str += Arg->str() + ", ";
        }
        Str = Str.substr(0, Str.length()-2);
    }
    Str += "] }";
    return Str;
}

ASTCallArg::ASTCallArg(ASTExpr *Value, ASTType *Type) : Value(Value), Type(Type) {

}

ASTExpr *ASTCallArg::getValue() const {
    return Value;
}

ASTType *ASTCallArg::getType() const {
    return Type;
}

void ASTCallArg::setType(ASTType *T) {
    Type = T;
}

std::string ASTCallArg::str() const {
    return "{ Value=" + (Value  ? Value->str() : "{}") +
           ", Type=" + (Type ? Type->str() : "{}") +
           " }";
}
