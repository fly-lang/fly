#include "Lex/Lexer.h"
#include "Basic/Diagnostic.h"
// #include "AST/ASTConsumer.h"
// #include "AST/ASTContext.h"
// #include "Frontend/CompilerInstance.h"
// #include "Frontend/CompilerInvocation.h"
// #include "Frontend/FrontendAction.h"
// #include "Frontend/FrontendActions.h"
// #include "AST/ASTConsumer.h"
// #include "Lex/Preprocessor.h"
// #include "Lex/PreprocessorOptions.h"
// #include "Sema/Sema.h"
// #include "Serialization/InMemoryModuleCache.h"
#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

using namespace fly;
using namespace llvm;

// namespace
// {
//     class TestASTFrontendAction : public ASTFrontendAction
//     {
//         std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
//                                                        StringRef InFile) override
//         {
//             return std::make_unique<ASTConsumer>();
//         }
//     };

// } // namespace

int main()
{
    // auto invocation = std::make_shared<CompilerInvocation>();
    // invocation->getPreprocessorOpts().addRemappedFile(
    //     "test.cc",
    //     MemoryBuffer::getMemBuffer("int main() { float x; }").release());
    // invocation->getFrontendOpts().Inputs.push_back(
    //     FrontendInputFile("test.cc", Language::CXX));
    // invocation->getFrontendOpts().ProgramAction = frontend::ParseSyntaxOnly;
    // invocation->getTargetOpts().Triple = "i386-unknown-linux-gnu";
    // CompilerInstance compiler;
    // compiler.setInvocation(std::move(invocation));
    // compiler.createDiagnostics();

    // TestASTFrontendAction test_action;
    // compiler.ExecuteAction(test_action);
    // std::cout << "Fly";
    return 0;
}