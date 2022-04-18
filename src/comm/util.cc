#include "util.h"

#include <iomanip>

#include "error.h"

namespace util {

std::string FormatTerminal(const std::string &text,
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
            throw InvalidParameterValueException(__FILE__, __LINE__, "Format",
                                                 "ForegroundColor fg_color",
                                                 std::to_string(fg_color));
        }
    }
    if (bg_color != kBGDefault) {
        if ((kBGBlack <= bg_color && bg_color <= kBGWhite)
            || (kBGBrightBlack <= bg_color && bg_color <= kBGBrightWhite)) {
            prefix += ';' + std::to_string(bg_color);
        } else {
            throw InvalidParameterValueException(__FILE__, __LINE__, "Format",
                                                 "BackgroundColor bg_color",
                                                 std::to_string(bg_color));
        }
    }
    for (Layout layout : layouts) {
        if (kDefault <= layout && layout <= kStrike) {
            prefix += ';' + std::to_string(layout);
        } else {
            throw InvalidParameterValueException(__FILE__, __LINE__, "Format",
                                                 "Layout layout",
                                                 std::to_string(layout));
        }
    }
    prefix += 'm';
    return prefix + text + suffix;
}

std::string FormatTerminalBold(const std::string &text,
                               ForegroundColor fg_color,
                               BackgroundColor bg_color) {
    return FormatTerminal(text, fg_color, bg_color, {kBold});
}

#ifdef SHOW_ALL_FORMAT
void howAllFormat() {
    for (int i = kFGBlack; i <= kFGWhite; ++i) {
        std::cout << FormatTerminal("text", static_cast<ForegroundColor>(i),
                                    kBGDefault, {})
                  << ' ';
    }
    std::cout << std::endl;
    for (int i = kFGBrightBlack; i <= kFGBrightWhite; ++i) {
        std::cout << FormatTerminal("text", static_cast<ForegroundColor>(i),
                                    kBGDefault, {})
                  << ' ';
    }
    std::cout << std::endl;
    for (int i = kBGBlack; i <= kBGWhite; ++i) {
        std::cout << FormatTerminal("text", kFGDefault,
                                    static_cast<BackgroundColor>(i), {})
                  << ' ';
    }
    std::cout << std::endl;
    for (int i = kBGBrightBlack; i <= kBGBrightWhite; ++i) {
        std::cout << FormatTerminal("text", kFGDefault,
                                    static_cast<BackgroundColor>(i), {})
                  << ' ';
    }
    std::cout << std::endl;
    for (int i = kDefault; i <= kStrike; ++i) {
        std::cout << FormatTerminal("text", kFGDefault, kBGDefault, {(Layout)i})
                  << ' ';
    }
    std::cout << std::endl;
}
#endif

std::string FormatHex32(std::uint32_t num) {
    std::stringstream buf;
    buf << std::setw(sizeof(num) * 2) << std::setfill('0');
    buf << std::hex << num;
    return "0x" + buf.str();
}

}  // namespace util
