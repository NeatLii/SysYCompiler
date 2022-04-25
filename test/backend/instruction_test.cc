#include "backend/instruction.h"

#include <gtest/gtest.h>

#include "error.h"

#define REG(id) (std::make_shared<backend::RegOperand>(id))
#define IMM32(imm) \
    (std::make_shared<backend::ImmOperand>(static_cast<std::int32_t>(imm)))
#define IMM16(imm) \
    (std::make_shared<backend::ImmOperand>(static_cast<std::int16_t>(imm)))
#define LABEL(label) (std::make_shared<backend::LabelOperand>(label))

TEST(InstructionTest, Mov) {
    ASSERT_THROW(backend::InsMov(REG(0), IMM32(0xf0f0f0f0)),
                 InvalidParameterException);
    ASSERT_NO_THROW(backend::InsMov(REG(0), IMM32(0xff000000)));
    ASSERT_NO_THROW(backend::InsMov(REG(0), IMM16(0xffff)));
    backend::InsMov mov(REG(0), REG(1));
    backend::InsMov mov1(REG(0), IMM32(10));
    EXPECT_STREQ("\tmov\tr0, r1", mov.Str().c_str());
    EXPECT_STREQ("\tmov\tr0, #10", mov1.Str().c_str());
}

TEST(InstructionTest, Ldr) {
    backend::InsLdr ldr(REG(0), REG(1));
    backend::InsLdr ldr1(REG(0), REG(1), IMM32(10));
    backend::InsLdr ldr2(REG(0), IMM32(10));
    backend::InsLdr ldr3(REG(0), LABEL("global_var_a"));
    EXPECT_STREQ("\tldr\tr0, [r1]", ldr.Str().c_str());
    EXPECT_STREQ("\tldr\tr0, [r1, #10]", ldr1.Str().c_str());
    EXPECT_STREQ("\tldr\tr0, =#10", ldr2.Str().c_str());
    EXPECT_STREQ("\tldr\tr0, =global_var_a", ldr3.Str().c_str());
}

TEST(InstructionTest, Str) {
    backend::InsStr str(REG(0), REG(1));
    backend::InsStr str1(REG(0), REG(1), IMM32(10));
    EXPECT_STREQ("\tstr\tr0, [r1]", str.Str().c_str());
    EXPECT_STREQ("\tstr\tr0, [r1, #10]", str1.Str().c_str());
}

TEST(InstructionTest, Push) {
    std::vector<std::shared_ptr<backend::RegOperand>> v;
    v.emplace_back(REG(0));
    backend::InsPush push(v);
    v.emplace_back(REG(7));
    v.emplace_back(REG(backend::RegOperand::kLr));
    backend::InsPush push1(v);
    EXPECT_STREQ("\tpush\t{r0}", push.Str().c_str());
    EXPECT_STREQ("\tpush\t{r0, r7, lr}", push1.Str().c_str());
}

TEST(InstructionTest, Pop) {
    std::vector<std::shared_ptr<backend::RegOperand>> v;
    v.emplace_back(REG(0));
    backend::InsPop pop(v);
    v.emplace_back(REG(7));
    v.emplace_back(REG(14));
    backend::InsPop pop1(v);
    EXPECT_STREQ("\tpop\t{r0}", pop.Str().c_str());
    EXPECT_STREQ("\tpop\t{r0, r7, lr}", pop1.Str().c_str());
}

TEST(InstructionTest, Cmp) {
    ASSERT_THROW(backend::InsCmp(REG(0), IMM32(0xf0f0f0f0)),
                 InvalidParameterException);
    ASSERT_NO_THROW(backend::InsCmp(REG(0), IMM32(0x0ff00000)));
    backend::InsCmp cmp(REG(0), REG(1));
    backend::InsCmp cmp1(REG(0), IMM32(10));
    EXPECT_STREQ("\tcmp\tr0, r1", cmp.Str().c_str());
    EXPECT_STREQ("\tcmp\tr0, #10", cmp1.Str().c_str());
}

TEST(InstructionTest, B) {
    backend::InsB b(LABEL("func.true"));
    backend::InsB b1(LABEL("func.true"), backend::InsB::kEQ);
    backend::InsB b2(LABEL("func.true"), backend::InsB::kNE);
    backend::InsB b3(LABEL("func.true"), backend::InsB::kGT);
    backend::InsB b4(LABEL("func.true"), backend::InsB::kGE);
    backend::InsB b5(LABEL("func.true"), backend::InsB::kLT);
    backend::InsB b6(LABEL("func.true"), backend::InsB::kLE);
    EXPECT_STREQ("\tb\tfunc.true", b.Str().c_str());
    EXPECT_STREQ("\tbeq\tfunc.true", b1.Str().c_str());
    EXPECT_STREQ("\tbne\tfunc.true", b2.Str().c_str());
    EXPECT_STREQ("\tbgt\tfunc.true", b3.Str().c_str());
    EXPECT_STREQ("\tbge\tfunc.true", b4.Str().c_str());
    EXPECT_STREQ("\tblt\tfunc.true", b5.Str().c_str());
    EXPECT_STREQ("\tble\tfunc.true", b6.Str().c_str());
}

