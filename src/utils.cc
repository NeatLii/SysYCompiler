#include "utils.h"

#include <iostream>
#include <string>

#include "errors.h"

std::string shell::Format(const std::string &s,
                          ForegroundColor fg_color,
                          BackgroundColor bg_color,
                          std::initializer_list<Layout> layouts) {
    std::string prefix = "\e[";
    std::string suffix = "\e[0m";
    if (fg_color != kFGDefault) {
        if ((kFGBlack <= fg_color && fg_color <= kFGWhite)
            || (kFGBrightBlack <= fg_color && fg_color <= kFGBrightWhite)) {
            prefix += std::to_string(fg_color);
        } else {
            throw InvalidParameterValueException(
                __FILE__, __LINE__, __FUNCTION__, "ForegroundColor fg_color",
                std::to_string(fg_color));
        }
    }
    if (bg_color != kBGDefault) {
        if ((kBGBlack <= bg_color && bg_color <= kBGWhite)
            || (kBGBrightBlack <= bg_color && bg_color <= kBGBrightWhite)) {
            prefix += ';' + std::to_string(bg_color);
        } else {
            throw InvalidParameterValueException(
                __FILE__, __LINE__, __FUNCTION__, "BackgroundColor bg_color",
                std::to_string(bg_color));
        }
    }
    for (Layout l : layouts) {
        if (kDefault <= l && l <= kStrike) {
            prefix += ';' + std::to_string(l);
        } else {
            throw InvalidParameterValueException(__FILE__, __LINE__,
                                                 __FUNCTION__, "Layout layout",
                                                 std::to_string(l));
        }
    }
    prefix += 'm';
    return prefix + s + suffix;
}

#ifdef SHOW_ALL_FORMAT
void shell::ShowAllFormat() {
    for (int i = kFGBlack; i <= kFGWhite; ++i) {
        std::cout << Format("text", (ForegroundColor)i, kBGDefault, {}) << ' ';
    }
    std::cout << std::endl;
    for (int i = kFGBrightBlack; i <= kFGBrightWhite; ++i) {
        std::cout << Format("text", (ForegroundColor)i, kBGDefault, {}) << ' ';
    }
    std::cout << std::endl;
    for (int i = kBGBlack; i <= kBGWhite; ++i) {
        std::cout << Format("text", kFGDefault, (BackgroundColor)i, {}) << ' ';
    }
    std::cout << std::endl;
    for (int i = kBGBrightBlack; i <= kBGBrightWhite; ++i) {
        std::cout << Format("text", kFGDefault, (BackgroundColor)i, {}) << ' ';
    }
    std::cout << std::endl;
    for (int i = kDefault; i <= kStrike; ++i) {
        std::cout << Format("text", kFGDefault, kBGDefault, {(Layout)i}) << ' ';
    }
    std::cout << std::endl;
}
#endif