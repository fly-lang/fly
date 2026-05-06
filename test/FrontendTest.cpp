//===--------------------------------------------------------------------------------------------------------------===//
// test/FrontendTest.cpp - Frontend and FrontendOptions tests
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "TestUtils.h"
#include "Basic/Archiver.h"
#include "Frontend/Frontend.h"
#include "Frontend/FrontendOptions.h"
#include "Frontend/InputFile.h"
#include "Frontend/CompilerInstance.h"
#include "CodeGen/BackendUtil.h"
#include "llvm/Support/FileSystem.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

namespace {
    using namespace fly;

    // ─── FrontendOptions ─────────────────────────────────────────────────────

    class FrontendOptionsTest : public ::testing::Test {};

    TEST_F(FrontendOptionsTest, DefaultValues) {
        FrontendOptions FO;
        EXPECT_TRUE(FO.getInputFiles().empty());
        EXPECT_TRUE(FO.getOutputFile().empty());
        EXPECT_FALSE(FO.isOutputLib());
        EXPECT_FALSE(FO.CreateLibrary);
        EXPECT_FALSE(FO.CreateHeader);
        EXPECT_FALSE(FO.Verbose);
        EXPECT_FALSE(FO.ShowVersion);
        EXPECT_FALSE(FO.ShowHelp);
        EXPECT_FALSE(FO.ShowStats);
        EXPECT_FALSE(FO.ShowTimers);
        EXPECT_TRUE(FO.StatsFile.empty());
    }

    TEST_F(FrontendOptionsTest, AddInputFile) {
        FrontendOptions FO;
        FO.addInputFile("a.fly");
        FO.addInputFile("b.fly");
        ASSERT_EQ(FO.getInputFiles().size(), 2u);
        EXPECT_EQ(FO.getInputFiles()[0], "a.fly");
        EXPECT_EQ(FO.getInputFiles()[1], "b.fly");
    }

    TEST_F(FrontendOptionsTest, SetOutputFile) {
        FrontendOptions FO;
        FO.setOutputFile("out.o");
        EXPECT_EQ(FO.getOutputFile(), "out.o");
        EXPECT_FALSE(FO.isOutputLib());
    }

    TEST_F(FrontendOptionsTest, SetOutputFileLib) {
        FrontendOptions FO;
        FO.setOutputFile("out.a", /*isLib=*/true);
        EXPECT_EQ(FO.getOutputFile(), "out.a");
        EXPECT_TRUE(FO.isOutputLib());
    }

    TEST_F(FrontendOptionsTest, SetOutputFileEmpty) {
        FrontendOptions FO;
        FO.setOutputFile("");
        EXPECT_TRUE(FO.getOutputFile().empty());
        EXPECT_FALSE(FO.isOutputLib());
    }

    TEST_F(FrontendOptionsTest, BackendActionDefault) {
        FrontendOptions FO;
        // BackendAction has no in-class initializer; Driver sets it to Backend_EmitObj.
        // Here we just verify it can be set and read back.
        FO.BackendAction = BackendActionKind::Backend_EmitObj;
        EXPECT_EQ(FO.BackendAction, BackendActionKind::Backend_EmitObj);
    }

    TEST_F(FrontendOptionsTest, AllBackendActions) {
        FrontendOptions FO;
        FO.BackendAction = BackendActionKind::Backend_EmitLL;
        EXPECT_EQ(FO.BackendAction, BackendActionKind::Backend_EmitLL);
        FO.BackendAction = BackendActionKind::Backend_EmitBC;
        EXPECT_EQ(FO.BackendAction, BackendActionKind::Backend_EmitBC);
        FO.BackendAction = BackendActionKind::Backend_EmitAssembly;
        EXPECT_EQ(FO.BackendAction, BackendActionKind::Backend_EmitAssembly);
        FO.BackendAction = BackendActionKind::Backend_EmitNothing;
        EXPECT_EQ(FO.BackendAction, BackendActionKind::Backend_EmitNothing);
    }

