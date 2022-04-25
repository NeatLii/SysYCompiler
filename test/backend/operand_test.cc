#include "backend/operand.h"

#include <gtest/gtest.h>

#include "error.h"

TEST(OperandTest, Register) {
    ASSERT_THROW(backend::RegOperand reg(-1), InvalidParameterValueException);
    backend::RegOperand r1(1);
    backend::RegOperand r7(7);
    backend::RegOperand fp(11);
    backend::RegOperand ip(12);
    backend::RegOperand sp(13);
    backend::RegOperand lr(14);
    backend::RegOperand pc(15);
    backend::RegOperand cpsr(16);
    backend::RegOperand r_virtual(20);
    EXPECT_STREQ("r1", r1.Str().c_str());
    EXPECT_STREQ("r7", r7.Str().c_str());
    EXPECT_STREQ("fp", fp.Str().c_str());
    EXPECT_STREQ("ip", ip.Str().c_str());
    EXPECT_STREQ("sp", sp.Str().c_str());
    EXPECT_STREQ("lr", lr.Str().c_str());
    EXPECT_STREQ("pc", pc.Str().c_str());
    EXPECT_STREQ("cpsr", cpsr.Str().c_str());
    EXPECT_STREQ("r20", r_virtual.Str().c_str());
    EXPECT_FALSE(r1.IsVirtual());
    EXPECT_TRUE(r_virtual.IsVirtual());
    EXPECT_FALSE(r7.IsSpecial());
    EXPECT_TRUE(sp.IsSpecial());
    EXPECT_FALSE(r_virtual.IsSpecial());
}

TEST(OperandTest, Immediate) {
    backend::ImmOperand imm(0);
    backend::ImmOperand imm1(1);
    backend::ImmOperand imm2(-1);
    backend::ImmOperand imm3(static_cast<std::int16_t>(0x7fff));  // 16-bit max
    backend::ImmOperand imm4(static_cast<std::int16_t>(0x8000));  // 16-bit min
    backend::ImmOperand imm5(
        static_cast<std::int32_t>(0x7fffffff));  // 32-bit max
    backend::ImmOperand imm6(
        static_cast<std::int32_t>(0x80000000));  // 32-bit min
    EXPECT_STREQ("#0", imm.Str().c_str());
    EXPECT_STREQ("#1", imm1.Str().c_str());
    EXPECT_STREQ("#-1", imm2.Str().c_str());
    EXPECT_STREQ("#32767", imm3.Str().c_str());
    EXPECT_STREQ("#-32768", imm4.Str().c_str());
    EXPECT_STREQ("#2147483647", imm5.Str().c_str());
    EXPECT_STREQ("#-2147483648", imm6.Str().c_str());
    EXPECT_TRUE(imm.IsImm16());
    EXPECT_TRUE(imm1.IsImm16());
    EXPECT_TRUE(imm2.IsImm16());
    EXPECT_TRUE(imm3.IsImm16());
    EXPECT_TRUE(imm4.IsImm16());
    EXPECT_FALSE(imm5.IsImm16());
    EXPECT_FALSE(imm6.IsImm16());
}

TEST(OperandTest, ImmediateIsImm8m) {
    backend::ImmOperand imm(
        static_cast<std::int32_t>(0x00ff0000));  // even number
    backend::ImmOperand imm1(
        static_cast<std::int32_t>(0x0007f800));  // odd number
    backend::ImmOperand imm2(
        static_cast<std::int32_t>(0x0001fe00));  // odd number
    backend::ImmOperand imm3(
        static_cast<std::int32_t>(0xf000000f));  // not exceeding 8-bit
    backend::ImmOperand imm4(
        static_cast<std::int32_t>(0xf00000ff));  // over 8-bit
    backend::ImmOperand imm5(
        static_cast<std::int32_t>(0xf0f0f0f0));  // over 8-bit
    EXPECT_STREQ("#16711680", imm.Str().c_str());
    EXPECT_STREQ("#522240", imm1.Str().c_str());
    EXPECT_STREQ("#130560", imm2.Str().c_str());
    EXPECT_STREQ("#-268435441", imm3.Str().c_str());
    EXPECT_STREQ("#-268435201", imm4.Str().c_str());
    EXPECT_STREQ("#-252645136", imm5.Str().c_str());
    EXPECT_TRUE(imm.IsImm8m());
    EXPECT_FALSE(imm1.IsImm8m());
    EXPECT_FALSE(imm2.IsImm8m());
    EXPECT_TRUE(imm3.IsImm8m());
    EXPECT_FALSE(imm4.IsImm8m());
    EXPECT_FALSE(imm5.IsImm8m());
}

TEST(OperandTest, Label) {
    backend::LabelOperand l_main("main");
    backend::LabelOperand l_func("func");
    EXPECT_STREQ("main", l_main.Str().c_str());
    EXPECT_STREQ("func", l_func.Str().c_str());
}
