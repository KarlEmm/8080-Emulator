add_library(Lib disassembler.cpp emulator.cpp)

target_include_directories(Lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})

set(installable_libs Lib)
install(TARGETS ${installable_libs} DESTINATION lib)
install(FILES disassembler.h auxiliary.h types.h DESTINATION include)