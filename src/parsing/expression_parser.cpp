#include "ion/parsing/parser.h"

std::optional<EqualsValueClause*> parse_equals_value_clause(ParseState& state)
{
    const auto equals_token = try_consume(state, SyntaxKind::Equals);
    if (!equals_token.has_value())
        return std::nullopt;

    auto value = parse_expression(state);
    return new EqualsValueClause(*equals_token, std::move(value));
}

template <typename NodeFactory, typename... Args>
expression_ptr_t parse_components_literal(ParseState& state, int arity, NodeFactory&& make_node, Args&&... extra_args)
{
    const auto l_arrow = expect(state, SyntaxKind::LArrow);
    const auto l_paren = expect(state, SyntaxKind::LParen);

    std::vector<expression_ptr_t> components;
    for (auto i = 0; i < arity; ++i)
    {
        components.push_back(parse_expression(state));
        if (i + 1 < arity)
            expect(state, SyntaxKind::Comma);
    }

    const auto r_paren = expect(state, SyntaxKind::RParen);
    const auto r_arrow = expect(state, SyntaxKind::RArrow);
    return make_node(l_arrow, r_arrow, std::move(components), std::forward<Args>(extra_args)...);
}

static expression_ptr_t parse_parenthesized(ParseState& state)
{
    const auto l_paren = previous_token_guaranteed(state);
    std::vector<expression_ptr_t> elements;
    do
        elements.push_back(parse_expression(state));
    while (match(state, SyntaxKind::Comma));

    const auto r_paren = expect(state, SyntaxKind::RParen);
    return elements.size() > 1
               ? TupleLiteral::create(l_paren, r_paren, std::move(elements))
               : Parenthesized::create(l_paren, r_paren, std::move(elements.front()));
}

static expression_ptr_t parse_rgb_literal(ParseState& state)
{
    const auto keyword = previous_token_guaranteed(state);
    return parse_components_literal(state, 3, [&](const auto& l_arrow, const auto& r_arrow, auto components)
    {
        return RgbLiteral::create(keyword, l_arrow, r_arrow, std::move(components[0]), std::move(components[1]), std::move(components[2]));
    });
}

static expression_ptr_t parse_hsv_literal(ParseState& state)
{
    const auto keyword = previous_token_guaranteed(state);
    return parse_components_literal(state, 3, [&](const auto& l_arrow, const auto& r_arrow, auto components)
    {
        return HsvLiteral::create(keyword, l_arrow, r_arrow, std::move(components[0]), std::move(components[1]), std::move(components[2]));
    });
}

static expression_ptr_t parse_vector_literal(ParseState& state)
{
    return parse_components_literal(state, 3, [&](const auto& l_arrow, const auto& r_arrow, auto components)
    {
        return VectorLiteral::create(l_arrow, r_arrow, std::move(components[0]), std::move(components[1]), std::move(components[2]));
    });
}

static expression_ptr_t parse_interpolated_string(ParseState& state)
{
    std::vector parts = { previous_token_guaranteed(state) };
    std::vector<expression_ptr_t> interpolations;

    while (match(state, SyntaxKind::InterpolationStart))
    {
        interpolations.push_back(parse_expression(state));
        expect(state, SyntaxKind::InterpolationEnd);
        parts.push_back(expect(state, SyntaxKind::InterpolatedStringPart));
    }

    return InterpolatedString::create(std::move(parts), std::move(interpolations));
}

static expression_ptr_t parse_array_literal(ParseState& state)
{
    const auto l_bracket = previous_token_guaranteed(state);
    std::vector<expression_ptr_t> elements;
    if (!check(state, SyntaxKind::RBracket))
        do
            elements.push_back(parse_expression(state));
        while (match(state, SyntaxKind::Comma));

    const auto r_bracket = expect(state, SyntaxKind::RBracket);
    return ArrayLiteral::create(l_bracket, r_bracket, std::move(elements));
}

