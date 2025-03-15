#pragma once
#include <set>
#include <vector>

#include "token.h"

class token_stream
{
    size_t position = 0;
    
public:
    std::vector<token> tokens;

    explicit token_stream(const std::vector<token>& tokens) noexcept;

    token first();
    token at(size_t pos);
    [[nodiscard]] size_t length() const;
    void consume(syntax_kind kind);
    bool match(syntax_kind kind);
    bool check_set(std::set<syntax_kind> kinds, size_t offset = 0);
    bool check(syntax_kind kind, size_t offset = 0);
    token advance();
    token current();
    token peek(size_t offset);
    [[nodiscard]] bool is_eof(size_t offset = 0) const;
};
