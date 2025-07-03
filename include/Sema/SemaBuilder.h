//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilder.h - Symbolic Table Builder
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_SEMA_BUILDER_H
#define FLY_SEMA_BUILDER_H

#include <AST/ASTCall.h>
#include <AST/ASTTypeRef.h>
#include <AST/ASTValue.h>
#include <Sema/SemaValue.h>
#include <Sema/SemaClassType.h>
#include <Sema/SemaLocalVar.h>
#include <Sema/SemaParam.h>

#include "SemaMemberVar.h"

namespace llvm {
	class StringRef;
}

namespace fly {

    class SymTable;
	class SemaNameSpace;
	class SemaModule;
	class SemaGlobalVar;
	class SemaFunction;
    class SemaClassType;
	class SemaClassAttribute;
	class SemaClassMethod;
    class SemaEnumType;
    class SemaEnumEntry;
    class SemaType;
    class ASTClass;
    class ASTNameSpace;
    class ASTImport;
    class ASTEnum;
    class ASTComment;
    class ASTVar;
    class ASTFunction;
	class SemaIntType;
	class SemaFloatType;

    class SemaBuilder {

    	friend class Sema;

    	Sema &S;

    	SemaBuilder *Builder;

    	explicit SemaBuilder(Sema &S);

    public:

    	void CreateTable();

    	SemaNameSpace *CreateDefaultNameSpace();

    	SemaNameSpace *CreateOrGetNameSpace(ASTNameSpace *AST);

    	SemaModule* CreateModule(SemaNameSpace *NameSpace, ASTModule *AST);

    	void CreateImport(SemaModule* Module, ASTImport* AST);

    	// TODO: remove GlobalVar
    	// SymGlobalVar *CreateGlobalVar(SymModule *Module, ASTVar *AST);

    	SemaFunction *CreateFunction(SemaModule *Module, ASTFunction *AST);

    	SemaClassType *CreateClass(SemaModule *Module, ASTClass *AST);

    	SemaVar *CreateThisAttribute(SemaClassType *Class);

    	SemaClassAttribute *CreateClassAttribute(SemaClassType *Class, ASTVar *AST, SemaComment *Comment);

    	SemaClassMethod *CreateClassMethod(SemaClassType *Class, ASTFunction *AST, SemaComment *Comment);

    	SemaEnumType *CreateEnum(SemaModule *Module, ASTEnum *AST);

    	SemaEnumEntry *CreateEnumEntry(SemaEnumType *Enum, ASTVar *AST, SemaComment *Comment);

    	SemaType *CreateType(SemaTypeKind Kind, std::string Name);

    	SemaIntType *CreateIntType(SemaIntTypeKind IntKind, std::string Name);

    	SemaFloatType *CreateFPType(SemaFloatTypeKind FPKind, std::string Name);

    	SemaArrayType *CreateArrayType(SemaType *Type);

	    SemaComment* CreateComment(ASTComment* AST);

    	SemaLocalVar *CreateLocalVar(ASTVar *AST);

    	SemaParam *CreateParam(fly::ASTVar* Param);

    	SemaMemberVar *CreateMemberVar(ASTVar *AST, SemaResult *Parent);

    	SemaCall *CreateCall(ASTCall *Call);

    	SemaBoolValue *CreateBoolValue(ASTBoolValue *Value);

    	SemaValue *CreateNumberValue(ASTNumberValue *Value);

    	SemaStringValue *CreateStringValue(ASTStringValue *Value);

    	SemaArrayValue *CreateArrayValue(ASTArrayValue *AST) ;

    	SemaStructValue *CreateStructValue(ASTStructValue *AST);

    };

}  // end namespace fly

#endif // FLY_SEMA_BUILDER_H