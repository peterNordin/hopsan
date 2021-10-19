find_package(Qt5 COMPONENTS Test REQUIRED)
set(CMAKE_AUTOMOC ON)

set(test_name tst_modelutils)
add_executable(${test_name}
               ${test_name}.cpp)
target_compile_definitions(${test_name} PRIVATE
                           DEFAULT_LIBRARY_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../../componentLibraries/defaultLibrary/\"
                           TEST_DATA_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/../HopsanCoreTests/SimulationTest/\")

target_link_libraries(${test_name} hopsanutils Qt5::Test)
add_test(NAME ${test_name} COMMAND ${test_name})

if (WIN32)
    copy_file_after_build(${test_name} $<TARGET_FILE:hopsancore> $<TARGET_FILE_DIR:${test_name}>)
endif()


#set(test_name tst_commonutils)
#add_executable(${test_name}
#               ${test_name}.cpp)
#target_compile_definitions(${test_name} PRIVATE
#                           DEFAULT_LIBRARY_ROOT=\"${CMAKE_CURRENT_BINARY_DIR}/../../componentLibraries/defaultLibrary/\"
#                           TEST_DATA_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/../HopsanCoreTests/SimulationTest/\")

#target_link_libraries(${test_name} hopsanutils Qt5::Test)
#add_test(NAME ${test_name} COMMAND ${test_name})

#if (WIN32)
#    copy_file_after_build(${test_name} $<TARGET_FILE:hopsancore> $<TARGET_FILE_DIR:${test_name}>)
#endif()
