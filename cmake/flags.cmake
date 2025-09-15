set(CMAKE_CXX_STANDARD 26)
add_compile_options(
        -Wall
        -Werror
        -Wextra
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-deprecated-declarations
        -Wno-missing-field-initializers
#        -Wno-unused-lambda-capture
)


if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(
            -g
            -O0
    )
    #    add_compile_definitions(
    #            DEBUG=1
    #    )

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        add_compile_options(
                -fsanitize=address,undefined
        )
        add_link_options(
                -fsanitize=address,undefined
        )
    endif ()
else ()
    add_compile_options(-O3 -g)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-flto=thin)
        add_link_options(-flto=thin)
    endif ()
endif ()