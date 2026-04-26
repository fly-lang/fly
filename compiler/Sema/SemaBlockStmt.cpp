//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBlockStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaAlloc.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaBlockStmt::SemaBlockStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_BLOCK, AST) {}

SemaBlockStmt::~SemaBlockStmt() {
    for (auto *S : Content) delete S;
    for (auto *A : Allocs) delete A;
}

void SemaBlockStmt::addContent(SemaStmt *Stmt) { Content.push_back(Stmt); }

const llvm::SmallVector<SemaStmt *, 8> &SemaBlockStmt::getContent() const { return Content; }

bool SemaBlockStmt::isEmpty() const { return Content.empty(); }

void SemaBlockStmt::addAlloc(SemaAlloc *Alloc) { Allocs.push_back(Alloc); }

const llvm::SmallVector<SemaAlloc *, 8> &SemaBlockStmt::getAllocs() const { return Allocs; }

void SemaBlockStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }
