//===--------------------------------------------------------------------------------------------------------------===//
// src/Sema/SemaBlockStmt.cpp
//===--------------------------------------------------------------------------------------------------------------===//

#include "Sema/SemaBlockStmt.h"
#include "Sema/SemaSmartAlloc.h"
#include "Sema/SemaVisitor.h"

using namespace fly;

SemaBlockStmt::SemaBlockStmt(ASTStmt *AST) : SemaStmt(SemaKind::STMT_BLOCK, AST) {}

SemaBlockStmt::~SemaBlockStmt() {
    for (auto *S : Content) delete S;
    for (auto *A : SmartAllocs) delete A;
}

void SemaBlockStmt::addContent(SemaStmt *Stmt) { Content.push_back(Stmt); }

const llvm::SmallVector<SemaStmt *, 8> &SemaBlockStmt::getContent() const { return Content; }

bool SemaBlockStmt::isEmpty() const { return Content.empty(); }

void SemaBlockStmt::addSmartAlloc(SemaSmartAlloc *Alloc) { SmartAllocs.push_back(Alloc); }

const llvm::SmallVector<SemaSmartAlloc *, 4> &SemaBlockStmt::getSmartAllocs() const { return SmartAllocs; }

void SemaBlockStmt::accept(SemaVisitor &Visitor) { Visitor.visit(*this); }

