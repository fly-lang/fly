add_library(FlySema
        Sema.cpp
        )

target_link_libraries(FlyParser
        FlyBasic
        FlyAST
        )