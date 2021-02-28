add_library(FlyDriver
        Driver.cpp
        DriverOptions.cpp
        )

target_link_libraries(FlyDriver
        LLVMOption
        LLVMSupport
        FlyFrontend
        )