#include <optional>
#include <vector>

#include "ion/diagnostics.h"
#include "ion/logger.h"
#include "ion/lexer.h"
#include "ion/source_file.h"

static void skip_whitespace(LexState& state)
{
    LexemeGuard guard(state);
    advance_while(state, is_whitespace);
}

/** Advances until the current character is not a line feed */
static void skip_newlines(LexState& state)
{
    LexemeGuard guard(state);
    state.line++;
    while (!is_eof(state) && current_character(state) == '\n')
    {
        state.position++;
        state.line++;
    }

    state.column = 0;
}

static void read_identifier_or_keyword(LexState& state)
{
    advance_while(state, [&](const auto character)
    {
        return is_valid_identifier_char(character, true);
    });

    const auto text = current_lexeme(state);
    if (keyword_syntaxes.contains(text))
        push_token(state, keyword_syntaxes.at(text));
    else
        push_token_override_text(state, SyntaxKind::Identifier, text);
}

static void read_non_decimal_number(LexState& state, bool (*is_valid_digit)(const LexState&))
{
    auto malformed = false;
    while (!is_eof(state) && is_valid_digit(state))
    {
        const auto next_character = advance(state);
        malformed = next_character.has_value() && std::isalnum(next_character.value()) && !is_valid_digit(state);
    }

    if (malformed)
    {
        advance(state);
        malformed_number(state);
    }

    push_token(state, SyntaxKind::NumberLiteral);
}

static void read_decimal_number(LexState& state)
{
    auto decimal_used = false;
    auto malformed = false;
    auto character = current_character(state);
    while (!is_eof(state) && (is_numeric_char(character) || character == '.'))
    {
        if (character == '.')
        {
            if (!decimal_used && peek(state, 1) == '.')
                break; // we have a range literal, stop right here and just return the current number

            malformed = decimal_used;
            decimal_used = true;
        }

        character = advance(state).value_or(character);
    }

    if (malformed)
        malformed_number(state);

    if (const auto is_scientific_notation = match(state, 'e'); is_scientific_notation)
        advance_while(state, is_numeric_char);
    else if (check(state, '%') || check(state, 's') || check(state, 'd'))
        advance(state);
    else if (check(state, 'm'))
    {
        advance(state);
        match(state, 's'); // ms
    }
    else if (check(state, 'h'))
    {
        advance(state);
        match(state, 'z'); // hz
    }

    push_token(state, SyntaxKind::NumberLiteral);
}

const std::vector hex_specifier = { 'x', 'X' };
const std::vector octal_specifier = { 'o', 'O' };
const std::vector binary_specifier = { 'b', 'B' };

static void read_number(LexState& state, const char first_character)
{
    if (first_character != '0')
        return read_decimal_number(state);

    if (match_any(state, hex_specifier))
        return read_non_decimal_number(state, is_hex_digit);
    if (match_any(state, octal_specifier))
        return read_non_decimal_number(state, is_octal_digit);
    if (match_any(state, binary_specifier))
        return read_non_decimal_number(state, is_binary_digit);

    read_decimal_number(state);
}

static void read_string(LexState& state, const char terminator)
{
    auto was_interpolated = false;
    auto is_interpolation = false;
    auto character = current_character(state);
    while (!is_eof(state) && character != terminator && character != '\n')
    {
        if (character == '#' && check(state, '{', 1))
        {
            was_interpolated = true;
            is_interpolation = true;
            const auto lexeme = current_lexeme(state);
            const auto text = unescape(lexeme.empty() ? lexeme : lexeme.substr(1));
            push_token_override_text(state, SyntaxKind::InterpolatedStringPart, text);
            character = *advance(state, 2);
            push_token(state, SyntaxKind::InterpolationStart);

            do
            {
                lex(state);
                character = current_character(state);
            } while (!check(state, '}'));
            continue;
        }

        if (is_interpolation && character == '}')
        {
            is_interpolation = false;
            advance(state);
            push_token(state, SyntaxKind::InterpolationEnd);
            if (check(state, terminator))
            {
                character = current_character(state);
                break;
            }
        }

        character = *advance(state);
    }

    advance(state);
    const auto text = unescape(current_lexeme(state));
    if (character != terminator)
        report_unterminated_string(current_span(state), text);

    const auto kind = was_interpolated ? SyntaxKind::InterpolatedStringPart : SyntaxKind::StringLiteral;
    push_token_override_text(state, kind, was_interpolated ? text.substr(0, text.size() - 1) : text);
}

static void skip_single_line_comment(LexState& state)
{
    LexemeGuard guard(state);
    advance_while(state, [&](const auto character)
    {
        return character != '\n';
    });
}

static void read_operator(LexState& state, const char character)
{
    const auto& [base, continuations] = operator_rules.at(character);
    auto kind = base;

    for (const auto& [pattern, next_kind] : continuations)
    {
        if (match_sequential(state, pattern))
        {
            kind = next_kind;
            break;
        }
    }

    if (kind == base)
        advance(state);

    push_token(state, kind);
}

void lex(LexState& state)
{
    const auto character = current_character(state);
    if (operator_rules.contains(character))
        return read_operator(state, character);

    advance(state);
    if (character == '"' || character == '\'')
        return read_string(state, character);
    if (character == '\n')
        return skip_newlines(state);
    if (character == '#' && match(state, '#'))
        return skip_single_line_comment(state);
    if (is_whitespace(character))
        return skip_whitespace(state);
    if (is_valid_identifier_char(character, false))
        return read_identifier_or_keyword(state);
    if (is_numeric_char(character))
        return read_number(state, character);
    if (single_character_syntaxes.contains(character))
        return push_token_override_text(state, single_character_syntaxes.at(character), std::string(1, character));

    report_unexpected_character(current_span(state), character);
}

std::vector<Token> tokenize(const SourceFile* file)
{
    logger::info("Lexing file: " + file->path);
    auto state = LexState {
        { .file = file },
        get_start_location(file),
        {}
    };

    while (!is_eof(state))
        lex(state);

    logger::info("Lexed " + std::to_string(state.tokens.size()) + " tokens");
    return state.tokens;
}