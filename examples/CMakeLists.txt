# Examples - each needs GUT_IMPLEMENTATION in exactly one TU

# Helper function for examples
function(add_gut_example name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE gut)
endfunction()

add_gut_example(hello_gut)
add_gut_example(button_demo)
add_gut_example(animation_demo)
add_gut_example(layout_demo)
add_gut_example(test_single_header)
