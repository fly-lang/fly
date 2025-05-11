//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaClassSymbols.h - SemaClassSymbols
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_ENUM_TYPE_H
#define FLY_SEMA_ENUM_TYPE_H

#include "SemaVisibilityKind.h"
#include "Sema/SemaType.h"
#include "llvm/ADT/StringMap.h"

namespace fly {

    class Sema;
    class ASTEnum;
    class SemaComment;
    class SemaEnumEntry;
    class SemaModule;
    enum class SemaVisibilityKind;

    class SemaEnumType : public SemaType {

        friend class SemaBuilder;
        friend class SemaResolver;
        friend class SemaValidator;

        ASTEnum *AST;

        SemaModule *Module;

        // Super Enums
        llvm::StringMap<SemaEnumType *> SuperEnums;

        SemaVisibilityKind Visibility = SemaVisibilityKind::DEFAULT;

        bool Constant;

        // Enum Entries
        llvm::StringMap<SemaEnumEntry *> Entries;

        SemaComment *Comment = nullptr;

        explicit SemaEnumType(ASTEnum *AST);

    public:

        ASTEnum *getAST();

        SemaModule *getModule() const;

        const llvm::StringMap<SemaEnumType *> &getSuperEnums() const;

        SemaVisibilityKind getVisibility() const;

        bool isConstant() const;

        const llvm::StringMap<SemaEnumEntry *> &getEntries() const;

        SemaComment *getComment() const;

    };

}  // end namespace fly

#endif // FLY_SEMA_ENUM_TYPE_H