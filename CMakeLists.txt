cmake_minimum_required(VERSION 3.13)

project("csbotc_mm")

find_package("Threads" REQUIRED)

add_library("csbotc_mm" MODULE
    "cvars.cpp"
    "Menu.cpp"
    "CSBotControl.cpp"
    "SharedFunctions.cpp"
    "recipientfilters.cpp"
    "BATMenu.cpp"
)

if(NOT (UNIX AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")))
    message(FATAL_ERROR "platform is temporary not supported")
endif(NOT (UNIX AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")))

set_property(TARGET "csbotc_mm" PROPERTY PREFIX "")
set_property(TARGET "csbotc_mm" PROPERTY OUTPUT_NAME "csbotc_mm")
set_property(TARGET "csbotc_mm" PROPERTY C_VISIBILITY_PRESET "hidden")
set_property(TARGET "csbotc_mm" PROPERTY CXX_VISIBILITY_PRESET "hidden")
set_property(TARGET "csbotc_mm" PROPERTY POSITION_INDEPENDENT_CODE TRUE)
set_property(TARGET "csbotc_mm" PROPERTY VISIBILITY_INLINES_HIDDEN TRUE)

target_include_directories("csbotc_mm" PRIVATE
     "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/public"
     "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/public/engine"
     "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/public/game/server"
     "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/public/tier0"
     "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/public/tier1"
     "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/game/server"
     "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/game/shared"
)

target_include_directories("csbotc_mm" PRIVATE
     "${CMAKE_CURRENT_SOURCE_DIR}/../metamod-source/core"
     "${CMAKE_CURRENT_SOURCE_DIR}/../metamod-source/core/sourcehook"
)

target_compile_options("csbotc_mm" PRIVATE "-funroll-loops" "-msse")

target_compile_definitions("csbotc_mm" PRIVATE -DLINUX -D_LINUX -DPOSIX -D_POSIX -DNDEBUG -DSOURCE_ENGINE=SOURCE_ENGINE_CSS)
target_compile_definitions("csbotc_mm" PRIVATE -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp)

target_link_libraries("csbotc_mm" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/lib/linux/tier1_i486.a")
target_link_libraries("csbotc_mm" PRIVATE "-L${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk/lib/linux" "tier0_srv.so" "vstdlib_srv.so")
target_link_libraries("csbotc_mm" PRIVATE "${VSTDLIB_LIBRARY}")
target_link_libraries("csbotc_mm" PRIVATE "m")
target_link_libraries("csbotc_mm" PRIVATE "dl")
target_link_libraries("csbotc_mm" PRIVATE "Threads::Threads")
target_link_options("csbotc_mm" PRIVATE "-static-libgcc" "-static-libstdc++")
target_link_options("csbotc_mm" PRIVATE "LINKER:--no-undefined")
