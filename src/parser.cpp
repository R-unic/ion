#include <iostream>
#include <memory>
#include <algorithm>
#include <utility>
#include <set>

#include "ion/logger.h"
#include "ion/diagnostics.h"
#include "ion/parser.h"

static bool is_eof(const ParseState& state, const int offset = 0)
{
    const int stream_size = state.token_stream.size();
    const auto offset_position = state.position + offset;
    return offset_position < 0 || offset_position >= stream_size;
}

static std::optional<Token> maybe_peek(const ParseState& state, const int offset)
{
    if (is_eof(state, offset))
        return std::nullopt;

    return state.token_stream.at(state.position + offset);
}

static Token peek(const ParseState& state, const int offset)
{
    if (const auto token = maybe_peek(state, offset); token.has_value())
        return *token;

    report_compiler_error("Parser attempted to access an out of bounds token index");
}

static Token current_token_guaranteed(const ParseState& state)
{
    return peek(state, 0);
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
    if (!state.token_stack.empty() && state.stack_position < state.token_stack.size())
        return state.token_stack.at(state.stack_position++);

    const auto token = current_token(state);
    state.stack_position = 0;
    state.token_stack = {};
    state.position++;

    return token;
}

static bool check_stack(const ParseState& state, const SyntaxKind kind)
{
    if (state.token_stack.empty() || state.stack_position >= state.token_stack.size())
        return false;

    const auto token = state.token_stack.at(state.stack_position);
    return token.kind == kind;
}

static bool check(const ParseState& state, const SyntaxKind kind, const int offset = 0)
{
    const auto token = maybe_peek(state, offset);
    return token.has_value() && token.value().kind == kind;
}

static bool check_any(const ParseState& state, const std::vector<SyntaxKind>& syntaxes, const int offset = 0)
{
    return !is_eof(state) && std::ranges::any_of(syntaxes, [&](const auto syntax)
    {
        return check(state, syntax, offset);
    });
}

static bool match(ParseState& state, const SyntaxKind kind)
{
    const auto is_match = check(state, kind);
    if (is_match)
        advance(state);

    return is_match;
}

static std::optional<Token> match_token(ParseState& state, const SyntaxKind kind)
{
    if (match(state, kind))
        return previous_token(state);

    return std::nullopt;
}

static bool match_any(ParseState& state, const std::vector<SyntaxKind>& syntaxes)
{
    const auto is_match = check_any(state, syntaxes);
    if (is_match)
        advance(state);

    return is_match;
}

static FileSpan empty_span(const ParseState& state)
{
    return create_span(get_start_location(state.file), get_start_location(state.file));
}

static FileSpan get_current_optional_span(const ParseState& state, const int offset = 0)
{
    if (const auto token = maybe_peek(state, offset); token.has_value())
        return token.value().span;

    if (!is_eof(state, offset - 1))
        return get_current_optional_span(state, offset - 1);

    return empty_span(state);
}

static Token consume(ParseState& state,
                     const SyntaxKind kind,
                     const std::string& custom_expected = "",
                     const std::optional<bool> custom_quote_expected = std::nullopt)
{
    const auto span = get_current_optional_span(state);
    const auto token = advance(state);

    if (token.has_value() && token.value().kind == kind)
        return *token;

    const auto quote_expected = custom_quote_expected.has_value() ? *custom_quote_expected : kind != SyntaxKind::Identifier;
    const auto expected = !custom_expected.empty()
                              ? custom_expected
                              : !quote_expected
                                    ? "identifier"
                                    : std::to_string(static_cast<int>(kind));

    const auto got = token.has_value() ? token->get_text() : "EOF";
    report_expected_different_syntax(span, expected, got, quote_expected);
}

