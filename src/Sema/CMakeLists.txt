add_library(FlySema
        Sema.cpp
        SemaBuilder.cpp
        SemaResolver.cpp
        )

target_link_libraries(FlyParser
        FlyBasic
        FlyAST
        )