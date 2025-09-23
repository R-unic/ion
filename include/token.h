#pragma once
#include <format>

#include "location.h"
#include "ast/source_file.h"

enum class SyntaxKind : uint8_t
{
    Plus,
    Minus,
    Star,
    Slash
};

struct Token {
    SyntaxKind kind;
    Span span;
    SourceFile file;
};

inline std::string get_text(const Token& token)
{
    const auto start_position = token.span.start.position;
    return token.file.text.substr(start_position, token.span.end.position - start_position);
}