foreach(target ${LLVM_TARGETS_TO_BUILD})
    list(APPEND targets "LLVM${target}CodeGen")
endforeach()

add_library(FlyDriver
        Driver.cpp
        DriverOptions.cpp
        ToolChain.cpp
        )

target_link_libraries(FlyDriver
        ${targets}
        LLVMOption
        LLVMSupport
        lldCommon
        lldCOFF
        lldELF
        lldMachO2
        lldMinGW
        lldWasm
        FlyFrontend
        )