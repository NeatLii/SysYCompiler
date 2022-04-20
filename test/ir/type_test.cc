#include "ir/type.h"

#include "gtest/gtest.h"

using namespace ir;

TEST(TypeTest, VoidType) {
    VoidType voidtype;
    EXPECT_STREQ("void", voidtype.Str().c_str());
}

TEST(TypeTest, FuncType) {
    FuncType func(new VoidType, {new IntType(ir::IntType::kI1),
                                 new IntType(ir::IntType::kI32)});
    EXPECT_STREQ("void (i1, i32)", func.Str().c_str());
}

TEST(TypeTest, IntType) {
    IntType int1(ir::IntType::kI1);
    IntType int32(ir::IntType::kI32);
    EXPECT_STREQ("i1", int1.Str().c_str());
    EXPECT_STREQ("i32", int32.Str().c_str());
}

TEST(TypeTest, PtrType) {
    PtrType ptr(new IntType(ir::IntType::kI32));
    EXPECT_STREQ("i32*", ptr.Str().c_str());
}

TEST(TypeTest, LabelType) {
    LabelType label;
    EXPECT_STREQ("label", label.Str().c_str());
}

TEST(TypeTest, ArrayType) {
    ArrayType array(new PtrType(new IntType(ir::IntType::kI32)), {4, 2, 1});
    EXPECT_STREQ("[4 x [2 x [1 x i32*]]]", array.Str().c_str());
}
