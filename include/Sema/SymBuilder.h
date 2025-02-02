//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SymBuilder.h - Symbolic Table Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SYM_BUILDER_H
#define FLY_SYM_BUILDER_H

#include <AST/ASTTypeRef.h>
#include <Sym/SymClass.h>

namespace llvm {
	class StringRef;
}

namespace fly {

    class SymTable;
	class SymNameSpace;
	class SymModule;
	class SymGlobalVar;
	class SymFunction;
    class SymClass;
	class SymClassAttribute;
	class SymClassMethod;
    class SymEnum;
    class SymEnumEntry;
    class SymType;
    class ASTClass;
    class ASTEnum;
class SymTypeInt;
class SymTypeFP;

    class SymBuilder {

    	friend class Sema;

    	Sema &S;

    	SymBuilder *Builder;

    	explicit SymBuilder(Sema &S);

    public:

    	SymTable* CreateTable();

    	SymNameSpace *CreateNameSpace();

    	SymNameSpace *CreateNameSpace(llvm::StringRef Name);

    	SymModule* CreateModule(ASTModule *AST);

    	SymGlobalVar *CreateGlobalVar(SymModule *Module, ASTVar *AST);

    	SymFunction *CreateFunction(SymModule *Module, ASTFunction *AST);

    	SymClass *CreateClass(SymModule *Module, ASTClass *AST);

    	SymClassAttribute *CreateClassAttribute(SymClass *Class, ASTVar *AST);

    	SymClassMethod *CreateClassFunction(SymClass *Class, ASTFunction *AST);

    	SymEnum *CreateEnum(SymModule *Module, ASTEnum *AST);

    	SymEnum *CreateEnumEntry(SymEnum *Enum, ASTVar *AST);

    	SymType * SymBuilder::CreateType(SymTypeKind Kind);

    	SymTypeInt *CreateIntType(SymIntTypeKind IntKind);

    	SymTypeFP *CreateFPType(SymFPTypeKind FPKind);

    	SymTypeArray *CreateArrayType(SymType Type, uint64_t Size);

	    SymComment* CreateComment(ASTComment* AST);

    };

}  // end namespace fly

#endif // FLY_SYM_BUILDER_H