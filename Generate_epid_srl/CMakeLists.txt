cmake_minimum_required(VERSION 3.10)

project(generate_epid_srl C CXX)

set(Sources
    Generate_epid_srl.cpp
)

if (HBGS_PARAMETER_SET_TEST1)
    add_executable(generate_epid_srl_129 ${Sources})
    target_include_directories(generate_epid_srl_129 SYSTEM PRIVATE ${include_dirs})
    target_link_libraries(generate_epid_srl_129 PRIVATE project_options project_warnings stdc++ hbgs_lib_129 ${ossl_libs} picnic sha3)
elseif (HBGS_PARAMETER_SET_TEST2)
    add_executable(generate_epid_srl_255 ${Sources})
    target_include_directories(generate_epid_srl_255 SYSTEM PRIVATE ${include_dirs})
    target_link_libraries(generate_epid_srl_255 PRIVATE project_options project_warnings stdc++ hbgs_lib_255 ${ossl_libs} picnic sha3)
else()
    message( FATAL_ERROR "No HBGS parameter set selected, CMake will exit." )
endif()
