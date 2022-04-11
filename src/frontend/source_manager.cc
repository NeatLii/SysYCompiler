#include <string>

#include "frontend/source_manager.h"
#include "util.h"

namespace ast {

std::string SourceRange::Dump() const {
    std::string begin = "<line:" + std::to_string(begin_line) + ':'
                        + std::to_string(begin_column) + ", ";
    std::string end;
    if (begin_line == end_line) {
        return "col:" + std::to_string(end_column) + '>';
    }
    return "line:" + std::to_string(end_line) + ':' + std::to_string(end_column)
           + '>';
}

std::string Token::DumpText() const {
    return util::FormatTerminal(text, util::kFGBrightBlue, util::kBGDefault,
                                {util::kBold});
}

std::string Token::DumpTextRef() const {
    return util::FormatTerminal('\'' + text + '\'', util::kFGBrightBlue,
                                util::kBGDefault, {util::kBold});
}

std::string Token::DumpRange() const {
    return util::FormatTerminal(range.Dump(), util::kFGYellow, util::kBGDefault,
                                {});
}

TokenLocation SourceManager::AddToken(const Token &token) {
    token_table.emplace_back(token);
    return token_table.size() - 1;
}

TokenLocation SourceManager::AddToken(const std::string &text,
                                      const SourceRange &range) {
    token_table.emplace_back(text, range);
    return token_table.size() - 1;
}

TokenLocation SourceManager::AddIdentToken(const IdentToken &ident) {
    token_table.emplace_back(ident);
    return token_table.size() - 1;
}

TokenLocation SourceManager::AddIdentToken(const std::string &text,
                                           const SourceRange &range,
                                           const int indent) {
    token_table.emplace_back(IdentToken(text, range, indent));
    return token_table.size() - 1;
}

void SourceManager::Dump(std::ostream &ostream) const {
    ostream << util::FormatTerminal("SourceFile ", util::kFGBrightGreen,
                                    util::kBGDefault, {util::kBold})
            << '<'
            << util::FormatTerminal(file_name, util::kFGYellow,
                                    util::kBGDefault, {})
            << '>' << std::endl;
    TokenLocation loc = 0;
    for (const auto &token : token_table) {
        ostream << util::FormatTerminal(util::FormatHex32(loc++),
                                        util::kFGYellow, util::kBGDefault, {})
                << ' ' << token.DumpRange() << ' ' << token.DumpText()
                << std::endl;
    }
}

}  // namespace ast
