//
// Created by KarlE on 2/15/2023.
//

#include "gtest/gtest.h"
#include <emulator.h>

class StatusTest : public ::testing::Test {
protected:
    void SetUp() override {
        emulator_.status_.pc = 0;
    }

    Emulator emulator_;
    Status& status = emulator_.status_;
};

TEST_F(StatusTest, LXI_B) {
    status.memory[0] = 0x01;
    status.memory[1] = 0x11;
    status.memory[2] = 0x22;
    emulator_.emulateOp();
    uint16_t bc = 0x2211;
    EXPECT_EQ(status.b << 8 | status.c, bc);
}

TEST_F(StatusTest, STAX_B) {
    status.memory[0] = 0x02;
    status.a = 9;
    status.b = 8;
    status.c = 1;
    emulator_.emulateOp();
    uint16_t offset = 0x0801;
    EXPECT_EQ(status.memory[offset], 9);
}

TEST_F(StatusTest, INX_B) {
    status.memory[0] = 0x03;
    status.b = 0xff;
    status.c = 0xff;
    emulator_.emulateOp();
    uint16_t bc = 0x0000;
    EXPECT_EQ(status.b << 8 | status.c, bc);

    status.pc = 0;
    status.b = 0x08;
    status.c = 0x0f;
    emulator_.emulateOp();
    bc = 0x0810;
    EXPECT_EQ(status.b << 8 | status.c, bc);
}

TEST_F(StatusTest, INR_B) {
    status.memory[0] = 0x04;
    status.b = 0xff;
    emulator_.emulateOp();
    Byte b = 0x00;
    EXPECT_EQ(status.b, b);
    EXPECT_FALSE(status.controls.s);
    EXPECT_TRUE(status.controls.z);
    EXPECT_TRUE(status.controls.p);

    status.pc = 0;
    status.b = 0x82;
    emulator_.emulateOp();
    b = 0x83;
    EXPECT_EQ(status.b, b);
    EXPECT_TRUE(status.controls.s);
    EXPECT_FALSE(status.controls.z);
    EXPECT_FALSE(status.controls.p);
}

TEST_F(StatusTest, DCR_B) {
    status.memory[0] = 0x05;
    status.b = 0x00;
    emulator_.emulateOp();
    Byte b = 0xff;
    EXPECT_EQ(status.b, b);
    EXPECT_TRUE(status.controls.s);
    EXPECT_FALSE(status.controls.z);
    EXPECT_TRUE(status.controls.p);

    status.pc = 0;
    status.b = 0x74;
    emulator_.emulateOp();
    b = 0x73;
    EXPECT_EQ(status.b, b);
    EXPECT_FALSE(status.controls.s);
    EXPECT_FALSE(status.controls.z);
    EXPECT_FALSE(status.controls.p);
}

TEST_F(StatusTest, MVI_B) {
    status.memory[0] = 0x06;
    status.memory[1] = 0x10;
    status.b = 0x00;
    emulator_.emulateOp();
    EXPECT_EQ(status.b, 0x10);
}

TEST_F(StatusTest, RLC) {
    status.memory[0] = 0x07;
    status.a = 0x91;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x23);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.a = 0x60;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0xC0);
    EXPECT_FALSE(status.controls.c);
}

TEST_F(StatusTest, DAD_B) {
    status.memory[0] = 0x09;
    status.b = 0x00;
    status.c = 0x01;
    status.h = 0x80;
    status.l = 0x01;
    status.memory[0x0001] = 255;
    status.memory[0x8001] = 1;
    emulator_.emulateOp();
    EXPECT_EQ(status.memory[0x8001], 0x00);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.memory[0x0001] = 0x02;
    status.memory[0x8001] = 0x03;
    emulator_.emulateOp();
    EXPECT_EQ(status.memory[0x8001], 0x05);
    EXPECT_FALSE(status.controls.c);
}

TEST_F(StatusTest, LDAX_B) {
    status.memory[0] = 0x0a;
    status.a = 0x00;
    status.b = 0x01;
    status.c = 0x02;
    status.memory[0x0102] = 0x05;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x05);
}

TEST_F(StatusTest, DCX_B) {
    status.memory[0] = 0x0b;
    status.b = 0x01;
    status.c = 0x02;
    status.memory[0x0102] = 0x05;
    emulator_.emulateOp();
    EXPECT_EQ(status.memory[0x0102], 0x04);
}

TEST_F(StatusTest, RRC) {
    status.memory[0] = 0x0f;
    status.a = 0x01;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x80);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.controls.c = false;
    status.a = 0x82;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x41);
    EXPECT_FALSE(status.controls.c);
}

