//===--------------------------------------------------------------------------------------------------------------===//
// test/AppTest.cpp - Full app compilation integration tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Driver/Driver.h"
#include "Driver/DriverOptions.h"
#include "AST/ASTBuilder.h"
#include "AST/ASTClass.h"
#include "AST/ASTEnum.h"
#include "AST/ASTEnumEntry.h"
#include "AST/ASTFunction.h"
#include "AST/ASTImport.h"
#include "AST/ASTModule.h"
#include "AST/ASTAttribute.h"
#include "AST/ASTParam.h"
#include "AST/ASTNameSpace.h"
#include "Frontend/InputFile.h"
#include "Parser/Parser.h"
#include "Sema/SemaContext.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Host.h"
#include "gtest/gtest.h"
#include <fstream>

extern bool DebugEnabled;

namespace {
    using namespace fly;

    // ─── Source strings (mirror the Sources/*.fly files) ─────────────────────

    // Library module: namespace utils with a public function
    static constexpr const char *UtilsAppSource = R"(
namespace utils

public add(int a, int b) {
    int r = a + b
}
)";

    // Main module: imports utils and exercises all language features in one file
    static constexpr const char *MainAppSource = R"(
import utils

public enum Color {
    RED, GREEN, BLUE
}

public interface Shape {
    area()
}

public struct Point {
    int x
    int y
}

public class Circle : Shape {
    int radius

    public area() {
        return 0
    }
}

main() {
    Color c = Color.RED
    Point p = new Point()
    p.x = 10
    p.y = 20
    Circle circle = new Circle()
}

