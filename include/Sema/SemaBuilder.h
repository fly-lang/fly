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

#include <AST/ASTValue.h>

namespace fly {

    class SymbolTable;
	class SemaNameSpace;
	class SemaImport;
	class SemaModule;
	class SemaFunction;
	class SemaFunctionBase;
    class SemaClassType;
	class SemaClassAttribute;
	class SemaClassInstance;
	class SemaClassMethod;
    class SemaEnumType;
    class SemaEnumValue;
    class SemaType;
	class SemaMember;
    class ASTClass;
    class ASTNameSpace;
    class ASTImport;
    class ASTEnum;
    class ASTComment;
    class ASTVar;
    class ASTAttribute;
    class ASTMethod;
    class ASTLocalVar;
    class ASTParam;
    class ASTFunction;
	class ASTBoolValue;
	class ASTNumberValue;
	class ASTStringValue;
	class ASTArrayValue;
	class ASTStructValue;
	class ASTMember;
	class ASTCall;
	class ASTUnary;
	class ASTBinary;
	class ASTTernary;
	class SemaLocalVar;
	class SemaParam;
	class SemaNode;
	class SemaExpr;
	class SemaValue;
	class SemaCall;
	class SemaComment;
	class SemaUnary;
	class SemaIntType;
	class SemaFloatType;
	class SemaBoolValue;
	class SemaStringValue;
	class SemaArrayValue;
	class SemaStructValue;
	class SemaBinary;
	class SemaTernary;
	class ASTBuilderIfStmt;
	class ASTBuilderSwitchStmt;
	class ASTBuilderLoopStmt;
	class ASTBuilderStmt;

    class SemaBuilder {

    public:

    	SemaBuilder() = delete;

    	static SemaImport *CreateImport(SemaModule &Module, ASTImport &AST);

    	static SemaFunction *CreateFunction(SemaModule &Module, SymbolTable *Symbols, ASTFunction &AST);

    	static SemaClassType *CreateClass(SemaModule &Module, SymbolTable *Symbols, ASTClass &AST);

    	static SemaClassInstance *CreateThisInstance(SemaClassType &Class);

    	static SemaClassAttribute *CreateClassAttribute(SemaClassType &Class, ASTAttribute &AST, SemaType *Type);

    	static SemaClassMethod *CreateDefaultConstructor(SemaClassType *Class);

    	static SemaClassMethod *CreateClassMethod(SemaClassType *Class, ASTMethod &AST);

    	static SemaEnumType *CreateEnum(SemaModule &Module, SymbolTable *Symbols, ASTEnum &AST);

    	static SemaEnumValue *CreateEnumValue(SemaEnumType *Enum, ASTEnumValue &AST);

	    static SemaComment* CreateComment(ASTComment &AST);

    	static SemaLocalVar *CreateLocalVar(ASTLocalVar &AST, SemaType *Type);

    	static SemaParam *CreateParam(ASTParam &Param, SemaType *Type);

    	static SemaMember *CreateMemberVar(ASTMember &AST, SemaExpr *Ref, SemaExpr *Parent = nullptr);

    	static SemaCall *CreateCall(ASTCall &Call, SemaType *Type, SemaFunctionBase *Function);

    	static SemaUnary *CreateUnary(ASTUnary &AST);

    	static SemaBinary *CreateBinary(ASTBinary &AST);

    	static SemaTernary *CreateTernary(ASTTernary &AST);

    	static SemaValue *CreateDefaultValue(SemaType &Type);

    	static SemaBoolValue *CreateBoolValue(ASTBoolValue &AST);

    	static SemaValue *CreateNumberValue(ASTNumberValue &AST);

    	static llvm::APInt CreateAPIntValue(StringRef ValStr);

    	static SemaIntValue *CreateIntValue(ASTNumberValue &AST, SemaIntType *IntType);

    	static SemaFloatValue *CreateFloatValue(ASTNumberValue &AST, SemaFloatType *FloatType);

    	static SemaStringValue *CreateStringValue(ASTStringValue &AST);

    	static SemaArrayValue *CreateArrayValue(ASTArrayValue &AST, SemaType *Type, llvm::SmallVector<SemaValue *, 8> &Values) ;

    	static SemaStructValue *CreateStructValue(ASTStructValue &AST, llvm::StringMap<SemaValue *> Values);

    	static SemaValue * CreateNullValue(ASTNullValue &AST);
    };

}  // end namespace fly

#endif // FLY_SEMA_BUILDER_H