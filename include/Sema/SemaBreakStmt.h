//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBreakStmt.h - break statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BREAKSTMT_H
#define FLY_SEMA_BREAKSTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaBreakStmt : public SemaStmt {

    public:

        explicit SemaBreakStmt(ASTStmt *AST);

        ~SemaBreakStmt() override = default;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_BREAKSTMT_H