static expression_ptr_t parse_primary(ParseState& state)
{
    const auto span = fallback_span(state);
    if (check(state, SyntaxKind::LArrow))
        return parse_vector_literal(state);

    const auto token_opt = advance(state);
    if (!token_opt.has_value())
        report_unexpected_eof(span);

    const auto& token = *token_opt;
    const auto text = token.get_text();
    switch (token.kind)
    {
        case SyntaxKind::Identifier:
            return Identifier::create(token);
        case SyntaxKind::RgbKeyword:
            return parse_rgb_literal(state);
        case SyntaxKind::HsvKeyword:
            return parse_hsv_literal(state);
        case SyntaxKind::InterpolatedStringPart:
            return parse_interpolated_string(state);
        case SyntaxKind::LBracket:
            return parse_array_literal(state);
        case SyntaxKind::LParen:
            return parse_parenthesized(state);

        case SyntaxKind::TrueKeyword:
        case SyntaxKind::FalseKeyword:
        case SyntaxKind::StringLiteral:
        case SyntaxKind::NumberLiteral:
            return PrimitiveLiteral::create(token, primitive_from_string(text));
        case SyntaxKind::NullKeyword:
            return PrimitiveLiteral::create(token, std::nullopt);

        default:
            report_expected_different_syntax(token.span, "expression", token.get_text(), false);
    }
}

static expression_ptr_t parse_invocation(ParseState& state, expression_ptr_t callee)
{
    const auto bang_token = try_consume(state, SyntaxKind::Bang);
    const auto type_arguments = parse_type_arguments(state);

    expect(state, SyntaxKind::LParen);
    const auto l_paren = previous_token_guaranteed(state);
    std::vector<expression_ptr_t> arguments;
    if (!check(state, SyntaxKind::RParen))
    {
        do
            arguments.push_back(parse_expression(state));
        while (match(state, SyntaxKind::Comma));
    }

    const auto r_paren = expect(state, SyntaxKind::RParen);
    return Invocation::create(l_paren, r_paren, std::move(callee), bang_token, type_arguments, std::move(arguments));
}

