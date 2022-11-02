//===--------------------------------------------------------------------------------------------------------------===//
// src/Driver/ToolChain.cpp - ToolChain
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//

#include "Frontend/FrontendOptions.h"
#include "Driver/ToolChain.h"
#include "Basic/Archiver.h"
#include "Config/Config.h"
#include "Basic/Debug.h"

#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/ConvertUTF.h"
#include "lld/Common/Driver.h"
#include "lld/Core/Writer.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
  #define NOGDI
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#endif

#ifdef _MSC_VER
// Don't support SetupApi on MinGW.
#define USE_MSVC_SETUP_API

// Make sure this comes before MSVCSetupApi.h
#include <comdef.h>

#include "MSVCSetupApi.h"
#include "llvm/Support/COM.h"
_COM_SMARTPTR_TYPEDEF(ISetupConfiguration, __uuidof(ISetupConfiguration));
_COM_SMARTPTR_TYPEDEF(ISetupConfiguration2, __uuidof(ISetupConfiguration2));
_COM_SMARTPTR_TYPEDEF(ISetupHelper, __uuidof(ISetupHelper));
_COM_SMARTPTR_TYPEDEF(IEnumSetupInstances, __uuidof(IEnumSetupInstances));
_COM_SMARTPTR_TYPEDEF(ISetupInstance, __uuidof(ISetupInstance));
_COM_SMARTPTR_TYPEDEF(ISetupInstance2, __uuidof(ISetupInstance2));
#endif



using namespace fly;

ToolChain::ToolChain(DiagnosticsEngine &Diag, const llvm::Triple &T, CodeGenOptions &CodeGenOpts) :
        Diag(Diag), T(T), CodeGenOpts(CodeGenOpts) {
    VFS = this->VFS = llvm::vfs::getRealFileSystem();

}

/**
 * Read /lib/base
 * @return
 */
bool BuildLib() {

    return false;
}

bool ToolChain::BuildOutput(const llvm::SmallVector<std::string, 4> &InFiles, FrontendOptions &FrontendOpts) {
    llvm::SmallVector<const char *, 4> Args = {"fly"};
    const std::string OutFileName = FrontendOpts.getOutputFile();

    // Select right options format by platform (Win or others)
    FLY_DEBUG_MESSAGE("ToolChain", "Link", "Output=" << OutFileName);
    if (FrontendOpts.CreateLibrary) {
        Archiver Library(Diag, OutFileName + ".lib");
        return Library.CreateLib(InFiles);
    } else {
        if (T.isWindowsMSVCEnvironment()) {
            return LinkWindows(InFiles, OutFileName);
        } else if (T.isOSDarwin()) {
            return LinkDarwin(InFiles, OutFileName);
        } else {
            return LinkLinux(InFiles, OutFileName);
        }
    }

    assert(0 && "Unknown Object Format");
}

void createLinkArgs(SmallVector<std::string, 16> &InArgs, SmallVector<const char*, 16> &OutArgs) {
    // Log arguments and generate link arguments
    std::string ArgStr;
    for (auto& A : InArgs) {
        ArgStr.append(A).append(" ");
        OutArgs.push_back(A.c_str());
    }
    FLY_DEBUG_MESSAGE("ToolChain", "createLinkArgs", "Args: " + ArgStr);
}

// Check various environment variables to try and find a toolchain.
static bool findVCToolChainViaEnvironment(std::string &Path, bool &isLegacyVersion) {
    // These variables are typically set by vcvarsall.bat
    // when launching a developer command prompt.
    if (llvm::Optional<std::string> VCToolsInstallDir = llvm::sys::Process::GetEnv("VCToolsInstallDir")) {
        // This is only set by newer Visual Studios, and it leads straight to
        // the toolchain directory.
        Path = std::move(*VCToolsInstallDir);
        isLegacyVersion = false;
        return true;
    }
    if (llvm::Optional<std::string> VCInstallDir = llvm::sys::Process::GetEnv("VCINSTALLDIR")) {
        // If the previous variable isn't set but this one is, then we've found
        // an older Visual Studio. This variable is set by newer Visual Studios too,
        // so this check has to appear second.
        // In older Visual Studios, the VC directory is the toolchain.
        Path = std::move(*VCInstallDir);
        isLegacyVersion = true;
        return true;
    }

    // We couldn't find any VC environment variables. Let's walk through PATH and
    // see if it leads us to a VC toolchain bin directory. If it does, pick the
    // first one that we find.
    if (llvm::Optional<std::string> PathEnv = llvm::sys::Process::GetEnv("PATH")) {
        llvm::SmallVector<llvm::StringRef, 8> PathEntries;
        llvm::StringRef(*PathEnv).split(PathEntries, llvm::sys::EnvPathSeparator);
        for (llvm::StringRef PathEntry : PathEntries) {
            if (PathEntry.empty())
                continue;

            llvm::SmallString<256> ExeTestPath;

            // If cl.exe doesn't exist, then this definitely isn't a VC toolchain.
            ExeTestPath = PathEntry;
            llvm::sys::path::append(ExeTestPath, "cl.exe");
            if (!llvm::sys::fs::exists(ExeTestPath))
                continue;

            // cl.exe existing isn't a conclusive test for a VC toolchain; clang also
            // has a cl.exe. So let's check for link.exe too.
            ExeTestPath = PathEntry;
            llvm::sys::path::append(ExeTestPath, "link.exe");
            if (!llvm::sys::fs::exists(ExeTestPath))
                continue;

            // whatever/VC/bin --> old toolchain, VC dir is toolchain dir.
            llvm::StringRef TestPath = PathEntry;
            bool IsBin = llvm::sys::path::filename(TestPath).equals_lower("bin");
            if (!IsBin) {
                // Strip any architecture subdir like "amd64".
                TestPath = llvm::sys::path::parent_path(TestPath);
                IsBin = llvm::sys::path::filename(TestPath).equals_lower("bin");
            }
            if (IsBin) {
                llvm::StringRef ParentPath = llvm::sys::path::parent_path(TestPath);
                llvm::StringRef ParentFilename = llvm::sys::path::filename(ParentPath);
                if (ParentFilename == "VC") {
                    Path = std::string(ParentPath);
                    isLegacyVersion = true;
                    return true;
                }
            } else {
                // This could be a new (>=VS2017) toolchain. If it is, we should find
                // path components with these prefixes when walking backwards through
                // the path.
                // Note: empty strings match anything.
                llvm::StringRef ExpectedPrefixes[] = {"",     "Host",  "bin", "",
                                                      "MSVC", "Tools", "VC"};

                auto It = llvm::sys::path::rbegin(PathEntry);
                auto End = llvm::sys::path::rend(PathEntry);
                for (llvm::StringRef Prefix : ExpectedPrefixes) {
                    if (It == End)
                        goto NotAToolChain;
                    if (!It->startswith(Prefix))
                        goto NotAToolChain;
                    ++It;
                }

                // We've found a new toolchain!
                // Back up 3 times (/bin/Host/arch) to get the root path.
                llvm::StringRef ToolChainPath(PathEntry);
                for (int i = 0; i < 3; ++i)
                    ToolChainPath = llvm::sys::path::parent_path(ToolChainPath);

                Path = std::string(ToolChainPath);
                isLegacyVersion = false;
                return true;
            }

            NotAToolChain:
            continue;
        }
    }
    return false;
}

