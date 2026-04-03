//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBlockStmt.h - Sema Block Statement header
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BLOCKSTMT_H
#define FLY_SEMA_BLOCKSTMT_H

#include "Sema/SemaStmt.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SemaBlockStmt : public SemaStmt {

        llvm::SmallVector<SemaStmt *, 8> Content;

    public:

        explicit SemaBlockStmt(ASTStmt *AST = nullptr);

        ~SemaBlockStmt() override;

        void addContent(SemaStmt *Stmt);

        const llvm::SmallVector<SemaStmt *, 8> &getContent() const;

        bool isEmpty() const;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_BLOCKSTMT_H

