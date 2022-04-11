#include "error.h"
#include "gtest/gtest.h"
#include "util.h"

TEST(UtilsTest, ShellFormat) {
    ASSERT_NO_THROW(util::FormatTerminal("message", util::kFGBlue, util::kBGRed,
                                         {util::kBold, util::kItalic}));
    ASSERT_THROW(
        util::FormatTerminal("message", (util::ForegroundColor)50, util::kBGRed,
                             {util::kBold, util::kItalic}),
        InvalidParameterValueException);
    ASSERT_THROW(util::FormatTerminal("message", util::kFGBlue,
                                      (util::BackgroundColor)50,
                                      {util::kBold, util::kItalic}),
                 InvalidParameterValueException);
    ASSERT_THROW(util::FormatTerminal("message", util::kFGBlue, util::kBGRed,
                                      {(util::Layout)50, util::kItalic}),
                 InvalidParameterValueException);
    EXPECT_STREQ("\e[34;41;1;3mmessage\e[0m",
                 util::FormatTerminal("message", util::kFGBlue, util::kBGRed,
                                      {util::kBold, util::kItalic})
                     .c_str());
}

TEST(UtilsTest, FormatHex32) {
    ASSERT_STREQ("0x00000000", util::FormatHex32(0U).c_str());
    ASSERT_STREQ("0x0000024a", util::FormatHex32(586U).c_str());
}