#ifdef _WIN32
static bool readFullStringValue(HKEY hkey, const char *valueName,
                                std::string &value) {
  std::wstring WideValueName;
  if (!llvm::ConvertUTF8toWide(valueName, WideValueName))
    return false;

  DWORD result = 0;
  DWORD valueSize = 0;
  DWORD type = 0;
  // First just query for the required size.
  result = RegQueryValueExW(hkey, WideValueName.c_str(), NULL, &type, NULL,
                            &valueSize);
  if (result != ERROR_SUCCESS || type != REG_SZ || !valueSize)
    return false;
  std::vector<BYTE> buffer(valueSize);
  result = RegQueryValueExW(hkey, WideValueName.c_str(), NULL, NULL, &buffer[0],
                            &valueSize);
  if (result == ERROR_SUCCESS) {
    std::wstring WideValue(reinterpret_cast<const wchar_t *>(buffer.data()),
                           valueSize / sizeof(wchar_t));
    if (valueSize && WideValue.back() == L'\0') {
      WideValue.pop_back();
    }
    // The destination buffer must be empty as an invariant of the conversion
    // function; but this function is sometimes called in a loop that passes in
    // the same buffer, however. Simply clear it out so we can overwrite it.
    value.clear();
    return llvm::convertWideToUTF8(WideValue, value);
  }
  return false;
}
#endif

/// Read registry string.
/// This also supports a means to look for high-versioned keys by use
/// of a $VERSION placeholder in the key path.
/// $VERSION in the key path is a placeholder for the version number,
/// causing the highest value path to be searched for and used.
/// I.e. "SOFTWARE\\Microsoft\\VisualStudio\\$VERSION".
/// There can be additional characters in the component.  Only the numeric
/// characters are compared.  This function only searches HKLM.
static bool getSystemRegistryString(const char *keyPath, const char *valueName,
                                    std::string &value, std::string *phValue) {
#ifndef _WIN32
    return false;
#else
    HKEY hRootKey = HKEY_LOCAL_MACHINE;
  HKEY hKey = NULL;
  long lResult;
  bool returnValue = false;

  const char *placeHolder = strstr(keyPath, "$VERSION");
  std::string bestName;
  // If we have a $VERSION placeholder, do the highest-version search.
  if (placeHolder) {
    const char *keyEnd = placeHolder - 1;
    const char *nextKey = placeHolder;
    // Find end of previous key.
    while ((keyEnd > keyPath) && (*keyEnd != '\\'))
      keyEnd--;
    // Find end of key containing $VERSION.
    while (*nextKey && (*nextKey != '\\'))
      nextKey++;
    size_t partialKeyLength = keyEnd - keyPath;
    char partialKey[256];
    if (partialKeyLength >= sizeof(partialKey))
      partialKeyLength = sizeof(partialKey) - 1;
    strncpy(partialKey, keyPath, partialKeyLength);
    partialKey[partialKeyLength] = '\0';
    HKEY hTopKey = NULL;
    lResult = RegOpenKeyExA(hRootKey, partialKey, 0, KEY_READ | KEY_WOW64_32KEY,
                            &hTopKey);
    if (lResult == ERROR_SUCCESS) {
      char keyName[256];
      double bestValue = 0.0;
      DWORD index, size = sizeof(keyName) - 1;
      for (index = 0; RegEnumKeyExA(hTopKey, index, keyName, &size, NULL, NULL,
                                    NULL, NULL) == ERROR_SUCCESS;
           index++) {
        const char *sp = keyName;
        while (*sp && !isDigit(*sp))
          sp++;
        if (!*sp)
          continue;
        const char *ep = sp + 1;
        while (*ep && (isDigit(*ep) || (*ep == '.')))
          ep++;
        char numBuf[32];
        strncpy(numBuf, sp, sizeof(numBuf) - 1);
        numBuf[sizeof(numBuf) - 1] = '\0';
        double dvalue = strtod(numBuf, NULL);
        if (dvalue > bestValue) {
          // Test that InstallDir is indeed there before keeping this index.
          // Open the chosen key path remainder.
          bestName = keyName;
          // Append rest of key.
          bestName.append(nextKey);
          lResult = RegOpenKeyExA(hTopKey, bestName.c_str(), 0,
                                  KEY_READ | KEY_WOW64_32KEY, &hKey);
          if (lResult == ERROR_SUCCESS) {
            if (readFullStringValue(hKey, valueName, value)) {
              bestValue = dvalue;
              if (phValue)
                *phValue = bestName;
              returnValue = true;
            }
            RegCloseKey(hKey);
          }
        }
        size = sizeof(keyName) - 1;
      }
      RegCloseKey(hTopKey);
    }
  } else {
    lResult =
        RegOpenKeyExA(hRootKey, keyPath, 0, KEY_READ | KEY_WOW64_32KEY, &hKey);
    if (lResult == ERROR_SUCCESS) {
      if (readFullStringValue(hKey, valueName, value))
        returnValue = true;
      if (phValue)
        phValue->clear();
      RegCloseKey(hKey);
    }
  }
  return returnValue;
#endif // _WIN32
}

