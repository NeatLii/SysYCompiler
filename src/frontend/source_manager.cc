#include "frontend/source_manager.h"

#include <string>

#include "util.h"

namespace ast {

/* struct SourceRange */

std::string SourceRange::DumpBegin() const {
    return "line:" + std::to_string(begin_line) + ':'
           + std::to_string(begin_column);
}

std::string SourceRange::DumpEnd() const {
    if (begin_line == end_line) { return "col:" + std::to_string(end_column); }
    return "line:" + std::to_string(end_line) + ':'
           + std::to_string(end_column);
}

std::string SourceRange::Dump() const {
    return '<' + DumpBegin() + ", " + DumpEnd() + '>';
}

/* struct Token */

std::string Token::DumpText() const {
    return util::FormatTerminalBold(text, util::kFGBrightBlue);
}

std::string Token::DumpTextRef() const {
    return util::FormatTerminalBold('\'' + text + '\'', util::kFGBrightBlue);
}

std::string Token::DumpRange() const {
    return '<' + util::FormatTerminal(range.DumpBegin(), util::kFGYellow) + ", "
           + util::FormatTerminal(range.DumpEnd(), util::kFGYellow) + '>';
}

/* struct TokenRange */

// std::string TokenRange::DumpBegin(const SourceManager &src) const {
//     return src.GetTokenRange(begin).DumpBegin();
// }

// std::string TokenRange::DumpEnd(const SourceManager &src) const {
//     const SourceRange &begin_src = src.GetTokenRange(begin);
//     const SourceRange &end_src = src.GetTokenRange(end);
//     if (begin_src.begin_line == begin_src.end_line
//         && begin_src.end_line == end_src.begin_line
//         && end_src.begin_line == end_src.end_line) {
//         return end_src.DumpEnd();
//     }
//     return "line:" + std::to_string(end_src.end_line) + ':'
//            + std::to_string(end_src.end_column);
// }

// std::string TokenRange::Dump(const SourceManager &src) const {
//     return '<' + DumpBegin(src) + ", " + DumpEnd(src) + '>';
// }

/* struct SourceManager */

TokenLocation SourceManager::AddToken(const Token &token) {
    token_table.emplace_back(token);
    return token_table.size() - 1;
}

TokenLocation SourceManager::AddToken(const std::string &text,
                                      const SourceRange &range) {
    token_table.emplace_back(text, range);
    return token_table.size() - 1;
}

void SourceManager::Dump(std::ostream &ostream) const {
    ostream << util::FormatTerminalBold("Dump tokens from file",
                                        util::kFGBrightGreen)
            << " '" << util::FormatTerminal(file_name, util::kFGYellow) << '\''
            << std::endl;
    TokenLocation loc = 0;
    for (const Token &token : token_table) {
        ostream << util::FormatTerminal(util::FormatHex32(loc++),
                                        util::kFGYellow)
                << ' ' << token.DumpRange() << ' ' << token.DumpText()
                << std::endl;
    }
}

}  // namespace ast
