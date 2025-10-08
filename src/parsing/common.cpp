#include "ion/parsing/common.h"

#include "ion/ast/expressions/binary_op.h"

void consume_semicolons(ParseState& state)
{
    while (match(state, SyntaxKind::Semicolon))
        continue;
}

expression_ptr_t parse_binary_expression(ParseState& state,
                                         const std::function<expression_ptr_t(ParseState&)>& subparser,
                                         const SyntaxKind operator_kind)
{
    auto left = subparser(state);
    while (match(state, operator_kind))
    {
        const auto operator_token = previous_token_guaranteed(state);
        auto right = subparser(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

expression_ptr_t parse_binary_expression(ParseState& state,
                                         const std::function<expression_ptr_t (ParseState&)>& subparser,
                                         const std::set<SyntaxKind>& operator_kinds)
{
    auto left = subparser(state);
    while (match_any(state, operator_kinds))
    {
        const auto op = previous_token_guaranteed(state);
        auto right = subparser(state);
        left = BinaryOp::create(op, std::move(left), std::move(right));
    }

    return left;
}