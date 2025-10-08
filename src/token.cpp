#include "ion/diagnostics.h"
#include "ion/source_file.h"
#include "ion/lexer.h"

bool FileSpan::has_line_break_between(const Token& other) const
{
    return has_line_break_between(other.span);
}

bool Token::is_kind(const SyntaxKind check_kind) const
{
    return kind == check_kind;
}

std::string Token::get_text() const
{
    if (text.has_value())
        return *text;

    if (const auto conversion = syntax_to_string(kind); conversion.has_value())
        return *conversion;

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
    COMPILER_ASSERT(syntax_count == text.size(),
                    "Failed to split token with text '" + text + "': Only " + std::to_string(syntax_count) + " syntaxes were provided");

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