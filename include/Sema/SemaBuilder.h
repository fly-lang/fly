//===--------------------------------------------------------------------------------------------------------------===//
// include/Sema/SemaBuilder.h - semantic analysis builder
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
    class SemaEnumEntry;
    class SemaEnumList;
    class SemaType;
	class SemaMember;
    class ASTClass;
    class ASTNameSpace;
    class ASTImport;
    class ASTEnum;
    class ASTEnumEntry;
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
	class ASTNullValue;
	class ASTUnsetValue;
	class ASTMember;
	class ASTCall;
	class ASTUnary;
	class ASTBinary;
	class ASTTernary;
	class ASTCast;
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
	class SemaIntValue;
	class SemaFloatValue;
	class SemaBinary;
	class SemaTernary;
	class SemaCast;
	class ASTBuilderIfStmt;
	class ASTBuilderSwitchStmt;
	class ASTBuilderLoopStmt;
	class ASTBuilderStmt;
	class SemaError;
	// Stmt forward declarations
	class ASTStmt;
	class SemaStmt;
	class SemaBlockStmt;
	class SemaDeclStmt;
	class SemaExprStmt;
	class SemaReturnStmt;
	class SemaIfStmt;
	class SemaSwitchStmt;
	class SemaLoopStmt;
	class SemaLoopInStmt;
	class SemaDeleteStmt;
	class SemaBreakStmt;
	class SemaContinueStmt;
	class SemaFailStmt;
	class SemaHandleStmt;

    class SemaBuilder {

    public:

    	SemaBuilder() = delete;

    	static SemaImport *CreateImport(SemaModule &Module, ASTImport &AST);

    	static SemaFunction *CreateFunction(SemaModule &Module, SymbolTable *Symbols, ASTFunction &AST);

    	static SemaClassType *CreateClass(SemaModule &Module, SymbolTable *Symbols, ASTClass &AST);

    	static SemaClassAttribute *CreateClassAttribute(SemaClassType &Class, ASTAttribute &AST, SemaType *Type);

    	static SemaClassMethod *CreateDefaultConstructor(SemaClassType *Class, SymbolTable* Scope);

    	static SemaClassMethod *CreateClassMethod(SemaClassType *Class, ASTMethod &AST, SymbolTable* Scope);

    	static SemaEnumType *CreateEnum(SemaModule &Module, SymbolTable *Symbols, ASTEnum &AST);

    	static SemaEnumEntry *CreateEnumEntry(SemaEnumType *Enum, ASTEnumEntry &AST);

    	static SemaEnumList *CreateEnumList(SemaEnumType *EnumType);

	    static SemaComment* CreateComment(ASTComment &AST);

    	static SemaLocalVar *CreateLocalVar(ASTLocalVar &AST, SemaType *Type);

    	static SemaParam *CreateParam(ASTParam &Param, SemaType *Type);

    	static SemaMember *CreateMemberVar(ASTMember &AST, SemaExpr *Ref, SemaExpr *Parent = nullptr);

		static SemaError * CreateErrorHandler();

    	static SemaCall *CreateCall(ASTCall &Call, SemaType *Type, SemaFunctionBase *Function);

    	static SemaUnary *CreateUnary(ASTUnary &AST, SemaExpr *Expr);

    	static SemaBinary *CreateBinary(ASTBinary &AST, SemaExpr *Left, SemaExpr *Right);

    	static SemaTernary *CreateTernary(ASTTernary &AST, SemaExpr *Cond, SemaExpr *TrueExpr, SemaExpr *FalseExpr);

    	static SemaCast *CreateCast(ASTCast &AST, SemaExpr *Expr, SemaType *ToType);

    	static SemaBoolValue *CreateBoolValue(ASTBoolValue &AST);

    	static SemaValue *CreateNumberValue(ASTNumberValue &AST);

    	static llvm::APInt CreateAPIntValue(StringRef ValStr);

    	static SemaIntValue *CreateIntValue(ASTNumberValue &AST, SemaIntType *IntType);

    	static SemaFloatValue *CreateFloatValue(ASTNumberValue &AST, SemaFloatType *FloatType);

    	static SemaStringValue *CreateStringValue(ASTStringValue &AST);

    	static SemaArrayValue *CreateArrayValue(ASTArrayValue &AST, SemaType *Type, llvm::SmallVector<SemaValue *, 8> &Values) ;

    	static SemaStructValue *CreateStructValue(ASTStructValue &AST, llvm::StringMap<SemaValue *> Values);

    	static SemaValue * CreateNullValue(ASTNullValue &AST);

    	static SemaValue * CreateUnsetValue(ASTUnsetValue &AST);

		// Stmt factory methods
    	static SemaBlockStmt  *CreateBlockStmt(ASTStmt *AST = nullptr);
    	static SemaDeclStmt   *CreateDeclStmt(ASTStmt *AST, SemaLocalVar *Var, SemaExpr *Expr = nullptr);
    	static SemaExprStmt   *CreateExprStmt(ASTStmt *AST, SemaExpr *Expr);
    	static SemaReturnStmt *CreateReturnStmt(ASTStmt *AST);
    	static SemaIfStmt     *CreateIfStmt(ASTStmt *AST, SemaExpr *Cond, SemaStmt *Then);
    	static SemaSwitchStmt *CreateSwitchStmt(ASTStmt *AST, SemaExpr *Expr);
    	static SemaLoopStmt   *CreateLoopStmt(ASTStmt *AST, bool VerifyAtEnd = false);
    	static SemaLoopInStmt *CreateLoopInStmt(ASTStmt *AST, SemaExpr *Item, SemaExpr *List, SemaStmt *Body);
    	static SemaDeleteStmt *CreateDeleteStmt(ASTStmt *AST, SemaExpr *Expr);
    	static SemaBreakStmt    *CreateBreakStmt(ASTStmt *AST);
    	static SemaContinueStmt *CreateContinueStmt(ASTStmt *AST);
    	static SemaFailStmt   *CreateFailStmt(ASTStmt *AST);
    	static SemaHandleStmt *CreateHandleStmt(ASTStmt *AST);

	};

}  // end namespace fly

#endif // FLY_SEMA_BUILDER_H