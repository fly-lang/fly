//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/FuncDecl.cpp - Function implementation
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "AST/FuncDecl.h"
#include "AST/VarDeclStmt.h"
#include "AST/Stmt.h"
#include "AST/BlockStmt.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTNode.h"
#include <string>

using namespace fly;

FuncDecl::FuncDecl(ASTNode *Node, const SourceLocation &Loc, TypeBase *RetType, const llvm::StringRef &Name) :
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

const std::vector<FuncCall *> &FuncDecl::getUnRefCalls() const {
    return UnRefCalls;
}

const FuncHeader *FuncDecl::getHeader() const {
    return Header;
}

TypeBase *FuncDecl::getType() const {
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

bool FuncDecl::isVarArg() {
    return Header->VarArg != NULL;
}

void FuncDecl::setVarArg(FuncParam* VarArg) {
    Header->VarArg = VarArg;
}

bool FuncDecl::addUnRefCall(FuncCall *Call) {
    UnRefCalls.push_back(Call);
    return true;
}

void FuncDecl::addUnRefGlobalVar(VarRef *Var) {
    UnRefGlobalVars.push_back(Var);
}

bool FuncDecl::ResolveCall(FuncCall *ResolvedCall, FuncCall *Call) {
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
            FuncArg *Arg = Args[i];
            if (Arg->getType() == NULL) {
                TypeBase *Ty = ASTNode::ResolveExprType(Arg->getValue());
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

bool FuncDecl::operator==(const FuncDecl &F) const {
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

size_t FuncDeclHash::operator()(FuncDecl *Decl) const noexcept {
    size_t Hash = (std::hash<std::string>()(Decl->getName().str()));
    Hash ^= (std::hash<std::string>()(Decl->getNameSpace()->getNameSpace().str()));
    for (auto &Param : Decl->getHeader()->getParams()) {
        Hash ^= (std::hash<std::string>()(Param->getType()->str()));
    }
    return Hash;
}

bool FuncDeclComp::operator()(const FuncDecl *C1, const FuncDecl *C2) const {
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

bool FuncDecl::Finalize() {

    // Resolve RefGlobalVars with GlobalVars of the NameSpace
    for (const auto &UnRefGVar : UnRefGlobalVars) {
        const auto &NS = Node->findNameSpace(UnRefGVar->getNameSpace());
        if (!NS) {
            assert("NameSpace not found");
        }
        const auto &GVar = NS->getGlobalVars().find(UnRefGVar->getName());
        if (GVar == NS->getGlobalVars().end()) {
            assert("Global Var reference not found");
        }
        UnRefGVar->setDecl((VarDecl *) GVar->getValue());
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
                assert(NS && "Namespace not found"); // FIXME Error Message

                auto ItNS = NS->getResolvedCalls().find(UnRefCall->getName());
                assert(ItNS != NS->getResolvedCalls().end() && "Unresolved Call"); // FIXME Error Message

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

FuncParam::FuncParam(const SourceLocation &Loc, TypeBase *Type, const llvm::StringRef &Name) :
        VarDecl(Type, Name), Location(Loc) {

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

ReturnStmt::ReturnStmt(const SourceLocation &Loc, BlockStmt *Block, Expr *Exp) : Stmt(Loc, Block),
                                                                                 Ty(Block->getTop()->getType()),
                                                                                 Exp(Exp) {}

Expr *ReturnStmt::getExpr() const {
    return Exp;
}

StmtKind ReturnStmt::getKind() const {
    return Kind;
}

FuncCall::FuncCall(const SourceLocation &Loc, const StringRef &NameSpace, const StringRef &Name) :
    Loc(Loc), NameSpace(NameSpace), Name(Name) {

}

const SourceLocation &FuncCall::getLocation() const {
    return Loc;
}

const llvm::StringRef &FuncCall::getName() const {
    return Name;
}

const std::vector<FuncArg*> FuncCall::getArgs() const {
    return Args;
}

FuncDecl *FuncCall::getDecl() const {
    return Decl;
}

void FuncCall::setDecl(FuncDecl *FDecl) {
    Decl = FDecl;
}

CodeGenCall *FuncCall::getCodeGen() const {
    return CGC;
}

void FuncCall::setCodeGen(CodeGenCall *CGC) {
    CGC = CGC;
}

FuncArg *FuncCall::addArg(FuncArg *Arg) {
    Args.push_back(Arg);
    return Arg;
}

const StringRef &FuncCall::getNameSpace() const {
    return NameSpace;
}

void FuncCall::setNameSpace(const llvm::StringRef &NS) {
    NameSpace = NS;
}

FuncCall *FuncCall::CreateCall(FuncDecl *FDecl) {
    FuncCall *FCall = new FuncCall(SourceLocation(), FDecl->getNameSpace()->getNameSpace(), FDecl->getName());
    FCall->setDecl(FDecl);
    for (auto &Param : FDecl->getHeader()->getParams()) {
        FCall->addArg(new FuncArg(NULL, Param->getType()));
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

FuncArg::FuncArg(Expr *Value, TypeBase *Ty) : Value(Value), Ty(Ty) {

}

Expr *FuncArg::getValue() const {
    return Value;
}

TypeBase *FuncArg::getType() const {
    return Ty;
}

void FuncArg::setType(TypeBase *T) {
    Ty = T;
}
