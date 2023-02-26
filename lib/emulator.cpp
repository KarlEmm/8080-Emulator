//
// Created by KarlE on 2/13/2023.
//
#include <iostream>
#include <utility>
#include "emulator.h"
#include "auxiliary.h"

NotImplementedInstruction::NotImplementedInstruction(uint8_t opcode): opcode_{opcode} {}
const char* NotImplementedInstruction::what() const noexcept {
    return "Instruction not implemented";
};

void Emulator::updateControls(uint16_t const result, std::unordered_set<ControlFlags> const& affected)
{
    for (auto const& a: affected)
    {
        switch (a) {
            case CARRY: { status_.controls.c = result > 0xff; break; }
            case PARITY: { status_.controls.p = popcount(result & 0xff) % 2 == 0; break; }
            case SIGN: { status_.controls.s = (result & 0x80) != 0; break; }
            case ZERO: { status_.controls.z = (result & 0xff) == 0; break; }
        }
    }
}

void Emulator::ana(Byte b) {
    status_.a &= b;
    updateControls(status_.a, {PARITY, SIGN, ZERO});
    status_.controls.c = false;
}

void Emulator::xra(Byte b) {
    status_.a ^= b;
    updateControls(status_.a, {PARITY, SIGN, ZERO});
    status_.controls.c = false;
}

void Emulator::ora(Byte b) {
    status_.a |= b;
    updateControls(status_.a, {PARITY, SIGN, ZERO});
    status_.controls.c = false;
}

void Emulator::cmp(Byte b) {
    uint16_t tmp = (uint16_t) status_.a - (uint16_t) b;
    updateControls(tmp, {CARRY, PARITY, SIGN, ZERO});
}

void Emulator::pop(Byte& high, Byte& low) {
    low = status_.memory[status_.sp];
    high = status_.memory[status_.sp+1];
    status_.sp += 2;
}

void Emulator::push(Byte& high, Byte& low) {
    status_.memory[status_.sp - 1] = high;
    status_.memory[status_.sp - 2] = low;
    status_.sp -= 2;
}

void Emulator::ret() {
    status_.pc = ((uint16_t) status_.memory[status_.sp + 1] << 8) | ((uint16_t) status_.memory[status_.sp]);
    status_.sp += 2;
}

void Emulator::jmp() {
    status_.pc = ((uint16_t) status_.memory[status_.pc+2] << 8) | ((uint16_t) status_.memory[status_.pc+1]);
}

void Emulator::call() {
    status_.memory[status_.sp - 1] = (status_.pc >> 8);
    status_.memory[status_.sp - 2] = (status_.pc & 0xff);
    status_.sp -= 2;
    status_.pc = ((uint16_t) status_.memory[status_.pc+2] << 8) | ((uint16_t) status_.memory[status_.pc+1]);
}

