#include "frontend/source_manager.h"

#include <ostream>

#include "util.h"

namespace ast {

/* struct SourceRange */

SourceRange SourceRange::operator+(const SourceRange &rhs) const {
    SourceRange sum{};
    // begin
    sum.begin_line = begin_line < rhs.begin_line ? begin_line : rhs.begin_line;
    if (begin_line == rhs.begin_line) {
        sum.begin_column
            = begin_column < rhs.begin_column ? begin_column : rhs.begin_column;
    } else {
        sum.begin_column
            = begin_line < rhs.begin_line ? begin_column : rhs.begin_column;
    }
    // end
    sum.end_line = end_line > rhs.end_line ? end_line : rhs.end_line;
    if (end_line == rhs.end_line) {
        sum.end_column
            = end_column > rhs.end_column ? end_column : rhs.end_column;
    } else {
        sum.end_column = end_line > rhs.end_line ? end_column : rhs.end_column;
    }
    return sum;
}

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
