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

#include <AST/ASTCall.h>
#include <AST/ASTTypeRef.h>
#include <Sym/SymCall.h>
#include <Sym/SymClass.h>
#include <Sym/SymLocalVar.h>
#include <Sym/SymParam.h>

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
    class ASTNameSpace;
    class ASTImport;
    class ASTEnum;
    class ASTComment;
    class ASTVar;
    class ASTFunction;
	class SymTypeInt;
	class SymTypeFP;

    class SymBuilder {

    	friend class Sema;

    	Sema &S;

    	SymBuilder *Builder;

    	explicit SymBuilder(Sema &S);

    public:

    	void CreateTable();

    	SymNameSpace *CreateDefaultNameSpace();

    	SymNameSpace *CreateOrGetNameSpace(ASTNameSpace *AST);

    	SymModule* CreateModule(SymNameSpace *NameSpace, ASTModule *AST);

    	void CreateImport(SymModule* Module, ASTImport* AST);

    	// TODO: remove GlobalVar
    	// SymGlobalVar *CreateGlobalVar(SymModule *Module, ASTVar *AST);

    	SymFunction *CreateFunction(SymModule *Module, ASTFunction *AST);

    	SymClass *CreateClass(SymModule *Module, ASTClass *AST);

    	SymClassAttribute *CreateClassAttribute(SymClass *Class, ASTVar *AST, SymComment *Comment);

    	SymClassMethod *CreateClassFunction(SymClass *Class, ASTFunction *AST, SymComment *Comment);

    	SymEnum *CreateEnum(SymModule *Module, ASTEnum *AST);

    	SymEnumEntry *CreateEnumEntry(SymEnum *Enum, ASTVar *AST, SymComment *Comment);

    	SymType *CreateType(SymTypeKind Kind, std::string Name);

    	SymTypeInt *CreateIntType(SymIntTypeKind IntKind, std::string Name);

    	SymTypeFP *CreateFPType(SymFPTypeKind FPKind, std::string Name);

    	SymTypeArray *CreateArrayType(SymType *Type);

	    SymComment* CreateComment(ASTComment* AST);

    	SymLocalVar *CreateLocalVar(ASTVar *AST);

    	SymParam *CreateParam(fly::ASTVar* Param);

    	SymCall *CreateCall(ASTCall *Call);

    };

}  // end namespace fly

#endif // FLY_SYM_BUILDER_H