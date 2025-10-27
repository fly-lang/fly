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

namespace llvm {
	class StringRef;
}

namespace fly {

    class SymbolTable;
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
	class SemaMemberVar;
    class ASTClass;
    class ASTNameSpace;
    class ASTImport;
    class ASTEnum;
    class ASTComment;
    class ASTVar;
    class ASTFunction;
	class ASTBoolValue;
	class ASTNumberValue;
	class ASTStringValue;
	class ASTArrayValue;
	class ASTStructValue;
	class ASTCall;
	class SemaLocalVar;
	class SemaParam;
	class SemaNode;
	class SemaResult;
	class SemaValue;
	class SemaIntType;
	class SemaFloatType;
	class SemaBoolValue;
	class SemaStringValue;
	class SemaArrayValue;
	class SemaStructValue;

    class SemaBuilder {

    public:

    	SemaBuilder() = delete;

    	static SemaFunction *CreateFunction(ASTFunction &AST);

    	static SemaClassType *CreateClass(ASTClass &AST);

    	static SemaClassInstance *CreateThisInstance(SemaClassType *Class);

    	static SemaClassAttribute *CreateClassAttribute(SemaClassType *Class, SemaClassInstance *This, ASTVar *AST, SemaComment *Comment);

    	static SemaClassMethod *CreateClassMethod(SemaClassType *Class, SemaClassInstance *This, ASTFunction *AST, SemaComment *Comment);

    	static SemaEnumType *CreateEnum(ASTEnum &AST);

    	static SemaEnumEntry *CreateEnumEntry(SemaEnumType *Enum, ASTVar *AST, SemaComment *Comment);

	    static SemaComment* CreateComment(ASTComment* AST);

    	static SemaLocalVar *CreateLocalVar(ASTVar *AST);

    	static SemaParam *CreateParam(fly::ASTVar* Param);

    	static SemaMemberVar *CreateMemberVar(ASTVar *AST, SemaResult *Parent);

    	static SemaCall *CreateCall(ASTCall *Call);

    	static SemaBoolValue *CreateBoolValue(ASTBoolValue *Value);

    	static SemaValue *CreateNumberValue(ASTNumberValue *Value);

    	static SemaStringValue *CreateStringValue(ASTStringValue *Value);

    	static SemaArrayValue *CreateArrayValue(ASTArrayValue *AST) ;

    	static SemaStructValue *CreateStructValue(ASTStructValue *AST);

    };

}  // end namespace fly

#endif // FLY_SEMA_BUILDER_H