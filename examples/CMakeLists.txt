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

# ---------------------------------------------------------------------------
# Windows D3D11 / D3D12 demos
# ---------------------------------------------------------------------------
if(WIN32)
    # D3D11 demo
    add_executable(d3d11_demo WIN32 d3d11_demo.cpp)
    target_link_libraries(d3d11_demo PRIVATE gut d3d11 d3dcompiler dxgi)
    target_compile_definitions(d3d11_demo PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

    # D3D12 demo
    add_executable(d3d12_demo WIN32 d3d12_demo.cpp)
    target_link_libraries(d3d12_demo PRIVATE gut d3d12 dxgi d3dcompiler)
    target_compile_definitions(d3d12_demo PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
