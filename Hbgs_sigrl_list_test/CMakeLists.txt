cmake_minimum_required(VERSION 3.10)

project(hbgs_sigrl_list_test C CXX)

set(Sources
    Hbgs_sigrl_list_test.cpp
)

if (HBGS_PARAMETER_SET_TEST1)
    add_executable(hbgs_sigrl_list_test_129 ${Sources})
    target_include_directories(hbgs_sigrl_list_test_129 SYSTEM PRIVATE ${include_dirs})
    target_link_libraries(hbgs_sigrl_list_test_129 PRIVATE project_options project_warnings stdc++ hbgs_lib_129 ${ossl_libs} picnic sha3)
elseif (HBGS_PARAMETER_SET_TEST2)
    add_executable(hbgs_sigrl_list_test_255 ${Sources})
    target_include_directories(hbgs_sigrl_list_test_255 SYSTEM PRIVATE ${include_dirs})
    target_link_libraries(hbgs_sigrl_list_test_255 PRIVATE project_options project_warnings stdc++ hbgs_lib_255 ${ossl_libs} picnic sha3)
else()
    message( FATAL_ERROR "No HBGS parameter set selected, CMake will exit." )
endif()
