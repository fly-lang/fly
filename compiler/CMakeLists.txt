add_executable(fly
        Fly.cpp
)

add_library(FlyCompiler

        # AST
        AST/ASTArg.cpp
        AST/ASTAttribute.cpp
        AST/ASTBase.cpp
        AST/ASTBuilder.cpp
        AST/ASTBuilderStmt.cpp
        AST/ASTBuilderIfStmt.cpp
        AST/ASTBuilderSwitchStmt.cpp
        AST/ASTBuilderLoopStmt.cpp
        AST/ASTBuilderLoopInStmt.cpp
        AST/ASTDeclStmt.cpp
        AST/ASTNode.cpp
        AST/ASTBlockStmt.cpp
        AST/ASTBreakStmt.cpp
        AST/ASTCall.cpp
        AST/ASTCast.cpp
        AST/ASTClass.cpp
        AST/ASTCaseStmt.cpp
        AST/ASTTestStmt.cpp
        AST/ASTComment.cpp
        AST/ASTContinueStmt.cpp
        AST/ASTDeleteStmt.cpp
        AST/ASTEnum.cpp
        AST/ASTEnumEntry.cpp
        AST/ASTExpr.cpp
        AST/ASTExprStmt.cpp
        AST/ASTFailStmt.cpp
        AST/ASTFunction.cpp
        AST/ASTHandleStmt.cpp
        AST/ASTIdentifier.cpp
        AST/ASTType.cpp
        AST/ASTIfStmt.cpp
        AST/ASTImport.cpp
        AST/ASTLocalVar.cpp
        AST/ASTLoopStmt.cpp
        AST/ASTLoopInStmt.cpp
        AST/ASTMethod.cpp
        AST/ASTName.cpp
        AST/ASTNameSpace.cpp
        AST/ASTModule.cpp
        AST/ASTMember.cpp
        AST/ASTUnary.cpp
        AST/ASTBinary.cpp
        AST/ASTTernary.cpp
        AST/ASTParam.cpp
        AST/ASTReturnStmt.cpp
        AST/ASTRuleStmt.cpp
        AST/ASTModifier.cpp
        AST/ASTStmt.cpp
        AST/ASTSwitchStmt.cpp
        AST/ASTValue.cpp
        AST/ASTVar.cpp

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
        CodeGen/CodeGenArrayValue.cpp
        CodeGen/CodeGenClass.cpp
        CodeGen/CodeGenClassMethod.cpp
        CodeGen/CodeGenEnum.cpp
        CodeGen/CodeGenEnumEntry.cpp
        CodeGen/CodeGenExpr.cpp
        CodeGen/CodeGenStdLibCLang.cpp
        CodeGen/CodeGenStdLibLLVM.cpp
        CodeGen/CodeGenStdLibRuntime.cpp
        CodeGen/CodeGenError.cpp
        CodeGen/CodeGenFunction.cpp
        CodeGen/CodeGenFunctionBase.cpp
        CodeGen/CodeGenHandle.cpp
        CodeGen/CodeGenModule.cpp
        CodeGen/CodeGenType.cpp
        CodeGen/CodeGenVar.cpp
        CodeGen/CodeGenHelper.cpp

        # Driver
        Driver/Driver.cpp
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
        Sema/SemaContext.cpp
        Sema/SemaBuilder.cpp
        Sema/SemaBuilderModifiers.cpp
        Sema/SemaBuiltin.cpp
        Sema/SemaValidator.cpp
        Sema/SemaBinary.cpp
        Sema/SemaCall.cpp
        Sema/SemaCast.cpp
        Sema/SemaClassType.cpp
        Sema/SemaComment.cpp
        Sema/SemaClassAttribute.cpp
        Sema/SemaClassInstance.cpp
        Sema/SemaClassMethod.cpp
        Sema/SemaEnumType.cpp
        Sema/SemaEnumEntry.cpp
        Sema/SemaEnumList.cpp
        Sema/SemaErrorHandler.cpp
        Sema/SemaFunction.cpp
        Sema/SemaFunctionBase.cpp
        Sema/SemaImport.cpp
        Sema/SemaLocalVar.cpp
        Sema/SemaMember.cpp
        Sema/SemaModule.cpp
        Sema/SemaNameSpace.cpp
        Sema/SemaNode.cpp
        Sema/SemaParam.cpp
        Sema/SemaExpr.cpp
        Sema/Symbol.cpp
        Sema/SymbolTable.cpp
        Sema/SemaTernary.cpp
        Sema/SemaType.cpp
        Sema/SemaUnary.cpp
        Sema/SemaAlloc.cpp
        Sema/SemaSmartAlloc.cpp
        Sema/SemaStringAlloc.cpp

        Sema/SemaVar.cpp
        Sema/SemaValue.cpp
        Sema/SemaStmt.cpp
        Sema/SemaBlockStmt.cpp
        Sema/SemaDeclStmt.cpp
        Sema/SemaExprStmt.cpp
        Sema/SemaReturnStmt.cpp
        Sema/SemaIfStmt.cpp
        Sema/SemaSwitchStmt.cpp
        Sema/SemaLoopStmt.cpp
        Sema/SemaLoopInStmt.cpp
        Sema/SemaDeleteStmt.cpp
        Sema/SemaBreakStmt.cpp
        Sema/SemaContinueStmt.cpp
        Sema/SemaFailStmt.cpp
        Sema/SemaHandleStmt.cpp
        Sema/SemaTestStmt.cpp
        Sema/SemaCaseStmt.cpp
        Sema/Registry.cpp
        Sema/Resolver.cpp
)

#if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    target_compile_options(FlyCompiler PRIVATE -Wall)
#endif ()

include(${CMAKE_SOURCE_DIR}/cmake/CLI11.cmake)

configure_file(
        ${CMAKE_SOURCE_DIR}/include/Config/Config.h.cmake
        ${CMAKE_BINARY_DIR}/include/Config/Config.h)

# Each build profile (Debug/Release) writes its own Config.h into the build
# tree. Prepend it so it shadows any stale copy in the source tree.
target_include_directories(FlyCompiler BEFORE PUBLIC
        ${CMAKE_BINARY_DIR}/include)

foreach(target ${LLVM_TARGETS_TO_BUILD})
    list(APPEND targets "LLVM${target}CodeGen")
endforeach()

target_link_libraries(FlyCompiler
        CLI11::CLI11
        LLVMCore
        LLVMMC
        LLVMMCParser
        LLVMObject
        LLVMSupport
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
        lldMachO
        lldMinGW
        lldWasm)

target_link_libraries(fly
        FlyCompiler
        FlyLib
        FlyRuntime
)

set_target_properties(fly PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(TARGETS fly RUNTIME DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/std/lib/" DESTINATION lib
        FILES_MATCHING PATTERN "*.fly.h")