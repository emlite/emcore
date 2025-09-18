cmake_minimum_required(VERSION 3.25)
project(emcore VERSION 0.1.4 LANGUAGES C)

add_library(emcore)
set(EMCORE_HEADERS include/emcore/emcore.h)
set(EMCORE_SRCS src/lib.c)

if (CMAKE_C_COMPILER_TARGET STREQUAL "wasm32-wasip2" OR CMAKE_CXX_COMPILER_TARGET STREQUAL "wasm32-wasip2")
  list(APPEND EMCORE_SRCS src/env.c src/wasip2adapter.c)
endif()

target_sources(emcore 
    PRIVATE 
        ${EMCORE_SRCS}
    PUBLIC 
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include
        FILES ${EMCORE_HEADERS}
)
set_target_properties(emcore PROPERTIES LINKER_LANGUAGE C)

set(EMLITE_WITH_EMSENV OFF)
if(EMSCRIPTEN AND NOT EMSCRIPTEN_STANDALONE_WASM)
    set(EMLITE_WITH_EMSENV ON)
    find_package(emsenv CONFIG QUIET)
    if (NOT emsenv_FOUND)
        add_subdirectory(emsenv)
    endif()
    target_link_libraries(emcore PRIVATE emsenv::emsenv)
endif()

set(EMLITE_WITH_DLMALLOC OFF)
if (EMLITE_USE_DLMALLOC)
    set(EMLITE_WITH_DLMALLOC ON)
    find_package(dlmalloc CONFIG QUIET)
    if(NOT dlmalloc_FOUND)
        add_subdirectory(dlmalloc)
    endif()
    target_compile_definitions(emcore PUBLIC HAVE_DLMALLOC)
    target_link_libraries(emcore PRIVATE dlmalloc)
endif()

add_library(emcore::emcore ALIAS emcore)

include(GNUInstallDirs)

set(emcore_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/emcore")

install(
  TARGETS emcore
  EXPORT  emcoreTargets
  FILE_SET  HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT      emcoreTargets
  FILE        emcoreTargets.cmake
  NAMESPACE   emcore::
  DESTINATION "${emcore_INSTALL_CMAKEDIR}"
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/emcoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfig.cmake"
  INSTALL_DESTINATION "${emcore_INSTALL_CMAKEDIR}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfigVersion.cmake"
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/emcoreConfigVersion.cmake"
  DESTINATION "${emcore_INSTALL_CMAKEDIR}"
)

if (CMAKE_C_COMPILER_TARGET STREQUAL "wasm32-wasip2"
 OR CMAKE_CXX_COMPILER_TARGET STREQUAL "wasm32-wasip2")

  install(FILES ${CMAKE_CURRENT_LIST_DIR}/src/env_component_type.o
          DESTINATION ${CMAKE_INSTALL_LIBDIR})

  add_library(emcore_component_type INTERFACE)
  target_link_libraries(emcore_component_type INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/env_component_type.o>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}/env_component_type.o>"
  )
  add_library(emcore::component_type ALIAS emcore_component_type)
  install(TARGETS emcore_component_type EXPORT emcoreTargets)
endif()
