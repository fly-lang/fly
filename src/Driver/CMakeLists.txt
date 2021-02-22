add_library(FlyDriver
        DriverOptions.cpp
        )

target_link_libraries(FlyDriver
        LLVMOption
        LLVMSupport
        )