// Query the Setup Config server for installs, then pick the newest version
// and find its default VC toolchain.
// This is the preferred way to discover new Visual Studios, as they're no
// longer listed in the registry.
static bool findVCToolChainViaSetupConfig(std::string &Path, bool &isLegacyVersion) {
#if !defined(USE_MSVC_SETUP_API)
    return false;
#else
    // FIXME: This really should be done once in the top-level program's main
  // function, as it may have already been initialized with a different
  // threading model otherwise.
  llvm::sys::InitializeCOMRAII COM(llvm::sys::COMThreadingMode::SingleThreaded);
  HRESULT HR;

  // _com_ptr_t will throw a _com_error if a COM calls fail.
  // The LLVM coding standards forbid exception handling, so we'll have to
  // stop them from being thrown in the first place.
  // The destructor will put the regular error handler back when we leave
  // this scope.
  struct SuppressCOMErrorsRAII {
    static void __stdcall handler(HRESULT hr, IErrorInfo *perrinfo) {}

    SuppressCOMErrorsRAII() { _set_com_error_handler(handler); }

    ~SuppressCOMErrorsRAII() { _set_com_error_handler(_com_raise_error); }

  } COMErrorSuppressor;

  ISetupConfigurationPtr Query;
  HR = Query.CreateInstance(__uuidof(SetupConfiguration));
  if (FAILED(HR))
    return false;

  IEnumSetupInstancesPtr EnumInstances;
  HR = ISetupConfiguration2Ptr(Query)->EnumAllInstances(&EnumInstances);
  if (FAILED(HR))
    return false;

  ISetupInstancePtr Instance;
  HR = EnumInstances->Next(1, &Instance, nullptr);
  if (HR != S_OK)
    return false;

  ISetupInstancePtr NewestInstance;
  Optional<uint64_t> NewestVersionNum;
  do {
    bstr_t VersionString;
    uint64_t VersionNum;
    HR = Instance->GetInstallationVersion(VersionString.GetAddress());
    if (FAILED(HR))
      continue;
    HR = ISetupHelperPtr(Query)->ParseVersion(VersionString, &VersionNum);
    if (FAILED(HR))
      continue;
    if (!NewestVersionNum || (VersionNum > NewestVersionNum)) {
      NewestInstance = Instance;
      NewestVersionNum = VersionNum;
    }
  } while ((HR = EnumInstances->Next(1, &Instance, nullptr)) == S_OK);

  if (!NewestInstance)
    return false;

  bstr_t VCPathWide;
  HR = NewestInstance->ResolvePath(L"VC", VCPathWide.GetAddress());
  if (FAILED(HR))
    return false;

  std::string VCRootPath;
  llvm::convertWideToUTF8(std::wstring(VCPathWide), VCRootPath);

  llvm::SmallString<256> ToolsVersionFilePath(VCRootPath);
  llvm::sys::path::append(ToolsVersionFilePath, "Auxiliary", "Build",
                          "Microsoft.VCToolsVersion.default.txt");

  auto ToolsVersionFile = llvm::MemoryBuffer::getFile(ToolsVersionFilePath);
  if (!ToolsVersionFile)
    return false;

  llvm::SmallString<256> ToolchainPath(VCRootPath);
  llvm::sys::path::append(ToolchainPath, "Tools", "MSVC",
                          ToolsVersionFile->get()->getBuffer().rtrim());
  if (!llvm::sys::fs::is_directory(ToolchainPath))
    return false;

  Path = std::string(ToolchainPath.str());
  isLegacyVersion = false;
  return true;
#endif
}

// Look in the registry for Visual Studio installs, and use that to get
// a toolchain path. VS2017 and newer don't get added to the registry.
// So if we find something here, we know that it's an older version.
static bool findVCToolChainViaRegistry(std::string &Path, bool &isLegacyVersion) {
    std::string VSInstallPath;
    if (getSystemRegistryString(R"(SOFTWARE\Microsoft\VisualStudio\$VERSION)",
                                "InstallDir", VSInstallPath, nullptr) ||
        getSystemRegistryString(R"(SOFTWARE\Microsoft\VCExpress\$VERSION)",
                                "InstallDir", VSInstallPath, nullptr)) {
        if (!VSInstallPath.empty()) {
            llvm::SmallString<256> VCPath(llvm::StringRef(
                    VSInstallPath.c_str(), VSInstallPath.find(R"(\Common7\IDE)")));
            llvm::sys::path::append(VCPath, "VC");

            Path = std::string(VCPath.str());
            isLegacyVersion = true;
            return true;
        }
    }
    return false;
}

// Similar to the above function, but for Visual Studios before VS2017.
static const char *llvmArchToLegacyVCArch(llvm::Triple::ArchType Arch) {
    using ArchType = llvm::Triple::ArchType;
    switch (Arch) {
        case ArchType::x86:
            // x86 is default in legacy VC toolchains.
            // e.g. x86 libs are directly in /lib as opposed to /lib/x86.
            return "";
        case ArchType::x86_64:
            return "amd64";
        case ArchType::arm:
            return "arm";
        case ArchType::aarch64:
            return "arm64";
        default:
            return "";
    }
}

// Windows SDKs and VC Toolchains group their contents into subdirectories based
// on the target architecture. This function converts an llvm::Triple::ArchType
// to the corresponding subdirectory name.
static const char *llvmArchToWindowsSDKArch(llvm::Triple::ArchType Arch) {
    using ArchType = llvm::Triple::ArchType;
    switch (Arch) {
        case ArchType::x86:
            return "x86";
        case ArchType::x86_64:
            return "x64";
        case ArchType::arm:
            return "arm";
        case ArchType::aarch64:
            return "arm64";
        default:
            return "";
    }
}

