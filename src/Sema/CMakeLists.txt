add_library(FlySema
        Sema.cpp
        SemaBuilder.cpp
        SemaNumber.cpp
        SemaResolver.cpp
        )

target_link_libraries(FlySema
        FlyBasic
        FlyAST
        )