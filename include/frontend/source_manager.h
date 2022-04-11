#ifndef __source_manager_h__
#define __source_manager_h__

#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace ast {

struct SourceRange {
    int begin_line, begin_column;
    int end_line, end_column;
    // not colorful
    std::string Dump() const;
};

#ifndef AST_SOURCE_RANGE_ONLY

struct Token {
    const std::string text;
    const SourceRange range;

    Token(std::string text, const SourceRange &range)
        : text(std::move(text)), range(range) {}
    virtual ~Token() = default;
    Token(const Token &) = default;
    Token &operator=(const Token &) = delete;
    Token(Token &&) = default;
    Token &operator=(Token &&) = delete;

    // colorful
    std::string DumpText() const;
    std::string DumpTextRef() const;
    std::string DumpRange() const;
};

struct IdentToken final : public Token {
    const int indent;
    static constexpr int kGlobal = 0;
    IdentToken(std::string text, const SourceRange &range, int indent)
        : Token(std::move(text), range), indent(indent) {}
};

// index in token table
using TokenLocation = std::vector<Token>::size_type;

// manage token table
class SourceManager final {
  public:
    explicit SourceManager(std::string file_name)
        : file_name(std::move(file_name)) {}

    std::string GetFileName() const { return file_name; }

    TokenLocation AddToken(const Token &token);
    TokenLocation AddToken(const std::string &text, const SourceRange &range);
    TokenLocation AddIdentToken(const IdentToken &ident);
    TokenLocation AddIdentToken(const std::string &text,
                                const SourceRange &range,
                                int indent);

    const Token &GetToken(TokenLocation loc) const { return token_table[loc]; }
    const std::string &GetTokenText(TokenLocation loc) const {
        return token_table[loc].text;
    }
    const SourceRange &GetTokenRange(TokenLocation loc) const {
        return token_table[loc].range;
    }
    int GetIdentTokenIndent(TokenLocation loc) const {
        return dynamic_cast<const IdentToken &>(token_table[loc]).indent;
    }

    void Dump(std::ostream &ostream) const;

  private:
    const std::string file_name;
    std::vector<Token> token_table;
};

#endif

}  // namespace ast

#endif
