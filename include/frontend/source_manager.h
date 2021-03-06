#ifndef __sysycompiler_frontend_source_manager_h__
#define __sysycompiler_frontend_source_manager_h__

#include <string>
#include <vector>

namespace ast {

/* definitions */

struct SourceRange {
    int begin_line, begin_column;
    int end_line, end_column;

    SourceRange operator+(const SourceRange &rhs) const;

    // not colorful
    std::string DumpBegin() const;
    std::string DumpEnd() const;
    std::string Dump() const;
};

struct Token {
    const std::string text;
    const SourceRange range;

    Token(std::string text, const SourceRange &range)
        : text(std::move(text)), range(range) {}

    // colorful
    std::string DumpText() const;
    std::string DumpTextRef() const;
    std::string DumpRange() const;
};

// index in token table
using TokenLocation = std::vector<Token>::size_type;

// manage token table
class SourceManager final {
  public:
    SourceManager() = default;
    explicit SourceManager(std::string file_name)
        : file_name(std::move(file_name)) {}

    void SetFileName(const std::string &file_name) {
        this->file_name = file_name;
    }
    const std::string &GetFileName() const { return file_name; }

    TokenLocation AddToken(const Token &token);
    TokenLocation AddToken(const std::string &text, const SourceRange &range);

    const Token &GetToken(const TokenLocation loc) const {
        return token_table[loc];
    }
    const std::string &GetTokenText(const TokenLocation loc) const {
        return token_table[loc].text;
    }
    const SourceRange &GetTokenRange(const TokenLocation loc) const {
        return token_table[loc].range;
    }

    void Dump(std::ostream &ostream) const;

  private:
    std::string file_name;
    std::vector<Token> token_table;
};

}  // namespace ast

#endif
