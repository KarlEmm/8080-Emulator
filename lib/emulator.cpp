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

void Emulator::emulateOp() {
    auto& mem = status_.memory;
    uint16_t pc = status_.pc;
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
            break;
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
    }

//    uint8_t bytesInOp = 1;
//    printf("%04x\t", pc);
//    switch(ops[pc]) {
//        case 0x00: { printf("NOP"); break; }
//        case 0x01: { printf("LXI B %02x %02x", ops[pc+1], ops[pc+2]); bytesInOp += 2; break; } // Load Byte 2 and Byte 3 into rp lower and higher bytes.
//        case 0x02: { printf("STAX B"); break; }  // Store accumulator indirect
//        case 0x03: { printf("INX B"); break; }   // Increment rp
//        case 0x04: { printf("INR B"); break; }   // Increment register
//        case 0x05: { printf("DCR B"); break; }   // Decrement register
//        case 0x06: { printf("MVI B %02x"     , ops[pc+1])           ; bytesInOp += 1; break; } // Move immediate
//        case 0x07: { printf("RLC  "); break; }   // Rotate left
//        case 0x08: { printf("*NOP") ; break; }
//        case 0x09: { printf("DAD B"); break; }   // Add BC to HL into HL
//        case 0x0a: { printf("LDAX B"); break; }  // Load rp into accumulator indirect
//        case 0x0b: { printf("DCX B"); break; }   // Decrement rp
//        case 0x0c: { printf("INR C"); break; }
//        case 0x0d: { printf("DCR C"); break; }
//        case 0x0e: { printf("MVI C %02x"     , ops[pc+1])           ; bytesInOp += 1; break; }
//        case 0x0f: { printf("RRC  "); break; }   // Rotate right
//        case 0x10: { printf("*NOP "); break; }
//        case 0x11: { printf("LXI D %02x %02x", ops[pc+1], ops[pc+2]); bytesInOp += 2; break; }
//        case 0x12: { printf("STAX D"); break; }
//        case 0x13: { printf("INX D"); break; }
//        case 0x14: { printf("INR D"); break; }
//        case 0x15: { printf("DCR D"); break; }
//        case 0x16: { printf("MVI D %02x"     , ops[pc+1])           ; bytesInOp += 1; break; }
//        case 0x17: { printf("RAL  "); break; }   // Rotate left through carry bit
//        case 0x18: { printf("*NOP"); break; }
//        case 0x19: { printf("DAD D"); break; }
//        case 0x1a: { printf("LDAX D"); break; }
//        case 0x1b: { printf("DCX D"); break; }
//        case 0x1c: { printf("INR E"); break; }
//        case 0x1d: { printf("DCR E"); break; }
//        case 0x1e: { printf("MVI E %02x"     , ops[pc+1])           ; bytesInOp += 1; break; }
//        case 0x1f: { printf("RAR  "); break; }
//        case 0x20: { printf("*NOP "); break; }
//        case 0x21: { printf("LXI H %02x %02x", ops[pc+1], ops[pc+2]); bytesInOp += 2; break; }
//        case 0x22: { printf("SHLD %02x %02x" , ops[pc+1], ops[pc+2]); bytesInOp += 2; break; } // (byte3 byte2) <- L and (byte3 byte2 + 1) <- H
//        case 0x23: { printf("INX H"); break; }
//        case 0x24: { printf("INR H"); break; }
//        case 0x25: { printf("DCR H"); break; }
//        case 0x26: { printf("MVI H %02x"     , ops[pc+1])           ; bytesInOp += 1; break; }
//        case 0x27: { printf("DAA") ; break; } // TODO: Not clear to me
//        case 0x28: { printf("*NOP"); break; }
//        case 0x29: { printf("DAD H"); break; }
//        case 0x2a: { printf("LHLD %02x %02x" , ops[pc+1], ops[pc+2]); bytesInOp += 2; break; } // L <- (byte3 byte2) and H <- (byte3 byte2 + 1)
//        case 0x2b: { printf("DCX H"); break; }
//        case 0x2c: { printf("INR L"); break; }
//        case 0x2d: { printf("DCR L"); break; }
//        case 0x2e: { printf("MVI L %02x"     , ops[pc+1])           ; bytesInOp += 1; break; }
//        case 0x2f: { printf("CNA  "); break; } // Complement accumulator
//        case 0x84: { printf("ADD L"); break; }
//        case 0x85: { printf("ADD M"); break; }
//        case 0xa5: { printf("ANA L"); break; }
//    }
//
//    printf("\n");
//
//    return bytesInOp;
}