TEST(InstructionTest, Bl) {
    backend::InsBl bl(LABEL("func"));
    EXPECT_STREQ("\tbl\tfunc(PLT)", bl.Str().c_str());
}

TEST(InstructionTest, Bx) {
    backend::InsBx bx(REG(backend::RegOperand::kLr));
    EXPECT_STREQ("\tbx\tlr", bx.Str().c_str());
}

TEST(InstructionTest, Add) {
    ASSERT_THROW(backend::InsAdd(REG(0), REG(1), IMM32(0xfff00000)),
                 InvalidParameterException);
    ASSERT_NO_THROW(backend::InsAdd(REG(0), REG(1), IMM32(0x0ff00000)));
    backend::InsAdd add(REG(0), REG(1), REG(2));
    backend::InsAdd add1(REG(0), REG(1), IMM32(10));
    EXPECT_STREQ("\tadd\tr0, r1, r2", add.Str().c_str());
    EXPECT_STREQ("\tadd\tr0, r1, #10", add1.Str().c_str());
}

TEST(InstructionTest, Sub) {
    ASSERT_THROW(backend::InsSub(REG(0), REG(1), IMM32(0xfff00000)),
                 InvalidParameterException);
    ASSERT_NO_THROW(backend::InsSub(REG(0), REG(1), IMM32(0x0ff00000)));
    backend::InsSub sub(REG(0), REG(1), REG(2));
    backend::InsSub sub1(REG(0), REG(1), IMM32(10));
    EXPECT_STREQ("\tsub\tr0, r1, r2", sub.Str().c_str());
    EXPECT_STREQ("\tsub\tr0, r1, #10", sub1.Str().c_str());
}

TEST(InstructionTest, Mul) {
    backend::InsMul mul(REG(0), REG(1), REG(2));
    EXPECT_STREQ("\tmul\tr0, r1, r2", mul.Str().c_str());
}

TEST(InstructionTest, SDiv) {
    backend::InsSDiv sdiv(REG(0), REG(1), REG(2));
    EXPECT_STREQ("\tsdiv\tr0, r1, r2", sdiv.Str().c_str());
}

TEST(InstructionTest, And) {
    ASSERT_THROW(backend::InsAnd(REG(0), REG(1), IMM32(0xfff00000)),
                 InvalidParameterException);
    ASSERT_NO_THROW(backend::InsAnd(REG(0), REG(1), IMM32(0x0ff00000)));
    backend::InsAnd and_(REG(0), REG(1), REG(2));
    backend::InsAnd and1(REG(0), REG(1), IMM32(10));
    EXPECT_STREQ("\tand\tr0, r1, r2", and_.Str().c_str());
    EXPECT_STREQ("\tand\tr0, r1, #10", and1.Str().c_str());
}

TEST(InstructionTest, Orr) {
    ASSERT_THROW(backend::InsOrr(REG(0), REG(1), IMM32(0xfff00000)),
                 InvalidParameterException);
    ASSERT_NO_THROW(backend::InsOrr(REG(0), REG(1), IMM32(0x0ff00000)));
    backend::InsOrr orr(REG(0), REG(1), REG(2));
    backend::InsOrr orr1(REG(0), REG(1), IMM32(10));
    EXPECT_STREQ("\torr\tr0, r1, r2", orr.Str().c_str());
    EXPECT_STREQ("\torr\tr0, r1, #10", orr1.Str().c_str());
}

TEST(InstructionTest, Nop) {
    backend::InsNop nop;
    EXPECT_STREQ("\tnop", nop.Str().c_str());
}

TEST(InstructionTest, Label) {
    backend::InsLabel label(LABEL("main"));
    EXPECT_STREQ("main:", label.Str().c_str());
}

class AssemblyTest : public testing::Test {
  protected:
    void SetUp() override {
        Init_a();
        Init_b();
        Init_func();
        Init_main();
    }

