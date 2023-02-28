#include <iostream>
#include <disassembler.h>
#include <emulator.h>

int main() {
    Emulator emulator {};
    emulator.setMemory("C:\\Users\\KarlE\\ClionProjects\\cpu8080\\space-invaders.rom");
    emulator.emulate();

    return 0;
}