void Emulator::emulateOp() {
    auto& mem = status_.memory;
    uint16_t& pc = status_.pc;
    Byte op = mem[pc];

    switch (op) {
        case 0x00:
            return;
        case 0x01: { // LXI_B
            status_.c = mem[pc + 1];
            status_.b = mem[pc + 2];
            status_.pc += 2;
            break;
        }
        case 0x02: { // STAX_B
            uint16_t addr = (status_.b << 8 | status_.c);
            mem[addr] = status_.a;
            break;
        }
        case 0x03: { // INX_B
            uint16_t bc = (status_.b << 8) | status_.c;
            bc += 1;
            status_.b = (bc >> 8);
            status_.c = bc & 0xff;
            break;
        }
        case 0x04: { // INR_B
            uint16_t tmp = (uint16_t) status_.b + 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.b = (tmp & 0xff);
            break;
        }
        case 0x05: { // DCR_B
            uint16_t tmp = status_.b - 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.b = (tmp & 0xff);
            break;
        }
        case 0x06: { // MVI_B
            status_.b = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x07: { // RLC
            uint16_t tmp = status_.a;
            tmp <<= 1;
            tmp |= ((tmp & 0x100) != 0);
            updateControls(tmp, {CARRY});
            status_.a = (tmp & 0xff);
            break;
        }
        case 0x08: {
            break;
        }
        case 0x09: { // DAD_B
            uint16_t hl = ((uint16_t) status_.h << 8) | ((uint16_t) status_.l);
            uint16_t tmp = mem[hl];
            uint16_t bc = ((uint16_t) status_.b << 8) | ((uint16_t) status_.c);
            uint16_t bcContent = mem[bc];
            tmp += bcContent;
            mem[hl] = tmp;
            updateControls(tmp, {CARRY});
            break;
        }
        case 0x0a: { // LDAX B
            uint16_t bc = ((uint16_t) status_.b << 8) | ((uint16_t) status_.c);
            status_.a = mem[bc];
            break;
        }
        case 0x0b: { // DCX B
            uint16_t bc = ((uint16_t) status_.b << 8) | ((uint16_t) status_.c);
            mem[bc] = mem[bc] - 1;
            break;
        }
        case 0x0c: { // INR_C
            uint16_t tmp = (uint16_t) status_.c + 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.c = (tmp & 0xff);
            break;
        }
        case 0x0d: { // DCR_C
            uint16_t tmp = status_.c - 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.c = (tmp & 0xff);
            break;
        }
        case 0x0e: { // MVI_C
            status_.c = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x0f: { // RRC
            bool carry = status_.a & 0x1;
            status_.a >>= 1;
            if (carry) {
                status_.controls.c = true;
                status_.a |= 0x80;
            }
            break;
        }
        case 0x10: { // NOP
            break;
        }
        case 0x11: { // LXI_D
            status_.e = mem[pc + 1];
            status_.d = mem[pc + 2];
            status_.pc += 2;
            break;
        }
        case 0x12: { // STAX_D
            uint16_t addr = (status_.d << 8 | status_.e);
            mem[addr] = status_.a;
            break;
        }
        case 0x13: { // INX_D
            uint16_t de = (status_.d << 8) | status_.e;
            de += 1;
            status_.d = (de >> 8);
            status_.e = de & 0xff;
            break;
        }
        case 0x14: { // INR_D
            uint16_t tmp = (uint16_t) status_.d + 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.d = (tmp & 0xff);
            break;
        }
        case 0x15: { // DCR_D
            uint16_t tmp = status_.d - 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.d = (tmp & 0xff);
            break;
        }
        case 0x16: { // MVI_D
            status_.d = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x17: { // RAL
            uint16_t tmp = status_.a;
            tmp <<= 1;
            tmp |= (status_.controls.c);
            updateControls(tmp, {CARRY});
            status_.a = (tmp & 0xff);
            break;
        }
        case 0x18: { // NOP
            break;
        }
        case 0x19: { // DAD_D
            uint16_t hl = ((uint16_t) status_.h << 8) | ((uint16_t) status_.l);
            uint16_t tmp = mem[hl];
            uint16_t de = ((uint16_t) status_.d << 8) | ((uint16_t) status_.e);
            uint16_t deContent = mem[de];
            tmp += deContent;
            mem[hl] = tmp;
            updateControls(tmp, {CARRY});
            break;
        }
        case 0x1a: { // LDAX_D
            uint16_t de = ((uint16_t) status_.d << 8) | ((uint16_t) status_.e);
            status_.a = mem[de];
            break;
        }
        case 0x1b: { // DCX_D
            uint16_t de = ((uint16_t) status_.d << 8) | ((uint16_t) status_.e);
            mem[de] = mem[de] - 1;
            break;
        }
        case 0x1c: { // INR_E
            uint16_t tmp = (uint16_t) status_.e + 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.e = (tmp & 0xff);
            break;
        }
        case 0x1d: { // DCR_E
            uint16_t tmp = status_.e - 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.e = (tmp & 0xff);
            break;
        }
        case 0x1e: { // MVI_E
            status_.e = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x1f: { // RAR
            uint8_t tmp = status_.a & 0x1;
            status_.a >>= 1;
            if (status_.controls.c) {
                status_.a |= 0x80;
            }
            status_.controls.c = tmp;
            break;
        }
        case 0x20: { // NOP
            break;
        }
        case 0x21: { // LXI_H
            status_.l = mem[pc + 1];
            status_.h = mem[pc + 2];
            status_.pc += 2;
            break;
        }
        case 0x22: { // SHLD
            uint16_t r = mem[pc + 1];
            uint16_t l = mem[pc + 2];
            uint16_t offset = l << 8 | r;
            mem[offset] = status_.l;
            mem[offset+1] = status_.h;

            status_.pc += 2;
            break;
        }
        case 0x23: { // INX_H
            uint16_t hl = (status_.h << 8) | status_.l;
            hl += 1;
            status_.h = (hl >> 8);
            status_.l = hl & 0xff;
            break;
        }
        case 0x24: { // INR_H
            uint16_t tmp = (uint16_t) status_.h + 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.h = (tmp & 0xff);
            break;
        }
        case 0x25: { // DCR_H
            uint16_t tmp = status_.h - 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.h = (tmp & 0xff);
            break;
        }
        case 0x26: { // MVI_H
            status_.h = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x27: { // DAA unused
            throw NotImplementedInstruction(0x27);
        }
        case 0x28: { // NOP
            break;
        }
        case 0x29: { // DAD_H
            uint16_t hl = ((uint16_t) status_.h << 8) | ((uint16_t) status_.l);
            uint16_t tmp = mem[hl];
            tmp += tmp;
            mem[hl] = tmp;
            updateControls(tmp, {CARRY});
            break;
        }
        case 0x2a: { // LHLD
            uint16_t r = mem[pc + 1];
            uint16_t l = mem[pc + 2];
            uint16_t offset = l << 8 | r;
            status_.l = mem[offset];
            status_.h = mem[offset+1];

            status_.pc += 2;
            break;
        }
        case 0x2b: { // DCX_H
            uint16_t hl = ((uint16_t) status_.h << 8) | ((uint16_t) status_.l);
            mem[hl] = mem[hl] - 1;
            break;
        }
        case 0x2c: { // INR_L
            uint16_t tmp = (uint16_t) status_.l + 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.l = (tmp & 0xff);
            break;
        }
        case 0x2d: { // DCR_L
            uint16_t tmp = status_.l - 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.l = (tmp & 0xff);
            break;
        }
        case 0x2e: { // MVI_L
            status_.l = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x2f: { // CMA
            status_.a = ~status_.a;
            break;
        }
        case 0x30: { // NOP
            break;
        }
        case 0x31: { // LXI_SP
            Byte lo = mem[pc + 1];
            Byte hi = mem[pc + 2];
            status_.sp = (hi << 8) | (lo);
            status_.pc += 2;
            break;
        }
        case 0x32: { // STA
            Byte lo = mem[pc + 1];
            Byte hi = mem[pc + 2];
            uint16_t addr = (hi << 8) | (lo);
            mem[addr] = status_.a;
            status_.pc += 2;
            break;
        }
        case 0x33: { // INX_SP
            status_.sp += 1;
            break;
        }
        case 0x34: { // INR_M
            uint16_t offset =  (status_.h << 8) | (status_.l);
            uint16_t tmp = mem[offset];
            tmp += 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            mem[offset] = (tmp & 0xff);
            break;
        }
        case 0x35: { // DCR_M
            uint16_t offset =  (status_.h << 8) | (status_.l);
            uint16_t tmp = mem[offset];
            tmp -= 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            mem[offset] = (tmp & 0xff);
            break;
        }
        case 0x36: { // MVI_M
            uint16_t offset =  (status_.h << 8) | (status_.l);
            mem[offset] = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x37: { // STC
            status_.controls.c = true;
            break;
        }
        case 0x38: { // NOP
            break;
        }
        case 0x39: { // DAD_SP
            uint16_t hl = (status_.h << 8) | (status_.l);
            uint16_t tmp = status_.sp + mem[hl];
            mem[hl] = tmp;
            updateControls(tmp, {CARRY});
            break;
        }
        case 0x3a: { // LDA
            uint16_t offset = (mem[pc + 2] << 8) | (mem[pc + 1]);
            status_.a = mem[offset];
            status_.pc += 2;
            break;
        }
        case 0x3b: { // DCX_SP
            status_.sp -= 1;
            break;
        }
        case 0x3c: { // INR_A
            uint16_t tmp = (uint16_t) status_.a + 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.a = (tmp & 0xff);
            break;
        }
        case 0x3d: { // DCR_A
            uint16_t tmp = status_.a - 1;
            updateControls(tmp, {SIGN, ZERO, PARITY});
            status_.a = (tmp & 0xff);
            break;
        }
        case 0x3e: { // MVI_A
            status_.a = mem[pc + 1];
            ++status_.pc;
            break;
        }
        case 0x3f: { // CMC
            status_.controls.c = !status_.controls.c;
            break;
        }
        case 0x40: { // MOV_BB
            status_.b = status_.b;
            break;
        }
        case 0x41: { // MOV_BC
            status_.b = status_.c;
            break;
        }
        case 0x42: { // MOV_BD
            status_.b = status_.d;
            break;
        }
        case 0x43: { // MOV_BE
            status_.b = status_.e;
            break;
        }
        case 0x44: { // MOV_BH
            status_.b = status_.h;
            break;
        }
        case 0x45: { // MOV_BL
            status_.b = status_.l;
            break;
        }
        case 0x46: { // MOV_BM
            status_.b = mem[(status_.h << 8) | (status_.l)];
            break;
        }
        case 0x47: { // MOV_BA
            status_.b = status_.a;
            break;
        }
        case 0x48: { // MOV_CB
            status_.c = status_.b;
            break;
        }
        case 0x49: { // MOV_CC
            status_.c = status_.c;
            break;
        }
        case 0x4a: { // MOV_CD
            status_.c = status_.d;
            break;
        }
        case 0x4b: { // MOV_CE
            status_.c = status_.e;
            break;
        }
        case 0x4c: { // MOV_CH
            status_.c = status_.h;
            break;
        }
        case 0x4d: { // MOV_CL
            status_.c = status_.l;
            break;
        }
        case 0x4e: { // MOV_CM
            status_.c = mem[(status_.h << 8) |  status_.l];
            break;
        }
        case 0x4f: { // MOV_CA
            status_.c = status_.a;
            break;
        }
        case 0x50: { // MOV_DB
            status_.d = status_.b;
            break;
        }
        case 0x51: { // MOV_DC
            status_.d = status_.c;
            break;
        }
        case 0x52: { // MOV_DD
            status_.d = status_.d;
            break;
        }
        case 0x53: { // MOV_DE
            status_.d = status_.e;
            break;
        }
        case 0x54: { // MOV_DH
            status_.d = status_.h;
            break;
        }
        case 0x55: { // MOV_DL
            status_.d = status_.l;
            break;
        }
        case 0x56: { // MOV_DM
            status_.d = mem[(status_.h << 8) | status_.l];
            break;
        }
        case 0x57: { // MOV_DA
            status_.d = status_.a;
            break;
        }
        case 0x58: { // MOV_EB
            status_.e = status_.b;
            break;
        }
        case 0x59: { // MOV_EC
            status_.e = status_.c;
            break;
        }
        case 0x5a: { // MOV_ED
            status_.e = status_.d;
            break;
        }
        case 0x5b: { // MOV_EE
            status_.e = status_.e;
            break;
        }
        case 0x5c: { // MOV_EH
            status_.e = status_.h;
            break;
        }
        case 0x5d: { // MOV_EL
            status_.e = status_.l;
            break;
        }
        case 0x5e: { // MOV_EM
            status_.e = mem[(status_.h << 8) | status_.l];
            break;
        }
        case 0x5f: { // MOV_EA
            status_.e = status_.a;
            break;
        }
        case 0x60: { // MOV_HB
            status_.h = status_.b;
            break;
        }
        case 0x61: { // MOV_HC
            status_.h = status_.c;
            break;
        }
        case 0x62: { // MOV_HD
            status_.h = status_.d;
            break;
        }
        case 0x63: { // MOV_HE
            status_.h = status_.e;
            break;
        }
        case 0x64: { // MOV_HH
            status_.h = status_.h;
            break;
        }
        case 0x65: { // MOV_HL
            status_.h = status_.l;
            break;
        }
        case 0x66: { // MOV_HM
            status_.h = mem[(status_.h << 8) | status_.l];
            break;
        }
        case 0x67: { // MOV_HA
            status_.h = status_.a;
            break;
        }
        case 0x68: { // MOV_LB
            status_.l = status_.b;
            break;
        }
        case 0x69: { // MOV_LC
            status_.l = status_.c;
            break;
        }
        case 0x6a: { // MOV_LD
            status_.l = status_.d;
            break;
        }
        case 0x6b: { // MOV_LE
            status_.l = status_.e;
            break;
        }
        case 0x6c: { // MOV_LH
            status_.l = status_.h;
            break;
        }
        case 0x6d: { // MOV_LL
            status_.l = status_.l;
            break;
        }
        case 0x6e: { // MOV_LM
            status_.l = mem[(status_.h << 8) | status_.l];
            break;
        }
        case 0x6f: { // MOV_LA
            status_.l = status_.a;
            break;
        }
        case 0x70: { // MOV_MB
            mem[(status_.h << 8) | status_.l] = status_.b;
            break;
        }
        case 0x71: { // MOV_MC
            mem[(status_.h << 8) | status_.l] = status_.c;
            break;
        }
        case 0x72: { // MOV_MD
            mem[(status_.h << 8) | status_.l] = status_.d;
            break;
        }
        case 0x73: { // MOV_ME
            mem[(status_.h << 8) | status_.l] = status_.e;
            break;
        }
        case 0x74: { // MOV_MH
            mem[(status_.h << 8) | status_.l] = status_.h;
            break;
        }
        case 0x75: { // MOV_ML
            mem[(status_.h << 8) | status_.l] = status_.l;
            break;
        }
        case 0x76: { // HLT
            throw NotImplementedInstruction(0x76);
        }
        case 0x77: { // MOV_MA
            mem[(status_.h << 8) | status_.l] = status_.a;
            break;
        }
        case 0x78: { // MOV_AB
            status_.a = status_.b;
            break;
        }
        case 0x79: { // MOV_AC
            status_.a = status_.c;
            break;
        }
        case 0x7a: { // MOV_AD
            status_.a = status_.d;
            break;
        }
        case 0x7b: { // MOV_AE
            status_.a = status_.e;
            break;
        }
        case 0x7c: { // MOV_AH
            status_.a = status_.h;
            break;
        }
        case 0x7d: { // MOV_AL
            status_.a = status_.l;
            break;
        }
        case 0x7e: { // MOV_AM
            status_.a = mem[(status_.h << 8) | status_.a];
            break;
        }
        case 0x7f: { // MOV_AA
            status_.a = status_.a;
            break;
        }
        case 0x80: { // ADD_B
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.b;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x81: { // ADD_C
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x82: { // ADD_D
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.d;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x83: { // ADD_E
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.e;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x84: { // ADD_H
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.h;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x85: { // ADD_L
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.l;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x86: { // ADD_M
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) mem[(status_.h << 8) | (status_.l)];
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x87: { // ADD_A
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.a;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x88: { // ADC_B
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.b + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x89: { // ADC_C
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.c + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x8a: { // ADC_D
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.d + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x8b: { // ADC_E
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.e + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x8c: { // ADC_H
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.h + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x8d: { // ADC_L
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.l + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x8e: { // ADC_M
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) mem[(status_.h << 8) | (status_.l)] + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x8f: { // ADC_A
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) status_.a + (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x90: { // SUB_B
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.b;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x91: { // SUB_C
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x92: { // SUB_D
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.d;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x93: { // SUB_E
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.e;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x94: { // SUB_H
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.h;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x95: { // SUB_L
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.l;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x96: { // SUB_M
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) mem[(status_.h << 8) | (status_.l)];
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x97: { // SUB_A
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.a;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x98: { // SBB_B
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.b - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x99: { // SBB_C
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.c - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x9a: { // SBB_D
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.d - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x9b: { // SBB_E
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.e - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x9c: { // SBB_H
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.h - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x9d: { // SBB_L
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.l - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x9e: { // SBB_M
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) mem[(status_.h << 8) | (status_.l)] - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0x9f: { // SBB_A
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) status_.a - (uint16_t) status_.controls.c;
            updateControls(tmp, {SIGN, ZERO, PARITY, CARRY});
            status_.a = tmp & 0xff;
            break;
        }
        case 0xa0: { // ANA_B
            ana(status_.b);
            break;
        }
        case 0xa1: { // ANA_C
            ana(status_.c);
            break;
        }
        case 0xa2: { // ANA_D
            ana(status_.d);
            break;
        }
        case 0xa3: { // ANA_E
            ana(status_.e);
            break;
        }
        case 0xa4: { // ANA_H
            ana(status_.h);
            break;
        }
        case 0xa5: { // ANA_L
            ana(status_.l);
            break;
        }
        case 0xa6: { // ANA_M
            ana(mem[(status_.h << 8) | (status_.l)]);
            break;
        }
        case 0xa7: { // ANA_A
            ana(status_.a);
            break;
        }
        case 0xa8: { // XRA_B
            xra(status_.b);
            break;
        }
        case 0xa9: { // XRA_C
            xra(status_.c);
            break;
        }
        case 0xaa: { // XRA_D
            xra(status_.d);
            break;
        }
        case 0xab: { // XRA_E
            xra(status_.e);
            break;
        }
        case 0xac: { // XRA_H
            xra(status_.h);
            break;
        }
        case 0xad: { // XRA_L
            xra(status_.l);
            break;
        }
        case 0xae: { // XRA_M
            xra(mem[(status_.h << 8) | (status_.l)]);
            break;
        }
        case 0xaf: { // XRA_A
            xra(status_.a);
            break;
        }
        case 0xb0: { // ORA_B
            ora(status_.b);
            break;
        }
        case 0xb1: { // ORA_C
            ora(status_.c);
            break;
        }
        case 0xb2: { // ORA_D
            ora(status_.d);
            break;
        }
        case 0xb3: { // ORA_E
            ora(status_.e);
            break;
        }
        case 0xb4: { // ORA_H
            ora(status_.h);
            break;
        }
        case 0xb5: { // ORA_L
            ora(status_.l);
            break;
        }
        case 0xb6: { // ORA_M
            ora(mem[(status_.h << 8) | (status_.l)]);
            break;
        }
        case 0xb7: { // ORA_A
            ora(status_.a);
            break;
        }
        case 0xb8: { // CMP_B
            cmp(status_.b);
            break;
        }
        case 0xb9: { // CMP_C
            cmp(status_.c);
            break;
        }
        case 0xba: { // CMP_D
            cmp(status_.d);
            break;
        }
        case 0xbb: { // CMP_E
            cmp(status_.e);
            break;
        }
        case 0xbc: { // CMP_H
            cmp(status_.h);
            break;
        }
        case 0xbd: { // CMP_L
            cmp(status_.l);
            break;
        }
        case 0xbe: { // CMP_M
            cmp(mem[(status_.h << 8) | (status_.l)]);
            break;
        }
        case 0xbf: { // CMP_A
            cmp(status_.a);
            break;
        }
        case 0xc0: { // RNZ
            if (status_.controls.z) break;

            status_.pc = (mem[status_.sp+1] << 8) | (mem[status_.sp]);
            status_.sp += 2;
            break;
        }
        case 0xc1: { // POP_B
            pop(status_.b, status_.c);
            break;
        }
        case 0xc2: { // JNZ
            if (status_.controls.z) break;
            jmp();
            break;
        }
        case 0xc3: { // JMP
            status_.pc = ((uint16_t) mem[pc+2] << 8) | ((uint16_t) mem[pc+1]);
            break;
        }
        case 0xc4: { // CNZ
            if (status_.controls.z) break;
            call();
            break;
        }
        case 0xc5: { // PUSH_B
            push(status_.b, status_.c);
            break;
        }
        case 0xc6: { // ADI
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) mem[pc+1];
            updateControls(tmp, {CARRY, PARITY, SIGN, ZERO});
            status_.a = tmp & 0xff;
            ++status_.pc;
            break;
        }
        case 0xc7: { // RST_0
            throw NotImplementedInstruction(0xc7);
        }
        case 0xc8: { // RZ
            if (!status_.controls.z) break;
            ret();
            break;
        }
        case 0xc9: { // RET
            ret();
            break;
        }
        case 0xca: { // JZ
            if (!status_.controls.z) break;
            jmp();
            break;
        }
        case 0xcb: { // JMP
            status_.pc = ((uint16_t) mem[status_.pc+2] << 8) | ((uint16_t) mem[status_.pc+1]);
            break;
        }
        case 0xcc: { // CZ
            if (!status_.controls.z) break;
            call();
            break;
        }
        case 0xcd: { // CALL
            call();
            break;
        }
        case 0xce: { // ACI
            uint16_t tmp = (uint16_t) status_.a + (uint16_t) mem[pc+1] + status_.controls.c;
            updateControls(tmp, {CARRY, PARITY, SIGN, ZERO});
            status_.a = tmp & 0xff;
            break;
        }
        case 0xcf: { // RST_1
            throw NotImplementedInstruction(0xcf);
        }
        case 0xd0: { // RNC
            if (status_.controls.c) break;
            ret();
            break;
        }
        case 0xd1: { // POP_D
            pop(status_.d, status_.e);
            break;
        }
        case 0xd2: { // JNC
            if (status_.controls.c) break;
            jmp();
            break;
        }
        case 0xd3: { // OUT
            ++status_.pc;
            break;
        }
        case 0xd4: { // CNC
            if (status_.controls.c) break;
            call();
            break;
        }
        case 0xd5: { // PUSH_D
            push(status_.d, status_.e);
            break;
        }
        case 0xd6: { // SUI
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) mem[pc+1];
            updateControls(tmp, {CARRY, PARITY, SIGN, ZERO});
            status_.a = tmp & 0xff;
            ++status_.pc;
            break;
        }
        case 0xd7: { // RST_2
            throw NotImplementedInstruction(0xd7);
        }
        case 0xd8: { // RC
            if (!status_.controls.c) break;
            ret();
            break;
        }
        case 0xd9: { // RET
            ret();
            break;
        }
        case 0xda: { // JC
            if (!status_.controls.c) break;
            jmp();
            break;
        }
        case 0xdb: { // IN
            ++status_.pc;
            break;
        }
        case 0xdc: { // CC
            if (!status_.controls.c) break;
            call();
            break;
        }
        case 0xdd: { // CALL
            call();
            break;
        }
        case 0xde: { // SBI
            uint16_t tmp = (uint16_t) status_.a - (uint16_t) mem[pc+1] - status_.controls.c;
            updateControls(tmp, {CARRY, PARITY, SIGN, ZERO});
            status_.a = tmp & 0xff;
            break;
        }
        case 0xdf: { // RST_3
            throw NotImplementedInstruction(0xdf);
        }
        case 0xe0: { // RPO
            if (status_.controls.p) break;
            ret();
            break;
        }
        case 0xe1: { // POP_H
            pop(status_.h, status_.l);
            break;
        }
        case 0xe2: { // JPO
            if (status_.controls.p) break;
            jmp();
            break;
        }
        case 0xe3: { // XTHL
            Byte tmp = status_.h;
            status_.h = mem[status_.sp+1];
            mem[status_.sp+1] = tmp;
            tmp = status_.l;
            status_.l = mem[status_.sp];
            mem[status_.sp] = tmp;
            break;
        }
        case 0xe4: { // CPO
            if (status_.controls.p) break;
            call();
            break;
        }
        case 0xe5: { // PUSH_H
            push(status_.h, status_.l);
            break;
        }
        case 0xe6: { // ANI
            ana(mem[status_.pc+1]);
            ++status_.pc;
            break;
        }
        case 0xe7: { // RST_4
            throw NotImplementedInstruction(0xe7);
        }
        case 0xe8: { // RPE
            if (!status_.controls.p) break;
            ret();
            break;
        }
        case 0xe9: { // PCHL
            status_.pc = ((uint16_t) status_.h << 8) | (status_.l);
            break;
        }
        case 0xea: { // JPE
            if (!status_.controls.p) break;
            jmp();
            break;
        }
        case 0xeb: { // XCHG
            Byte tmp = status_.h;
            status_.h = status_.d;
            status_.d = tmp;
            tmp = status_.l;
            status_.l = status_.e;
            status_.e = tmp;
            break;
        }
        case 0xec: { // CPE
            if (!status_.controls.p) break;
            call();
            break;
        }
        case 0xed: { // CALL
            call();
            break;
        }
        case 0xee: { // XRI
            xra(mem[status_.pc+1]);
            ++status_.pc;
            break;
        }
        case 0xef: { // RST_5
            throw NotImplementedInstruction(0xef);
        }
        case 0xf0: { // RP
            if (status_.controls.s) break;
            ret();
            break;
        }
        case 0xf1: { // POP_PSW
            Byte b = mem[status_.sp];
            status_.controls.c = b & 0x01;
            status_.controls.p = b & 0x04;
            status_.controls.z = b & 0x40;
            status_.controls.s = b & 0x80;
            status_.a = mem[status_.sp+1];
            status_.sp += 2;
            break;
        }
        case 0xf2: { // JP
            if (status_.controls.s) break;
            jmp();
            break;
        }
        case 0xf3: { // DI
            status_.is_interrupt_enabled = false;
            break;
        }
        case 0xf4: { // CP
            if (status_.controls.s) break;
            call();
            break;
        }
        case 0xf5: { // PUSH_PSW
            mem[status_.sp-1] = status_.a;
            Controls const& controls = status_.controls;
            Byte psw = controls.s << 7 | controls.z << 6 | controls.p << 2 | 0x02 | controls.c;
            mem[status_.sp-2] = psw;
            status_.sp -= 2;
            break;
        }
        case 0xf6: { // ORI
            ora(mem[status_.pc+1]);
            ++status_.pc;
            break;
        }
        case 0xf7: { // RST_6
            throw NotImplementedInstruction(0xf7);
        }
        case 0xf8: { // RM
            if (!status_.controls.s) break;
            ret();
            break;
        }
        case 0xf9: { // SPHL
            status_.sp = ((uint16_t) status_.h << 8) | (status_.l);
            break;
        }
        case 0xfa: { // JM
            if (!status_.controls.s) break;
            jmp();
            break;
        }
        case 0xfb: { // EI
            status_.is_interrupt_enabled = true;
            break;
        }
        case 0xfc: { // CM
            if (!status_.controls.s) break;
            call();
            break;
        }
        case 0xfd: { // CALL
            call();
            break;
        }
        case 0xfe: { // CPI
            cmp(mem[status_.pc+1]);
            ++status_.pc;
            break;
        }
        case 0xff: { // RST_7
            throw NotImplementedInstruction(0xff);
        }
    }
}
