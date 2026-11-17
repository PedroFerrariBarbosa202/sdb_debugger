add_library(sdb_cli 
  sdb_cli.cpp
  view/general_views.cpp
  view/register_views.cpp
  view/error_views.cpp
  view/breakpoint_views.cpp)

target_link_libraries(sdb_cli
    PUBLIC sdb::libsdb
)
