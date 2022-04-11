#ifndef __utils_h__
#define __utils_h__

#include <cstdint>
#include <initializer_list>
#include <string>

namespace util {

// Reference: https://en.wikipedia.org/wiki/ANSI_escape_code

enum ForegroundColor {
    kFGDefault = 0,
    kFGBlack = 30,
    kFGRed,
    kFGGreen,
    kFGYellow,
    kFGBlue,
    kFGMagenta,
    kFGCyan,
    kFGWhite,
    kFGBrightBlack = 90,
    kFGBrightRed,
    kFGBrightGreen,
    kFGBrightYellow,
    kFGBrightBlue,
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
    kBGBlue,
    kBGMagenta,
    kBGCyan,
    kBGWhite,
    kBGBrightBlack = 100,
    kBGBrightRed,
    kBGBrightGreen,
    kBGBrightYellow,
    kBGBrightBlue,
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

// colorful terminal output
std::string FormatTerminal(const std::string &text,
                           ForegroundColor fg_color,
                           BackgroundColor bg_color,
                           std::initializer_list<Layout> layouts);

#ifdef SHOW_ALL_FORMAT
void ShowAllFormat();
#endif

std::string FormatHex32(std::uint32_t num);

}  // namespace util

#endif
