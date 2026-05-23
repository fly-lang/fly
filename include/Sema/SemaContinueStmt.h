//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaContinueStmt.h - continue statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_CONTINUESTMT_H
#define FLY_SEMA_CONTINUESTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaContinueStmt : public SemaStmt {

    public:

        explicit SemaContinueStmt(ASTStmt *AST);

        ~SemaContinueStmt() override = default;

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_CONTINUESTMT_H

