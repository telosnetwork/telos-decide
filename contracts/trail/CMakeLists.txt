add_contract( trail trail 
   ${CMAKE_CURRENT_SOURCE_DIR}/src/ballot.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/committee.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/trail.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/treasury.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/voter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/worker.cpp
)

target_include_directories( trail
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/eosio.contracts/contracts/eosio.token/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/eosio.contracts/contracts/eosio.system/include
)

set_target_properties( trail
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/resources/trail.contracts.md ${CMAKE_CURRENT_BINARY_DIR}/resources/trail.contracts.md @ONLY )

target_compile_options( trail PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/resources -R${CMAKE_CURRENT_BINARY_DIR}/resources )
target_compile_options( trail PUBLIC -Wunknown-pragmas )