# Minimum CMake required
cmake_minimum_required(VERSION 3.11)

# Project
project(yagi LANGUAGES CXX VERSION 1.0.0)

# Config
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /WX")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /WX")

# Add cmake folder for 
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add ghidra subdir
add_subdirectory(ghidra)

# The IDA plugin
add_subdirectory(yagi)
add_subdirectory(tests)

# Summary
message(STATUS "Configuration summary")
message(STATUS "Project name                 : ${PROJECT_NAME}")
message(STATUS "Project version              : ${PROJECT_VERSION}")

# CPack part
set(CPACK_GENERATOR "WIX")
set(CPACK_WIX_UPGRADE_GUID "c64ed0aa-c54a-4590-93fb-72e152edefb7")
set(CPACK_WIX_UNINSTALL "1")
set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/install/patch.xml")
include(CPack)
