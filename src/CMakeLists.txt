cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)

find_package(PkgConfig)
find_package(GTK2 REQUIRED gtk)
find_package(X11 REQUIRED)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)

include_directories(
  ../deps/sqlite
  ../deps/uuidgen
  ${GLIB2_INCLUDE_DIRS})

set(LINT_CMD clang --analyze)
set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-value -pedantic")
set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

set(SOURCE
  eldb.cpp        eldb.hpp
  eldb_sqlite.cpp eldb_sqlite.hpp
  eldb_util.cpp   eldb_util.hpp)

set(CMD_SOURCE  eldb_cli.cpp)
set(GTK_SOURCE  eldb_gtk.cpp eldb_gtk_globalkeybind.cpp)


set(SQLITE3_SOURCE ../deps/sqlite/sqlite3.c ../deps/sqlite/sqlite3.h)
add_library(sqlite3 STATIC ${SQLITE3_SOURCE})
target_link_libraries(sqlite3 dl pthread)
set_target_properties(sqlite3 PROPERTIES COMPILE_FLAGS
  "-Wno-parentheses-equality -Wno-unused-parameter -Wno-unused-value -Wno-unused-but-set-variable")


set(UUID_SOURCE
 ../deps/uuidgen/md5.c
 ../deps/uuidgen/token.c
)
add_library(uuid STATIC ${UUID_SOURCE})


#SOURCE_FULL_PATH will hold the full path to each source file to work with custom_commands
foreach(S ${SOURCE})
  set(SOURCE_FULL_PATH ${SOURCE_FULL_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/${S})
endforeach(S)

add_library(el STATIC ${SOURCE})
target_link_libraries(el sqlite3 uuid ${GLIB2_LIBRARIES})

add_custom_command(OUTPUT tags COMMAND ctags ${SOURCE_FULL_PATH} DEPENDS ${SOURCE_FULL_PATH})
add_custom_command(OUTPUT lint COMMAND ${LINT_CMD} ${SOURCE_FULL_PATH})

add_executable(eldb ${CMD_SOURCE})
target_link_libraries(eldb el)

if(GTK2_FOUND)
if(X11_FOUND)
  include_directories(${GTK2_INCLUDE_DIRS})

  add_executable(elgtk ${GTK_SOURCE})
  target_link_libraries(elgtk el ${GTK2_LIBRARIES} ${X11_LIBRARIES})

  INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/elgtk DESTINATION bin)
endif()
endif()

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/eldb DESTINATION bin)
