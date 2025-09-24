#include "lexer.h"

#include <iostream>
#include <unordered_map>
#include <vector>

#include "diagnostics.h"

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

static void push_token(const LexState& state, const SyntaxKind kind)
{
    const auto token = Token {
        .kind = kind,
        .span = current_span(state)
    };

    state.tokens->push_back(token);
}

const std::unordered_map<char, SyntaxKind> single_char_syntaxes = {
    {'+', SyntaxKind::Plus},
    {'-', SyntaxKind::Minus},
    {'*', SyntaxKind::Star},
    {'/', SyntaxKind::Slash},
};

static void skip_whitespace(LexState& state)
{
    while (!is_eof(state) && std::isblank(current_character(state)))
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

static void lex(LexState& state)
{
    const auto character = current_character(state);
    advance(state);

    switch (character)
    {
    case '\n':
        return skip_newlines(state);
    }

    if (std::isblank(current_character(state)))
        return skip_whitespace(state);
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