add_executable(fly
        Fly.cpp)

add_library(flyLib

        # AST
        AST/ASTBase.cpp
        AST/ASTBlockStmt.cpp
        AST/ASTBreakStmt.cpp
        AST/ASTCall.cpp
        AST/ASTClass.cpp
        AST/ASTClassMethod.cpp
        AST/ASTClassType.cpp
        AST/ASTClassAttribute.cpp
        AST/ASTContext.cpp
        AST/ASTContinueStmt.cpp
        AST/ASTDeleteStmt.cpp
        AST/ASTEnum.cpp
        AST/ASTEnumEntry.cpp
        AST/ASTEnumType.cpp
        AST/ASTExpr.cpp
        AST/ASTExprStmt.cpp
        AST/ASTFailStmt.cpp
        AST/ASTFunction.cpp
        AST/ASTFunctionBase.cpp
        AST/ASTGlobalVar.cpp
        AST/ASTHandleStmt.cpp
        AST/ASTIdentifier.cpp
        AST/ASTIdentity.cpp
        AST/ASTIdentityType.cpp
        AST/ASTIfStmt.cpp
        AST/ASTImport.cpp
        AST/ASTLocalVar.cpp
        AST/ASTLoopStmt.cpp
        AST/ASTLoopInStmt.cpp
        AST/ASTNameSpace.cpp
        AST/ASTModule.cpp
        AST/ASTParam.cpp
        AST/ASTScopes.cpp
        AST/ASTStmt.cpp
        AST/ASTSwitchStmt.cpp
        AST/ASTType.cpp
        AST/ASTValue.cpp
        AST/ASTVar.cpp
        AST/ASTVarRef.cpp
        AST/ASTVarStmt.cpp

        # Basic
        Basic/Archiver.cpp
        Basic/Builtins.cpp
        Basic/CharInfo.cpp
        Basic/CodeGenOptions.cpp
        Basic/Cuda.cpp
        Basic/Diagnostic.cpp
        Basic/DiagnosticIDs.cpp
        Basic/DiagnosticOptions.cpp
        Basic/FileManager.cpp
        Basic/FileSystemStatCache.cpp
        Basic/FixedPoint.cpp
        Basic/IdentifierTable.cpp
        Basic/Module.cpp
        Basic/OperatorPrecedence.cpp
        Basic/SanitizerBlacklist.cpp
        Basic/SanitizerSpecialCaseList.cpp
        Basic/Sanitizers.cpp
        Basic/SourceLocation.cpp
        Basic/SourceManager.cpp
        Basic/Stack.cpp
        Basic/TargetInfo.cpp
        Basic/Targets.cpp
        Basic/Targets/AArch64.cpp
        Basic/Targets/AMDGPU.cpp
        Basic/Targets/ARC.cpp
        Basic/Targets/ARM.cpp
        Basic/Targets/AVR.cpp
        Basic/Targets/BPF.cpp
        Basic/Targets/Hexagon.cpp
        Basic/Targets/Lanai.cpp
        Basic/Targets/Le64.cpp
        Basic/Targets/MSP430.cpp
        Basic/Targets/Mips.cpp
        Basic/Targets/NVPTX.cpp
        Basic/Targets/PNaCl.cpp
        Basic/Targets/PPC.cpp
        Basic/Targets/RISCV.cpp
        Basic/Targets/SPIR.cpp
        Basic/Targets/Sparc.cpp
        Basic/Targets/SystemZ.cpp
        Basic/Targets/TCE.cpp
        Basic/Targets/VE.cpp
        Basic/Targets/WebAssembly.cpp
        Basic/Targets/X86.cpp
        Basic/Targets/XCore.cpp
        Basic/TokenKinds.cpp
        Basic/Warnings.cpp

        #CodeGen
        CodeGen/BackendUtil.cpp
        CodeGen/CodeGen.cpp
        CodeGen/CodeGenClass.cpp
        CodeGen/CodeGenClassFunction.cpp
        CodeGen/CodeGenClassVar.cpp
        CodeGen/CodeGenEnum.cpp
        CodeGen/CodeGenEnumEntry.cpp
        CodeGen/CodeGenExpr.cpp
        CodeGen/CodeGenError.cpp
        CodeGen/CodeGenFunction.cpp
        CodeGen/CodeGenFunctionBase.cpp
        CodeGen/CodeGenGlobalVar.cpp
        CodeGen/CodeGenHandle.cpp
        CodeGen/CodeGenInstance.cpp
        CodeGen/CodeGenHeader.cpp
        CodeGen/CodeGenModule.cpp
        CodeGen/CodeGenVar.cpp

        # Driver
        Driver/Driver.cpp
        Driver/DriverOptions.cpp
        Driver/ToolChain.cpp

        # Frontend
        Frontend/ChainedDiagnosticConsumer.cpp
        Frontend/CompilerInstance.cpp
        Frontend/DiagnosticRenderer.cpp
        Frontend/Frontend.cpp
        Frontend/FrontendOptions.cpp
        Frontend/InputFile.cpp
        Frontend/LogDiagnosticPrinter.cpp
        Frontend/TextDiagnostic.cpp
        Frontend/TextDiagnosticBuffer.cpp
        Frontend/TextDiagnosticPrinter.cpp

        # Lex
        Lex/Lexer.cpp
        Lex/LiteralSupport.cpp
        Lex/HeaderMap.cpp

        # Parser
        Parser/ClassParser.cpp
        Parser/EnumParser.cpp
        Parser/ExprParser.cpp
        Parser/FunctionParser.cpp
        Parser/Parser.cpp

        # Sema
        Sema/Sema.cpp
        Sema/SemaBuilder.cpp
        Sema/SemaResolver.cpp
        Sema/SemaValidator.cpp
)

configure_file(
        ${CMAKE_SOURCE_DIR}/include/Config/Config.h.cmake
        ${CMAKE_SOURCE_DIR}/include/Config/Config.h)

foreach(target ${LLVM_TARGETS_TO_BUILD})
    list(APPEND targets "LLVM${target}CodeGen")
endforeach()

target_link_libraries(flyLib
        LLVMCore
        LLVMMC
        LLVMMCParser
        LLVMObject
        LLVMSupport
        LLVMOption
        LLVMPasses
        LLVMLTO
        LLVMTarget
        ${targets}
        LLVMAArch64AsmParser
        LLVMAMDGPUAsmParser
        LLVMARMAsmParser
        LLVMAVRAsmParser
        LLVMBPFAsmParser
        LLVMMipsAsmParser
        LLVMMSP430AsmParser
        LLVMPowerPCAsmParser
        LLVMRISCVAsmParser
        LLVMSparcAsmParser
        LLVMSystemZAsmParser
        LLVMWebAssemblyAsmParser
        LLVMX86AsmParser
        lldCommon
        lldCOFF
        lldELF
        lldMachO2
        lldMinGW
        lldWasm)

target_link_libraries(fly
        flyLib)