    void Init_a() {
        backend::GlobalVar var_a("a", std::vector<std::int32_t>{100});
        std::string result_a = "\n"
                               "\t.global a\n"
                               "\t.type a, %object\n"
                               "\t.size a, 4\n"
                               "a:\n"
                               "\t.word 100\n";
        vars.emplace_back(var_a, result_a);
    }
    void Init_b() {
        backend::GlobalVar var_b("b", std::vector<std::int32_t>{200});
        std::string result_b = "\n"
                               "\t.global b\n"
                               "\t.type b, %object\n"
                               "\t.size b, 4\n"
                               "b:\n"
                               "\t.word 200\n";
        vars.emplace_back(var_b, result_b);
    }
    void Init_func() {
        backend::Function func("func");
        func.AddInst(std::make_shared<backend::InsPush>(
            std::vector<std::shared_ptr<backend::RegOperand>>{
                REG(backend::RegOperand::kFp), REG(backend::RegOperand::kLr)}));
        func.AddInst(std::make_shared<backend::InsMov>(REG(1), IMM32(10)));
        func.AddInst(std::make_shared<backend::InsMov>(REG(2), IMM32(20)));
        func.AddInst(std::make_shared<backend::InsCmp>(REG(1), REG(2)));
        func.AddInst(std::make_shared<backend::InsB>(LABEL("func.false"),
                                                     backend::InsCmp::kGT));
        func.AddInst(std::make_shared<backend::InsMov>(REG(0), REG(1)));
        func.AddInst(std::make_shared<backend::InsB>(LABEL("func.end")));
        func.AddInst(std::make_shared<backend::InsLabel>(LABEL("func.false")));
        func.AddInst(std::make_shared<backend::InsMov>(REG(0), REG(2)));
        func.AddInst(std::make_shared<backend::InsLabel>(LABEL("func.end")));
        func.AddInst(std::make_shared<backend::InsBl>(LABEL("putint")));
        func.AddInst(std::make_shared<backend::InsMov>(REG(0), IMM32(10)));
        func.AddInst(std::make_shared<backend::InsBl>(LABEL("putch")));
        func.AddInst(std::make_shared<backend::InsPop>(
            std::vector<std::shared_ptr<backend::RegOperand>>{
                REG(backend::RegOperand::kFp), REG(backend::RegOperand::kPc)}));
        std::string result = "\n"
                             "\t.global func\n"
                             "\t.type func, %function\n"
                             "func:\n"
                             "\tpush\t{fp, lr}\n"
                             "\tmov\tr1, #10\n"
                             "\tmov\tr2, #20\n"
                             "\tcmp\tr1, r2\n"
                             "\tbgt\tfunc.false\n"
                             "\tmov\tr0, r1\n"
                             "\tb\tfunc.end\n"
                             "func.false:\n"
                             "\tmov\tr0, r2\n"
                             "func.end:\n"
                             "\tbl\tputint(PLT)\n"
                             "\tmov\tr0, #10\n"
                             "\tbl\tputch(PLT)\n"
                             "\tpop\t{fp, pc}\n";
        funcs.emplace_back(func, result);
    }
    void Init_main() {
        backend::Function func("main");
        func.AddInst(std::make_shared<backend::InsPush>(
            std::vector<std::shared_ptr<backend::RegOperand>>{
                REG(backend::RegOperand::kFp), REG(backend::RegOperand::kLr)}));
        func.AddInst(std::make_shared<backend::InsBl>(LABEL("func")));
        func.AddInst(std::make_shared<backend::InsMov>(REG(0), IMM32(0)));
        func.AddInst(std::make_shared<backend::InsPop>(
            std::vector<std::shared_ptr<backend::RegOperand>>{
                REG(backend::RegOperand::kFp), REG(backend::RegOperand::kPc)}));
        std::string result = "\n"
                             "\t.global main\n"
                             "\t.type main, %function\n"
                             "main:\n"
                             "\tpush\t{fp, lr}\n"
                             "\tbl\tfunc(PLT)\n"
                             "\tmov\tr0, #0\n"
                             "\tpop\t{fp, pc}\n";
        funcs.emplace_back(func, result);
    }

    std::vector<std::pair<backend::GlobalVar, std::string>> vars;
    std::vector<std::pair<backend::Function, std::string>> funcs;
};

TEST_F(AssemblyTest, GlobalVarDump) {
    for (auto &pair : vars) {
        std::ostringstream s;
        pair.first.Dump(s);
        EXPECT_STREQ(pair.second.c_str(), s.str().c_str());
    }
}

TEST_F(AssemblyTest, FunctionDump) {
    for (auto &pair : funcs) {
        std::ostringstream s;
        pair.first.Dump(s);
        EXPECT_STREQ(pair.second.c_str(), s.str().c_str());
    }
}

TEST_F(AssemblyTest, AssemblyDump) {
    backend::Assembly assembly;
    std::string result = "\t.arch armv7-a\n";
    result += "\n\t.data\n";
    for (auto &pair : vars) {
        assembly.AddVar(std::make_shared<backend::GlobalVar>(pair.first));
        result += pair.second;
    }
    result += "\n\t.text\n";
    for (auto &pair : funcs) {
        assembly.AddFunc(std::make_shared<backend::Function>(pair.first));
        result += pair.second;
    }
    std::ostringstream s;
    assembly.Dump(s);
    EXPECT_STREQ(result.c_str(), s.str().c_str());
}