bool ToolChain::LinkWindows(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile) {
    FLY_DEBUG_MESSAGE("ToolChain", "LinkWindows", "Out: " + OutFile);
    llvm::SmallVector<std::string, 16> CmdArgs;
    CmdArgs.push_back("lld-link");

    // Out file
    std::string Out = "/out:" + OutFile + ".exe";
    CmdArgs.push_back(Out.c_str());

    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
    CmdArgs.push_back("/defaultlib:libcmt"); // DLL import library for the UCRT.

    // Check the environment first, since that's probably the user telling us
    // what they want to use.
    // Failing that, just try to find the newest Visual Studio version we can
    // and use its default VC toolchain.
    std::string VCToolChainPath;
    bool isLegacyVersion;
    findVCToolChainViaEnvironment(VCToolChainPath, isLegacyVersion) ||
    findVCToolChainViaSetupConfig(VCToolChainPath, isLegacyVersion) ||
    findVCToolChainViaRegistry(VCToolChainPath, isLegacyVersion);

    // Ex. -libpath:C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.29.30133\\lib\\x64"
    const char *SubdirName = isLegacyVersion ?
                             llvmArchToLegacyVCArch(T.getArch()) :
                             llvmArchToWindowsSDKArch(T.getArch());
    llvm::SmallString<256> VCToolChainLibPath(VCToolChainPath);
    llvm::sys::path::append(VCToolChainLibPath, "lib", SubdirName);
    const std::string VCToolChainLibPathStr = VCToolChainLibPath.str().str();
    CmdArgs.push_back("/libpath:\"" + VCToolChainLibPathStr + "\"");

    // Ex. -libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.19041.0\\ucrt\\x64"
    std::string UniversalCRTLibPath;
    if (getUniversalCRTLibraryPath(UniversalCRTLibPath))
        CmdArgs.push_back("/libpath:\"" + UniversalCRTLibPath + "\"");

    // Ex. -libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.19041.0\\um\\x64
    std::string WindowsSdkLibPath;
    if (getWindowsSDKLibraryPath(WindowsSdkLibPath))
        CmdArgs.push_back("/libpath:\"" + WindowsSdkLibPath + "\"");

    // Add Inputs
    for (const std::string& ObjFile : InFiles) {
        FLY_DEBUG_MESSAGE("ToolChain", "LinkWindows", "Input=" << ObjFile);
        CmdArgs.push_back(ObjFile.c_str());
    }

    if (T.getObjectFormat() == llvm::Triple::COFF) {
        SmallVector<const char*, 16> LinkArgs;
        createLinkArgs(CmdArgs, LinkArgs);
        return lld::coff::link(LinkArgs, false, llvm::outs(), llvm::errs());
    }

    return false;
}

// Find the most recent version of Universal CRT or Windows 10 SDK.
// vcvarsqueryregistry.bat from Visual Studio 2015 sorts entries in the include
// directory by name and uses the last one of the list.
// So we compare entry names lexicographically to find the greatest one.
static bool getWindows10SDKVersionFromPath(const std::string &SDKPath,
                                           std::string &SDKVersion) {
    SDKVersion.clear();

    std::error_code EC;
    llvm::SmallString<128> IncludePath(SDKPath);
    llvm::sys::path::append(IncludePath, "Include");
    for (llvm::sys::fs::directory_iterator DirIt(IncludePath, EC), DirEnd;
         DirIt != DirEnd && !EC; DirIt.increment(EC)) {
        if (!llvm::sys::fs::is_directory(DirIt->path()))
            continue;
        StringRef CandidateName = llvm::sys::path::filename(DirIt->path());
        // If WDK is installed, there could be subfolders like "wdf" in the
        // "Include" directory.
        // Allow only directories which names start with "10.".
        if (!CandidateName.startswith("10."))
            continue;
        if (CandidateName > SDKVersion)
            SDKVersion = std::string(CandidateName);
    }

    return !SDKVersion.empty();
}

static bool getUniversalCRTSdkDir(std::string &Path, std::string &UCRTVersion) {
    // vcvarsqueryregistry.bat for Visual Studio 2015 queries the registry
    // for the specific key "KitsRoot10". So do we.
    if (!getSystemRegistryString(
            "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", "KitsRoot10",
            Path, nullptr))
        return false;

    return getWindows10SDKVersionFromPath(Path, UCRTVersion);
}

bool ToolChain::getUniversalCRTLibraryPath(std::string &Path) const {
    std::string UniversalCRTSdkPath;
    std::string UCRTVersion;

    Path.clear();
    if (!getUniversalCRTSdkDir(UniversalCRTSdkPath, UCRTVersion))
        return false;

    StringRef ArchName = llvmArchToWindowsSDKArch(T.getArch());
    if (ArchName.empty())
        return false;

    llvm::SmallString<128> LibPath(UniversalCRTSdkPath);
    llvm::sys::path::append(LibPath, "Lib", UCRTVersion, "ucrt", ArchName);

    Path = std::string(LibPath.str());
    return true;
}

