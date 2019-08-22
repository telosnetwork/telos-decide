add_contract(example example ${CMAKE_CURRENT_SOURCE_DIR}/src/example.cpp)

target_include_directories(example
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(example
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/resources/example.contracts.md ${CMAKE_CURRENT_BINARY_DIR}/resources/example.contracts.md @ONLY )

target_compile_options( example PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/resources -R${CMAKE_CURRENT_BINARY_DIR}/resources )
