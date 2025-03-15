#include "token_stream.h"

#include <algorithm>
#include <format>

token_stream::token_stream(const std::vector<token>& tokens) noexcept
    : tokens(tokens)
{
}

token token_stream::first()
{
    return tokens.front();
}

token token_stream::at(const size_t pos)
{
    return tokens.at(pos);
}

size_t token_stream::length() const
{
    return tokens.size();
}

void token_stream::consume(syntax_kind kind)
{
    if (is_eof())
        throw std::runtime_error(std::format("Expected {}, got EOF", kind).c_str());

    token token = advance();
    if (token.kind == kind) return;
    throw std::runtime_error(std::format("Expected {}, got {}", kind, token.kind).c_str());
}

bool token_stream::match(const syntax_kind kind)
{
    if (check(kind))
    {
        advance();
        return true;
    }

    return false;
}

bool token_stream::check_set(std::set<syntax_kind> kinds, size_t offset)
{
    return std::ranges::any_of(kinds, [this, offset](const auto kind) {
        return check(kind, offset);
    });
}

bool token_stream::check(syntax_kind kind, size_t offset)
{
    return !is_eof(offset) && peek(offset).kind == kind;
}

token token_stream::advance()
{
    token t = current();
    position++;

    return t;
}

token token_stream::current()
{
    return peek(0);
}

token token_stream::peek(const size_t offset)
{
    return tokens.at(position + offset);
}

bool token_stream::is_eof(const size_t offset) const
{
    return position + offset >= length();
}
