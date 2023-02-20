//
// Created by KarlE on 2/13/2023.
//

#ifndef CPU8080_EMULATOR_H
#define CPU8080_EMULATOR_H

#include <stdint.h>
#include <vector>
#include <memory>
#include <unordered_set>

#include "types.h"

class Controls {
public:
    bool s {false};
    bool z {false};
    bool p {false};
    bool c {false};
};

class Status {
public:
    Status(): memory(1<<16, 0) {};
    Byte a {0};
    Byte b {0};
    Byte c {0};
    Byte d {0};
    Byte e {0};
    Byte h {0};
    Byte l {0};
    uint16_t sp {0};
    uint16_t pc {0};
    std::vector<Byte> memory;

    Controls controls;
};

class NotImplementedInstruction : public std::exception {
private:
    uint8_t opcode_;
    std::string msg_;
public:
    NotImplementedInstruction(uint8_t opcode);
    const char * what() const noexcept override;
};

class Emulator {
public:
    void updateControls(uint16_t result, std::unordered_set<ControlFlags> const& affected);
    void emulateOp();

    Status status_;
};

#endif //CPU8080_EMULATOR_H
