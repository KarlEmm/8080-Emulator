#include <iostream>
#include <disassembler.h>
#include <emulator.h>

int main() {
    Disassembler disassembler {};
    disassembler.disassembleFile("C:\\Users\\KarlE\\ClionProjects\\cpu8080\\space_invaders.a26");
    Emulator emulator;
//    uint8_t a = 1;
//    uint8_t b = a << 8;
//    std::cout << (int) b << '\n';
    return 0;
}
