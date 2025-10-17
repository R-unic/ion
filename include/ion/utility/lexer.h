#pragma once
#include <algorithm>
#include <functional>

#include "ion/diagnostics.h"
#include "ion/token.h"
#include "ion/source_file.h"

struct LexState : FileLocation
{
    FileLocation lexeme_start;
    std::vector<Token> tokens;
};

/** Is at end of file? */
inline bool is_eof(const LexState& state, const int offset = 0)
{
    const auto offset_position = state.position + offset;
    return offset_position < 0 || offset_position >= static_cast<int>(state.file->text.length());
}

/** Returns the character `offset` characters ahead of the current position */
inline char peek(const LexState& state, const int offset)
{
    return state.file->text[state.position + offset];
}

inline char current_character(const LexState& state)
{
    COMPILER_ASSERT(!is_eof(state), "Lexer attempted to access an out of bounds character (index " + std::to_string(state.position) + ")");
    return peek(state, 0);
}

/** Returns the next unadvanced character */
inline std::optional<char> advance(LexState& state, const int amount = 1)
{
    state.position += amount;
    state.column += amount;
    if (is_eof(state))
        return std::nullopt;

    return current_character(state);
}

static bool ident(char)
{
    return true;
}

static void advance_while(LexState& state, const std::function<bool (char)>& predicate = ident)
{
    while (!is_eof(state) && predicate(current_character(state)))
        advance(state);
}

/** Returns whether the current character is `character` */
inline bool check(const LexState& state, const char character, const int offset = 0)
{
    return !is_eof(state, offset) && peek(state, offset) == character;
}

/** Returns whether the current character is in `characters` */
inline bool check_any(LexState& state, const std::vector<char>& characters)
{
    return !is_eof(state) && std::ranges::any_of(characters, [&](const auto character)
    {
        return check(state, character);
    });
}

/** Returns whether the next N characters are the same characters (in order) in `characters` */
inline bool check_sequential(const LexState& state, const std::vector<char>& characters)
{
    auto match = true;
    auto offset = 0;
    for (const auto& character : characters)
    {
        match = match && check(state, character, offset++);
        if (!match)
            break;
    }

    return match;
}

/** Returns whether the current character is `character`, and advances if it is */
inline bool match(LexState& state, const char character)
{
    const auto is_match = check(state, character);
    if (is_match)
        advance(state);

    return is_match;
}

/** Returns whether the current character is in `characters`, and advances if it is */
inline bool match_any(LexState& state, const std::vector<char>& characters)
{
    const auto is_match = check_any(state, characters);
    if (is_match)
        advance(state);

    return is_match;
}

/** Returns whether the next N characters are the same characters (in order) in `pattern`, and advances if they are */
inline bool match_sequential(LexState& state, const std::string& pattern)
{
    const std::vector characters(pattern.begin(), pattern.end());
    const auto is_match = check_sequential(state, characters);
    if (is_match)
        advance(state, static_cast<int>(characters.size()));

    return is_match;
}

/** Creates a `FileLocation` from the current lexer position */
inline FileLocation current_location(const LexState& state)
{
    // slicing is intentional here
    return { state }; // NOLINT(*-slicing)
}

/** Creates a `FileSpan` from the start of the current lexeme to the current location */
inline FileSpan current_span(const LexState& state)
{
    return create_span(state.lexeme_start, current_location(state));
}

inline std::string current_lexeme(const LexState& state)
{
    const auto start_position = state.lexeme_start.position;
    return state.file->text.substr(start_position, state.position - start_position);
}

/** Reset `lexeme_start` back to the current location */
inline void start_new_lexeme(LexState& state)
{
    state.lexeme_start = current_location(state);
}

/** Returns the current lexeme and starts a new one */
inline std::string consume_lexeme(LexState& state)
{
    const auto lexeme = current_lexeme(state);
    start_new_lexeme(state);

    return lexeme;
}

/**
 * Guard struct that consumes the current lexeme when it goes out of scope.
 * Useful for skipping characters without having to remember to consume the lexeme.
 */
struct LexemeGuard
{
    LexState& state;
    std::string result;
    bool consumed = false;

    ~LexemeGuard()
    {
        consume();
    }

    std::string release()
    {
        consume();
        return result;
    }

private:
    void consume()
    {
        if (consumed)
            return;

        result = consume_lexeme(state);
        consumed = true;
    }
};

[[noreturn]] inline void malformed_number(const LexState& state)
{
    report_malformed_number(current_span(state), current_lexeme(state));
}

/** Push a token with custom text not from the file */
inline void push_token_override_text(LexState& state, const SyntaxKind kind, const std::string& text)
{
    const auto token = Token { kind, current_span(state), text };
    start_new_lexeme(state);
    state.tokens.push_back(token);
}

inline void push_token(LexState& state, const SyntaxKind kind)
{
    const auto token = Token { kind, current_span(state) };
    start_new_lexeme(state);
    state.tokens.push_back(token);
}

inline bool is_whitespace(const char character)
{
    return character == ' '
           || character == '\t'
           || character == '\r'
           || character == '\v'
           || character == '\f';
}

inline bool is_valid_identifier_char(const char character, const bool include_numbers)
{
    return character == '_' || (
               include_numbers
                   ? std::isalnum(character)
                   : std::isalpha(character)
           );
}

inline bool is_numeric_char(const char character)
{
    return std::isdigit(character) || character == '_';
}

inline bool is_hex_digit(const LexState& state)
{
    const auto character = current_character(state);
    return std::isdigit(character) || (character >= 'A' && character <= 'F');
}

inline bool is_octal_digit(const LexState& state)
{
    const auto character = current_character(state);
    return character >= '0' && character <= '7';
}

inline bool is_binary_digit(const LexState& state)
{
    const auto character = current_character(state);
    return character == '1' || character == '0';
}

inline std::string unescape(const std::string& s)
{
    std::string result;
    result.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] != '\\' || i + 1 >= s.size())
        {
            result += s[i];
            continue;
        }

        switch (s[++i])
        {
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'v':
                result += '\v';
                break;
            case 'a':
                result += '\a';
                break;
            case 'e':
                result += '\x1B';
                break; // escape
            case '\\':
                result += '\\';
                break;
            case '#':
                result += '#';
                break;
            case '"':
                result += '"';
                break;
            case '\'':
                result += '\'';
                break;
            default:
                result += s[i];
                break; // unknown escape: keep literal
        }
    }

    return result;
}