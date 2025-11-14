add_executable(fly
        Fly.cpp
)

add_library(flyLib

        # AST
        AST/ASTArg.cpp
        AST/ASTBase.cpp
        AST/ASTBuilder.cpp
        AST/ASTBuilderStmt.cpp
        AST/ASTBuilderIfStmt.cpp
        AST/ASTBuilderSwitchStmt.cpp
        AST/ASTBuilderLoopStmt.cpp
        AST/ASTNode.cpp
        AST/ASTBlockStmt.cpp
        AST/ASTBreakStmt.cpp
        AST/ASTCall.cpp
        AST/ASTCallExpr.cpp
        AST/ASTCastExpr.cpp
        AST/ASTClass.cpp
        AST/ASTComment.cpp
        AST/ASTContinueStmt.cpp
        AST/ASTDeleteStmt.cpp
        AST/ASTEnum.cpp
        AST/ASTExpr.cpp
        AST/ASTExprStmt.cpp
        AST/ASTFailStmt.cpp
        AST/ASTFunction.cpp
        AST/ASTHandleStmt.cpp
        AST/ASTIdentifier.cpp
        AST/ASTType.cpp
        AST/ASTIfStmt.cpp
        AST/ASTImport.cpp
        AST/ASTLoopStmt.cpp
        AST/ASTLoopInStmt.cpp
        AST/ASTNameSpace.cpp
        AST/ASTModule.cpp
        AST/ASTOpExpr.cpp
        AST/ASTReturnStmt.cpp
        AST/ASTRuleStmt.cpp
        AST/ASTModifier.cpp
        AST/ASTStmt.cpp
        AST/ASTSwitchStmt.cpp
        AST/ASTValue.cpp
        AST/ASTValueExpr.cpp
        AST/ASTVar.cpp
        AST/ASTVarRefExpr.cpp
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
        Basic/Logger.cpp
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
        CodeGen/CodeGenClassMethod.cpp
        CodeGen/CodeGenEnumEntry.cpp
        CodeGen/CodeGenExpr.cpp
        CodeGen/CodeGenError.cpp
        CodeGen/CodeGenFunction.cpp
        CodeGen/CodeGenFunctionBase.cpp
#        CodeGen/CodeGenGlobalVar.cpp
        CodeGen/CodeGenHandle.cpp
        #CodeGen/CodeGenHeader.cpp
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

        # Parser
        Parser/Lexer.cpp
        Parser/LiteralSupport.cpp
        Parser/ParserClass.cpp
        Parser/ParserEnum.cpp
        Parser/ParserExpr.cpp
        Parser/ParserFunction.cpp
        Parser/Parser.cpp

        # Sema
        Sema/Helper.cpp
        Sema/Sema.cpp
        Sema/SemaBuilder.cpp
        Sema/SemaBuilderModifiers.cpp
        Sema/SemaBuiltin.cpp
        Sema/SemaValidator.cpp
        Sema/SemaCall.cpp
        Sema/SemaClassType.cpp
        Sema/SemaComment.cpp
        Sema/SemaClassAttribute.cpp
        Sema/SemaClassInstance.cpp
        Sema/SemaClassMethod.cpp
        Sema/SemaEnumType.cpp
        Sema/SemaEnumEntry.cpp
        Sema/SemaErrorHandler.cpp
        Sema/SemaFunction.cpp
        Sema/SemaFunctionBase.cpp
#        Sema/SemaGlobalVar.cpp
        Sema/SemaImport.cpp
        Sema/SemaLocalVar.cpp
        Sema/SemaMemberVar.cpp
        Sema/SemaModule.cpp
        Sema/SemaNameSpace.cpp
        Sema/SemaNode.cpp
        Sema/SemaParam.cpp
        Sema/SemaResult.cpp
        Sema/Symbol.cpp
        Sema/SymbolTable.cpp
        Sema/SemaType.cpp
        Sema/SemaVar.cpp
        Sema/SemaValue.cpp
        Sema/Registry.cpp
        Sema/Resolver.cpp
)

#if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    target_compile_options(flyLib PRIVATE -Wall)
#endif ()

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