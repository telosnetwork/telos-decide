add_contract(watcher watcher ${CMAKE_CURRENT_SOURCE_DIR}/src/watcher.cpp)

target_include_directories(watcher
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(watcher
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/resources/watcher.contracts.md ${CMAKE_CURRENT_BINARY_DIR}/resources/watcher.contracts.md @ONLY )

target_compile_options( watcher PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/resources -R${CMAKE_CURRENT_BINARY_DIR}/resources )
