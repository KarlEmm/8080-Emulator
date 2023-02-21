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