    TEST_F(FrontendOptionsTest, FlagFields) {
        FrontendOptions FO;
        FO.CreateLibrary = true;
        FO.CreateHeader  = true;
        FO.Verbose       = true;
        FO.ShowStats     = true;
        FO.ShowTimers    = true;
        FO.StatsFile     = "stats.json";
        EXPECT_TRUE(FO.CreateLibrary);
        EXPECT_TRUE(FO.CreateHeader);
        EXPECT_TRUE(FO.Verbose);
        EXPECT_TRUE(FO.ShowStats);
        EXPECT_TRUE(FO.ShowTimers);
        EXPECT_EQ(FO.StatsFile, "stats.json");
    }

    // ─── InputFile ────────────────────────────────────────────────────────────

    class InputFileTest : public ::testing::Test {
    protected:
        std::shared_ptr<CompilerInstance> CI;
        DiagnosticsEngine &Diags;
        SourceManager &SourceMgr;

        InputFileTest()
            : CI(TestUtils::CreateCompilerInstance()),
              Diags(CI->getDiagnostics()),
              SourceMgr(CI->getSourceManager()) {}
    };

    TEST_F(InputFileTest, ExtFly) {
        InputFile F(Diags, SourceMgr, "main.fly");
        EXPECT_EQ(F.getExt(), FileExt::FLY);
        EXPECT_EQ(F.getName(), "main");
        EXPECT_EQ(F.getFileName(), "main.fly");
    }

    TEST_F(InputFileTest, ExtLib) {
        InputFile F(Diags, SourceMgr, "mylib.lib");
        EXPECT_EQ(F.getExt(), FileExt::LIB);
        EXPECT_EQ(F.getName(), "mylib");
    }

    TEST_F(InputFileTest, ExtObj) {
        InputFile F(Diags, SourceMgr, "foo.o");
        EXPECT_EQ(F.getExt(), FileExt::OBJ);
    }

    TEST_F(InputFileTest, ExtUnknown) {
        InputFile F(Diags, SourceMgr, "readme.txt");
        EXPECT_EQ(F.getExt(), FileExt::UNKNOWN);
    }

    TEST_F(InputFileTest, IsEmptyFalse) {
        InputFile F(Diags, SourceMgr, "main.fly");
        EXPECT_FALSE(F.isEmpty());
    }

    TEST_F(InputFileTest, IsBufferFalseBeforeLoad) {
        InputFile F(Diags, SourceMgr, "main.fly");
        EXPECT_FALSE(F.isBuffer());
    }

    TEST_F(InputFileTest, LoadFromSource) {
        InputFile F(Diags, SourceMgr, "test");
        bool ok = F.Load("func main() {}\n");
        EXPECT_TRUE(ok);
        EXPECT_TRUE(F.isBuffer());
        EXPECT_TRUE(F.getFileID().isValid());
        EXPECT_NE(F.getBuffer(), nullptr);
    }

    TEST_F(InputFileTest, LoadFromSourceContent) {
        InputFile F(Diags, SourceMgr, "test");
        llvm::StringRef src = "int x = 1;\n";
        F.Load(src);
        llvm::StringRef buf = F.getBuffer()->getBuffer();
        EXPECT_EQ(buf, src);
    }

    TEST_F(InputFileTest, LoadFromDiskMissingFile) {
        InputFile F(Diags, SourceMgr, "nonexistent_file_xyz.fly");
        bool ok = F.Load();
        EXPECT_FALSE(ok);
    }

    TEST_F(InputFileTest, LoadFromDiskExistingFile) {
        // Write a temporary .fly file and load it
        const char *path = "tmp_input_test.fly";
        {
            std::ofstream ofs(path);
            ofs << "func main() {}\n";
        }
        InputFile F(Diags, SourceMgr, path);
        bool ok = F.Load();
        EXPECT_TRUE(ok);
        EXPECT_TRUE(F.isBuffer());
        EXPECT_TRUE(F.getFileID().isValid());
        remove(path);
    }

    // ─── Frontend::Execute ────────────────────────────────────────────────────

    class FrontendTest : public ::testing::Test {
    protected:
        std::shared_ptr<CompilerInstance> CI;

