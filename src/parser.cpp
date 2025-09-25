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

static Token consume(ParseState& state, const SyntaxKind kind)
{
    const auto token = current_token(state);
    advance(state);
    
    if (token.has_value() && token.value().kind == kind)
        return *token;

    report_expected_different_syntax(token.value().span, std::to_string(static_cast<int>(kind)), get_text(*token));
}

static expression_ptr_t parse_primary(ParseState& state)
{
    const auto token_opt = advance(state);
    if (!token_opt.has_value())
    {
        const auto last_token = previous_token(state);
        const auto span = last_token.has_value() 
            ? last_token.value().span
            : create_span(get_start_location(state.file), get_start_location(state.file));
        
        report_unexpected_eof(span);
    }

    switch (const auto& token = token_opt.value(); token.kind)
    {
    case SyntaxKind::TrueKeyword:
        return Literal::create(true);
    case SyntaxKind::FalseKeyword:
        return Literal::create(false);
    case SyntaxKind::NullKeyword:
        return Literal::create(std::nullopt);
    case SyntaxKind::StringLiteral:
        return Literal::create(token.text->substr(1, token.text->size() - 1));
    case SyntaxKind::NumberLiteral:
        // TODO
        return Literal::create(69);
        
    default:
        report_unexpected_syntax(token);
    }
}

static expression_ptr_t parse_addition(ParseState& state)
{
    auto left = parse_primary(state);
    while (match(state, SyntaxKind::Plus))
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