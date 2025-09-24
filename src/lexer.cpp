#include <vector>

#include "diagnostics.h"
#include "lexer.h"

static void advance(LexState& state)
{
    state.position++;
    state.column++;
}

static bool is_eof(const LexState& state)
{
    return static_cast<int>(state.file.text.length()) <= state.position;
}

static char current_character(const LexState& state)
{
    if (is_eof(state))
        report_compiler_error("Lexer attempted to access an out of bounds file source index");
    
    return state.file.text[state.position];
}

static bool match(LexState& state, const char character)
{
    const auto is_match = !is_eof(state) && current_character(state) == character;
    if (is_match)
        advance(state);
    
    return is_match;
}

static FileLocation current_location(const LexState& state)
{
    return static_cast<FileLocation>(state);
}

static FileSpan current_span(const LexState& state)
{
    return create_span(state.lexeme_start, current_location(state));
}

static std::string current_lexeme(const LexState& state)
{
    const auto start_position = state.lexeme_start.position;
    return state.file.text.substr(start_position, state.position - start_position);
}

static std::string consume_lexeme(LexState& state)
{
    const auto lexeme = current_lexeme(state);
    state.lexeme_start = current_location(state);

    return lexeme;
}

static void push_token(LexState& state, const SyntaxKind kind)
{
    const auto token = Token {
        .kind = kind,
        .span = current_span(state)
    };

    consume_lexeme(state);
    state.tokens->push_back(token);
}

static bool is_whitespace(const char character)
{
    return character == ' '
        || character == '\t'
        || character == '\r'
        || character == '\v'
        || character == '\f';
}

static void skip_whitespace(LexState& state)
{
    while (!is_eof(state) && is_whitespace(current_character(state)))
        advance(state);

    const auto text = consume_lexeme(state);
}

static void skip_newlines(LexState& state)
{
    while (!is_eof(state) && current_character(state) == '\n')
    {
        state.position++;
        state.line++;
    }

    state.column = 0;
    const auto text = consume_lexeme(state);
}

static bool is_valid_identifier_char(const char character, const bool include_numbers)
{
    return character == '_' || (
        include_numbers
            ? std::isalnum(character)
            : std::isalpha(character)
    );
}

static void read_identifier_or_keyword(LexState& state)
{
    while (!is_eof(state) && is_valid_identifier_char(current_character(state), true))
        advance(state);

    const auto text = current_lexeme(state);
    const auto kind = keyword_syntaxes.contains(text) ? keyword_syntaxes.at(text) : SyntaxKind::Identifier;
    push_token(state, kind);
}

static bool is_hex_digit(const LexState& state)
{
    const auto character = current_character(state);
    return std::isdigit(character) || (character >= 'A' && character <= 'F');
}

static void read_hex_number(LexState& state)
{
    while (!is_eof(state) && is_hex_digit(state))
        advance(state);

    push_token(state, SyntaxKind::NumberLiteral);
}

static bool is_numeric_char(const char character)
{
    return std::isdigit(character) || character == '_';
}

static void read_decimal_number(LexState& state)
{
    bool decimal_used = false;
    bool decimal_used_twice = false;
    char character;
    while (!is_eof(state) && (is_numeric_char(character = current_character(state)) || character == '.'))
    {
        if (character == '.')
        {
            if (decimal_used)
                decimal_used_twice = true;
            
            decimal_used = true;
        }
        
        advance(state);
    }

    if (decimal_used_twice)
        report_malformed_number(state.lexeme_start, current_lexeme(state));

    push_token(state, SyntaxKind::NumberLiteral);
}

static void read_number(LexState& state, const char first_character)
{
    if (first_character == '0')
    {
        if (match(state, 'x'))
            return read_hex_number(state);
    }

    return read_decimal_number(state);
}

static void read_string(LexState& state, const char terminator)
{
    char character;
    while (!is_eof(state) && (character = current_character(state)) != terminator && character != '\n')
        advance(state);

    const auto terminated = current_character(state) == terminator;
    if (!terminated)
        report_unterminated_string(state.lexeme_start, current_lexeme(state));

    advance(state);
    push_token(state, SyntaxKind::StringLiteral);
}

