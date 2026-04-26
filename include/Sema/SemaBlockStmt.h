//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBlockStmt.h - Sema Block Statement header
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BLOCKSTMT_H
#define FLY_SEMA_BLOCKSTMT_H

#include "Sema/SemaStmt.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SemaAlloc;

    class SemaBlockStmt : public SemaStmt {

        llvm::SmallVector<SemaStmt *, 8> Content;

        // All scope-managed allocations in this block (smart pointers + heap strings).
        // Owned by this block; destroyed in the destructor.
        llvm::SmallVector<SemaAlloc *, 8> Allocs;

    public:

        explicit SemaBlockStmt(ASTStmt *AST = nullptr);

        ~SemaBlockStmt() override;

        void addContent(SemaStmt *Stmt);

        const llvm::SmallVector<SemaStmt *, 8> &getContent() const;

        bool isEmpty() const;

        void addAlloc(SemaAlloc *Alloc);

        const llvm::SmallVector<SemaAlloc *, 8> &getAllocs() const;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_BLOCKSTMT_H