static Token consume_r_arrow(ParseState& state)
{
    if (check(state, SyntaxKind::RArrow) || check_stack(state, SyntaxKind::RArrow))
        return consume(state, SyntaxKind::RArrow);

    if (check(state, SyntaxKind::RArrowRArrow) || check_stack(state, SyntaxKind::RArrowRArrow))
    {
        const auto base = consume(state, SyntaxKind::RArrowRArrow);
        const auto arrows = base.split({ 2, SyntaxKind::RArrow });

        state.token_stack.push_back(arrows.back());
        return arrows.front();
    }

    if (check(state, SyntaxKind::RArrowRArrowRArrow))
    {
        const auto base = consume(state, SyntaxKind::RArrowRArrowRArrow);
        const auto arrows = base.split({ 3, SyntaxKind::RArrow });
        const auto second_last = arrows.at(1);
        const auto last = arrows.at(2);
        state.token_stack.emplace_back(
            SyntaxKind::RArrowRArrow,
            create_span(second_last.span.start, last.span.end),
            ">>"
        );

        return arrows.front();
    }

    return consume(state, SyntaxKind::RArrow);
}

static expression_ptr_t parse_parenthesized(ParseState& state)
{
    const auto l_paren = peek(state, -2);
    auto expression = parse_expression(state);
    const auto r_paren = consume(state, SyntaxKind::RParen);

    return Parenthesized::create(l_paren, r_paren, std::move(expression));
}

static expression_ptr_t parse_rgb_literal(ParseState& state)
{
    const auto keyword = peek(state, -2);
    const auto l_arrow = consume(state, SyntaxKind::LArrow);
    const auto l_paren = consume(state, SyntaxKind::LParen);
    auto r = parse_expression(state);
    consume(state, SyntaxKind::Comma);
    auto g = parse_expression(state);
    consume(state, SyntaxKind::Comma);
    auto b = parse_expression(state);
    const auto r_paren = consume(state, SyntaxKind::RParen);
    const auto r_arrow = consume(state, SyntaxKind::RArrow);

    return RgbLiteral::create(keyword, l_arrow, r_arrow, std::move(r), std::move(g), std::move(b));
}

static expression_ptr_t parse_hsv_literal(ParseState& state)
{
    const auto keyword = peek(state, -2);
    const auto l_arrow = consume(state, SyntaxKind::LArrow);
    const auto l_paren = consume(state, SyntaxKind::LParen);
    auto h = parse_expression(state);
    consume(state, SyntaxKind::Comma);
    auto s = parse_expression(state);
    consume(state, SyntaxKind::Comma);
    auto v = parse_expression(state);
    const auto r_paren = consume(state, SyntaxKind::RParen);
    const auto r_arrow = consume(state, SyntaxKind::RArrow);

    return HsvLiteral::create(keyword, l_arrow, r_arrow, std::move(h), std::move(s), std::move(v));
}

static expression_ptr_t parse_vector_literal(ParseState& state)
{
    const auto l_arrow = *previous_token(state);
    const auto l_paren = consume(state, SyntaxKind::LParen);
    auto x = parse_expression(state);
    consume(state, SyntaxKind::Comma);
    auto y = parse_expression(state);
    consume(state, SyntaxKind::Comma);
    auto z = parse_expression(state);
    const auto r_paren = consume(state, SyntaxKind::RParen);
    const auto r_arrow = consume(state, SyntaxKind::RArrow);

    return VectorLiteral::create(l_arrow, r_arrow, std::move(x), std::move(y), std::move(z));
}

static expression_ptr_t parse_primary(ParseState& state)
{
    const auto span = get_current_optional_span(state);
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
        case SyntaxKind::LArrow:
            return parse_vector_literal(state);
        case SyntaxKind::LParen:
            return parse_parenthesized(state);

        case SyntaxKind::TrueKeyword:
            return PrimitiveLiteral::create(token, true);
        case SyntaxKind::FalseKeyword:
            return PrimitiveLiteral::create(token, false);
        case SyntaxKind::NullKeyword:
            return PrimitiveLiteral::create(token, std::nullopt);
        case SyntaxKind::StringLiteral:
            return PrimitiveLiteral::create(token, text.substr(1, text.size() - 2));
        case SyntaxKind::NumberLiteral:
            return PrimitiveLiteral::create(token, to_number(text));

        default:
            report_unexpected_syntax(token);
    }
}

static std::optional<TypeListClause*> parse_type_arguments(ParseState& state)
{
    const auto l_arrow = match_token(state, SyntaxKind::LArrow);
    if (!l_arrow.has_value())
        return std::nullopt;

    std::vector<type_ref_ptr_t> list;
    do
        list.push_back(parse_type(state));
    while (match(state, SyntaxKind::Comma));

    const auto r_arrow = consume_r_arrow(state);
    return new TypeListClause(*l_arrow, std::move(list), r_arrow);
}