TEST_F(StatusTest, RAL) {
    status.memory[0] = 0x17;
    status.a = 0x01;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x02);
    EXPECT_FALSE(status.controls.c);

    status.pc = 0;
    status.controls.c = false;
    status.a = 0x81;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x02);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.controls.c = true;
    status.a = 0x81;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x03);
    EXPECT_TRUE(status.controls.c);
}

TEST_F(StatusTest, RAR) {
    status.memory[0] = 0x1f;
    status.a = 0x01;
    status.controls.c = false;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x0);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.controls.c = true;
    status.a = 0x01;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x80);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.controls.c = true;
    status.a = 0x10;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x88);
    EXPECT_FALSE(status.controls.c);
}

TEST_F(StatusTest, SHLD) {
    status.memory[0] = 0x22;
    status.memory[1] = 0x10;
    status.memory[2] = 0x23;
    status.h = 0x10;
    status.l = 0x11;
    emulator_.emulateOp();
    EXPECT_EQ(status.memory[0x2310], 0x11);
    EXPECT_EQ(status.memory[0x2311], 0x10);
}

TEST_F(StatusTest, LHLD) {
    status.memory[0] = 0x2a;
    status.memory[1] = 0x10;
    status.memory[2] = 0x23;
    status.memory[0x2310] = 0x12;
    status.memory[0x2311] = 0x23;
    status.h = 0x00;
    status.l = 0x00;
    emulator_.emulateOp();
    EXPECT_EQ(status.l, 0x12);
    EXPECT_EQ(status.h, 0x23);
}

TEST_F(StatusTest, CMA) {
    status.memory[0] = 0x2f;
    status.a = 0x9a;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x65);
}

TEST_F(StatusTest, LXI_SP) {
    status.memory[0] = 0x31;
    status.memory[1] = 0x81;
    status.memory[2] = 0x03;
    emulator_.emulateOp();
    EXPECT_EQ(status.sp, 0x0381);
}

TEST_F(StatusTest, INR_M) {
    status.memory[0] = 0x34;
    status.h = 0x04;
    status.l = 0x03;
    status.memory[0x0403] = 0x02;
    emulator_.emulateOp();
    EXPECT_EQ(status.memory[0x0403], 0x03);
    EXPECT_FALSE(status.controls.s);
    EXPECT_TRUE(status.controls.p);
    EXPECT_FALSE(status.controls.z);
}

TEST_F(StatusTest, CMC) {
    status.memory[0] = 0x3f;
    status.controls.c = true;
    emulator_.emulateOp();
    EXPECT_FALSE(status.controls.c);

    status.pc = 0;
    status.controls.c = false;
    emulator_.emulateOp();
    EXPECT_TRUE(status.controls.c);
}

TEST_F(StatusTest,  MOV_BM) {
    status.memory[0] = 0x46;
    status.b = 0x00;
    status.h = 0x10;
    status.l = 0x01;
    status.memory[0x1001] = 0x11;
    emulator_.emulateOp();
    EXPECT_EQ(status.b, 0x11);
}

TEST_F(StatusTest,  ADD_B) {
    status.memory[0] = 0x80;
    status.a = 0x80;
    status.b = 0x81;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x01);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.a = 0x10;
    status.b = 0x81;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x91);
    EXPECT_FALSE(status.controls.c);
}

TEST_F(StatusTest,  ADC_B) {
    status.memory[0] = 0x88;
    status.a = 0x80;
    status.b = 0x81;
    status.controls.c = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x02);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.a = 0x10;
    status.b = 0x81;
    status.controls.c = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x92);
    EXPECT_FALSE(status.controls.c);

    status.pc = 0;
    status.a = 0x10;
    status.b = 0x81;
    status.controls.c = false;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x91);
    EXPECT_FALSE(status.controls.c);
}

TEST_F(StatusTest,  SUB_B) {
    status.memory[0] = 0x90;
    status.a = 0x80;
    status.b = 0x7f;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x01);
    EXPECT_FALSE(status.controls.c);

    status.pc = 0;
    status.a = 0x80;
    status.b = 0x81;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0xff);
    EXPECT_TRUE(status.controls.c);
}

TEST_F(StatusTest,  SBB_B) {
    status.memory[0] = 0x98;
    status.a = 0x82;
    status.b = 0x81;
    status.controls.c = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x00);
    EXPECT_FALSE(status.controls.c);

    status.pc = 0;
    status.a = 0x80;
    status.b = 0x7f;
    status.controls.c = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x00);
    EXPECT_FALSE(status.controls.c);

    status.pc = 0;
    status.a = 0x81;
    status.b = 0x81;
    status.controls.c = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0xff);
    EXPECT_TRUE(status.controls.c);

    status.pc = 0;
    status.a = 0x81;
    status.b = 0x80;
    status.controls.c = false;
    emulator_.emulateOp();
    EXPECT_EQ(status.a, 0x01);
    EXPECT_FALSE(status.controls.c);
}