        FrontendTest() : CI(TestUtils::CreateCompilerInstance()) {
            CI->getTargetOptions()->Triple =
                llvm::Triple::normalize(llvm::sys::getProcessTriple());
            CI->getTargetOptions()->CodeModel = "default";
        }
    };

    TEST_F(FrontendTest, ExecuteNoInputFiles) {
        CI->getFrontendOptions().BackendAction = BackendActionKind::Backend_EmitNothing;
        Frontend Front(*CI);
        bool result = Front.Execute();
        EXPECT_FALSE(result);
    }

    TEST_F(FrontendTest, ExecuteWithEmptyFlyFile) {
        const char *path = "tmp_frontend_empty.fly";
        {
            std::ofstream ofs(path);
            ofs << "";
        }
        CI->getFrontendOptions().addInputFile(path);
        CI->getFrontendOptions().BackendAction = BackendActionKind::Backend_EmitNothing;
        Frontend Front(*CI);
        bool result = Front.Execute();
        EXPECT_TRUE(result);
        remove(path);
    }

    TEST_F(FrontendTest, ExecuteWithValidFlyFile) {
        const char *path = "tmp_frontend_valid.fly";
        {
            std::ofstream ofs(path);
            ofs << "main() {}\n";
        }
        CI->getFrontendOptions().addInputFile(path);
        CI->getFrontendOptions().BackendAction = BackendActionKind::Backend_EmitNothing;
        Frontend Front(*CI);
        bool result = Front.Execute();
        EXPECT_TRUE(result);
        remove(path);
    }

    TEST_F(FrontendTest, ExecuteWithUnknownExtension) {
        const char *path = "tmp_frontend_test.txt";
        {
            std::ofstream ofs(path);
            ofs << "some content\n";
        }
        CI->getFrontendOptions().addInputFile(path);
        CI->getFrontendOptions().BackendAction = BackendActionKind::Backend_EmitNothing;
        Frontend Front(*CI);
        bool result = Front.Execute();
        // Unknown extension triggers a diagnostic error → Execute returns false
        EXPECT_FALSE(result);
        remove(path);
    }

    TEST_F(FrontendTest, GetOutputFilesEmpty) {
        Frontend Front(*CI);
        EXPECT_TRUE(Front.getOutputFiles().empty());
    }

    // ─── Archiver::ExtractLib (defined in Frontend.cpp) ──────────────────────

    TEST_F(FrontendTest, ExtractFiles_MissingArchive) {
        std::string missing = (std::filesystem::temp_directory_path()
                               / "fly_nonexistent_archive_xyz.a").string();
        Frontend Front(*CI);
        std::vector<llvm::StringRef> files = Front.ExtractFiles(missing);
        EXPECT_TRUE(files.empty());
    }

    TEST_F(FrontendTest, ExtractFiles_InvalidArchive) {
        std::string path = (std::filesystem::temp_directory_path()
                            / "fly_invalid_archive.a").string();
        { std::ofstream f(path); f << "not an archive"; }

        Frontend Front(*CI);
        std::vector<llvm::StringRef> files = Front.ExtractFiles(path);
        EXPECT_TRUE(files.empty());

        remove(path.c_str());
    }

    TEST_F(FrontendTest, ExtractFiles_ValidArchive) {
        auto tmpDir    = std::filesystem::temp_directory_path();
        std::string member = (tmpDir / "fly_ext_member.o").string();
        std::string arc    = (tmpDir / "fly_ext_archive.a").string();

        { std::ofstream f(member); f << "member content"; }
        remove(arc.c_str());

        // Build archive via CreateLib (defined in ToolChain.cpp)
        {
            Archiver ar(CI->getDiagnostics(), arc);
            llvm::SmallVector<std::string, 4> files = {member};
            ASSERT_TRUE(ar.CreateLib(files));
        }

        Frontend Front(*CI);
        std::vector<llvm::StringRef> extracted = Front.ExtractFiles(arc);
        EXPECT_FALSE(extracted.empty());

        // Clean up extracted files
        for (auto &f : extracted)
            remove(std::string(f).c_str());

        remove(member.c_str());
        remove(arc.c_str());
    }

} // anonymous namespace
