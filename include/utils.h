#ifndef __utils_h__
#define __utils_h__

#include <initializer_list>
#include <string>

/* Reference: https://en.wikipedia.org/wiki/ANSI_escape_code */

namespace shell {

enum ForegroundColor {
    kFGDefault = 0,
    kFGBlack = 30,
    kFGRed,
    kFGGreen,
    kFGYellow,
    KFGBlue,
    kFGMagenta,
    kFGCyan,
    kFGWhite,
    kFGBrightBlack = 90,
    kFGBrightRed,
    kFGBrightGreen,
    kFGBrightYellow,
    KFGBrightBlue,
    kFGBrightMagenta,
    kFGBrightCyan,
    kFGBrightWhite
};

enum BackgroundColor {
    kBGDefault = 0,
    kBGBlack = 40,
    kBGRed,
    kBGGreen,
    kBGYellow,
    KBGBlue,
    kBGMagenta,
    kBGCyan,
    kBGWhite,
    kBGBrightBlack = 100,
    kBGBrightRed,
    kBGBrightGreen,
    kBGBrightYellow,
    KBGBrightBlue,
    kBGBrightMagenta,
    kBGBrightCyan,
    kBGBrightWhite
};

enum Layout {
    kDefault = 0,
    kBold,
    kDim,
    kItalic,
    kUnderLine,
    kBlink,
    kRapidBlink,
    kReverse,
    kHide,
    kStrike
};

std::string Format(const std::string &s,
                   ForegroundColor fg_color,
                   BackgroundColor bg_color,
                   std::initializer_list<Layout> layouts);

#ifdef SHOW_ALL_FORMAT
void ShowAllFormat();
#endif

}  // namespace shell

#endif