TEST_F(StatusTest,  RNZ) {
    status.pc = 0;
    status.sp = 0x3000;
    status.memory[0] = 0xc0;
    status.memory[status.sp] = 0x20;
    status.memory[status.sp+1] = 0x10;
    status.controls.z = false;
    emulator_.emulateOp();
    EXPECT_EQ(status.pc, 0x1020);
    EXPECT_EQ(status.sp, 0x3002);

    status.pc = 0;
    status.sp = 0x3000;
    status.controls.z = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.pc, 0x0);
    EXPECT_EQ(status.sp, 0x3000);
}

TEST_F(StatusTest,  POP_B) {
    status.pc = 0;
    status.sp = 0x3000;
    status.memory[0] = 0xc1;
    status.memory[status.sp] = 0x20;
    status.memory[status.sp+1] = 0x10;
    emulator_.emulateOp();
    EXPECT_EQ(status.b, 0x10);
    EXPECT_EQ(status.c, 0x20);
    EXPECT_EQ(status.sp, 0x3002);
}

TEST_F(StatusTest,  JNZ) {
    status.pc = 0;
    status.sp = 0x3000;
    status.memory[0] = 0xc2;
    status.memory[1] = 0x12;
    status.memory[2] = 0x34;
    status.controls.z = false;
    emulator_.emulateOp();
    EXPECT_EQ(status.pc, 0x3412);

    status.pc = 0;
    status.controls.z = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.pc, 0x0000);
}

TEST_F(StatusTest, JMP) {
    status.pc = 0;
    status.memory[0] = 0xc3;
    status.memory[1] = 0x10;
    status.memory[2] = 0x20;

    emulator_.emulateOp();
    EXPECT_EQ(status.pc, 0x2010);
}

TEST_F(StatusTest, CNZ) {
    status.pc = 0x1122;
    status.memory[status.pc] = 0xc4;
    status.sp = 0x3000;
    status.memory[status.sp-1] = 0x00;
    status.memory[status.sp-2] = 0x00;
    status.memory[0x1123] = 0x15;
    status.memory[0x1124] = 0x16;

    status.controls.z = false;
    emulator_.emulateOp();
    EXPECT_EQ(status.memory[0x2fff], 0x11);
    EXPECT_EQ(status.memory[0x2ffe], 0x22);
    EXPECT_EQ(status.sp, 0x2ffe);
    EXPECT_EQ(status.pc, 0x1615);

    status.pc = 0x1122;
    status.memory[status.pc] = 0xc4;
    status.sp = 0x3000;
    status.memory[status.sp-1] = 0x00;
    status.memory[status.sp-2] = 0x00;
    status.memory[0x1123] = 0x15;
    status.memory[0x1124] = 0x16;

    status.controls.z = true;
    emulator_.emulateOp();
    EXPECT_EQ(status.memory[0x2fff], 0x00);
    EXPECT_EQ(status.memory[0x2ffe], 0x00);
    EXPECT_EQ(status.sp, 0x3000);
    EXPECT_EQ(status.pc, 0x1122);
}

TEST_F(StatusTest, PUSH_B) {
    status.pc = 0x00;
    status.memory[status.pc] = 0xc5;
    status.sp = 0x5566;
    status.memory[status.sp-1] = 0x00;
    status.memory[status.sp-2] = 0x00;
    status.b = 0x11;
    status.c = 0x22;
    emulator_.emulateOp();

    EXPECT_EQ(status.memory[0x5565], 0x11);
    EXPECT_EQ(status.memory[0x5564], 0x22);
    EXPECT_EQ(status.sp, 0x5564);
}

TEST_F(StatusTest, ADI) {
    status.memory[status.pc] = 0xc6;
    status.a = 0xff;
    status.memory[status.pc+1] = 0x01;
    emulator_.emulateOp();

    EXPECT_EQ(status.a, 0x00);
    EXPECT_TRUE(status.controls.c);
    EXPECT_TRUE(status.controls.z);
    EXPECT_FALSE(status.controls.s);

    status.pc = 0x0000;
    status.a = 0xfe;
    emulator_.emulateOp();

    EXPECT_EQ(status.a, 0xff);
    EXPECT_FALSE(status.controls.c);
    EXPECT_FALSE(status.controls.z);
    EXPECT_TRUE(status.controls.s);
}