static expression_ptr_t parse_member_access(ParseState& state, expression_ptr_t expression, const std::optional<Token>& question_token)
{
    const auto token = previous_token_guaranteed(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    return question_token.has_value()
               ? OptionalMemberAccess::create(token, *question_token, std::move(expression), name)
               : MemberAccess::create(token, std::move(expression), name);
}

static expression_ptr_t parse_element_access(ParseState& state, expression_ptr_t expression)
{
    const auto l_bracket = previous_token_guaranteed(state);
    auto index_expression = parse_expression(state);
    const auto r_bracket = expect(state, SyntaxKind::RBracket);

    return ElementAccess::create(l_bracket, r_bracket, std::move(expression), std::move(index_expression));
}

static bool is_at_invocation(const ParseState& state)
{
    auto offset = 0;
    if (check(state, SyntaxKind::LArrow, offset))
    {
        do
            offset++;
        while (!is_eof(state, offset) && check_any(state, type_argument_syntaxes, offset));
    }

    if (check(state, SyntaxKind::Bang, offset))
        offset++;

    return check(state, SyntaxKind::LParen, offset);
}

static bool is_at_member_access(const ParseState& state)
{
    const auto optional = check(state, SyntaxKind::Question);
    return check_any(state, member_access_syntaxes, optional ? 1 : 0);
}

static expression_ptr_t parse_postfix(ParseState& state)
{
    auto expression = parse_primary(state);
    while (true)
    {
        if (is_at_invocation(state))
            expression = parse_invocation(state, std::move(expression));
        else if (match(state, SyntaxKind::LBracket))
            expression = parse_element_access(state, std::move(expression));
        else if (is_at_member_access(state))
        {
            const auto question_token = try_consume(state, SyntaxKind::Question);
            match_any(state, member_access_syntaxes);

            expression = parse_member_access(state, std::move(expression), question_token);
        }
        // else if (match(state, SyntaxKind::MatchKeyword))
        // {
        //     expression = parse_match_expression(state, std::move(expression));
        // }
        else if (match_any(state, postfix_op_syntaxes))
        {
            assert_assignment_target(expression);
            const auto operator_token = previous_token_guaranteed(state);
            expression = PostfixUnaryOp::create(operator_token, std::move(expression));
        }
        else
            break;
    }

    return expression;
}

static expression_ptr_t parse_unary(ParseState& state)
{
    if (match(state, SyntaxKind::NameOfKeyword))
    {
        const auto keyword = previous_token_guaranteed(state);
        auto expression = parse_postfix(state);
        assert_nameof_target(expression);

        const auto identifier = expression->get_last_token();
        return NameOf::create(keyword, identifier);
    }

    if (match(state, SyntaxKind::TypeOfKeyword))
    {
        const auto keyword = previous_token_guaranteed(state);
        auto expression = parse_expression(state);
        return TypeOf::create(keyword, std::move(expression));
    }

    if (match(state, SyntaxKind::AwaitKeyword))
    {
        const auto keyword = previous_token_guaranteed(state);
        auto expression = parse_expression(state);
        return Await::create(keyword, std::move(expression));
    }

    while (match_any(state, unary_syntaxes))
    {
        const auto operator_token = previous_token_guaranteed(state);
        auto operand = parse_postfix(state);
        return UnaryOp::create(operator_token, std::move(operand));
    }

    return parse_postfix(state);
}

DEFINE_BINARY_PARSER(parse_exponentiation, parse_unary, SyntaxKind::Caret);
DEFINE_BINARY_PARSER(parse_multiplication, parse_exponentiation, multiplicative_syntaxes);
DEFINE_BINARY_PARSER(parse_addition, parse_multiplication, additive_syntaxes);
DEFINE_BINARY_PARSER(parse_bit_shift, parse_addition, bit_shift_syntaxes);
DEFINE_BINARY_PARSER(parse_bitwise_and, parse_bit_shift, SyntaxKind::Ampersand);
DEFINE_BINARY_PARSER(parse_bitwise_xor, parse_bitwise_and, SyntaxKind::Tilde);
DEFINE_BINARY_PARSER(parse_bitwise_or, parse_bitwise_xor, SyntaxKind::Pipe);

static expression_ptr_t parse_range_literal(ParseState& state)
{
    auto left = parse_bitwise_or(state);
    while (match(state, SyntaxKind::DotDot))
    {
        const auto dot_dot_token = previous_token_guaranteed(state);
        auto right = parse_bitwise_or(state);
        left = RangeLiteral::create(std::move(left), dot_dot_token, std::move(right));
    }

    return left;
}

DEFINE_BINARY_PARSER(parse_comparison, parse_range_literal, comparison_syntaxes);
DEFINE_BINARY_PARSER(parse_logical_and, parse_comparison, SyntaxKind::AmpersandAmpersand);
DEFINE_BINARY_PARSER(parse_logical_or, parse_logical_and, SyntaxKind::PipePipe);
DEFINE_BINARY_PARSER(parse_null_coalesce, parse_logical_or, SyntaxKind::QuestionQuestion);

static expression_ptr_t parse_ternary_op(ParseState& state)
{
    auto condition = parse_null_coalesce(state);
    while (match(state, SyntaxKind::Question))
    {
        const auto question_token = previous_token_guaranteed(state);
        auto when_true = parse_expression(state);
        const auto colon_token = expect(state, SyntaxKind::Colon);
        auto when_false = parse_expression(state);

        condition = TernaryOp::create(question_token, colon_token, std::move(condition), std::move(when_true), std::move(when_false));
    }

    return condition;
}

static expression_ptr_t parse_assignment(ParseState& state)
{
    auto left = parse_ternary_op(state);
    while (match_any(state, assignment_syntaxes))
    {
        assert_assignment_target(left);
        const auto operator_token = previous_token_guaranteed(state);
        auto right = parse_ternary_op(state);
        left = AssignmentOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

expression_ptr_t parse_expression(ParseState& state)
{
    return parse_assignment(state);
}