add_library(libsdb
    process.cpp
    pipe.cpp
    registers.cpp
)

add_library(sdb::libsdb ALIAS libsdb)

set_target_properties(libsdb PROPERTIES
    OUTPUT_NAME sdb
)

target_compile_features(libsdb PUBLIC cxx_std_17)

target_include_directories(libsdb
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