TEST_F(StatusTest, RZ) {
    status.memory[status.pc] = 0xc8;
    status.sp = 0x3000;
    status.memory[status.sp] = 0x10;
    status.memory[status.sp+1] = 0x20;
    status.controls.z = true;

    emulator_.emulateOp();

    EXPECT_EQ(status.pc, 0x2010);
    EXPECT_EQ(status.sp, 0x3002);

    status.pc = 0;
    status.sp = 0x3000;
    status.memory[status.sp] = 0x10;
    status.memory[status.sp+1] = 0x20;
    status.controls.z = false;

    emulator_.emulateOp();

    EXPECT_EQ(status.pc, 0x0000);
    EXPECT_EQ(status.sp, 0x3000);
}

TEST_F(StatusTest, JZ) {
    status.memory[status.pc] = 0xca;
    status.memory[status.pc+1] = 0x10;
    status.memory[status.pc+2] = 0x30;
    status.controls.z = true;

    emulator_.emulateOp();

    EXPECT_EQ(status.pc, 0x3010);

    status.pc = 0;
    status.memory[status.pc+1] = 0x10;
    status.memory[status.pc+2] = 0x30;
    status.controls.z = false;

    emulator_.emulateOp();

    EXPECT_EQ(status.pc, 0x0000);
}

TEST_F(StatusTest, ACI) {
    status.memory[status.pc] = 0xce;
    status.a = 0xfe;
    status.memory[status.pc+1] = 0x01;
    status.controls.c = true;
    emulator_.emulateOp();

    EXPECT_EQ(status.a, 0x00);
    EXPECT_TRUE(status.controls.c);
    EXPECT_TRUE(status.controls.z);
    EXPECT_FALSE(status.controls.s);

    status.a = 0xfd;
    status.controls.c = false;
    emulator_.emulateOp();

    EXPECT_EQ(status.a, 0xfe);
    EXPECT_FALSE(status.controls.c);
    EXPECT_FALSE(status.controls.z);
    EXPECT_TRUE(status.controls.s);
}

TEST_F(StatusTest, IN) {
    EXPECT_TRUE(false);
}

TEST_F(StatusTest, OUT) {
    EXPECT_TRUE(false);
}

TEST_F(StatusTest, XCHG) {
    status.memory[status.pc] = 0xeb;
    status.d = 0x33;
    status.e = 0x44;
    status.h = 0x11;
    status.l = 0x22;

    emulator_.emulateOp();

    EXPECT_EQ(status.d, 0x11);
    EXPECT_EQ(status.e, 0x22);
    EXPECT_EQ(status.h, 0x33);
    EXPECT_EQ(status.l, 0x44);
}

TEST_F(StatusTest, PCHL) {
    status.memory[status.pc] = 0xe9;
    status.h = 0x11;
    status.l = 0x22;

    emulator_.emulateOp();

    EXPECT_EQ(status.pc, 0x1122);
}

TEST_F(StatusTest, POP_PSW) {
    status.memory[status.pc] = 0xf1;
    status.sp = 0x3000;
    status.memory[status.sp] = 0xc5;
    status.memory[status.sp+1] = 0x88;
    status.a = 0x00;
    status.controls.c = false;
    status.controls.p = false;
    status.controls.s = false;
    status.controls.z = false;

    emulator_.emulateOp();

    EXPECT_EQ(status.a, 0x88);
    EXPECT_EQ(status.sp, 0x3002);
    EXPECT_TRUE(status.controls.c);
    EXPECT_TRUE(status.controls.p);
    EXPECT_TRUE(status.controls.s);
    EXPECT_TRUE(status.controls.z);
}

TEST_F(StatusTest, XTHL) {
    status.memory[status.pc] = 0xe3;
    status.sp = 0x3000;
    status.memory[status.sp] = 0x33;
    status.memory[status.sp+1] = 0x44;
    status.h = 0x11;
    status.l = 0x22;

    emulator_.emulateOp();

    EXPECT_EQ(status.memory[status.sp], 0x22);
    EXPECT_EQ(status.memory[status.sp+1], 0x11);
    EXPECT_EQ(status.h, 0x44);
    EXPECT_EQ(status.l, 0x33);
}

TEST_F(StatusTest, PUSH_PSW) {
    status.memory[status.pc] = 0xf5;
    status.sp = 0x0005;
    status.memory[status.sp-1] = 0x20;
    status.memory[status.sp-2] = 0x10;
    status.a = 0x05;
    status.controls.c = true;
    status.controls.p = true;
    status.controls.s = true;
    status.controls.z = true;

    emulator_.emulateOp();

    EXPECT_EQ(status.memory[0x0004], 0x05);
    EXPECT_EQ(status.memory[0x0003], 0xc7);
    EXPECT_EQ(status.sp, 0x0003);
}
