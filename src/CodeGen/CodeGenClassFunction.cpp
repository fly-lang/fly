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
#include "AST/ASTClassFunction.h"
#include "AST/ASTClassVar.h"
#include "AST/ASTNameSpace.h"
#include "AST/ASTClass.h"
#include "Basic/Debug.h"

using namespace fly;

CodeGenClassFunction::CodeGenClassFunction(CodeGenModule *CGM, ASTClassFunction *AST, llvm::PointerType *TypePtr) : CodeGenFunctionBase(CGM, AST) {
    llvm::SmallVector<llvm::Type *, 8> ParamTypes;
    llvm::Type *RetType = CGM->GenType(AST->getType());
    if (TypePtr) // Instance method
        ParamTypes.push_back(TypePtr);
    CodeGenFunctionBase::GenTypes(CGM, ParamTypes, AST->getParams());

    // Set LLVM Function Name %MODULE_CLASS_METHOD (if MODULE == default is empty)
    FnTy = llvm::FunctionType::get(RetType, ParamTypes, AST->getParams()->getEllipsis() != nullptr);

    ASTClass *Class = AST->getClass();
    std::string Name = CodeGen::toIdentifier(getAST()->getName(), Class->getNameSpace()->getName(), Class->getName());
    Fn = llvm::Function::Create(FnTy, llvm::GlobalValue::ExternalLinkage, Name, CGM->getModule());
}

void CodeGenClassFunction::GenBody() {
    FLY_DEBUG("CodeGenFunctionBase", "GenBody");
    ASTClass *Class = ((ASTClassFunction *) AST)->getClass();
    Type *ClassType = Class->getCodeGen()->getTypePtr();
    setInsertPoint();

    // Class Method (not static)
    if (!((ASTClassFunction *) AST)->isStatic()) {

        //Alloca, Store, Load first arg which is the instance
        llvm::Argument *ClassTypePtr = Fn->getArg(0); // FIXME remove?

        AllocaInst *Instance = CGM->Builder->CreateAlloca(ClassType);
        CGM->Builder->CreateStore(ClassTypePtr, Instance);
        llvm::LoadInst *Load = CGM->Builder->CreateLoad(Instance);

        // All Class Vars
        for (auto &Entry : Class->getVars()) {
            ASTClassVar *Var = Entry.second;

            // Set CodeGen Class Instance
            CodeGenClassVar *CGVar = (CodeGenClassVar *) Var->getCodeGen();
            CGVar->setInstance(Load);
            CGVar->Init();

            // Save all default var values
            if (((ASTClassFunction *) AST)->isConstructor()) {
                llvm::Value *V = CGM->GenExpr(Fn, Var->getType(), Var->getExpr());
                CGVar->Store(V);
            }
        }
    }

    // Alloca Function Local Vars and generate body
    AllocaVars();
    CGM->GenBlock(Fn, AST->getBody()->getContent());

    // Add return Void
    BasicBlock &BB = *Fn->getBasicBlockList().end();
    Instruction &I = *BB.end();
    if (FnTy->getReturnType()->isVoidTy()) {
        CGM->Builder->CreateRetVoid();
    }
}
