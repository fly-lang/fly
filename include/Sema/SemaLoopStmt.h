//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaLoopStmt.h - loop statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_LOOPSTMT_H
#define FLY_SEMA_LOOPSTMT_H

#include "Sema/SemaStmt.h"
#include "llvm/ADT/SmallVector.h"

namespace fly {

    class SemaExpr;

    class SemaLoopStmt : public SemaStmt {

        llvm::SmallVector<SemaStmt *, 4> Init;

        SemaExpr *Cond = nullptr;  // nullable for infinite loops

        SemaStmt *Body = nullptr;

        llvm::SmallVector<SemaStmt *, 4> Post;

        bool VerifyConditionAtEnd;

    public:

        explicit SemaLoopStmt(ASTStmt *AST, bool VerifyConditionAtEnd = false);

        ~SemaLoopStmt() override = default;

        void addInit(SemaStmt *Stmt);

        const llvm::SmallVector<SemaStmt *, 4> &getInit() const;

        SemaExpr *getCond() const;

        void setCond(SemaExpr *Cond);

        SemaStmt *getBody() const;

        void setBody(SemaStmt *Body);

        void addPost(SemaStmt *Stmt);

        const llvm::SmallVector<SemaStmt *, 4> &getPost() const;

        bool hasVerifyConditionAtEnd() const;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_LOOPSTMT_H