/// Get Windows SDK installation directory.
static bool getWindowsSDKDir(std::string &Path, int &Major,
                             std::string &WindowsSDKIncludeVersion,
                             std::string &WindowsSDKLibVersion) {
    std::string RegistrySDKVersion;
    // Try the Windows registry.
    if (!getSystemRegistryString(
            "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\$VERSION",
            "InstallationFolder", Path, &RegistrySDKVersion))
        return false;
    if (Path.empty() || RegistrySDKVersion.empty())
        return false;

    WindowsSDKIncludeVersion.clear();
    WindowsSDKLibVersion.clear();
    Major = 0;
    std::sscanf(RegistrySDKVersion.c_str(), "v%d.", &Major);
    if (Major <= 7)
        return true;
    if (Major == 8) {
        // Windows SDK 8.x installs libraries in a folder whose names depend on the
        // version of the OS you're targeting.  By default choose the newest, which
        // usually corresponds to the version of the OS you've installed the SDK on.
        const char *Tests[] = {"winv6.3", "win8", "win7"};
        for (const char *Test : Tests) {
            llvm::SmallString<128> TestPath(Path);
            llvm::sys::path::append(TestPath, "Lib", Test);
            if (llvm::sys::fs::exists(TestPath.c_str())) {
                WindowsSDKLibVersion = Test;
                break;
            }
        }
        return !WindowsSDKLibVersion.empty();
    }
    if (Major == 10) {
        if (!getWindows10SDKVersionFromPath(Path, WindowsSDKIncludeVersion))
            return false;
        WindowsSDKLibVersion = WindowsSDKIncludeVersion;
        return true;
    }
    // Unsupported SDK version
    return false;
}

// Gets the library path required to link against the Windows SDK.
bool ToolChain::getWindowsSDKLibraryPath(std::string &path) const {
    std::string sdkPath;
    int sdkMajor = 0;
    std::string windowsSDKIncludeVersion;
    std::string windowsSDKLibVersion;

    path.clear();
    if (!getWindowsSDKDir(sdkPath, sdkMajor, windowsSDKIncludeVersion,
                          windowsSDKLibVersion))
        return false;

    llvm::SmallString<128> libPath(sdkPath);
    llvm::sys::path::append(libPath, "Lib");
    if (sdkMajor >= 8) {
        llvm::sys::path::append(libPath, windowsSDKLibVersion, "um",
                                llvmArchToWindowsSDKArch(T.getArch()));
    } else {
        switch (T.getArch()) {
            // In Windows SDK 7.x, x86 libraries are directly in the Lib folder.
            case llvm::Triple::x86:
                break;
            case llvm::Triple::x86_64:
                llvm::sys::path::append(libPath, "x64");
                break;
            case llvm::Triple::arm:
                // It is not necessary to link against Windows SDK 7.x when targeting ARM.
                return false;
            default:
                return false;
        }
    }

    path = std::string(libPath.str());
    return true;
}

bool ToolChain::LinkDarwin(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile) {
    llvm::SmallVector<std::string, 16> CmdArgs;
    CmdArgs.push_back("ld64.lld");

    CmdArgs.push_back("-e");
    CmdArgs.push_back("_main");
    CmdArgs.push_back("-o");
    CmdArgs.push_back(OutFile.c_str());

    for(const std::string &InFile : InFiles) {
        FLY_DEBUG_MESSAGE("ToolChain", "LinkDarwin", "Input=" << InFile);
        CmdArgs.push_back(InFile.c_str());
    }

    if (T.getObjectFormat() == llvm::Triple::MachO) {
        SmallVector<const char*, 16> LinkArgs;
        createLinkArgs(CmdArgs, LinkArgs);
        return lld::macho::link(LinkArgs, false, llvm::outs(), llvm::errs());
    }

    return false;
}

