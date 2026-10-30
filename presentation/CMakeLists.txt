add_library(sdb_cli sdb_cli.cpp)

target_link_libraries(sdb_cli
    PUBLIC sdb::libsdb
)
