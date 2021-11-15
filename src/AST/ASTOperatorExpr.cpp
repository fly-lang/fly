//===--------------------------------------------------------------------------------------------------------------===//
// src/AST/ASTOperatorExpr.cpp - AST Operator Expression
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//


#include "AST/ASTOperatorExpr.h"
#include "AST/ASTVar.h"

using namespace fly;

ASTOperatorExpr::ASTOperatorExpr(const SourceLocation &Loc, OpTypeKind TypeKind) : ASTExpr(Loc), TypeKind(TypeKind) {

}

ASTType *ASTOperatorExpr::getType() const {
    return nullptr;
}

ASTExprKind ASTOperatorExpr::getKind() const {
    return Kind;
}

OpTypeKind ASTOperatorExpr::getTypeKind() const {
    return TypeKind;
}

std::string ASTOperatorExpr::str() const {
    return "Kind=" + std::to_string(Kind);
}

ASTUnaryExpr::ASTUnaryExpr(const SourceLocation &Loc, ASTOperatorExpr *OperatorExpr, ASTVarRef *VarRef,
                           UnaryOpKind UKind) : OperatorExpr(OperatorExpr), VarRef(VarRef), UnaryKind(UKind),
                                                ASTOperatorExpr(Loc, TY_UNARY){

}

OpKind ASTUnaryExpr::getOpKind() {
    return OperatorExpr->getOpKind();
}

UnaryOpKind ASTUnaryExpr::getUnaryKind() const {
    return UnaryKind;
}

ASTOperatorExpr *ASTUnaryExpr::getOperatorExpr() const {
    return OperatorExpr;
}

ASTVarRef *ASTUnaryExpr::getVarRef() const {
    return VarRef;
}

std::string ASTUnaryExpr::str() const {
    return ASTOperatorExpr::str() +
           ", VarRef=" + VarRef->str() +
           ", Operator=" + OperatorExpr->str() +
           ", UnaryKind=" + std::to_string(UnaryKind);
}

ASTBinaryExpr::ASTBinaryExpr(const SourceLocation &Loc) : ASTOperatorExpr(Loc, TY_BINARY) {

}

ASTArithExpr::ASTArithExpr(const SourceLocation &Loc, const ArithOpKind &AKind) : ASTBinaryExpr(Loc),
                                                                                  ArithKind(AKind) {

}

OpKind ASTArithExpr::getOpKind() {
    return OperatorKind;
}

ArithOpKind ASTArithExpr::getArithKind() const {
    return ArithKind;
}

std::string ASTArithExpr::str() const {
    return "{ " +
           ASTOperatorExpr::str() +
           ", OperatorKind=" + std::to_string(OperatorKind) +
           ", ArithKind=" + std::to_string(ArithKind) +
           " }";
}

ASTLogicExpr::ASTLogicExpr(const SourceLocation &Loc, const LogicOpKind &LKind) : ASTBinaryExpr(Loc), LogicKind(LKind) {
    assert(LKind == LogicOpKind::LOGIC_AND || LKind == LogicOpKind::LOGIC_OR && "Only && or ||");
}

OpKind ASTLogicExpr::getOpKind() {
    return OperatorKind;
}

LogicOpKind ASTLogicExpr::getLogicKind() const {
    return LogicKind;
}

std::string ASTLogicExpr::str() const {
    return "{ " +
           ASTOperatorExpr::str() +
           ", OperatorKind=" + std::to_string(OperatorKind) +
           ", LogicKind=" + std::to_string(LogicKind) +
           " }";
}

ASTComparisonExpr::ASTComparisonExpr(const SourceLocation &Loc, const ComparisonOpKind &CKind) : ASTBinaryExpr(Loc),
                                                                                                 ComparisonKind(CKind) {

}

OpKind ASTComparisonExpr::getOpKind() {
    return OperatorKind;
}

ComparisonOpKind ASTComparisonExpr::getComparisonKind() const {
    return ComparisonKind;
}

std::string ASTComparisonExpr::str() const {
    return "{ " +
           ASTOperatorExpr::str() +
           ", OperatorKind=" + std::to_string(OperatorKind) +
           ", ComparisonKind=" + std::to_string(ComparisonKind) +
           " }";
}

ASTTernaryExpr::ASTTernaryExpr(const SourceLocation &Loc, ASTExpr *Condition, ASTExprStmt *Stmt1, ASTExprStmt *Stmt2) :
        ASTOperatorExpr(Loc, TY_TERNARY), Condition(Condition), Stmt1(Stmt1), Stmt2(Stmt2) {

}

OpKind ASTTernaryExpr::getOpKind() {
    return OperatorKind;
}

std::string ASTTernaryExpr::str() const {
    return "{ " +
           ASTOperatorExpr::str() +
           ", OperatorKind=" + std::to_string(OperatorKind) +
           " }";
}
