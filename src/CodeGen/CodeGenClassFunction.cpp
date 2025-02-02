//===--------------------------------------------------------------------------------------------------------------===//
// src/CodeGen/CodeGenClassFunction.cpp - Code Generator Class
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "CodeGen/CodeGenClassFunction.h"
#include "CodeGen/CodeGenClassVar.h"
#include "CodeGen/CodeGenClass.h"
#include "CodeGen/CodeGen.h"
#include "CodeGen/CodeGenModule.h"
#include "CodeGen/CodeGenError.h"
#include "CodeGen/CodeGenVar.h"
#include "Sym/SymModule.h"
#include "Sym/SymClassMethod.h"
#include "Sym/SymClassAttribute.h"
#include "Sym/SymNameSpace.h"
#include "Sym/SymClass.h"
#include "AST/ASTClass.h"
#include "AST/ASTFunction.h"
#include "Basic/Debug.h"
#include "CodeGen/CodeGenVar.h"

#include <AST/ASTTypeRef.h>
#include <AST/ASTVar.h>

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, SymClassMethod *Sym, llvm::PointerType *TypePtr) :
	CodeGenFunctionBase(CGM, Sym) {
	SymClass *Class = Sym->getClass();

    // Generate return type
    GenReturnType();

    // Add ErrorHandler to params, Struct doesn't use ErrorHandler
    if (Class->getAST()->getClassKind() != ASTClassKind::STRUCT) {
        ParamTypes.push_back(CGM->ErrorPtrTy);
    }

    // Add the instance var of the class type to the parameters of the function
    if (TypePtr)
        ParamTypes.push_back(TypePtr);

    // Generate param types
    GenParamTypes(CGM, ParamTypes, Sym->getAST()->getParams());

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    FnType = llvm::FunctionType::get(RetType, ParamTypes, false);

    std::string Id = CodeGen::toIdentifier(Sym->getAST()->getName(), Class->getModule()->getNameSpace()->getName(), Class->getAST()->getName());
    Fn = llvm::Function::Create(FnType, llvm::GlobalValue::ExternalLinkage, Id, CGM->getModule());
}

void CodeGenClassFunction::GenBody() {
    FLY_DEBUG("CodeGenFunctionBase", "GenBody");
    SymClass *Class = Sym->getClass();
    llvm::Type *ClassType = Class->getCodeGen()->getType();
    setInsertPoint();

    // the first argument is the error handler
    if (Sym->getClass()->getAST()->getClassKind() != ASTClassKind::STRUCT) {
        ErrorHandler = Fn->getArg(0);

        // Alloca Error Handler Var
        AllocaErrorHandler();
    }

    // Alloca Method Class Instance
    CodeGenVar *CGI = nullptr;
    if (!Sym->isStatic()) {
        CGI = new CodeGenVar(CGM, ClassType);
        CGI->Alloca();
    }

    // Alloca Local Vars
    AllocaLocalVars();

    if (Class->getAST()->getClassKind() != ASTClassKind::STRUCT) {
        // Store Error Handler Var
        StoreErrorHandler(false);
    }

    // Instance Class Method (not static)
    if (CGI) {

        //Alloca, Store, Load the second arg which is the instance
        llvm::Argument *ClassInstancePtr = Class->getAST()->getClassKind() == ASTClassKind::STRUCT ? Fn->getArg(0) : Fn->getArg(1);

        // Save Class instance and get Pointer
        CGI->Store(ClassInstancePtr);
        CGI->Load();

        // Set var Index offset in the struct type
        uint32_t Index = Class->getAST()->getClassKind() == ASTClassKind::STRUCT ? 0 : 1;
        // All Class Vars
        SymClassMethod *ClassMethod = (SymClassMethod *) Sym;
        for (auto AttributeEntry : ClassMethod->getClass()->getAttributes()) {
        	SymClassAttribute *Attribute = AttributeEntry.getValue();

            // Set CodeGen Class Instance
            llvm::Type *Ty = CGM->GenType(Attribute->getAST()->getTypeRef()->getDef());
            CodeGenClassVar *CGV = new CodeGenClassVar(CGM, Ty, CGI, Index); // FIXME replace con CodeGenClassVar
            Attribute->setCodeGen(CGV);

            // Store attribute default value
            if (ClassMethod->isConstructor()) {
                Value *AttrValue = CGM->GenExpr(Attribute->getAST()->getExpr());
                CGV->Store(AttrValue);
            }
            Index++;
        }
    }

    // Alloca Function Local Vars and generate body
    StoreParams(false);
    CGM->GenBlock(this, Sym->getAST()->getBody());

    // Add return Void
    if (FnType->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