static expression_ptr_t parse_invocation(ParseState& state, expression_ptr_t callee)
{
    const auto bang_token = match_token(state, SyntaxKind::Bang);
    const auto type_arguments = parse_type_arguments(state);

    consume(state, SyntaxKind::LParen);
    const auto l_paren = *previous_token(state);
    std::vector<expression_ptr_t> arguments;
    if (!check(state, SyntaxKind::RParen))
    {
        do
            arguments.push_back(parse_expression(state));
        while (match(state, SyntaxKind::Comma));
    }

    const auto r_paren = consume(state, SyntaxKind::RParen);
    return Invocation::create(l_paren, r_paren, std::move(callee), bang_token, type_arguments, std::move(arguments));
}

static expression_ptr_t parse_member_access(ParseState& state, expression_ptr_t expression)
{
    const auto token = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    return MemberAccess::create(token, std::move(expression), name);
}

static expression_ptr_t parse_element_access(ParseState& state, expression_ptr_t expression)
{
    const auto l_bracket = *previous_token(state);
    auto index_expression = parse_expression(state);
    const auto r_bracket = consume(state, SyntaxKind::RBracket);

    return ElementAccess::create(l_bracket, r_bracket, std::move(expression), std::move(index_expression));
}

const std::vector type_argument_syntaxes = {
    SyntaxKind::Identifier,
    SyntaxKind::LArrow,
    SyntaxKind::RArrow, SyntaxKind::RArrowRArrow, SyntaxKind::RArrowRArrowRArrow,
    SyntaxKind::Comma
};

static bool is_invocation(const ParseState& state)
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

const std::vector member_access_syntaxes = { SyntaxKind::Dot, SyntaxKind::ColonColon, SyntaxKind::At };
const std::vector postfix_op_syntaxes = { SyntaxKind::PlusPlus, SyntaxKind::MinusMinus };

static expression_ptr_t parse_postfix(ParseState& state)
{
    auto expression = parse_primary(state);
    while (true)
    {
        if (is_invocation(state))
            expression = parse_invocation(state, std::move(expression));
        else if (match(state, SyntaxKind::LBracket))
            expression = parse_element_access(state, std::move(expression));
        else if (match_any(state, member_access_syntaxes))
            expression = parse_member_access(state, std::move(expression));
        else if (match_any(state, postfix_op_syntaxes))
        {
            assert_assignment_target(expression);
            const auto operator_token = *previous_token(state);
            expression = PostfixUnaryOp::create(operator_token, std::move(expression));
        }
        else
            break;
    }

    return expression;
}

const std::vector unary_syntaxes = { SyntaxKind::Bang, SyntaxKind::Tilde, SyntaxKind::Minus };

static expression_ptr_t parse_unary(ParseState& state)
{
    if (match(state, SyntaxKind::NameOfKeyword))
    {
        const auto keyword = *previous_token(state);
        const auto identifier = consume(state, SyntaxKind::Identifier);
        return NameOf::create(keyword, identifier);
    }

    if (match(state, SyntaxKind::TypeOfKeyword))
    {
        const auto keyword = *previous_token(state);
        auto expression = parse_expression(state);
        return TypeOf::create(keyword, std::move(expression));
    }

    if (match(state, SyntaxKind::AwaitKeyword))
    {
        const auto keyword = *previous_token(state);
        auto expression = parse_expression(state);
        return Await::create(keyword, std::move(expression));
    }

    while (match_any(state, unary_syntaxes))
    {
        const auto operator_token = *previous_token(state);
        auto operand = parse_postfix(state);
        return UnaryOp::create(operator_token, std::move(operand));
    }

    return parse_postfix(state);
}

