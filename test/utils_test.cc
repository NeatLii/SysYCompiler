#include "utils.h"

#include "errors.h"
#include "gtest/gtest.h"

TEST(UtilsTest, Format) {
    ASSERT_NO_THROW(shell::Format("message", shell::KFGBlue, shell::kBGRed,
                                  {shell::kBold, shell::kItalic}));
    ASSERT_THROW(shell::Format("message", (shell::ForegroundColor)50,
                               shell::kBGRed, {shell::kBold, shell::kItalic}),
                 InvalidParameterValueException);
    ASSERT_THROW(
        shell::Format("message", shell::KFGBlue, (shell::BackgroundColor)50,
                      {shell::kBold, shell::kItalic}),
        InvalidParameterValueException);
    ASSERT_THROW(shell::Format("message", shell::KFGBlue, shell::kBGRed,
                               {(shell::Layout)50, shell::kItalic}),
                 InvalidParameterValueException);
    EXPECT_STREQ("\e[34;41;1;3mmessage\e[0m",
                 shell::Format("message", shell::KFGBlue, shell::kBGRed,
                               {shell::kBold, shell::kItalic})
                     .c_str());
}
