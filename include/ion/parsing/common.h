#pragma once
#include <set>

#include "ion/token.h"
#include "ion/ast/ast.h"
#include "ion/utility/parser.h"

void consume_semicolons(ParseState& state);

template <typename T, typename ParseFn>
std::vector<T> parse_comma_separated(ParseState& state, ParseFn parse_fn)
{
    std::vector<T> items;
    do
        items.push_back(parse_fn(state));
    while (match(state, SyntaxKind::Comma));

    return items;
}

template <typename F>
auto parse_optional(ParseState& state, SyntaxKind kind, F&& parser) -> std::optional<decltype(parser(state))>
{
    if (check(state, kind))
        return parser(state);

    return std::nullopt;
}

expression_ptr_t parse_binary_expression(ParseState&, const std::function<expression_ptr_t(ParseState&)>&, SyntaxKind);
expression_ptr_t parse_binary_expression(ParseState&, const std::function<expression_ptr_t (ParseState&)>&, const std::set<SyntaxKind>&);

#define DEFINE_BINARY_PARSER(name, subparser, ops) \
    static expression_ptr_t name(ParseState& state) { \
        return parse_binary_expression(state, subparser, ops); \
    }