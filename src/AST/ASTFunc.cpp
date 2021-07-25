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

ASTFunc::ASTFunc(ASTNode *Node, const SourceLocation &Loc, ASTType *RetType, const llvm::StringRef &Name) :
        Kind(TopDeclKind::DECL_FUNCTION), ASTTopDecl(Node, Loc), Type(RetType), Name(Name), Header(new ASTFuncHeader),
        Body(new ASTBlock(Loc, this, nullptr)) {}

TopDeclKind ASTFunc::getKind() const {
return Kind;
}

const llvm::StringRef &ASTFunc::getName() const {
    return Name;
}

bool ASTFunc::isConstant() const {
    return Constant;
}

ASTBlock *ASTFunc::getBody() {
    return Body;
}

const std::vector<ASTFuncCall *> &ASTFunc::getUnRefCalls() const {
    return UnRefCalls;
}

const ASTFuncHeader *ASTFunc::getHeader() const {
    return Header;
}

ASTType *ASTFunc::getType() const {
    return Type;
}

CodeGenFunction *ASTFunc::getCodeGen() const {
    return CodeGen;
}

void ASTFunc::setCodeGen(CodeGenFunction *CGF) {
    CodeGen = CGF;
}

ASTFuncParam *ASTFunc::addParam(const SourceLocation &Loc, ASTType *Type, const StringRef &Name) {
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

bool ASTFunc::addUnRefCall(ASTFuncCall *Call) {
    UnRefCalls.push_back(Call);
    return true;
}

void ASTFunc::addUnRefGlobalVar(ASTVarRef *Var) {
    UnRefGlobalVars.push_back(Var);
}

bool ASTFunc::ResolveCall(ASTFuncCall *ResolvedCall, ASTFuncCall *Call) {
    const auto &Params = ResolvedCall->getDecl()->getHeader()->getParams();
    const bool isVarArg = ResolvedCall->getDecl()->isVarArg();
    const auto &Args = Call->getArgs();

    // Check Number of Args on First
    if (isVarArg) {
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
        if (isLast && isVarArg) {
            for (int n = i; n < Args.size(); n++) {
                // Check Equal Type
                if (Params[i]->getType()->getKind() == Args[n]->getType()->getKind()) {
                    return false;
                }
            }
        } else {
            ASTFuncArg *Arg = Args[i];
            if (Arg->getType() == nullptr) {
                ASTType *Ty = ASTNode::ResolveExprType(Arg->getValue());
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

bool ASTFunc::operator==(const ASTFunc &F) const {
    bool Result = this->getName().equals(F.getName()) &&
            this->getNameSpace()->getNameSpace().equals(F.getNameSpace()->getNameSpace()) &&
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
    size_t Hash = (std::hash<std::string>()(Decl->getName().str()));
    Hash ^= (std::hash<std::string>()(Decl->getNameSpace()->getNameSpace().str()));
    for (auto &Param : Decl->getHeader()->getParams()) {
        Hash ^= (std::hash<std::string>()(Param->getType()->str()));
    }
    return Hash;
}

bool std::equal_to<ASTFunc *>::operator()(const ASTFunc *C1, const ASTFunc *C2) const {
    bool Result = C1->getName().equals(C2->getName()) &&
                  C1->getNameSpace()->getNameSpace().equals(C2->getNameSpace()->getNameSpace()) &&
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

bool ASTFunc::Finalize() {

    // Resolve RefGlobalVars with GlobalVars of the NameSpace
    for (const auto &UnRefGVar : UnRefGlobalVars) {
        const auto &NS = Node->findNameSpace(UnRefGVar->getNameSpace());
        if (!NS) {
            getNode()->getContext().Diag(UnRefGVar->getLocation(), diag::err_namespace_notfound)
                << NS->getNameSpace();
            return false;
        }
        const auto &GVar = NS->getGlobalVars().find(UnRefGVar->getName());
        if (GVar == NS->getGlobalVars().end()) {
            getNode()->getContext().Diag(UnRefGVar->getLocation(), diag::err_gvar_notfound)
                << UnRefGVar->getName();
            return false;
        }
        UnRefGVar->setDecl((ASTVar *) GVar->getValue());
    }

    // Resolve Calls with FuncDecl by searching into ResolvedCalls
    for (auto *UnRefCall : UnRefCalls) {
        if (UnRefCall->getDecl() == nullptr) {

            // Set Call NameSpace of the Node if not specified
            if (UnRefCall->getNameSpace().empty()) {
                UnRefCall->setNameSpace(Node->getNameSpace()->getNameSpace());
            }

            // Search into Node
            const auto &It = Node->getResolvedCalls().find(UnRefCall->getName());
            if (It != Node->getResolvedCalls().end()) {
                for (auto &ResolvedCall : It->getValue()) {
                    if (ResolveCall(ResolvedCall, UnRefCall)) {
                        UnRefCall->setDecl(ResolvedCall->getDecl());
                    }
                }
            } else {
                // Search into NameSpace
                const ASTNameSpace *NS = Node->findNameSpace(UnRefCall->getNameSpace());
                if (NS == nullptr) {
                    getNode()->getContext().Diag(UnRefCall->getLocation(), diag::err_namespace_notfound)
                            << UnRefCall->getNameSpace();
                    return false;
                }

                auto ItNS = NS->getResolvedCalls().find(UnRefCall->getName());
                if (ItNS == NS->getResolvedCalls().end()) {
                    getNode()->getContext().Diag(UnRefCall->getLocation(), diag::err_func_notfound)
                            << UnRefCall->getName();
                    return false;
                }

                for (auto &ResolvedCall : ItNS->getValue()) {
                    if (ResolveCall(ResolvedCall, UnRefCall)) {
                        UnRefCall->setDecl(ResolvedCall->getDecl());
                    }
                }
            }
        }
    }
    return true;
}

ASTFuncParam::ASTFuncParam(const SourceLocation &Loc, ASTType *Type, const llvm::StringRef &Name) :
        ASTVar(Type, Name), Location(Loc) {

}

ASTExpr *ASTFuncParam::getExpr() const {
    return Expr;
}

void ASTFuncParam::setExpr(ASTExpr *E) {
    assert(E->getKind() == EXPR_VALUE && "Invalid Value for GlobalVar");
    Expr = (ASTValueExpr *)E;
}

CodeGenVar *ASTFuncParam::getCodeGen() const {
    return CodeGen;
}

void ASTFuncParam::setCodeGen(CodeGenVar *CG) {
    CodeGen = CG;
}

const std::vector<ASTFuncParam *> &ASTFuncHeader::getParams() const {
    return Params;
}

const ASTFuncParam *ASTFuncHeader::getVarArg() const {
    return VarArg;
}

ASTReturn::ASTReturn(const SourceLocation &Loc, ASTBlock *Block, ASTExpr *Expr) : ASTStmt(Loc, Block),
                                                                                  Ty(Block->getTop()->getType()),
                                                                                  Expr(Expr) {}

ASTExpr *ASTReturn::getExpr() const {
    return Expr;
}

StmtKind ASTReturn::getKind() const {
    return Kind;
}

ASTFuncCall::ASTFuncCall(const SourceLocation &Loc, const StringRef &NameSpace, const StringRef &Name) :
    Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const SourceLocation &ASTFuncCall::getLocation() const {
    return Loc;
}

const llvm::StringRef &ASTFuncCall::getName() const {
    return Name;
}

const std::vector<ASTFuncArg*> ASTFuncCall::getArgs() const {
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

ASTFuncArg *ASTFuncCall::addArg(ASTFuncArg *Arg) {
    Args.push_back(Arg);
    return Arg;
}

const StringRef &ASTFuncCall::getNameSpace() const {
    return NameSpace;
}

void ASTFuncCall::setNameSpace(const llvm::StringRef &NS) {
    NameSpace = NS;
}

ASTFuncCall *ASTFuncCall::CreateCall(ASTFunc *FDecl) {
    ASTFuncCall *FCall = new ASTFuncCall(SourceLocation(), FDecl->getNameSpace()->getNameSpace(), FDecl->getName());
    FCall->setDecl(FDecl);
    for (auto &Param : FDecl->getHeader()->getParams()) {
        FCall->addArg(new ASTFuncArg(nullptr, Param->getType()));
    }
    return FCall;
}

ASTFuncCallStmt::ASTFuncCallStmt(const SourceLocation &Loc, ASTBlock *Block, ASTFuncCall *Call) :
    ASTStmt(Loc, Block), Call(Call) {

}

StmtKind ASTFuncCallStmt::getKind() const {
    return STMT_FUNC_CALL;
}

ASTFuncCall *ASTFuncCallStmt::getCall() const {
    return Call;
}

ASTFuncArg::ASTFuncArg(ASTExpr *Value, ASTType *Ty) : Value(Value), Ty(Ty) {

}

ASTExpr *ASTFuncArg::getValue() const {
    return Value;
}

ASTType *ASTFuncArg::getType() const {
    return Ty;
}

void ASTFuncArg::setType(ASTType *T) {
    Ty = T;
}
