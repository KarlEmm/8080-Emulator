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
    bool is_interrupt_enabled {true};

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
    void ana(Byte b);
    void xra(Byte b);
    void ora(Byte b);
    void cmp(Byte b);
    void pop(Byte& high, Byte& low);
    void push(Byte& high, Byte& low);
    void ret();
    void jmp();
    void call();
    void loadi(Byte& rpHigh, Byte& rpLow);
    void loadsp();
    void ldax(Byte const& rpHigh, Byte const& rpLow);
    void stax(Byte const& rpHigh, Byte const& rpLow);
    void inx(Byte& rpHigh, Byte& rpLow);
    void dcx(Byte& rpHigh, Byte& rpLow);
    void inr(Byte& regr);
    void dcr(Byte& regr);
    void mvi(Byte& regr);
    void mov(Byte& dest, Byte const& src);
    void dad(Byte const& rpHigh, Byte const& rpLow);
    void add(Byte& dest, Byte const& operand);
    void adc(Byte& dest, Byte const& operand);
    void sub(Byte& dest, Byte const& operand);
    void sbb(Byte& dest, Byte const& operand);
    void updateControls(uint16_t result, std::unordered_set<ControlFlags> const& affected);
    void emulate();
    void emulateOp();

    void setMemory(std::string const& filename);

    Status status_;
};

#endif //CPU8080_EMULATOR_H
