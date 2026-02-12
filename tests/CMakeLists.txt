# Gut unit tests

add_executable(test_gut test_gut.cpp)
target_link_libraries(test_gut PRIVATE gut)
add_test(NAME test_gut COMMAND test_gut)
