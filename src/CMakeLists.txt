add_executable(fly
        Fly.cpp)

add_library(flyLib

        # AST
        AST/ASTBlock.cpp
        AST/ASTClass.cpp
        AST/ASTClassVar.cpp
        AST/ASTClassFunction.cpp
        AST/ASTContext.cpp
        AST/ASTExpr.cpp
        AST/ASTExprStmt.cpp
        AST/ASTClassVar.cpp
        AST/ASTForBlock.cpp
        AST/ASTFunction.cpp
        AST/ASTFunctionBase.cpp
        AST/ASTFunctionCall.cpp
        AST/ASTGlobalVar.cpp
        AST/ASTIfBlock.cpp
        AST/ASTImport.cpp
        AST/ASTLocalVar.cpp
        AST/ASTNameSpace.cpp
        AST/ASTNode.cpp
        AST/ASTNodeBase.cpp
        AST/ASTParams.cpp
        AST/ASTStmt.cpp
        AST/ASTSwitchBlock.cpp
        AST/ASTTopDef.cpp
        AST/ASTType.cpp
        AST/ASTUnref.cpp
        AST/ASTValue.cpp
        AST/ASTVarAssign.cpp
        AST/ASTVarRef.cpp
        AST/ASTWhileBlock.cpp

        # Basic
        Basic/Archiver.cpp
        Basic/Builtins.cpp
        Basic/CharInfo.cpp
        Basic/CodeGenOptions.cpp
        Basic/Cuda.cpp
        Basic/Debuggable.cpp
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
        CodeGen/CodeGenExpr.cpp
        CodeGen/CodeGenFunction.cpp
        CodeGen/CodeGenGlobalVar.cpp
        CodeGen/CodeGenHeader.cpp
        CodeGen/CodeGenModule.cpp
        CodeGen/CodeGenVar.cpp
        CodeGen/CodeGenVarBase.cpp

        # Driver
        Driver/Driver.cpp
        Driver/DriverOptions.cpp
        Driver/ToolChain.cpp

        # Frontend
        Frontend/ChainedDiagnosticConsumer.cpp
        Frontend/FrontendAction.cpp
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
        Parser/ExprParser.cpp
        Parser/FunctionParser.cpp
        Parser/Parser.cpp

        # Sema
        Sema/Sema.cpp
        Sema/SemaBuilder.cpp
        Sema/SemaResolver.cpp
        Sema/SemaValidator.cpp)

#add_subdirectory(Basic)
#add_subdirectory(Lex)
#add_subdirectory(Parser)
#add_subdirectory(AST)
#add_subdirectory(CodeGen)
#add_subdirectory(Sema)
#add_subdirectory(Driver)
#add_subdirectory(Frontend)

#target_link_libraries(fly
#        FlyBasic
#        FlyLex
#        FlyParser
#        FlyAST
#        FlyCodeGen
#        FlySema
#        FlyDriver
#        FlyFrontend
#)

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