bool ToolChain::LinkLinux(const llvm::SmallVector<std::string, 4> &InFiles, const std::string &OutFile) {
    FLY_DEBUG_MESSAGE("ToolChain", "LinkLinux", "Out: " + OutFile);
    llvm::SmallVector<std::string, 16> CmdArgs;
    CmdArgs.push_back("ld.lld");

//    switch (T.getArch()) {
//        default:
//            break;
//        case llvm::Triple::x86:
//        case llvm::Triple::ppc:
//            CmdArgs.push_back("-m32");
//            break;
//        case llvm::Triple::x86_64:
//        case llvm::Triple::ppc64:
//        case llvm::Triple::ppc64le:
//            CmdArgs.push_back("-m64");
//            break;
//        case llvm::Triple::sparcel:
//            CmdArgs.push_back("-EL");
//            break;
//    }

    const llvm::Triple::ArchType Arch = T.getArch();
    const bool isAndroid = T.isAndroid();
    const bool IsIAMCU = T.isOSIAMCU();
    const bool IsVE = T.isVE();
    const bool IsPIE = getPIE();
    const bool IsStaticPIE = CodeGenOpts.StaticPIE;
    const bool IsStatic = CodeGenOpts.Static && !CodeGenOpts.StaticPIE;
    const bool HasCRTBeginEndFiles = T.hasEnvironment() || (T.getVendor() != llvm::Triple::MipsTechnologies);

    if (IsPIE)
        CmdArgs.push_back("-pie");

    if (IsStaticPIE) {
        CmdArgs.push_back("-static");
        CmdArgs.push_back("-pie");
        CmdArgs.push_back("--no-dynamic-linker");
        CmdArgs.push_back("-z");
        CmdArgs.push_back("text");
    }

    if (CodeGenOpts.RDynamic)
        CmdArgs.push_back("-export-dynamic");

    // ARM
    if (T.isARM() || T.isThumb() || T.isAArch64()) {
        bool IsBigEndian = isArmBigEndian();
        if (IsBigEndian)
            CmdArgs.push_back("--be8");
        IsBigEndian = IsBigEndian || Arch == llvm::Triple::aarch64_be;
        CmdArgs.push_back(IsBigEndian ? "-EB" : "-EL");
    }

    // Most Android ARM64 targets should enable the linker fix for erratum
    // 843419. Only non-Cortex-A53 devices are allowed to skip this flag.
//    if (Arch == llvm::Triple::aarch64 && isAndroid) {
//        std::string CPU = getCPUName(Args, T);
//        if (CPU.empty() || CPU == "generic" || CPU == "cortex-a53")
//            CmdArgs.push_back("--fix-cortex-a53-843419");
//    }

    // Android does not allow shared text relocations. Emit a warning if the
    // user's code contains any.
    if (T.isAndroid())
        CmdArgs.push_back("--warn-shared-textrel");

    CmdArgs.push_back("--eh-frame-hdr");

    if (const char *LDMOption = getLDMOption()) {
        CmdArgs.push_back("-m");
        CmdArgs.push_back(LDMOption);
    } else {
        Diag.Report(diag::err_target_unknown_triple) << T.getTriple();
        return false;
    }

    if (IsStatic) {
        if (Arch == llvm::Triple::arm || Arch == llvm::Triple::armeb ||
            Arch == llvm::Triple::thumb || Arch == llvm::Triple::thumbeb)
            CmdArgs.push_back("-Bstatic");
        else
            CmdArgs.push_back("-static");
    } else if (CodeGenOpts.Shared) {
        CmdArgs.push_back("-shared");
    }

    if (!IsStatic) {
        if (CodeGenOpts.RDynamic)
            CmdArgs.push_back("-export-dynamic");

//        if (!CodeGenOpts.Shared && !IsStaticPIE) {
//            CmdArgs.push_back("-dynamic-linker");
//            CmdArgs.push_back(getDynamicLinker(Args)));
//        }
    }

    // Output
    CmdArgs.push_back("-o");
    CmdArgs.push_back(OutFile.c_str());

    // Create Path List
    SmallVector<std::string, 16> PathList = CreatePathList();;

    // Add crt1 object
    if (!isAndroid && !IsIAMCU) {
        const char *crt1 = nullptr;
        if (!CodeGenOpts.Shared) {
            if (IsPIE)
                crt1 = "Scrt1.o";
            else if (IsStaticPIE)
                crt1 = "rcrt1.o";
            else
                crt1 = "crt1.o";
        }
        if (crt1) {
            CmdArgs.push_back(GetFilePath(crt1, PathList));
        }
        CmdArgs.push_back(GetFilePath("crti.o", PathList));
    }

    if (IsVE) {
        CmdArgs.push_back("-z");
        CmdArgs.push_back("max-page-size=0x4000000");
    }

    // Add crtbegin object
    if (IsIAMCU) {
        const std::string crt0 = GetFilePath("crt0.o", PathList);
        if (getVFS().exists(crt0))
            CmdArgs.push_back(crt0);
    } else if (HasCRTBeginEndFiles) {
        std::string P;
        if (!isAndroid) {
            std::string crtbegin = getCompilerRT("crtbegin.o", PathList);
            if (getVFS().exists(crtbegin))
                P = crtbegin;
        }
        if (P.empty()) {
            const char *crtbegin;
            if (IsStatic)
                crtbegin = isAndroid ? "crtbegin_static.o" : "crtbeginT.o";
            else if (CodeGenOpts.Shared)
                crtbegin = isAndroid ? "crtbegin_so.o" : "crtbeginS.o";
            else if (IsPIE || IsStaticPIE)
                crtbegin = isAndroid ? "crtbegin_dynamic.o" : "crtbeginS.o";
            else
                crtbegin = isAndroid ? "crtbegin_dynamic.o" : "crtbegin.o";
            P = GetFilePath(crtbegin, PathList);
        }
        CmdArgs.push_back(P);
    }

    // Add crtfastmath.o if available and fast math is enabled.
    const std::string &FastMathPath = GetFilePath("crtfastmath.o", PathList);
    if (!FastMathPath.empty()) {
        CmdArgs.push_back(FastMathPath);
    }

    // Add Inputs
    for(const std::string &ObjFile : InFiles) {
        FLY_DEBUG_MESSAGE("ToolChain", "Link", "Input=" << ObjFile);
        CmdArgs.push_back(ObjFile);
    }

    // Add Library Paths
    for(const std::string &Path : PathList) {
        FLY_DEBUG_MESSAGE("ToolChain", "Link", "LibPath=" << Path);
        CmdArgs.push_back(("-L" + Path));
    }

    if (IsStatic || IsStaticPIE)
        CmdArgs.push_back("--start-group");

    CmdArgs.push_back("-lc");
    CmdArgs.push_back("-lgcc");
    CmdArgs.push_back("-lgcc_eh");

    // Add IAMCU specific libs, if needed.
    if (IsIAMCU)
        CmdArgs.push_back("-lgloss");

    if (IsStatic || IsStaticPIE)
        CmdArgs.push_back("--end-group");

    // Add IAMCU specific libs (outside the group), if needed.
    if (IsIAMCU) {
        CmdArgs.push_back("--as-needed");
        CmdArgs.push_back("-lsoftfp");
        CmdArgs.push_back("--no-as-needed");
    }

    // Add crtend object
    if (!IsIAMCU) {
        if (HasCRTBeginEndFiles) {
            std::string P;
            if (!isAndroid) {
                std::string crtend = getCompilerRT("crtend.o", PathList);
                if (getVFS().exists(crtend))
                    P = crtend;
            }
            if (P.empty()) {
                std::string crtend;
                if (CodeGenOpts.Shared)
                    crtend = isAndroid ? "crtend_so.o" : "crtendS.o";
                else if (IsPIE || IsStaticPIE)
                    crtend = isAndroid ? "crtend_android.o" : "crtendS.o";
                else
                    crtend = isAndroid ? "crtend_android.o" : "crtend.o";
                P = GetFilePath(crtend, PathList);
            }
            CmdArgs.push_back(P);
        }
        if (!isAndroid) {
            const std::string P = GetFilePath("crtn.o", PathList);
            CmdArgs.push_back(P);
        }
    }

    if (T.getObjectFormat() == llvm::Triple::ELF) {
        SmallVector<const char*, 16> LinkArgs;
        createLinkArgs(CmdArgs, LinkArgs);
        return lld::elf::link(LinkArgs, false, llvm::outs(), llvm::errs());
    }

    return false;
}

bool ToolChain::getPIE() {
    if (CodeGenOpts.Shared || CodeGenOpts.Static)
        return false;
    return CodeGenOpts.PIE;
}

bool ToolChain::isArmBigEndian() {
    return T.getArch() == llvm::Triple::armeb || T.getArch() == llvm::Triple::thumbeb;
}

