#include "ion/lexer.h"
#include "ion/source_file.h"

bool FileSpan::has_line_break_between(const Token& other) const
{
    return has_line_break_between(other.span);
}

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

std::vector<Token> Token::split(const std::vector<SyntaxKind>& syntaxes) const
{
    const auto text = get_text();
    const auto syntax_count = syntaxes.size();
    if (syntax_count != text.size())
        report_compiler_error(
            "Failed to split token with text '" + text + "' because only " + std::to_string(syntax_count) + " syntaxes were provided");

    std::vector<Token> result;
    result.reserve(syntax_count);

    for (auto i = 0; i < syntax_count; i++)
    {
        const auto character = text.at(i);
        const auto syntax = syntaxes.at(i);
        const auto new_start = span.start + i;
        const auto new_end = new_start + 1;

        result.emplace_back(
            syntax,
            create_span(new_start, new_end),
            std::string(1, character)
        );
    }

    return result;
}