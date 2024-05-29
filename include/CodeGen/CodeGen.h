//===--------------------------------------------------------------------------------------------------------------===//
// include/CodeGen/CodeGen.h - Code Generator
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
/// \file
/// Defines the fly::CodeGen interface.
///
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef FLY_CODEGEN_H
#define FLY_CODEGEN_H

#include "CodeGen/BackendUtil.h"
#include "Basic/Diagnostic.h"
#include "AST/ASTContext.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Linker/Linker.h"
#include <memory>

namespace llvm {
    class Constant;
    class LLVMContext;
    class Module;
    class StringRef;
}

namespace fly {

    class CodeGenHeader;
    class CodeGenModule;
    class TargetInfo;
    class FrontendOptions;

    class CodeGen {

    public:

        /// void
        llvm::Type *VoidTy;

        /// i8, i16, i32, and i64
        llvm::IntegerType *BoolTy, *Int8Ty, *Int16Ty, *Int32Ty, *Int64Ty;
        /// half, bfloat, float, double
        llvm::Type *HalfTy, *BFloatTy, *FloatTy, *DoubleTy;

        /// int
        llvm::IntegerType *IntTy;

        /// intptr_t, size_t, and ptrdiff_t, which we assume are the same size.
        union {
            llvm::IntegerType *IntPtrTy;
            llvm::IntegerType *SizeTy;
            llvm::IntegerType *PtrDiffTy;
        };

        /// void* in address space 0
        union {
            llvm::PointerType *VoidPtrTy;
            llvm::PointerType *Int8PtrTy;
        };

        /// void** in address space 0
        union {
            llvm::PointerType *VoidPtrPtrTy;
            llvm::PointerType *Int8PtrPtrTy;
        };

        /// void* in alloca address space
        union {
            llvm::PointerType *AllocaVoidPtrTy;
            llvm::PointerType *AllocaInt8PtrTy;
        };

        /// The width of a pointer into the generic address space.
        unsigned char PointerWidthInBits;

        /// The size and alignment of a pointer into the generic address space.
        union {
            unsigned char PointerAlignInBytes;
            unsigned char PointerSizeInBytes;
        };

        /// The size and alignment of size_t.
        union {
            unsigned char SizeSizeInBytes; // sizeof(size_t)
            unsigned char SizeAlignInBytes;
        };

        /// The size and alignment of the builtin C type 'int'.  This comes
        /// up enough in various ABI lowering tasks to be worth pre-computing.
        union {
            unsigned char IntSizeInBytes;
            unsigned char IntAlignInBytes;
        };

    protected:
        DiagnosticsEngine &Diags;
        CodeGenOptions &CodeGenOpts;
        TargetOptions &TargetOpts;
        IntrusiveRefCntPtr<TargetInfo> Target;
        llvm::LLVMContext LLVMCtx;
        BackendActionKind ActionKind;
        bool ShowTimers;

    public:
        CodeGen(DiagnosticsEngine &Diags, CodeGenOptions &CodeGenOpts,
                const std::shared_ptr<TargetOptions> &TargetOpts,
                BackendActionKind BackendAction, bool ShowTimers = false);

        std::string getOutputFileName(StringRef BaseInput);

        void Emit(llvm::Module *M, llvm::StringRef OutName);

        static TargetInfo* CreateTargetInfo(DiagnosticsEngine &Diags,
                                                 const std::shared_ptr<TargetOptions> &TargetOpts);

        /// Get the current target info.
        TargetInfo &getTargetInfo() const;

        llvm::LLVMContext &getLLVMCtx();

        std::vector<llvm::Module *> GenerateModules(ASTContext &Context);

        CodeGenModule *GenerateModule(ASTModule &AST);

        void GenerateHeaders(ASTContext &context);

        void GenerateHeader(ASTModule &Module);

        static std::string toIdentifier(llvm::StringRef Name, llvm::StringRef NameSpace, llvm::StringRef ClassName = "");
    };
}

#endif //FLY_CODEGEN_H