const char *ToolChain::getLDMOption() {
    switch (T.getArch()) {
        case llvm::Triple::x86:
            if (T.isOSIAMCU())
                return "elf_iamcu";
            return "elf_i386";
        case llvm::Triple::aarch64:
            return "aarch64linux";
        case llvm::Triple::aarch64_be:
            return "aarch64linuxb";
        case llvm::Triple::arm:
        case llvm::Triple::thumb:
        case llvm::Triple::armeb:
        case llvm::Triple::thumbeb:
            return isArmBigEndian() ? "armelfb_linux_eabi" : "armelf_linux_eabi";
        case llvm::Triple::ppc:
            return "elf32ppclinux";
        case llvm::Triple::ppc64:
            return "elf64ppc";
        case llvm::Triple::ppc64le:
            return "elf64lppc";
        case llvm::Triple::riscv32:
            return "elf32lriscv";
        case llvm::Triple::riscv64:
            return "elf64lriscv";
        case llvm::Triple::sparc:
        case llvm::Triple::sparcel:
            return "elf32_sparc";
        case llvm::Triple::sparcv9:
            return "elf64_sparc";
        case llvm::Triple::mips:
            return "elf32btsmip";
        case llvm::Triple::mipsel:
            return "elf32ltsmip";
        case llvm::Triple::mips64:
//            if (tools::mips::hasMipsAbiArg(Args, "n32") ||
//                T.getEnvironment() == llvm::Triple::GNUABIN32)
//                return "elf32btsmipn32";
            return "elf64btsmip";
        case llvm::Triple::mips64el:
//            if (tools::mips::hasMipsAbiArg(Args, "n32") ||
//                T.getEnvironment() == llvm::Triple::GNUABIN32)
//                return "elf32ltsmipn32";
            return "elf64ltsmip";
        case llvm::Triple::systemz:
            return "elf64_s390";
        case llvm::Triple::x86_64:
            if (T.getEnvironment() == llvm::Triple::GNUX32)
                return "elf32_x86_64";
            return "elf_x86_64";
        case llvm::Triple::ve:
            return "elf64ve";
        default:
            return nullptr;
    }
}

std::string ToolChain::GetFilePath(llvm::Twine Name, SmallVector<std::string, 16> &PathList) const {
    // Search for Name in a list of paths.
    auto SearchPaths = [&](const llvm::SmallVectorImpl<std::string> &P)
            -> llvm::Optional<std::string> {
        // Respect a limited subset of the '-Bprefix' functionality in GCC by
        // attempting to use this prefix when looking for file paths.
        for (const auto &Dir : P) {
            if (Dir.empty())
                continue;
            SmallString<128> Path(Dir);
            llvm::sys::path::append(Path, Name);
            if (llvm::sys::fs::exists(Twine(Path))) {
                FLY_DEBUG_MESSAGE("ToolChain", "GetFilePath", "Path exists " << Path);
                return std::string(Path);
            }
        }
        return None;
    };

    if (auto P = SearchPaths(PathList))
        return *P;

    return "";
}

vfs::FileSystem &ToolChain::getVFS() const {
    return *VFS;
}

std::string ToolChain::getCompilerRT(const char *Name, SmallVector<std::string, 16> &PathList) {
    for (const auto &LibPath : PathList) {
        SmallString<128> P(LibPath);
        llvm::sys::path::append(P, Name);
        if (getVFS().exists(P))
            return std::string(P.str());
    }
    return "/lib";
}

/// Get our best guess at the multiarch triple for a target.
///
/// Debian-based systems are starting to use a multiarch setup where they use
/// a target-triple directory in the library and header search paths.
/// Unfortunately, this triple does not align with the vanilla target triple,
/// so we provide a rough mapping here.
std::string ToolChain::getMultiarch() const {
    llvm::Triple::EnvironmentType TargetEnvironment = T.getEnvironment();
    bool IsAndroid = T.isAndroid();
    bool IsMipsR6 = T.getSubArch() == llvm::Triple::MipsSubArch_r6;
    bool IsMipsN32Abi = T.getEnvironment() == llvm::Triple::GNUABIN32;

    // For most architectures, just use whatever we have rather than trying to be
    // clever.
    switch (T.getArch()) {
        default:
            break;

            // We use the existence of '/lib/<triple>' as a directory to detect some
            // common linux triples that don't quite match the Clang triple for both
            // 32-bit and 64-bit targets. Multiarch fixes its install triples to these
            // regardless of what the actual target triple is.
        case llvm::Triple::arm:
        case llvm::Triple::thumb:
            if (IsAndroid) {
                return "arm-linux-androideabi";
            } else if (TargetEnvironment == llvm::Triple::GNUEABIHF) {
                if (getVFS().exists("/lib/arm-linux-gnueabihf"))
                    return "arm-linux-gnueabihf";
            } else {
                if (getVFS().exists("/lib/arm-linux-gnueabi"))
                    return "arm-linux-gnueabi";
            }
            break;
        case llvm::Triple::armeb:
        case llvm::Triple::thumbeb:
            if (TargetEnvironment == llvm::Triple::GNUEABIHF) {
                if (getVFS().exists("/lib/armeb-linux-gnueabihf"))
                    return "armeb-linux-gnueabihf";
            } else {
                if (getVFS().exists("/lib/armeb-linux-gnueabi"))
                    return "armeb-linux-gnueabi";
            }
            break;
        case llvm::Triple::x86:
            if (IsAndroid)
                return "i686-linux-android";
            if (getVFS().exists("/lib/i386-linux-gnu"))
                return "i386-linux-gnu";
            break;
        case llvm::Triple::x86_64:
            if (IsAndroid)
                return "x86_64-linux-android";
            // We don't want this for x32, otherwise it will match x86_64 libs
            if (TargetEnvironment != llvm::Triple::GNUX32 &&
                getVFS().exists("/lib/x86_64-linux-gnu"))
                return "x86_64-linux-gnu";
            break;
        case llvm::Triple::aarch64:
            if (IsAndroid)
                return "aarch64-linux-android";
            if (getVFS().exists("/lib/aarch64-linux-gnu"))
                return "aarch64-linux-gnu";
            break;
        case llvm::Triple::aarch64_be:
            if (getVFS().exists("/lib/aarch64_be-linux-gnu"))
                return "aarch64_be-linux-gnu";
            break;
        case llvm::Triple::mips: {
            std::string MT = IsMipsR6 ? "mipsisa32r6-linux-gnu" : "mips-linux-gnu";
            if (getVFS().exists("/lib/" + MT))
                return MT;
            break;
        }
        case llvm::Triple::mipsel: {
            if (IsAndroid)
                return "mipsel-linux-android";
            std::string MT = IsMipsR6 ? "mipsisa32r6el-linux-gnu" : "mipsel-linux-gnu";
            if (getVFS().exists("/lib/" + MT))
                return MT;
            break;
        }
        case llvm::Triple::mips64: {
            std::string MT = std::string(IsMipsR6 ? "mipsisa64r6" : "mips64") +
                             "-linux-" + (IsMipsN32Abi ? "gnuabin32" : "gnuabi64");
            if (getVFS().exists("/lib/" + MT))
                return MT;
            if (getVFS().exists("/lib/mips64-linux-gnu"))
                return "mips64-linux-gnu";
            break;
        }
        case llvm::Triple::mips64el: {
            if (IsAndroid)
                return "mips64el-linux-android";
            std::string MT = std::string(IsMipsR6 ? "mipsisa64r6el" : "mips64el") +
                             "-linux-" + (IsMipsN32Abi ? "gnuabin32" : "gnuabi64");
            if (getVFS().exists("/lib/" + MT))
                return MT;
            if (getVFS().exists("/lib/mips64el-linux-gnu"))
                return "mips64el-linux-gnu";
            break;
        }
        case llvm::Triple::ppc:
            if (getVFS().exists("/lib/powerpc-linux-gnuspe"))
                return "powerpc-linux-gnuspe";
            if (getVFS().exists("/lib/powerpc-linux-gnu"))
                return "powerpc-linux-gnu";
            break;
        case llvm::Triple::ppc64:
            if (getVFS().exists("/lib/powerpc64-linux-gnu"))
                return "powerpc64-linux-gnu";
            break;
        case llvm::Triple::ppc64le:
            if (getVFS().exists("/lib/powerpc64le-linux-gnu"))
                return "powerpc64le-linux-gnu";
            break;
        case llvm::Triple::sparc:
            if (getVFS().exists("/lib/sparc-linux-gnu"))
                return "sparc-linux-gnu";
            break;
        case llvm::Triple::sparcv9:
            if (getVFS().exists("/lib/sparc64-linux-gnu"))
                return "sparc64-linux-gnu";
            break;
        case llvm::Triple::systemz:
            if (getVFS().exists("/lib/s390x-linux-gnu"))
                return "s390x-linux-gnu";
            break;
    }
    return T.str();
}

