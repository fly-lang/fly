//===-------------------------------------------------------------------------------------------------------------===//
// include/AST/ASTFailStmt.h - AST Fail Statement header
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_AST_FAILSTMT_H
#define FLY_AST_FAILSTMT_H

#include "ASTStmt.h"

namespace fly {

    class ASTHandleStmt;
	class ASTValue;

    class ASTFailStmt : public ASTStmt {

        friend class ASTBuilder;
        friend class ASTBuilderStmt;

        ASTExpr *FirstExpr = nullptr;

    	ASTExpr *SecondExpr = nullptr;

    	ASTExpr *ThirdExpr = nullptr;

        ASTFailStmt(const SourceLocation &Loc);

    public:

        void accept(ASTVisitor& Visitor) override;

        ASTExpr *getFirstExpr() const;

        void setFirstExpr(ASTExpr *);

    	ASTExpr *getSecondExpr() const;

    	void setSecondExpr(ASTExpr *);

    	ASTExpr *getThirdExpr() const;

    	void setThirdExpr(ASTExpr *);

        std::string str() const override;
    };
}

#endif //FLY_AST_FAILSTMT_H
