//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTFunc.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/ASTFunction.h"
#include "AST/ASTParams.h"
#include "AST/ASTLocalVar.h"
#include "AST/ASTStmt.h"
#include "AST/ASTBlock.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include "AST/ASTContext.h"
#include <string>

using namespace fly;

ASTFunction::ASTFunction(const SourceLocation &Loc, ASTNode *Node, ASTType *ReturnType, const std::string &Name,
                         VisibilityKind Visibility) :
        ASTTopDecl(Loc, Node, TopDeclKind::DECL_FUNCTION, Visibility),
        ReturnType(ReturnType), Name(Name), Params(new ASTParams),
        Body(new ASTBlock(Loc, this, nullptr)) {

}

const std::string &ASTFunction::getName() const {
    return Name;
}

const ASTBlock *ASTFunction::getBody() const {
    return Body;
}

const ASTParams *ASTFunction::getParams() const {
    return Params;
}

ASTType *ASTFunction::getType() const {
    return ReturnType;
}

const std::vector<ASTLocalVar *> &ASTFunction::getLocalVars() const {
    return LocalVars;
}

void ASTFunction::addLocalVar(ASTLocalVar *LocalVar) {
    LocalVars.push_back(LocalVar);
}

CodeGenFunction *ASTFunction::getCodeGen() const {
    return CodeGen;
}

void ASTFunction::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

bool ASTFunction::isVarArg() {
    return Params->getEllipsis();
}

std::string ASTFunction::str() const {
    std::string Str = "{ Name=" + Name +
            ", Params=[";
    if(!Params->getList().empty()) {
        for (ASTParam *Param: Params->getList()) {
            Str += Param->str() + ", ";
        }
        Str = Str.substr(0, Str.length()-2);
    }
    Str += "], ReturnType=" + ReturnType->str();
    return Str;
}

bool ASTFunction::operator==(const ASTFunction &F) const {
    bool Result = this->getName() == F.getName() &&
            this->getNameSpace()->getName() == F.getNameSpace()->getName() &&
            this->getParams()->getList().size() == F.getParams()->getList().size();
    if (Result) {
        for (int i = 0; i < this->getParams()->getList().size(); i++) {
            if (!this->getParams()->getList()[i]->getType()->equals(F.getParams()->getList()[i]->getType())) {
                return false;
            }
        }
    }
    return Result;
}

size_t std::hash<ASTFunction *>::operator()(ASTFunction *F) const noexcept {
    size_t Hash = (std::hash<std::string>()(F->getName()));
    Hash ^= (std::hash<std::string>()(F->getNameSpace()->getName()));
    for (auto &Param : F->getParams()->getList()) {
        Hash ^= (std::hash<std::string>()(Param->getType()->str()));
    }
    return Hash;
}

bool std::equal_to<ASTFunction *>::operator()(const ASTFunction *F1, const ASTFunction *F2) const {
    bool Result = F1->getName() == F2->getName() &&
                  F1->getNameSpace()->getName() == F2->getNameSpace()->getName() &&
                  F1->getParams()->getList().size() == F2->getParams()->getList().size();
    if (Result) {
        for (int i = 0; i < F1->getParams()->getList().size(); i++) {
            if (!F1->getParams()->getList()[i]->getType()->equals(F2->getParams()->getList()[i]->getType())) {
                return false;
            }
        }
    }
    return Result;
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