std::string ToolChain::getOSLibDir() {
    // FIXME
//    if (Triple.isMIPS()) {
//        if (Triple.isAndroid()) {
//            StringRef CPUName;
//            StringRef ABIName;
//            tools::mips::getMipsCPUAndABI(Args, Triple, CPUName, ABIName);
//            if (CPUName == "mips32r6")
//                return "libr6";
//            if (CPUName == "mips32r2")
//                return "libr2";
//        }
//        // lib32 directory has a special meaning on MIPS targets.
//        // It contains N32 ABI binaries. Use this folder if produce
//        // code for N32 ABI only.
//        if (tools::mips::hasMipsAbiArg(Args, "n32"))
//            return "lib32";
//        return Triple.isArch32Bit() ? "lib" : "lib64";
//    }

    // It happens that only x86 and PPC use the 'lib32' variant of oslibdir, and
    // using that variant while targeting other architectures causes problems
    // because the libraries are laid out in shared system roots that can't cope
    // with a 'lib32' library search path being considered. So we only enable
    // them when we know we may need it.
    //
    // FIXME: This is a bit of a hack. We should really unify this code for
    // reasoning about oslibdir spellings with the lib dir spellings in the
    // GCCInstallationDetector, but that is a more significant refactoring.
    if (T.getArch() == llvm::Triple::x86 ||
        T.getArch() == llvm::Triple::ppc)
        return "lib32";

    if (T.getArch() == llvm::Triple::x86_64 &&
        T.getEnvironment() == llvm::Triple::GNUX32)
        return "libx32";

    if (T.getArch() == llvm::Triple::riscv32)
        return "lib32";

    return T.isArch32Bit() ? "lib" : "lib64";
}

SmallVector<std::string, 16> ToolChain::CreatePathList() {
    SmallVector<std::string, 16> PathList;
    const std::string &MultiarchTriple = getMultiarch();
    const std::string &OSLibDir = getOSLibDir();

    // Examples: /lib32 /lib64 /lib ...
    SmallString<128> OSLib("/" + getOSLibDir());
    if (getVFS().exists(OSLib))
        PathList.push_back(OSLib.str().str());

    // /lib
    if (OSLib != "/lib") {
        SmallString<128> Lib("/lib");
        if (getVFS().exists(Lib))
            PathList.push_back(Lib.str().str());
    }

    // /usr/lib
    SmallString<128> UsrLib("/usr/lib");
    if (getVFS().exists(UsrLib))
        PathList.push_back(UsrLib.str().str());

    // Example: /lib/x86_64-linux-gnu
    SmallString<128> LibArch("/lib");
    llvm::sys::path::append(LibArch, MultiarchTriple);
    if (getVFS().exists(LibArch))
        PathList.push_back(LibArch.str().str());

    // Example: /usr/lib/x86_64-linux-gnu
    SmallString<128> UsrLibArch("/usr/lib");
    llvm::sys::path::append(UsrLibArch, MultiarchTriple);
    if (getVFS().exists(UsrLibArch))
        PathList.push_back(UsrLibArch.str().str());

    // Example: /lib/gcc/x86_64-linux-gnu/10
    if (getVFS().exists(GCC_LIB_PATH))
        PathList.push_back(GCC_LIB_PATH);
    return PathList;
}
