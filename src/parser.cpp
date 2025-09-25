#include <memory>

#include "diagnostics.h"
#include "parser.h"

#include <algorithm>
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

static bool check(const ParseState& state, const SyntaxKind kind)
{
    const auto token = current_token(state);
    return token.has_value() && token.value().kind == kind;
}

static bool match(ParseState& state, const SyntaxKind kind)
{
    const auto is_match = check(state, kind);
    if (is_match)
        advance(state);
    
    return is_match;
}

static bool match_any(ParseState& state, const std::vector<SyntaxKind>& characters)
{
    const auto is_match = !is_eof(state) && std::ranges::any_of(characters, [&](const auto syntax)
    {
        return check(state, syntax);
    });
    
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

static expression_ptr_t parse_exponentation(ParseState& state)
{
    auto left = parse_primary(state);
    while (match(state, SyntaxKind::Carat))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_primary(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector multiplicative_syntaxes = {SyntaxKind::Star, SyntaxKind::Slash, SyntaxKind::Percent};
static expression_ptr_t parse_multiplication(ParseState& state)
{
    auto left = parse_exponentation(state);
    while (match_any(state, multiplicative_syntaxes))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_exponentation(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector additive_syntaxes = {SyntaxKind::Plus, SyntaxKind::Minus};
static expression_ptr_t parse_addition(ParseState& state)
{
    auto left = parse_multiplication(state);
    while (match_any(state, additive_syntaxes))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_multiplication(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector bit_shift_syntaxes = {SyntaxKind::LArrowLArrow, SyntaxKind::RArrowRArrow, SyntaxKind::RArrowRArrowRArrow};
static expression_ptr_t parse_bit_shift(ParseState& state)
{
    auto left = parse_addition(state);
    while (match_any(state, bit_shift_syntaxes))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_addition(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_bitwise_and(ParseState& state)
{
    auto left = parse_bit_shift(state);
    while (match(state, SyntaxKind::Ampersand))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_bit_shift(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_bitwise_or(ParseState& state)
{
    auto left = parse_bitwise_and(state);
    while (match(state, SyntaxKind::Pipe))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_bitwise_and(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_bitwise_xor(ParseState& state)
{
    auto left = parse_bitwise_or(state);
    while (match(state, SyntaxKind::Tilde))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_bitwise_or(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector comparison_syntaxes = {
    SyntaxKind::EqualsEquals,
    SyntaxKind::BangEquals,
    SyntaxKind::LArrow,
    SyntaxKind::LArrowEquals,
    SyntaxKind::RArrow,
    SyntaxKind::RArrowEquals
};
static expression_ptr_t parse_comparison(ParseState& state)
{
    auto left = parse_bitwise_xor(state);
    while (match_any(state, comparison_syntaxes))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_bitwise_xor(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_logical_and(ParseState& state)
{
    auto left = parse_comparison(state);
    while (match(state, SyntaxKind::AmpersandAmpersand))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_comparison(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_logical_or(ParseState& state)
{
    auto left = parse_logical_and(state);
    while (match(state, SyntaxKind::PipePipe))
    {
        const auto operator_token = previous_token(state);
        auto right = parse_logical_and(state);
        left = BinaryOp::create(*operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_expression(ParseState& state)
{
    return parse_logical_or(state);
}

expression_ptr_t parse(const SourceFile* file)
{
    const auto tokens = tokenize(file);
    auto state = ParseState { .file = file, .tokens = tokens };
    return parse_expression(state);
}