static expression_ptr_t parse_exponentation(ParseState& state)
{
    auto left = parse_unary(state);
    while (match(state, SyntaxKind::Carat))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_unary(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector multiplicative_syntaxes = { SyntaxKind::Star, SyntaxKind::Slash, SyntaxKind::Percent };

static expression_ptr_t parse_multiplication(ParseState& state)
{
    auto left = parse_exponentation(state);
    while (match_any(state, multiplicative_syntaxes))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_exponentation(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector additive_syntaxes = { SyntaxKind::Plus, SyntaxKind::Minus };

static expression_ptr_t parse_addition(ParseState& state)
{
    auto left = parse_multiplication(state);
    while (match_any(state, additive_syntaxes))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_multiplication(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector bit_shift_syntaxes = {
    SyntaxKind::LArrowLArrow, SyntaxKind::RArrowRArrow, SyntaxKind::RArrowRArrowRArrow
};

static expression_ptr_t parse_bit_shift(ParseState& state)
{
    auto left = parse_addition(state);
    while (match_any(state, bit_shift_syntaxes))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_addition(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_bitwise_and(ParseState& state)
{
    auto left = parse_bit_shift(state);
    while (match(state, SyntaxKind::Ampersand))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_bit_shift(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_bitwise_xor(ParseState& state)
{
    auto left = parse_bitwise_and(state);
    while (match(state, SyntaxKind::Tilde))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_bitwise_and(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_bitwise_or(ParseState& state)
{
    auto left = parse_bitwise_xor(state);
    while (match(state, SyntaxKind::Pipe))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_bitwise_xor(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_range_literal(ParseState& state)
{
    auto left = parse_bitwise_or(state);
    while (match(state, SyntaxKind::DotDot))
    {
        const auto dot_dot_token = *previous_token(state);
        auto right = parse_bitwise_or(state);
        left = RangeLiteral::create(std::move(left), dot_dot_token, std::move(right));
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
    auto left = parse_range_literal(state);
    while (match_any(state, comparison_syntaxes))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_range_literal(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_logical_and(ParseState& state)
{
    auto left = parse_comparison(state);
    while (match(state, SyntaxKind::AmpersandAmpersand))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_comparison(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_logical_or(ParseState& state)
{
    auto left = parse_logical_and(state);
    while (match(state, SyntaxKind::PipePipe))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_logical_and(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

const std::vector assignment_syntaxes = {
    SyntaxKind::Equals,
    SyntaxKind::PlusEquals,
    SyntaxKind::MinusEquals,
    SyntaxKind::StarEquals,
    SyntaxKind::SlashEquals,
    SyntaxKind::CaratEquals,
    SyntaxKind::PercentEquals,
    SyntaxKind::AmpersandEquals,
    SyntaxKind::PipeEquals,
    SyntaxKind::TildeEquals,
    SyntaxKind::LArrowLArrowEquals,
    SyntaxKind::RArrowRArrowEquals,
    SyntaxKind::RArrowRArrowRArrowEquals,
    SyntaxKind::AmpersandAmpersandEquals,
    SyntaxKind::PipePipeEquals
};

static expression_ptr_t parse_assignment(ParseState& state)
{
    auto left = parse_logical_or(state);
    while (match_any(state, assignment_syntaxes))
    {
        assert_assignment_target(left);
        const auto operator_token = *previous_token(state);
        auto right = parse_assignment(state);
        left = AssignmentOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_ternary_op(ParseState& state)
{
    auto condition = parse_assignment(state);
    while (match(state, SyntaxKind::Question))
    {
        const auto question_token = *previous_token(state);
        auto when_true = parse_expression(state);
        const auto colon_token = consume(state, SyntaxKind::Colon);
        auto when_false = parse_expression(state);

        condition = TernaryOp::create(question_token, colon_token, std::move(condition), std::move(when_true),
                                      std::move(when_false));
    }

    return condition;
}

expression_ptr_t parse_expression(ParseState& state)
{
    return parse_ternary_op(state);
}

static statement_ptr_t parse_block(ParseState& state)
{
    const auto l_brace = *previous_token(state);
    std::vector<statement_ptr_t> statements;
    while (!check(state, SyntaxKind::RBrace))
        statements.push_back(parse_statement(state));

    const auto r_brace = consume(state, SyntaxKind::RBrace);
    return Block::create(l_brace, r_brace, std::move(statements));
}

static std::optional<ColonTypeClause*> parse_colon_type_clause(ParseState& state, const bool required = false)
{
    const auto colon_token = required ? consume(state, SyntaxKind::Colon) : match_token(state, SyntaxKind::Colon);
    if (!colon_token.has_value())
        return std::nullopt;

    auto type = parse_type(state);
    return new ColonTypeClause(*colon_token, std::move(type));
}

static ColonTypeClause* parse_required_colon_type_clause(ParseState& state)
{
    return *parse_colon_type_clause(state, true);
}

static std::optional<EqualsValueClause*> parse_equals_value_clause(ParseState& state)
{
    const auto equals_token = match_token(state, SyntaxKind::Equals);
    if (!equals_token.has_value())
        return std::nullopt;

    auto value = parse_expression(state);
    return new EqualsValueClause(*equals_token, std::move(value));
}

static statement_ptr_t parse_variable_declaration(ParseState& state)
{
    const auto let_keyword = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_colon_type_clause(state);
    const auto equals_value_clause = parse_equals_value_clause(state);

    return VariableDeclaration::create(let_keyword, name, colon_type_clause, equals_value_clause);
}

static std::optional<TypeListClause*> parse_type_parameters(ParseState& state)
{
    const auto l_arrow = match_token(state, SyntaxKind::LArrow);
    if (!l_arrow.has_value())
        return std::nullopt;

    std::vector<type_ref_ptr_t> list;
    do
        list.push_back(parse_type_parameter(state));
    while (match(state, SyntaxKind::Comma));

    const auto r_arrow = consume(state, SyntaxKind::RArrow);
    return new TypeListClause(*l_arrow, std::move(list), r_arrow);
}

static statement_ptr_t parse_type_declaration(ParseState& state)
{
    const auto type_keyword = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    const auto equals_token = consume(state, SyntaxKind::Equals);
    auto type = parse_type(state);

    return TypeDeclaration::create(type_keyword, name, type_parameters, equals_token, std::move(type));
}

static statement_ptr_t parse_event_declaration(ParseState& state)
{
    const auto event_keyword = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    const auto l_paren = match_token(state, SyntaxKind::LParen);
    std::vector<type_ref_ptr_t> parameter_types;
    std::optional<Token> r_paren = std::nullopt;
    if (l_paren.has_value())
    {
        do
            parameter_types.push_back(parse_type(state));
        while (match(state, SyntaxKind::Comma));

        r_paren = consume(state, SyntaxKind::RParen);
    }

    return EventDeclaration::create(event_keyword, name, type_parameters, l_paren, std::move(parameter_types), r_paren);
}

static statement_ptr_t parse_enum_member(ParseState& state)
{
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto equals_value_clause = parse_equals_value_clause(state);
    return EnumMember::create(name, equals_value_clause);
}

static statement_ptr_t parse_enum_declaration(ParseState& state)
{
    const auto enum_keyword = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto l_brace = consume(state, SyntaxKind::LBrace);
    std::vector<statement_ptr_t> members;

    while (!check(state, SyntaxKind::RBrace))
    {
        members.push_back(parse_enum_member(state));
        match(state, SyntaxKind::Comma);
    }

    const auto r_brace = consume(state, SyntaxKind::RBrace);
    return EnumDeclaration::create(enum_keyword, name, l_brace, std::move(members), r_brace);
}

static statement_ptr_t parse_parameter(ParseState& state)
{
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_colon_type_clause(state);
    const auto equals_value_clause = parse_equals_value_clause(state);
    return Parameter::create(name, colon_type_clause, equals_value_clause);
}

static statement_ptr_t parse_function_declaration(ParseState& state, const std::optional<Token>& async_keyword)
{
    const auto fn_keyword = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    const auto l_paren = match_token(state, SyntaxKind::LParen);
    std::vector<statement_ptr_t> parameters;
    std::optional<Token> r_paren = std::nullopt;
    if (l_paren.has_value())
    {
        do
            parameters.push_back(parse_parameter(state));
        while (match(state, SyntaxKind::Comma));

        r_paren = consume(state, SyntaxKind::RParen);
    }

    const auto return_type = parse_colon_type_clause(state);
    const auto long_arrow = match_token(state, SyntaxKind::LongArrow);
    std::optional<expression_ptr_t> expression_body = std::nullopt;
    std::optional<statement_ptr_t> body = std::nullopt;
    std::optional<Token> l_brace = std::nullopt,
                         r_brace = std::nullopt;

    if (long_arrow.has_value())
        expression_body = parse_expression(state);
    else if (match(state, SyntaxKind::LBrace))
    {
        l_brace = *previous_token(state);
        body = parse_block(state);
        r_brace = *previous_token(state);
    }
    else
    {
        const auto last_token = peek(state, -1);
        const auto token = current_token(state);
        const auto location = last_token.span.end;
        const auto span = create_span(location, location);
        const auto text = token.has_value() ? token->get_text() : "EOF";

        report_expected_different_syntax(span, "function body", text, false);
    }

    return FunctionDeclaration::create(async_keyword, fn_keyword, name, type_parameters, l_paren, std::move(parameters), r_paren,
                                       return_type, long_arrow, l_brace, std::move(body), std::move(expression_body), r_brace);
}

const std::set<std::string> primitive_type_names = { "number", "string", "bool", "void" };

static statement_ptr_t parse_instance_declarator_with_initializer(ParseState& state)
{
    const auto at_token = match_token(state, SyntaxKind::At);
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto colon_token = consume(state, SyntaxKind::Colon);
    auto value = parse_expression(state);

    return at_token.has_value()
               ? InstanceAttributeDeclarator::create(*at_token, name, colon_token, std::move(value))
               : InstancePropertyDeclarator::create(name, colon_token, std::move(value));
}

static statement_ptr_t parse_instance_declarator(ParseState& state)
{
    if (const auto name_literal = match_token(state, SyntaxKind::StringLiteral); name_literal.has_value())
        return InstanceNameDeclarator::create(*name_literal);

    if (const auto hashtag_token = match_token(state, SyntaxKind::Hashtag); hashtag_token.has_value())
        return InstanceTagDeclarator::create(*hashtag_token, consume(state, SyntaxKind::Identifier));

    return parse_instance_declarator_with_initializer(state);
}

static statement_ptr_t parse_instance_constructor(ParseState& state)
{
    const auto instance_keyword = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_required_colon_type_clause(state);
    const auto clone_keyword = match_token(state, SyntaxKind::CloneKeyword);
    std::optional<expression_ptr_t> clone_target = std::nullopt;
    if (clone_keyword.has_value())
        clone_target = parse_expression(state);

    const auto l_brace = match_token(state, SyntaxKind::LBrace);
    std::vector<statement_ptr_t> property_declarators;
    std::optional<Token> r_brace = std::nullopt;
    if (l_brace.has_value())
    {
        while (!check(state, SyntaxKind::RBrace))
        {
            property_declarators.push_back(parse_instance_declarator(state));
            match(state, SyntaxKind::Comma);
        }

        r_brace = consume(state, SyntaxKind::RBrace);
    }

    const auto long_arrow = match_token(state, SyntaxKind::LongArrow);
    std::optional<expression_ptr_t> parent = std::nullopt;
    if (long_arrow.has_value())
        parent = parse_expression(state);

    return InstanceConstructor::create(instance_keyword, name, colon_type_clause,
                                       clone_keyword, std::move(clone_target), l_brace, std::move(property_declarators), r_brace,
                                       long_arrow, std::move(parent));
}

static statement_ptr_t parse_if(ParseState& state)
{
    const auto if_keyword = *previous_token(state);
    auto condition = parse_expression(state);
    auto then_branch = parse_statement(state);
    const auto else_keyword = match_token(state, SyntaxKind::ElseKeyword);
    std::optional<statement_ptr_t> else_branch = std::nullopt;
    if (else_keyword.has_value())
        else_branch = parse_statement(state);

    return If::create(if_keyword, std::move(condition), std::move(then_branch), else_keyword, std::move(else_branch));
}

static statement_ptr_t parse_while(ParseState& state)
{
    const auto keyword = *previous_token(state);
    auto condition = parse_expression(state);
    auto statement = parse_statement(state);

    return While::create(keyword, std::move(condition), std::move(statement));
}

static statement_ptr_t parse_repeat(ParseState& state)
{
    const auto repeat_keyword = *previous_token(state);
    auto statement = parse_statement(state);
    const auto while_keyword = consume(state, SyntaxKind::WhileKeyword);
    auto condition = parse_expression(state);
    std::cout << condition->get_text() << '\n';

    return Repeat::create(repeat_keyword, std::move(statement), while_keyword, std::move(condition));
}

static std::vector<Token> parse_name_list(ParseState& state)
{
    std::vector<Token> names;
    do
    {
        const auto name = match_token(state, SyntaxKind::Star).value_or(consume(state, SyntaxKind::Identifier));
        names.push_back(name);
    } while (match(state, SyntaxKind::Comma));

    return names;
}

static statement_ptr_t parse_for(ParseState& state)
{
    const auto keyword = *previous_token(state);
    const auto names = parse_name_list(state);
    const auto colon_token = consume(state, SyntaxKind::Colon);
    auto iterable = parse_expression(state);
    auto statement = parse_statement(state);

    return For::create(keyword, names, colon_token, std::move(iterable), std::move(statement));
}

static statement_ptr_t parse_after(ParseState& state)
{
    const auto keyword = *previous_token(state);
    auto time_expression = parse_expression(state);
    auto statement = parse_statement(state);

    return After::create(keyword, std::move(time_expression), std::move(statement));
}

static statement_ptr_t parse_every(ParseState& state)
{
    const auto keyword = *previous_token(state);
    auto time_expression = parse_expression(state);
    auto statement = parse_statement(state);

    return Every::create(keyword, std::move(time_expression), std::move(statement));
}

static statement_ptr_t parse_import(ParseState& state)
{
    const auto import_keyword = *previous_token(state);
    auto names = parse_name_list(state);

    if (names.empty())
    {
        if (match(state, SyntaxKind::Star))
        {
            const auto star_token = *previous_token(state);
            names.push_back(star_token);
        }
        else
        {
            const auto token = current_token(state).value_or(import_keyword);
            report_expected_different_syntax(import_keyword.span, "import name", token.get_text(), false);
        }
    }

    const auto from_keyword = consume(state, SyntaxKind::FromKeyword);
    const auto module_name = consume(state, SyntaxKind::Identifier, "module name");
    return Import::create(import_keyword, names, from_keyword, module_name);
}

static statement_ptr_t parse_return(ParseState& state)
{
    const auto keyword = *previous_token(state);
    std::optional<expression_ptr_t> expression = std::nullopt;

    if (!is_eof(state) && !keyword.span.has_line_break_between(current_token_guaranteed(state)))
        expression = parse_expression(state);

    return Return::create(keyword, std::move(expression));
}

static statement_ptr_t parse_expression_statement(ParseState& state)
{
    auto expression = parse_expression(state);
    return ExpressionStatement::create(std::move(expression));
}

static statement_ptr_t parse_declaration(ParseState& state)
{
    std::optional<Token> export_keyword = std::nullopt;
    if (match(state, SyntaxKind::ExportKeyword))
        export_keyword = previous_token(state);

    std::optional<Token> async_keyword = std::nullopt;
    if (check(state, SyntaxKind::AsyncKeyword) && check(state, SyntaxKind::FnKeyword, 1))
        async_keyword = advance(state);

    std::optional<statement_ptr_t> statement = std::nullopt;
    if (match(state, SyntaxKind::LetKeyword))
        statement = parse_variable_declaration(state);
    else if (match(state, SyntaxKind::FnKeyword))
        statement = parse_function_declaration(state, async_keyword);
    else if (match(state, SyntaxKind::EventKeyword))
        statement = parse_event_declaration(state);
    else if (match(state, SyntaxKind::TypeKeyword))
        statement = parse_type_declaration(state);
    else if (match(state, SyntaxKind::EnumKeyword))
        statement = parse_enum_declaration(state);
    else if (match(state, SyntaxKind::InstanceKeyword))
        statement = parse_instance_constructor(state);

    if (export_keyword.has_value())
    {
        if (!statement.has_value())
            report_invalid_export(parse_expression_statement(state));

        return Export::create(std::move(*export_keyword), std::move(*statement));
    }

    return statement.has_value()
               ? std::move(statement.value())
               : parse_expression_statement(state);
}

statement_ptr_t parse_statement(ParseState& state)
{
    if (match(state, SyntaxKind::LBrace))
        return parse_block(state);
    if (match(state, SyntaxKind::IfKeyword))
        return parse_if(state);
    if (match(state, SyntaxKind::WhileKeyword))
        return parse_while(state);
    if (match(state, SyntaxKind::RepeatKeyword))
        return parse_repeat(state);
    if (match(state, SyntaxKind::ForKeyword))
        return parse_for(state);
    if (match(state, SyntaxKind::AfterKeyword))
        return parse_after(state);
    if (match(state, SyntaxKind::EveryKeyword))
        return parse_every(state);
    if (match(state, SyntaxKind::ImportKeyword))
        return parse_import(state);
    if (match(state, SyntaxKind::ReturnKeyword))
        return parse_return(state);
    if (match(state, SyntaxKind::BreakKeyword))
    {
        const auto keyword = *previous_token(state);
        return Break::create(keyword);
    }
    if (match(state, SyntaxKind::ContinueKeyword))
    {
        const auto keyword = *previous_token(state);
        return Continue::create(keyword);
    }

    return parse_declaration(state);
}

type_ref_ptr_t parse_primitive_type(ParseState& state)
{
    const auto token_opt = advance(state);
    if (!token_opt.has_value() || token_opt.value().kind != SyntaxKind::Identifier)
    {
        const auto last_token = previous_token(state);
        if (!last_token.has_value())
            report_unexpected_eof(get_current_optional_span(state, -2));

        report_expected_different_syntax(last_token->span, "type", last_token->get_text(), false);
    }

    const auto& token = *token_opt;
    if (primitive_type_names.contains(token.get_text()))
        return PrimitiveTypeRef::create(token);

    const auto type_arguments = parse_type_arguments(state);
    return TypeNameRef::create(token, type_arguments);
}

static type_ref_ptr_t parse_union_type(ParseState& state)
{
    const auto first_pipe_token = match_token(state, SyntaxKind::Pipe);
    std::vector<Token> pipe_tokens;
    std::vector<type_ref_ptr_t> types;
    types.push_back(parse_primitive_type(state));
    if (first_pipe_token.has_value())
        pipe_tokens.push_back(*first_pipe_token);

    while (match(state, SyntaxKind::Pipe))
    {
        const auto pipe_token = *previous_token(state);
        pipe_tokens.push_back(pipe_token);
        types.push_back(parse_primitive_type(state));
    }

    if (types.size() > 1)
        return UnionTypeRef::create(pipe_tokens, std::move(types));

    return std::move(types.front());
}

static type_ref_ptr_t parse_intersection_type(ParseState& state)
{
    const auto first_ampersand_token = match_token(state, SyntaxKind::Ampersand);
    std::vector<Token> ampersand_tokens;
    std::vector<type_ref_ptr_t> types;
    types.push_back(parse_union_type(state));
    if (first_ampersand_token.has_value())
        ampersand_tokens.push_back(*first_ampersand_token);

    while (match(state, SyntaxKind::Ampersand))
    {
        const auto ampersand_token = *previous_token(state);
        ampersand_tokens.push_back(ampersand_token);
        types.push_back(parse_union_type(state));
    }

    if (types.size() > 1)
        return IntersectionTypeRef::create(ampersand_tokens, std::move(types));

    return std::move(types.front());
}

static type_ref_ptr_t parse_nullable_type(ParseState& state)
{
    auto non_nullable_type = parse_intersection_type(state);
    return match(state, SyntaxKind::Question)
               ? NullableTypeRef::create(std::move(non_nullable_type), *previous_token(state))
               : std::move(non_nullable_type);
}

type_ref_ptr_t parse_type(ParseState& state)
{
    return parse_nullable_type(state);
}

type_ref_ptr_t parse_type_parameter(ParseState& state)
{
    const auto name = consume(state, SyntaxKind::Identifier, "type parameter");
    const auto colon_token = match_token(state, SyntaxKind::Colon);
    std::optional<type_ref_ptr_t> base_type = std::nullopt;
    if (colon_token.has_value())
        base_type = parse_type(state);

    return TypeParameterRef::create(name, colon_token, std::move(base_type));
}

std::vector<statement_ptr_t> parse(SourceFile* file)
{
    const auto tokens = tokenize(file);
    logger::info("Parsing file: " + file->path);
    auto state = ParseState { .file = file, .token_stream = tokens };
    while (!is_eof(state))
        file->statements.push_back(parse_statement(state));

    logger::info("Parsed " + std::to_string(file->statements.size()) + " statements");
    return std::move(file->statements);
}