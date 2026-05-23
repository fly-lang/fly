//===--------------------------------------------------------------------------------------------------------------===//
// compiler/Sema/SemaLoopStmt.cpp - loop statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaLoopStmt.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaLoopStmt::SemaLoopStmt(ASTStmt *AST, bool VerifyConditionAtEnd)
    : SemaStmt(SemaKind::STMT_LOOP, AST), VerifyConditionAtEnd(VerifyConditionAtEnd) {}

void SemaLoopStmt::addInit(SemaStmt *Stmt) { Init.push_back(Stmt); }
const llvm::SmallVector<SemaStmt *, 4> &SemaLoopStmt::getInit() const { return Init; }

SemaExpr *SemaLoopStmt::getCond() const { return Cond; }
void SemaLoopStmt::setCond(SemaExpr *C) { Cond = C; }

SemaStmt *SemaLoopStmt::getBody() const { return Body; }
void SemaLoopStmt::setBody(SemaStmt *B) { Body = B; }

void SemaLoopStmt::addPost(SemaStmt *Stmt) { Post.push_back(Stmt); }
const llvm::SmallVector<SemaStmt *, 4> &SemaLoopStmt::getPost() const { return Post; }

bool SemaLoopStmt::hasVerifyConditionAtEnd() const { return VerifyConditionAtEnd; }

void SemaLoopStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

