//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaHandleStmt.h - handle statement semantic analysis
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_HANDLESTMT_H
#define FLY_SEMA_HANDLESTMT_H

#include "Sema/SemaStmt.h"

namespace fly {

    class SemaError;
    class SemaBlockStmt;

    class SemaHandleStmt : public SemaStmt {

        SemaError *ErrorHandler = nullptr;

        SemaBlockStmt *Handle = nullptr;

    public:

        explicit SemaHandleStmt(ASTStmt *AST);

        ~SemaHandleStmt() override = default;

        SemaError *getErrorHandler() const;

        void setErrorHandler(SemaError *ErrorHandler);

        SemaBlockStmt *getHandle() const;

        void setHandle(SemaBlockStmt *Handle);

        void accept(SemaVisitor &Visitor) override;
    };
}

#endif //FLY_SEMA_HANDLESTMT_H

