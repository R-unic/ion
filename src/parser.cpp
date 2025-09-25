#include <memory>

#include "diagnostics.h"
#include "parser.h"

#include <iostream>

#include "lexer.h"
#include "ast/node.h"
#include "ast/expressions/binary_op.h"
#include "ast/expressions/literal.h"

static bool is_eof(const ParseState& state, const int offset = 0)
{
    const auto offset_position = state.position + offset;
    return offset_position < 0 || static_cast<int>(state.tokens.size()) <= offset_position;
}

static std::optional<Token> maybe_peek(const ParseState& state, const int offset)
{
    if (is_eof(state, offset))
        return std::nullopt;
    
    return state.tokens.at(state.position + offset);
}

static Token peek(const ParseState& state, const int offset)
{
    if (const auto token = maybe_peek(state, offset); token.has_value())
        return *token;
    
    report_compiler_error("Parser attempted to access an out of bounds token index");
}

static std::optional<Token> current_token(const ParseState& state)
{
    return maybe_peek(state, 0);
}

static std::optional<Token> previous_token(const ParseState& state)
{
    return maybe_peek(state, -1);
}

/** Returns the current token before incrementing the position */
static std::optional<Token> advance(ParseState& state)
{
    auto token = current_token(state);
    state.position++;
    
    return token;
}

static bool match(ParseState& state, const SyntaxKind kind)
{
    const auto token = current_token(state);
    const auto is_match = token.has_value() && token.value().kind == kind;
    if (is_match)
        advance(state);
    
    return is_match;
}

static FileSpan empty_span(const ParseState& state)
{
    return create_span(get_start_location(state.file), get_start_location(state.file));
}

static FileSpan get_optional_span(const ParseState& state, const std::optional<Token>& token)
{
    return token.has_value()
        ? token.value().span
        : empty_span(state);
}

static Token consume(ParseState& state, const SyntaxKind kind)
{
    const auto token = current_token(state);
    advance(state);
    
    if (token.has_value() && token.value().kind == kind)
        return *token;

    const auto span = get_optional_span(state, token);
    const auto expected = std::to_string(static_cast<int>(kind));
    const auto got = get_text(*token);
    report_expected_different_syntax(span, expected, got);
}

static double to_number(const std::string& s) {
    if (s.starts_with("0x") || s.starts_with("0X"))
        return static_cast<double>(std::stoll(s, nullptr, 16));
    if (s.starts_with("0o") || s.starts_with("0O"))
        return static_cast<double>(std::stoll(s.substr(2), nullptr, 8));
    if (s.starts_with("0b") || s.starts_with("0B"))
        return static_cast<double>(std::stoll(s.substr(2), nullptr, 2));
        
    return std::stod(s);
}

static expression_ptr_t parse_primary(ParseState& state)
{
    const auto token_opt = advance(state);
    if (!token_opt.has_value())
    {
        const auto last_token = previous_token(state);
        const auto span = get_optional_span(state, last_token);
        report_unexpected_eof(span);
    }
    
    const auto& token = token_opt.value();
    const auto text = get_text(token);
    switch (token.kind)
    {
    case SyntaxKind::TrueKeyword:
        return Literal::create(true);
    case SyntaxKind::FalseKeyword:
        return Literal::create(false);
    case SyntaxKind::NullKeyword:
        return Literal::create(std::nullopt);
    case SyntaxKind::StringLiteral:
        return Literal::create(text.substr(1, text.size() - 1));
    case SyntaxKind::NumberLiteral:
        return Literal::create(to_number(text));
        
    default:
        report_unexpected_syntax(token);
    }
}

static expression_ptr_t parse_addition(ParseState& state)
{
    auto left = parse_primary(state);
    while (match(state, SyntaxKind::Plus) || match(state, SyntaxKind::Minus))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_primary(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_expression(ParseState& state)
{
    return parse_addition(state);
}

expression_ptr_t parse(const SourceFile* file)
{
    const auto tokens = tokenize(file);
    auto state = ParseState { .file = file, .tokens = tokens };
    return parse_expression(state);
}