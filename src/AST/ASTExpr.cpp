//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTExpr.cpp - Expression into a statement
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTExpr.h"
#include "AST/ASTVar.h"
#include "AST/ASTValue.h"
#include "AST/ASTFunc.h"

using namespace fly;

ASTExpr::ASTExpr(const SourceLocation &Loc) : Loc(Loc) {

}

const SourceLocation &ASTExpr::getLocation() const {
    return Loc;
}

ASTValueExpr::ASTValueExpr(const ASTValue *Val) : ASTExpr(Val->getLocation()), Val(Val) {

}

ASTExprKind ASTValueExpr::getKind() const {
    return Kind;
}

const ASTValue &ASTValueExpr::getValue() const {
    return *Val;
}

ASTType *ASTValueExpr::getType() const {
    return Val->getType();
}

std::string ASTValueExpr::str() const {
    return "{ Type=" + getType()->str() +
           ", Kind=" + std::to_string(Kind) +
           ", Value=" + Val->str() +
           " }";
}

ASTVarRefExpr::ASTVarRefExpr(ASTVarRef *Ref) : ASTExpr(Ref->getLocation()), Ref(Ref) {

}

ASTExprKind ASTVarRefExpr::getKind() const {
    return Kind;
}

ASTVarRef *ASTVarRefExpr::getVarRef() const {
    return Ref;
}

ASTType *ASTVarRefExpr::getType() const {
    return Ref->getDecl() == nullptr ? nullptr : Ref->getDecl()->getType();
}

std::string ASTVarRefExpr::str() const {
    return "{ Type=" +  (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string(Kind) +
           ", VarRef=" + Ref->str() +
           " }";
}

ASTFuncCallExpr::ASTFuncCallExpr(ASTFuncCall *Ref) : ASTExpr(Ref->getLocation()), Call(Ref) {}

ASTExprKind ASTFuncCallExpr::getKind() const {
    return Kind;
}

ASTFuncCall *ASTFuncCallExpr::getCall() const {
    return Call;
}

ASTType *ASTFuncCallExpr::getType() const {
    return Call->getDecl() == nullptr ? nullptr : Call->getDecl()->getType();
}

std::string ASTFuncCallExpr::str() const {
    return "{ Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string(Kind) +
           ", Call=" + Call->str() +
           " }";
}

ASTGroupExpr::ASTGroupExpr(const SourceLocation &Loc,
                           ASTExprGroupKind GroupKind) :
                           ASTExpr(Loc),
                           GroupKind(GroupKind) {

}

ASTExprKind ASTGroupExpr::getKind() const {
    return Kind;
}

ASTExprGroupKind ASTGroupExpr::getGroupKind() {
    return GroupKind;
}

ASTUnaryGroupExpr::ASTUnaryGroupExpr(const SourceLocation &Loc, UnaryOpKind Operator,
                                     UnaryOptionKind Option, ASTVarRefExpr *First) :
        ASTGroupExpr(Loc, GROUP_UNARY), OperatorKind(Operator), OptionKind(Option), First(First) {

}

UnaryOpKind ASTUnaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

UnaryOptionKind ASTUnaryGroupExpr::getOptionKind() const {
    return OptionKind;
}

const ASTVarRefExpr *ASTUnaryGroupExpr::getFirst() const {
    return First;
}

ASTType *ASTUnaryGroupExpr::getType() const {
    return First->getType();
}

std::string ASTUnaryGroupExpr::str() const {
    return "{ First=" + First->str() +
           ", Operator=" + std::to_string(OperatorKind) +
           ", Option=" + std::to_string(OptionKind) +
           ", Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string(getKind());
}

ASTBinaryGroupExpr::ASTBinaryGroupExpr(const SourceLocation &Loc,
                                       BinaryOpKind Operator,
                                       ASTExpr *First,
                                       ASTExpr *Second) :
        ASTGroupExpr(Loc, GROUP_BINARY),
        OperatorKind(Operator),
        OptionKind(Operator < 300 ? (Operator < 200 ?  BINARY_ARITH : BINARY_LOGIC) : BINARY_COMPARISON),
        First(First),
        Second(Second) {

}

BinaryOpKind ASTBinaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

BinaryOptionKind ASTBinaryGroupExpr::getOptionKind() const {
    return OptionKind;
}

const ASTExpr *ASTBinaryGroupExpr::getFirst() const {
    return First;
}

const ASTExpr *ASTBinaryGroupExpr::getSecond() const {
    return Second;
}

ASTType *ASTBinaryGroupExpr::getType() const {
    switch (OptionKind) {

        case BINARY_ARITH:
            return First->getType();
        case BINARY_LOGIC:
            return new ASTBoolType(SourceLocation());
        case BINARY_COMPARISON:
            return new ASTBoolType(SourceLocation());
    }
}

std::string ASTBinaryGroupExpr::str() const {
    return "{ First=" + First->str() +
           ", Operator=" + std::to_string(OperatorKind) +
           ", Second=" + Second->str() +
           ", Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string(getKind());
}

ASTTernaryGroupExpr::ASTTernaryGroupExpr(const SourceLocation &Loc,
                                         ASTExpr *First,
                                         ASTExpr *Second,
                                         ASTExpr *Third) :
                                         ASTGroupExpr(Loc, GROUP_TERNARY),
                                         First(First),
                                         Second(Second),
                                         Third(Third) {

}

TernaryOpKind ASTTernaryGroupExpr::getOperatorKind() const {
    return OperatorKind;
}

ASTType *ASTTernaryGroupExpr::getType() const {
    return Second->getType();
}

const ASTExpr *ASTTernaryGroupExpr::getFirst() const {
    return First;
}

const ASTExpr *ASTTernaryGroupExpr::getSecond() const {
    return Second;
}

const ASTExpr *ASTTernaryGroupExpr::getThird() const {
    return Third;
}

std::string ASTTernaryGroupExpr::str() const {
    return "{ First=" + First->str() +
           ", Second=" + Second->str() +
           ", Third=" + Third->str() +
           ", Type=" + (getType() ? getType()->str() : "") +
           ", Kind=" + std::to_string(getKind());
}