helper(int a, int b) {
    int r = a + b
}
)";

    // ─── Driver-level fixture ─────────────────────────────────────────────────

    class AppTest : public ::testing::Test {
    public:
        const char *mainfly  = "main.fly";
        const char *utilsfly = "utils.fly";

        AppTest() {
            DebugEnabled = true;
            { std::ofstream f(mainfly);  f << MainAppSource; }
            { std::ofstream f(utilsfly); f << UtilsAppSource; }
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();
        }

        ~AppTest() override {
            remove(mainfly);
            remove(utilsfly);
            llvm::outs().flush();
        }

        static void deleteFile(const char *path) { remove(path); }
    };

    // ─── Driver: meta options ─────────────────────────────────────────────────

    TEST_F(AppTest, ShowVersion) {
        const char *argv[] = {"fly", "-version"};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    TEST_F(AppTest, NoOutput) {
        const char *argv[] = {"fly", "-no-output", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        EXPECT_TRUE(drv.Execute());
    }

    // ─── Driver: emit formats ─────────────────────────────────────────────────

    TEST_F(AppTest, EmitLL) {
        deleteFile("main.fly.ll");
        deleteFile("utils.fly.ll");

        const char *argv[] = {"fly", "-emit-ll", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.ll").good());
        EXPECT_TRUE(std::ifstream("utils.fly.ll").good());

        deleteFile("main.fly.ll");
        deleteFile("utils.fly.ll");
    }

    TEST_F(AppTest, EmitBC) {
        deleteFile("main.fly.bc");
        deleteFile("utils.fly.bc");

        const char *argv[] = {"fly", "-emit-bc", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.bc").good());
        EXPECT_TRUE(std::ifstream("utils.fly.bc").good());

        deleteFile("main.fly.bc");
        deleteFile("utils.fly.bc");
    }

    TEST_F(AppTest, EmitAS) {
        deleteFile("main.fly.s");
        deleteFile("utils.fly.s");

        const char *argv[] = {"fly", "-emit-as", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.s").good());
        EXPECT_TRUE(std::ifstream("utils.fly.s").good());

        deleteFile("main.fly.s");
        deleteFile("utils.fly.s");
    }

    TEST_F(AppTest, EmitObj) {
        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");

        const char *argv[] = {"fly", mainfly, utilsfly};
        Driver drv(argv);
        drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        EXPECT_TRUE(std::ifstream("main.fly.o").good());
        EXPECT_TRUE(std::ifstream("utils.fly.o").good());

        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");
    }

    TEST_F(AppTest, EmitOut) {
        deleteFile("out");
        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");

        const char *argv[] = {"fly", mainfly, utilsfly, "-o", "out"};
        Driver drv(argv);
        CompilerInstance &CI = drv.BuildCompilerInstance();
        ASSERT_TRUE(drv.Execute());

        const llvm::Triple &T = TargetInfo::CreateTargetInfo(
            CI.getDiagnostics(), CI.getTargetOptions())->getTriple();
        if (T.isWindowsMSVCEnvironment()) {
            EXPECT_TRUE(std::ifstream("out.exe").good());
            deleteFile("out.exe");
        } else {
            EXPECT_TRUE(std::ifstream("out").good());
            deleteFile("out");
        }

        deleteFile("main.fly.o");
        deleteFile("utils.fly.o");
    }

//     // ─── AST-level fixture ────────────────────────────────────────────────────
//
//     class AppASTTest : public ::testing::Test {
//     protected:
//         std::shared_ptr<CompilerInstance> CI;
//         DiagnosticsEngine &Diags;
//         ASTBuilder *Builder;
//         llvm::SmallVector<ASTModule *, 8> ASTModules;
//
//         AppASTTest()
//             : CI(TestUtils::CreateCompilerInstance()),
//               Diags(CI->getDiagnostics()),
//               Builder(new ASTBuilder(CI->getDiagnostics())) {
//             CI->getTargetOptions()->Triple =
//                 llvm::Triple::normalize(llvm::sys::getProcessTriple());
//             CI->getTargetOptions()->CodeModel = "default";
//         }
//
//         ~AppASTTest() override { delete Builder; }
//
//         ASTModule *Parse(const std::string &name, llvm::StringRef source) {
//             Diags.getClient()->BeginSourceFile();
//             auto *FID = new InputFile(Diags, CI->getSourceManager(), name);
//             FID->Load(source);
//             Parser *P = new Parser(FID, CI->getSourceManager(), Diags, *Builder);
//             ASTModule *M = P->ParseModule();
//             Diags.getClient()->EndSourceFile();
//             ASTModules.push_back(M);
//             delete P;
//             return M;
//         }
//
//         bool Resolve() {
//             SemaContext S(Diags);
//             S.Resolve(ASTModules);
//             return !Diags.hasErrorOccurred();
//         }
//
//         void TearDown() override {
//             ASTModules.clear();
//             Diags.Reset();
//         }
//
//         // Return first module-level node matching kind and name
//         template <typename T>
//         static T *FindNode(ASTModule *M, ASTKind kind, llvm::StringRef name) {
//             for (auto *N : M->getNodes())
//                 if (N->getKind() == kind &&
//                     static_cast<T *>(N)->getName() == name)
//                     return static_cast<T *>(N);
//             return nullptr;
//         }
//
//         // Return first ASTClass matching ClassKind and name
//         static ASTClass *FindClass(ASTModule *M, ASTClassKind ck, llvm::StringRef name) {
//             for (auto *N : M->getNodes()) {
//                 if (N->getKind() == ASTKind::AST_CLASS) {
//                     auto *C = static_cast<ASTClass *>(N);
//                     if (C->getClassKind() == ck && C->getName() == name)
//                         return C;
//                 }
//             }
//             return nullptr;
//         }
//
//         template <typename T>
//         static T *As(ASTNode *N) { return static_cast<T *>(N); }
//     };
//
//     // ─── Import ───────────────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseImport) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTImport *Imp = nullptr;
//         for (auto *N : M->getNodes())
//             if (N->getKind() == ASTKind::AST_IMPORT) {
//                 Imp = static_cast<ASTImport *>(N);
//                 break;
//             }
//         ASSERT_NE(Imp, nullptr);
//         ASSERT_EQ(Imp->getNames().size(), 1u);
//         EXPECT_EQ(Imp->getNames()[0]->getName(), "utils");
//     }
//
//     // ─── Enum ─────────────────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseEnum) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTEnum *E = FindNode<ASTEnum>(M, ASTKind::AST_ENUM, "Color");
//         ASSERT_NE(E, nullptr);
//         ASSERT_EQ(E->getNodes().size(), 3u);
//         EXPECT_EQ(As<ASTEnumEntry>(E->getNodes()[0])->getName(), "RED");
//         EXPECT_EQ(As<ASTEnumEntry>(E->getNodes()[1])->getName(), "GREEN");
//         EXPECT_EQ(As<ASTEnumEntry>(E->getNodes()[2])->getName(), "BLUE");
//     }
//
//     TEST_F(AppASTTest, ParseEnumEntriesIndexed) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTEnum *E = FindNode<ASTEnum>(M, ASTKind::AST_ENUM, "Color");
//         ASSERT_NE(E, nullptr);
//         EXPECT_EQ(As<ASTEnumEntry>(E->getNodes()[0])->getIndex(), 1u);
//         EXPECT_EQ(As<ASTEnumEntry>(E->getNodes()[1])->getIndex(), 2u);
//         EXPECT_EQ(As<ASTEnumEntry>(E->getNodes()[2])->getIndex(), 3u);
//     }
//
//     // ─── Interface ────────────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseInterface) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTClass *Iface = FindClass(M, ASTClassKind::INTERFACE, "Shape");
//         ASSERT_NE(Iface, nullptr);
//         ASSERT_EQ(Iface->getNodes().size(), 1u);
//         EXPECT_EQ(As<ASTFunction>(Iface->getNodes()[0])->getName(), "area");
//     }
//
//     // ─── Struct ───────────────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseStruct) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTClass *S = FindClass(M, ASTClassKind::STRUCT, "Point");
//         ASSERT_NE(S, nullptr);
//         ASSERT_EQ(S->getNodes().size(), 2u);
//         EXPECT_EQ(As<ASTAttribute>(S->getNodes()[0])->getName(), "x");
//         EXPECT_EQ(As<ASTAttribute>(S->getNodes()[1])->getName(), "y");
//     }
//
//     // ─── Class ────────────────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseClass) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTClass *C = FindClass(M, ASTClassKind::CLASS, "Circle");
//         ASSERT_NE(C, nullptr);
//         // Circle has: int radius (attribute) + area() (method) = 2 nodes
//         EXPECT_EQ(C->getNodes().size(), 2u);
//     }
//
//     TEST_F(AppASTTest, ParseClassInheritsInterface) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTClass *C = FindClass(M, ASTClassKind::CLASS, "Circle");
//         ASSERT_NE(C, nullptr);
//         EXPECT_EQ(C->getBases().size(), 1u);
//     }
//
//     // ─── Functions ────────────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseMainFunction) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTFunction *F = FindNode<ASTFunction>(M, ASTKind::AST_FUNCTION, "main");
//         ASSERT_NE(F, nullptr);
//         EXPECT_TRUE(F->getParams().empty());
//     }
//
//     TEST_F(AppASTTest, ParseHelperFunction) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTFunction *F = FindNode<ASTFunction>(M, ASTKind::AST_FUNCTION, "helper");
//         ASSERT_NE(F, nullptr);
//         ASSERT_EQ(F->getParams().size(), 2u);
//         EXPECT_EQ(F->getParams()[0]->getName(), "a");
//         EXPECT_EQ(F->getParams()[1]->getName(), "b");
//     }
//
//     // ─── Library module ───────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseLibNamespace) {
//         ASTModule *M = Parse("utils", UtilsAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASSERT_NE(M->getNameSpace(), nullptr);
//         ASSERT_FALSE(M->getNameSpace()->getNames().empty());
//         EXPECT_EQ(M->getNameSpace()->getNames()[0]->getName(), "utils");
//     }
//
//     TEST_F(AppASTTest, ParseLibFunction) {
//         ASTModule *M = Parse("utils", UtilsAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         ASTFunction *F = FindNode<ASTFunction>(M, ASTKind::AST_FUNCTION, "add");
//         ASSERT_NE(F, nullptr);
//         ASSERT_EQ(F->getParams().size(), 2u);
//         EXPECT_EQ(F->getParams()[0]->getName(), "a");
//         EXPECT_EQ(F->getParams()[1]->getName(), "b");
//     }
//
//     // ─── Full node count ──────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, ParseAppNodeCount) {
//         ASTModule *M = Parse("main", MainAppSource);
//         ASSERT_FALSE(Diags.hasErrorOccurred());
//
//         // import + enum + interface + struct + class + main() + helper()
//         EXPECT_EQ(M->getNodes().size(), 7u);
//     }
//
//     // ─── Sema tests ───────────────────────────────────────────────────────────
//
//     TEST_F(AppASTTest, SemaFunctions) {
//         Parse("funcs", R"(
// add(int a, int b) {
//     int r = a + b
// }
//
// main() {
//     add(1, 2)
// }
// )");
//         EXPECT_TRUE(Resolve());
//     }
//
//     TEST_F(AppASTTest, SemaEnum) {
//         Parse("enum_test", R"(
// public enum Color {
//     RED, GREEN, BLUE
// }
//
// main() {
//     Color c = Color.RED
// }
// )");
//         EXPECT_TRUE(Resolve());
//     }
//
//     TEST_F(AppASTTest, SemaStruct) {
//         Parse("struct_test", R"(
// public struct Point {
//     int x
//     int y
// }
//
// main() {
//     Point p = new Point()
//     p.x = 1
//     p.y = 2
//     delete p
// }
// )");
//         EXPECT_TRUE(Resolve());
//     }
//
//     TEST_F(AppASTTest, SemaClass) {
//         Parse("class_test", R"(
// public class Counter {
//     int value
//
//     public increment() {
//         value = value + 1
//     }
// }
//
// main() {
//     Counter c = new Counter()
//     c.increment()
//     delete c
// }
// )");
//         EXPECT_TRUE(Resolve());
//     }
//
//     TEST_F(AppASTTest, SemaInterfaceWithImpl) {
//         Parse("iface_test", R"(
// public interface Shape {
//     area()
// }
//
// public class Circle : Shape {
//     int radius
//
//     public area() {
//         return 0
//     }
// }
//
// main() {
//     Circle c = new Circle()
//     delete c
// }
// )");
//         EXPECT_TRUE(Resolve());
//     }
//
//     TEST_F(AppASTTest, SemaWithImport) {
//         Parse("utils", UtilsAppSource);
//         Parse("main", R"(
// import utils
//
// main() {
//     utils.add(1, 2)
// }
// )");
//         EXPECT_TRUE(Resolve());
//     }
//
//     TEST_F(AppASTTest, SemaFullApp) {
//         Parse("utils", UtilsAppSource);
//         Parse("main", MainAppSource);
//         EXPECT_TRUE(Resolve());
//     }

} // anonymous namespace
