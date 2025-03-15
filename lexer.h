#pragma once
#include <string>
#include <vector>

#include "source_file.h"
#include "syntax/token.h"

class lexer
{
    std::string source;
    std::vector<token> tokens = std::vector<token>();
    location lexeme_start_location;
    size_t position = 0;
    size_t column = 0;
    size_t line = 1;

    void lex();
    void read_identifier_or_keyword();
    void read_string(char terminator);
    void read_number();
    void skip_whitespace();
    void push_token(syntax_kind kind, const token::value_t& value = std::monostate());
    [[nodiscard]] std::string current_lexeme() const;
    [[nodiscard]] span current_span() const;
    [[nodiscard]] location current_location() const;
    [[nodiscard]] char current_char() const;
    [[nodiscard]] char peek_char(size_t offset) const;
    bool match_char(char expected);
    void advance(size_t amount = 1);
    void advance_new_line();
    [[nodiscard]] bool is_eof(size_t offset = 0) const;
    
public:
    explicit lexer(const source_file& file);

    token_stream tokenize();
};
