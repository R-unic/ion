#include "ion/lexer.h"
#include "ion/source_file.h"

std::string Token::get_text() const
{
    if (text.has_value())
        return *text;

    if (keyword_lexemes.contains(kind))
        return keyword_lexemes.at(kind);

    if (single_char_lexemes.contains(kind))
        return { 1, single_char_lexemes.at(kind) };

    return span.get_text();
}

std::string Token::format() const
{
    return get_text() + " (" + std::to_string(static_cast<int>(kind)) + ") - " + format_location(span.start);
}