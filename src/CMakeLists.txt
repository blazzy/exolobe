cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)

find_package(GTK2 REQUIRED gtk)

set(LINT_CMD clang --analyze)
set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-value -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

set(SOURCE
  mcdb.c  mcdb.h
  mcstd.c mcstd.h)

set(CMD_SOURCE  main.c)
set(GTK_SOURCE  gtk_main.cpp)

set(SQLITE3_SOURCE sqlite3.c sqlite3.h)


add_library(sqlite3 STATIC ${SQLITE3_SOURCE})
target_link_libraries(sqlite3 dl pthread)
 


#SOURCE_FULL_PATH will hold the full path to each source file to work with custom_commands
foreach(S ${SOURCE})
  set(SOURCE_FULL_PATH ${SOURCE_FULL_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/${S})
endforeach(S)

add_library(mc STATIC ${SOURCE} ${SQLITE3_SOURCE})
target_link_libraries(mc sqlite3 uuid glib-2.0) #TODO Find uuid and glib instead of assuming they exist
                                                #TODO Determine alternatives for MS Windows.

add_custom_command(OUTPUT tags COMMAND ctags ${SOURCE_FULL_PATH} DEPENDS ${SOURCE_FULL_PATH})
add_custom_command(OUTPUT lint COMMAND ${LINT_CMD} ${SOURCE_FULL_PATH})

add_executable(mcdb ${CMD_SOURCE} tags)
target_link_libraries(mcdb mc)

if(GTK2_FOUND)
  include_directories(${GTK2_INCLUDE_DIRS})

  add_executable(mcgtk ${GTK_SOURCE})
  target_link_libraries(mcgtk mc ${GTK2_LIBRARIES})

  INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mcgtk DESTINATION bin)
endif()

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mcdb DESTINATION bin)
