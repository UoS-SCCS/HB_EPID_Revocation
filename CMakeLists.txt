# Based on:
#
# https://github.com/lefticus/cpp_starter_project/blob/master/CMakeLists.txt
#

cmake_minimum_required(VERSION 3.13)

# Set the project name to your project name
project(hbgs_impl CXX C)

set(code_dir $ENV{HOME}/Code)
set(hbgs_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(picnic_dir ${hbgs_dir}/HBGS_picnic)

include(${code_dir}/Cmake-options/StandardProjectSettings.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(${code_dir}/Cmake-options/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(${code_dir}/Cmake-options/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(${code_dir}/Cmake-options/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(${code_dir}/Cmake-options/StaticAnalysers.cmake)

# Debugging option to print varaibles
#    e.g. cmake_print_variables(<variable>)
include(CMakePrintHelpers)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin) 

set(include_dirs ${utils_dir}/Include ${CMAKE_SOURCE_DIR}/Include ${CMAKE_SOURCE_DIR}/HBGS_picnic)

include(${hbgs_dir}/Picnic.cmake)

set(Lib_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Io_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Clock_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Hb_epid_revocation_lists.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Picnic_mpc_functions.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Mpc_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Lowmc32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Lowmc64.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Mpc_lowmc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Mpc_lowmc64.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Mpc_seeds_and_tapes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Mpc_signature_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Mpc_working_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Mpc_sigrl_entries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Hbgs_epid_signature.cpp
)

option(HBGS_PARAMETER_SET_TEST1 "HBGS parameter set for 129 bits (129,10,32,2,16,36,250)" ON)
option(HBGS_PARAMETER_SET_TEST2 "HBGS parameter set for 255 bits (255,10,64,2,16,68,601)" OFF)

# Picnic3_L1 = 1, Picnic3_L5 = 2, using the original parameter sets for revocation. 
if (HBGS_PARAMETER_SET_TEST1)
    add_compile_definitions(HBGS_N=129 HBGS_QA=10 HBGS_DS=32 HBGS_H=1 HBGS_D=4 HBGS_K=11 PICNIC_PS=1 HBGS_MPC_R=250 HBGS_MPC_O=36)
    message(STATUS "HBGS parameter set for 129 bits selected")
    add_library(hbgs_lib_129 STATIC ${Lib_sources})
    target_include_directories(hbgs_lib_129 SYSTEM PRIVATE ${include_dirs})
    target_link_libraries(hbgs_lib_129 PRIVATE project_options project_warnings)
elseif (HBGS_PARAMETER_SET_TEST2)
    add_compile_definitions(HBGS_N=255 HBGS_QA=10 HBGS_DS=64 HBGS_H=1 HBGS_D=4 HBGS_K=11 PICNIC_PS=2 HBGS_MPC_R=601 HBGS_MPC_O=68)
    message(STATUS "HBGS parameter set for 255 bits selected")
    add_library(hbgs_lib_255 STATIC ${Lib_sources})
    target_include_directories(hbgs_lib_255 SYSTEM PRIVATE ${include_dirs})
    target_link_libraries(hbgs_lib_255 PRIVATE project_options project_warnings)
else()
    message( FATAL_ERROR "Failed to correctly select a parameter set, CMake will exit." )
endif()

# set these for profiing - there must be a better way
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
#SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

add_subdirectory(Generate_epid_srl)
add_subdirectory(Hbgs_sigrl_list_test)
