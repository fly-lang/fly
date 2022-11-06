add_library(FlySema
        Sema.cpp
        SemaBuilder.cpp
        SemaResolver.cpp
        SemaValidator.cpp
        )

target_link_libraries(FlySema
        FlyBasic
        FlyAST
        )