static void lex(LexState& state)
{
    const auto character = current_character(state);
    advance(state);

    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (character) // NOLINT(hicpp-multiway-paths-covered)
    {
    case '+':
        {
            auto kind = SyntaxKind::Plus;
            if (match(state, '+'))
                kind = SyntaxKind::PlusPlus;
            else if (match(state, '='))
                kind = SyntaxKind::PlusEquals;

            return push_token(state, kind);
        }
    case '-':
        {
            auto kind = SyntaxKind::Minus;
            if (match(state, '-'))
                kind = SyntaxKind::MinusMinus;
            else if (match(state, '='))
                kind = SyntaxKind::MinusEquals;

            return push_token(state, kind);
        }
    case '*':
        {
            auto kind = SyntaxKind::Star;
            if (match(state, '='))
                kind = SyntaxKind::StarEquals;

            return push_token(state, kind);
        }
    case '/':
        {
            auto kind = SyntaxKind::Slash;
            if (match(state, '='))
                kind = SyntaxKind::SlashEquals;

            return push_token(state, kind);
        }
    case '^':
        {
            auto kind = SyntaxKind::Carat;
            if (match(state, '='))
                kind = SyntaxKind::CaratEquals;

            return push_token(state, kind);
        }
    case '&':
        {
            auto kind = SyntaxKind::Ampersand;
            if (match(state, '='))
                kind = SyntaxKind::AmpersandEquals;
            else if (match(state, '&'))
            {
                kind = SyntaxKind::AmpersandAmpersand;
                if (match(state, '='))
                    kind = SyntaxKind::AmpersandAmpersandEquals;
            }

            return push_token(state, kind);
        }
    case '|':
        {
            auto kind = SyntaxKind::Pipe;
            if (match(state, '='))
                kind = SyntaxKind::PipeEquals;
            else if (match(state, '|'))
            {
                kind = SyntaxKind::PipePipe;
                if (match(state, '='))
                    kind = SyntaxKind::PipePipeEquals;
            }

            return push_token(state, kind);
        }
    case '~':
        {
            auto kind = SyntaxKind::Tilde;
            if (match(state, '='))
                kind = SyntaxKind::TildeEquals;

            return push_token(state, kind);
        }
    case '<':
        {
            auto kind = SyntaxKind::LArrow;
            if (match(state, '='))
                kind = SyntaxKind::LArrowEquals;
            else if (match(state, '<'))
            {
                kind = SyntaxKind::LArrowLArrow;
                if (match(state, '='))
                    kind = SyntaxKind::LArrowLArrowEquals;
            }

            return push_token(state, kind);
        }
    case '>':
        {
            auto kind = SyntaxKind::RArrow;
            if (match(state, '='))
                kind = SyntaxKind::RArrowEquals;
            else if (match(state, '>'))
            {
                kind = SyntaxKind::RArrowRArrow;
                if (match(state, '='))
                    kind = SyntaxKind::RArrowRArrowEquals;
                else if (match(state, '>'))
                {
                    kind = SyntaxKind::RArrowRArrowRArrow;
                    if (match(state, '='))
                        kind = SyntaxKind::RArrowRArrowRArrowEquals;
                }
            }

            return push_token(state, kind);
        }
    case '!':
        {
            auto kind = SyntaxKind::Bang;
            if (match(state, '='))
                kind = SyntaxKind::BangEquals;

            return push_token(state, kind);
        }
    case '=':
        {
            auto kind = SyntaxKind::Equals;
            if (match(state, '='))
                kind = SyntaxKind::EqualsEquals;

            return push_token(state, kind);
        }
    case '?':
        {
            auto kind = SyntaxKind::Question;
            if (match(state, '?'))
            {
                kind = SyntaxKind::QuestionQuestion;
                if (match(state, '='))
                    kind = SyntaxKind::QuestionQuestionEquals;
            }

            return push_token(state, kind);
        }
    case ':':
        {
            auto kind = SyntaxKind::Colon;
            if (match(state, ':'))
                kind = SyntaxKind::ColonColon;

            return push_token(state, kind);
        }
        
    case '\n':
        return skip_newlines(state);
    case '"':
    case '\'':
        return read_string(state, character);
    }

    if (is_whitespace(character))
        return skip_whitespace(state);
    if (is_valid_identifier_char(character, false))
        return read_identifier_or_keyword(state);
    if (is_numeric_char(character))
        return read_number(state, character);
    if (single_char_syntaxes.contains(character))
        return push_token(state, single_char_syntaxes.at(character));

    report_unexpected_character(state, character);
}

std::vector<Token> tokenize(const SourceFile& file)
{
    auto tokens = std::vector<Token>();
    auto state = LexState {
        { .file = file },
        get_start_location(file),
        &tokens
    };
    
    while (!is_eof(state))
        lex(state);

    return *state.tokens;
}