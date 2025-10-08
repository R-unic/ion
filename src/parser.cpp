#include <algorithm>
#include <functional>
#include <utility>

#include "ion/logger.h"
#include "ion/diagnostics.h"
#include "ion/source_file.h"
#include "ion/lexer.h"
#include "ion/parser.h"

#include "ion/ast/statements/decorator.h"

static void consume_semicolons(ParseState& state)
{
    while (match(state, SyntaxKind::Semicolon))
        continue;
}

static expression_ptr_t parse_parenthesized(ParseState& state)
{
    const auto l_paren = *previous_token(state);
    std::vector<expression_ptr_t> elements;
    do
        elements.push_back(parse_expression(state));
    while (match(state, SyntaxKind::Comma));

    const auto r_paren = expect(state, SyntaxKind::RParen);
    return elements.size() > 1
               ? TupleLiteral::create(l_paren, r_paren, std::move(elements))
               : Parenthesized::create(l_paren, r_paren, std::move(elements.front()));
}

/** Parses a literal with multiple components surrounded by '<(' and ')>' */
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

static expression_ptr_t parse_rgb_literal(ParseState& state)
{
    const auto keyword = peek(state, -1);
    return parse_components_literal(state, 3, [&](const auto& l_arrow, const auto& r_arrow, auto components)
    {
        return RgbLiteral::create(keyword, l_arrow, r_arrow, std::move(components[0]), std::move(components[1]), std::move(components[2]));
    });
}

static expression_ptr_t parse_hsv_literal(ParseState& state)
{
    const auto keyword = peek(state, -1);
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
    std::vector parts = { *previous_token(state) };
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
    const auto l_bracket = *previous_token(state);
    std::vector<expression_ptr_t> elements;
    if (!check(state, SyntaxKind::RBracket))
        do
            elements.push_back(parse_expression(state));
        while (match(state, SyntaxKind::Comma));

    const auto r_bracket = expect(state, SyntaxKind::RBracket);
    return ArrayLiteral::create(l_bracket, r_bracket, std::move(elements));
}

static expression_ptr_t parse_tuple_literal(ParseState& state)
{
    const auto l_paren = *previous_token(state);
    std::vector<expression_ptr_t> elements;
    do
        elements.push_back(parse_expression(state));
    while (match(state, SyntaxKind::Comma));

    const auto r_paren = expect(state, SyntaxKind::RParen);
    return TupleLiteral::create(l_paren, r_paren, std::move(elements));
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
    const auto l_paren = *previous_token(state);
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
    const auto token = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    return question_token.has_value()
               ? OptionalMemberAccess::create(token, *question_token, std::move(expression), name)
               : MemberAccess::create(token, std::move(expression), name);
}

static expression_ptr_t parse_element_access(ParseState& state, expression_ptr_t expression)
{
    const auto l_bracket = *previous_token(state);
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
            const auto operator_token = *previous_token(state);
            expression = PostfixUnaryOp::create(operator_token, std::move(expression));
        }
        else
            break;
    }

    return expression;
}

template <typename T>
static std::unique_ptr<T> unsafe_cast(expression_ptr_t& expression)
{
    return std::unique_ptr<T>(static_cast<T*>(expression.release()));
}

static expression_ptr_t parse_unary(ParseState& state)
{
    if (match(state, SyntaxKind::NameOfKeyword))
    {
        const auto keyword = *previous_token(state);
        auto expression = parse_postfix(state);
        assert_nameof_target(expression);

        const auto identifier = expression->get_last_token();
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


static expression_ptr_t parse_binary_expression(ParseState& state,
                                                const std::function<expression_ptr_t(ParseState&)>& subparser,
                                                const SyntaxKind operator_kind)
{
    auto left = subparser(state);
    while (match(state, operator_kind))
    {
        const auto operator_token = *previous_token(state);
        auto right = subparser(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_binary_expression(ParseState& state,
                                                const std::function<expression_ptr_t (ParseState&)>& subparser,
                                                const std::set<SyntaxKind>& operator_kinds)
{
    auto left = subparser(state);
    while (match_any(state, operator_kinds))
    {
        const auto op = *previous_token(state);
        auto right = subparser(state);
        left = BinaryOp::create(op, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_exponentiation(ParseState& state)
{
    return parse_binary_expression(state, parse_unary, SyntaxKind::Caret);
}

static expression_ptr_t parse_multiplication(ParseState& state)
{
    return parse_binary_expression(state, parse_exponentiation, multiplicative_syntaxes);
}

static expression_ptr_t parse_addition(ParseState& state)
{
    return parse_binary_expression(state, parse_multiplication, additive_syntaxes);
}

static expression_ptr_t parse_bit_shift(ParseState& state)
{
    return parse_binary_expression(state, parse_addition, bit_shift_syntaxes);
}

static expression_ptr_t parse_bitwise_and(ParseState& state)
{
    return parse_binary_expression(state, parse_bit_shift, SyntaxKind::Ampersand);
}

static expression_ptr_t parse_bitwise_xor(ParseState& state)
{
    return parse_binary_expression(state, parse_bitwise_and, SyntaxKind::Tilde);
}

static expression_ptr_t parse_bitwise_or(ParseState& state)
{
    return parse_binary_expression(state, parse_bitwise_xor, SyntaxKind::Pipe);
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

static expression_ptr_t parse_comparison(ParseState& state)
{
    return parse_binary_expression(state, parse_range_literal, comparison_syntaxes);
}

static expression_ptr_t parse_logical_and(ParseState& state)
{
    return parse_binary_expression(state, parse_comparison, SyntaxKind::AmpersandAmpersand);
}

static expression_ptr_t parse_logical_or(ParseState& state)
{
    return parse_binary_expression(state, parse_logical_and, SyntaxKind::PipePipe);
}

static expression_ptr_t parse_null_coalesce(ParseState& state)
{
    return parse_binary_expression(state, parse_logical_or, SyntaxKind::QuestionQuestion);
}

static expression_ptr_t parse_ternary_op(ParseState& state)
{
    auto condition = parse_null_coalesce(state);
    while (match(state, SyntaxKind::Question))
    {
        const auto question_token = *previous_token(state);
        auto when_true = parse_expression(state);
        const auto colon_token = expect(state, SyntaxKind::Colon);
        auto when_false = parse_expression(state);

        condition = TernaryOp::create(question_token, colon_token, std::move(condition), std::move(when_true),
                                      std::move(when_false));
    }

    return condition;
}

static expression_ptr_t parse_assignment(ParseState& state)
{
    auto left = parse_ternary_op(state);
    while (match_any(state, assignment_syntaxes))
    {
        assert_assignment_target(left);
        const auto operator_token = *previous_token(state);
        auto right = parse_ternary_op(state);
        left = AssignmentOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

expression_ptr_t parse_expression(ParseState& state)
{
    return parse_assignment(state);
}

static BracedStatementList* parse_braced_statement_list(ParseState& state,
                                                        const std::function<statement_ptr_t (ParseState&)>& parse_list_statement,
                                                        const bool comma_allowed = true)
{
    const auto l_brace = expect(state, SyntaxKind::LBrace);
    std::vector<statement_ptr_t> statements;

    while (!check(state, SyntaxKind::RBrace))
    {
        statements.push_back(parse_list_statement(state));
        if (comma_allowed)
            match(state, SyntaxKind::Comma);
    }

    const auto r_brace = expect(state, SyntaxKind::RBrace);
    return new BracedStatementList(l_brace, std::move(statements), r_brace);
}

static BracedStatementList* parse_braced_statement_list_comma_separated(ParseState& state,
                                                                        const std::function<statement_ptr_t (ParseState&)>&
                                                                        parse_list_statement)
{
    const auto l_brace = expect(state, SyntaxKind::LBrace);
    std::vector<statement_ptr_t> statements;
    if (!check(state, SyntaxKind::RBrace))
        do
            statements.push_back(parse_list_statement(state));
        while (match(state, SyntaxKind::Comma));

    const auto r_brace = expect(state, SyntaxKind::RBrace);
    return new BracedStatementList(l_brace, std::move(statements), r_brace);
}

static statement_ptr_t parse_block(ParseState& state)
{
    const auto braced_statement_list = parse_braced_statement_list(state, parse_statement, false);
    logger::info("Checking for unreachable code at block level");
    check_for_unreachable_code(braced_statement_list->statements);

    return Block::create(braced_statement_list);
}

template <typename F>
auto parse_optional(ParseState& state, const SyntaxKind kind, F&& parser) -> std::optional<decltype(parser(state))>
{
    if (check(state, kind))
        return parser(state);

    return std::nullopt;
}

static ColonTypeClause* parse_colon_type_clause(ParseState& state)
{
    const auto colon_token = expect(state, SyntaxKind::Colon);
    auto type = parse_type(state);
    return new ColonTypeClause(colon_token, std::move(type));
}

static std::optional<EqualsValueClause*> parse_equals_value_clause(ParseState& state)
{
    const auto equals_token = try_consume(state, SyntaxKind::Equals);
    if (!equals_token.has_value())
        return std::nullopt;

    auto value = parse_expression(state);
    return new EqualsValueClause(*equals_token, std::move(value));
}

static statement_ptr_t parse_variable_declaration(ParseState& state)
{
    const auto let_keyword = *previous_token(state);
    const auto const_keyword = try_consume(state, SyntaxKind::ConstKeyword);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_optional(state, SyntaxKind::Colon, parse_colon_type_clause);
    const auto equals_value_clause = parse_equals_value_clause(state);

    return VariableDeclaration::create(let_keyword, const_keyword, name, colon_type_clause, equals_value_clause);
}

static std::vector<type_ref_ptr_t> parse_type_list(ParseState& state,
                                                   const std::function<type_ref_ptr_t (ParseState&)>& subparser = parse_type)
{
    std::vector<type_ref_ptr_t> list;
    do
        list.push_back(subparser(state));
    while (match(state, SyntaxKind::Comma));

    return list;
}

std::optional<TypeListClause*> parse_type_arguments(ParseState& state)
{
    const auto l_arrow = try_consume(state, SyntaxKind::LArrow);
    if (!l_arrow.has_value())
        return std::nullopt;

    auto list = parse_type_list(state);
    const auto r_arrow = expect_r_arrow(state);
    return new TypeListClause(*l_arrow, std::move(list), r_arrow);
}

std::optional<TypeListClause*> parse_type_parameters(ParseState& state)
{
    const auto l_arrow = try_consume(state, SyntaxKind::LArrow);
    if (!l_arrow.has_value())
        return std::nullopt;

    auto list = parse_type_list(state, parse_type_parameter);
    const auto r_arrow = expect(state, SyntaxKind::RArrow);
    return new TypeListClause(*l_arrow, std::move(list), r_arrow);
}

static statement_ptr_t parse_type_declaration(ParseState& state)
{
    const auto type_keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    const auto equals_token = expect(state, SyntaxKind::Equals);
    auto type = parse_type(state);

    return TypeDeclaration::create(type_keyword, name, type_parameters, equals_token, std::move(type));
}

static statement_ptr_t parse_event_declaration(ParseState& state)
{
    const auto event_keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    const auto l_paren = try_consume(state, SyntaxKind::LParen);
    std::vector<type_ref_ptr_t> parameter_types;
    std::optional<Token> r_paren = std::nullopt;
    if (l_paren.has_value())
    {
        if (!check(state, SyntaxKind::RParen))
            parameter_types = parse_type_list(state);

        r_paren = expect(state, SyntaxKind::RParen);
    }

    return EventDeclaration::create(event_keyword, name, type_parameters, l_paren, std::move(parameter_types), r_paren);
}

static statement_ptr_t parse_enum_member(ParseState& state)
{
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto equals_value_clause = parse_equals_value_clause(state);
    return EnumMember::create(name, equals_value_clause);
}

static statement_ptr_t parse_enum_declaration(ParseState& state)
{
    const auto keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto braced_statement_list = parse_braced_statement_list(state, parse_enum_member);

    return EnumDeclaration::create(keyword, name, braced_statement_list);
}

static statement_ptr_t parse_interface_member(ParseState& state)
{
    if (const auto fn_keyword = try_consume(state, SyntaxKind::FnKeyword); fn_keyword.has_value())
    {
        const auto name = expect(state, SyntaxKind::Identifier);
        const auto type_parameters = parse_type_parameters(state);
        const auto l_paren = expect(state, SyntaxKind::LParen);
        std::vector<type_ref_ptr_t> parameter_types;
        if (!check(state, SyntaxKind::RParen))
            parameter_types = parse_type_list(state);

        const auto r_paren = expect(state, SyntaxKind::RParen);
        const auto colon_token = expect(state, SyntaxKind::Colon);
        auto return_type = parse_type(state);

        return InterfaceMethod::create(*fn_keyword, name, type_parameters, l_paren, std::move(parameter_types), r_paren,
                                       colon_token, std::move(return_type));
    }

    const auto const_keyword = try_consume(state, SyntaxKind::ConstKeyword);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_token = expect(state, SyntaxKind::Colon);
    auto type = parse_type(state);
    return InterfaceField::create(const_keyword, name, colon_token, std::move(type));
}

static statement_ptr_t parse_interface_declaration(ParseState& state)
{
    const auto keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto braced_statement_list = parse_braced_statement_list(state, parse_interface_member);

    return InterfaceDeclaration::create(keyword, name, braced_statement_list);
}

static statement_ptr_t parse_parameter(ParseState& state)
{
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_optional(state, SyntaxKind::Colon, parse_colon_type_clause);
    const auto equals_value_clause = parse_equals_value_clause(state);

    return Parameter::create(name, colon_type_clause, equals_value_clause);
}

static FunctionBody* parse_function_body(ParseState& state)
{
    const auto long_arrow = try_consume(state, SyntaxKind::LongArrow);
    std::optional<ExpressionBody*> expression_body = std::nullopt;
    std::optional<statement_ptr_t> block = std::nullopt;

    if (long_arrow.has_value())
        expression_body = new ExpressionBody(*long_arrow, parse_expression(state));
    else if (check(state, SyntaxKind::LBrace))
        block = parse_block(state);
    else
    {
        const auto last_token = peek(state, -1);
        const auto token = current_token(state);
        const auto location = last_token.span.end;
        const auto span = create_span(location, location);
        const auto text = token.has_value() ? token->get_text() : "EOF";

        report_expected_different_syntax(span, "function body", text, false);
    }

    return new FunctionBody(expression_body, std::move(block));
}

static ParameterListClause* parse_parameter_list(ParseState& state)
{
    const auto l_paren = expect(state, SyntaxKind::LParen);
    std::vector<statement_ptr_t> list;
    if (!check(state, SyntaxKind::RParen))
        do
            list.push_back(parse_parameter(state));
        while (match(state, SyntaxKind::Comma));

    const auto r_paren = expect(state, SyntaxKind::RParen);
    return new ParameterListClause(l_paren, std::move(list), r_paren);
}

static statement_ptr_t parse_decorator(ParseState& state)
{
    const auto at_token = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto l_paren = try_consume(state, SyntaxKind::LParen);
    std::vector<expression_ptr_t> arguments;
    if (l_paren.has_value() && !check(state, SyntaxKind::RParen))
        do
            arguments.push_back(parse_expression(state));
        while (match(state, SyntaxKind::Comma));

    std::optional<Token> r_paren = std::nullopt;
    if (l_paren.has_value())
        r_paren = expect(state, SyntaxKind::RParen);

    return Decorator::create(at_token, name, l_paren, r_paren, std::move(arguments));
}

static std::vector<statement_ptr_t> parse_decorator_list(ParseState& state)
{
    std::vector<statement_ptr_t> list;
    while (match(state, SyntaxKind::At))
        list.push_back(parse_decorator(state));

    return list;
}

static statement_ptr_t parse_function_declaration(ParseState& state, const std::optional<Token>& async_keyword,
                                                  std::vector<statement_ptr_t> decorator_list)
{
    const auto fn_keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    const auto parameters = parse_optional(state, SyntaxKind::LParen, parse_parameter_list);
    const auto return_type = parse_optional(state, SyntaxKind::Colon, parse_colon_type_clause);
    const auto body = parse_function_body(state);

    return FunctionDeclaration::create(std::move(decorator_list), async_keyword, fn_keyword, name,
                                       type_parameters, parameters, return_type, body);
}

static statement_ptr_t parse_instance_declarator_with_initializer(ParseState& state)
{
    const auto at_token = try_consume(state, SyntaxKind::At);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_token = expect(state, SyntaxKind::Colon);
    auto value = parse_expression(state);

    return at_token.has_value()
               ? InstanceAttributeDeclarator::create(*at_token, name, colon_token, std::move(value))
               : InstancePropertyDeclarator::create(name, colon_token, std::move(value));
}

static statement_ptr_t parse_instance_declarator(ParseState& state)
{
    if (const auto name_literal = try_consume(state, SyntaxKind::StringLiteral); name_literal.has_value())
        return InstanceNameDeclarator::create(*name_literal);

    if (const auto hashtag_token = try_consume(state, SyntaxKind::Hashtag); hashtag_token.has_value())
        return InstanceTagDeclarator::create(*hashtag_token, expect(state, SyntaxKind::Identifier));

    return parse_instance_declarator_with_initializer(state);
}

static statement_ptr_t parse_instance_constructor(ParseState& state)
{
    const auto instance_keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_colon_type_clause(state);
    const auto clone_keyword = try_consume(state, SyntaxKind::CloneKeyword);
    std::optional<expression_ptr_t> clone_target = std::nullopt;
    if (clone_keyword.has_value())
        clone_target = parse_expression(state);

    const auto declarators = parse_optional(state, SyntaxKind::Colon, [&](ParseState&)
    {
        return parse_braced_statement_list(state, parse_instance_declarator);
    });
    const auto long_arrow = try_consume(state, SyntaxKind::LongArrow);
    std::optional<expression_ptr_t> parent = std::nullopt;
    if (long_arrow.has_value())
        parent = parse_expression(state);

    return InstanceConstructor::create(instance_keyword, name, colon_type_clause, clone_keyword, std::move(clone_target),
                                       declarators, long_arrow, std::move(parent));
}

static statement_ptr_t parse_if(ParseState& state)
{
    const auto if_keyword = *previous_token(state);
    auto condition = parse_expression(state);
    check_for_ambiguous_equals(condition);

    auto then_branch = parse_statement(state);
    const auto else_keyword = try_consume(state, SyntaxKind::ElseKeyword);
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
    check_for_ambiguous_equals(condition);

    return While::create(keyword, std::move(condition), std::move(statement));
}

static statement_ptr_t parse_repeat(ParseState& state)
{
    const auto repeat_keyword = *previous_token(state);
    auto statement = parse_statement(state);
    const auto while_keyword = expect(state, SyntaxKind::WhileKeyword);
    auto condition = parse_expression(state);
    check_for_ambiguous_equals(condition);

    return Repeat::create(repeat_keyword, std::move(statement), while_keyword, std::move(condition));
}

static std::vector<Token> parse_name_list(ParseState& state)
{
    std::vector<Token> names;
    do
    {
        const auto name = try_consume(state, SyntaxKind::Star).value_or(expect(state, SyntaxKind::Identifier));
        names.push_back(name);
    } while (match(state, SyntaxKind::Comma));

    return names;
}

static statement_ptr_t parse_for(ParseState& state)
{
    const auto keyword = *previous_token(state);
    const auto names = parse_name_list(state);
    const auto colon_token = expect(state, SyntaxKind::Colon);
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

static std::pair<Token, statement_ptr_t> parse_match_case_block(ParseState& state)
{
    auto long_arrow = expect(state, SyntaxKind::LongArrow);
    const auto is_block = check(state, SyntaxKind::LBrace);
    auto statement = is_block
                         ? parse_block(state)
                         : ExpressionStatement::create(parse_expression(state));

    return std::make_pair<Token, statement_ptr_t>(std::move(long_arrow), std::move(statement));
}

using ParseCaseBodyFn = std::function<std::pair<Token, statement_ptr_t> (ParseState&)>;

static statement_ptr_t parse_match_else_case(ParseState& state, const ParseCaseBodyFn& parse_case_body)
{
    const auto keyword = *previous_token(state);
    const auto name = try_consume(state, SyntaxKind::Identifier);
    auto [long_arrow, statement] = parse_case_body(state);

    return MatchElseCase::create(keyword, name, long_arrow, std::move(statement));
}

static statement_ptr_t parse_match_case(ParseState& state, const ParseCaseBodyFn& parse_case_body)
{
    if (match(state, SyntaxKind::ElseKeyword))
        return parse_match_else_case(state, parse_case_body);

    std::vector<expression_ptr_t> comparands;
    do
        comparands.push_back(parse_expression(state));
    while (match(state, SyntaxKind::Comma));

    auto [long_arrow, statement] = parse_case_body(state);
    return MatchCase::create(std::move(comparands), long_arrow, std::move(statement));
}

static statement_ptr_t parse_match(ParseState& state)
{
    const auto keyword = *previous_token(state);
    auto expression = parse_expression(state);
    const auto cases = parse_braced_statement_list_comma_separated(state, [&](const auto& _)
    {
        return parse_match_case(state, parse_match_case_block);
    });

    return Match::create(keyword, std::move(expression), cases);
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

    const auto from_keyword = expect(state, SyntaxKind::FromKeyword);
    const auto module_name = expect(state, SyntaxKind::Identifier, "module name");
    return Import::create(import_keyword, names, from_keyword, module_name);
}

static statement_ptr_t parse_return(ParseState& state)
{
    const auto keyword = *previous_token(state);
    std::optional<expression_ptr_t> expression = std::nullopt;

    const auto has_line_break_between = keyword.span.has_line_break_between(current_token_guaranteed(state));
    if (!is_eof(state) && !check(state, SyntaxKind::Semicolon) && !has_line_break_between)
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
    std::vector<statement_ptr_t> decorator_list;
    if (check(state, SyntaxKind::At))
        decorator_list = parse_decorator_list(state);

    std::optional<Token> export_keyword = std::nullopt;
    if (match(state, SyntaxKind::ExportKeyword))
        export_keyword = previous_token(state);

    std::optional<Token> async_keyword = std::nullopt;
    const auto is_sync_function = check(state, SyntaxKind::FnKeyword);
    const auto is_async_function = check(state, SyntaxKind::AsyncKeyword) && check(state, SyntaxKind::FnKeyword, 1);
    if (is_async_function)
        async_keyword = advance(state);

    std::optional<statement_ptr_t> statement = std::nullopt;
    if (match(state, SyntaxKind::LetKeyword))
        statement = parse_variable_declaration(state);
    else if (match(state, SyntaxKind::FnKeyword))
        statement = parse_function_declaration(state, async_keyword, std::move(decorator_list));
    else if (match(state, SyntaxKind::EventKeyword))
        statement = parse_event_declaration(state);
    else if (match(state, SyntaxKind::TypeKeyword))
        statement = parse_type_declaration(state);
    else if (match(state, SyntaxKind::EnumKeyword))
        statement = parse_enum_declaration(state);
    else if (match(state, SyntaxKind::InterfaceKeyword))
        statement = parse_interface_declaration(state);
    else if (match(state, SyntaxKind::InstanceKeyword))
        statement = parse_instance_constructor(state);

    if (!is_async_function && !is_sync_function && !decorator_list.empty())
    {
        if (statement.has_value())
            report_invalid_decorator_target(*statement);

        report_invalid_decorator_target(current_token(state).value_or(*previous_token(state)));
    }

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

static statement_ptr_t parse_break(const ParseState& state)
{
    const auto keyword = *previous_token(state);
    return Break::create(keyword);
}

static statement_ptr_t parse_continue(const ParseState& state)
{
    const auto keyword = *previous_token(state);
    return Continue::create(keyword);
}

static const std::unordered_map<SyntaxKind, std::function<statement_ptr_t (ParseState&)>> statement_parsers = {
    { SyntaxKind::IfKeyword, parse_if },
    { SyntaxKind::WhileKeyword, parse_while },
    { SyntaxKind::RepeatKeyword, parse_repeat },
    { SyntaxKind::ForKeyword, parse_for },
    { SyntaxKind::AfterKeyword, parse_after },
    { SyntaxKind::EveryKeyword, parse_every },
    { SyntaxKind::MatchKeyword, parse_match },
    { SyntaxKind::ImportKeyword, parse_import },
    { SyntaxKind::ReturnKeyword, parse_return },
    { SyntaxKind::BreakKeyword, parse_break },
    { SyntaxKind::ContinueKeyword, parse_continue }
};

statement_ptr_t parse_statement(ParseState& state)
{
    if (check(state, SyntaxKind::LBrace))
        return parse_block(state);

    for (const auto& [syntax, subparser] : statement_parsers)
    {
        if (!match(state, syntax))
            continue;

        auto statement = subparser(state);
        consume_semicolons(state);
        return statement;
    }

    auto declaration = parse_declaration(state);
    consume_semicolons(state);

    return declaration;
}

type_ref_ptr_t parse_type_name(ParseState& state)
{
    const auto token = *previous_token(state);
    if (primitive_type_names.contains(token.get_text()))
        return PrimitiveTypeRef::create(token);

    const auto type_arguments = parse_type_arguments(state);
    return TypeNameRef::create(token, type_arguments);
}

type_ref_ptr_t parse_literal_type(const ParseState& state)
{
    const auto token = *previous_token(state);
    const auto value = primitive_from_string(token.get_text());
    return LiteralTypeRef::create(token, value);
}

type_ref_ptr_t parse_tuple_type(ParseState& state)
{
    const auto l_paren = *previous_token(state);
    auto elements = parse_type_list(state);
    const auto r_paren = expect(state, SyntaxKind::RParen);

    return TupleTypeRef::create(l_paren, std::move(elements), r_paren);
}

type_ref_ptr_t parse_singular_type(ParseState& state)
{
    if (is_eof(state))
        report_unexpected_eof(fallback_span(state, -2));

    if (match(state, SyntaxKind::Identifier))
        return parse_type_name(state);
    if (match_any(state, primitive_literal_syntaxes))
        return parse_literal_type(state);
    if (match(state, SyntaxKind::LParen))
        return parse_tuple_type(state);

    const auto token = current_token_guaranteed(state);
    report_expected_different_syntax(token.span, "type", token.get_text(), false);
}

static type_ref_ptr_t parse_union_type(ParseState& state)
{
    const auto first_pipe_token = try_consume(state, SyntaxKind::Pipe);
    std::vector<Token> pipe_tokens;
    std::vector<type_ref_ptr_t> types;
    types.push_back(parse_singular_type(state));
    if (first_pipe_token.has_value())
        pipe_tokens.push_back(*first_pipe_token);

    while (match(state, SyntaxKind::Pipe))
    {
        const auto pipe_token = *previous_token(state);
        pipe_tokens.push_back(pipe_token);
        types.push_back(parse_singular_type(state));
    }

    return types.size() > 1
               ? UnionTypeRef::create(pipe_tokens, std::move(types))
               : std::move(types.front());
}

static type_ref_ptr_t parse_intersection_type(ParseState& state)
{
    const auto first_ampersand_token = try_consume(state, SyntaxKind::Ampersand);
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

    return types.size() > 1
               ? IntersectionTypeRef::create(ampersand_tokens, std::move(types))
               : std::move(types.front());
}

static type_ref_ptr_t parse_array_type(ParseState& state)
{
    auto element_type = parse_intersection_type(state);
    while (match(state, SyntaxKind::LBracket))
    {
        const auto l_bracket = *previous_token(state);
        const auto r_bracket = expect(state, SyntaxKind::RBracket);
        element_type = ArrayTypeRef::create(std::move(element_type), l_bracket, r_bracket);
    }

    return std::move(element_type);
}

static type_ref_ptr_t parse_function_type(ParseState& state)
{
    if (!check(state, SyntaxKind::LArrow) && !check(state, SyntaxKind::LParen))
        return parse_array_type(state);

    std::optional<TypeListClause*> type_parameters = std::nullopt;
    if (check(state, SyntaxKind::LArrow))
        type_parameters = parse_type_parameters(state);

    const auto l_paren = expect(state, SyntaxKind::LParen);
    auto parameter_types = parse_type_list(state);
    const auto r_paren = expect(state, SyntaxKind::RParen);
    const auto long_arrow = expect(state, SyntaxKind::LongArrow);
    auto return_type = parse_type(state);

    return FunctionTypeRef::create(type_parameters, l_paren, std::move(parameter_types), r_paren,
                                   long_arrow, std::move(return_type));
}

static type_ref_ptr_t parse_nullable_type(ParseState& state)
{
    auto non_nullable_type = parse_function_type(state);
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
    const auto name = expect(state, SyntaxKind::Identifier, "type parameter");
    const auto colon_token = try_consume(state, SyntaxKind::Colon);
    std::optional<type_ref_ptr_t> base_type = std::nullopt;
    if (colon_token.has_value())
        base_type = parse_type(state);

    const auto equals_token = try_consume(state, SyntaxKind::Equals);
    std::optional<type_ref_ptr_t> default_type = std::nullopt;
    if (equals_token.has_value())
        default_type = parse_type(state);

    return TypeParameterRef::create(name, colon_token, std::move(base_type), equals_token, std::move(default_type));
}

std::vector<statement_ptr_t> parse(SourceFile* file)
{
    const auto tokens = tokenize(file);
    logger::info("Parsing file: " + file->path);
    auto state = ParseState { .file = file, .token_stream = tokens };
    while (!is_eof(state))
        file->statements.push_back(parse_statement(state));

    logger::info("Checking for unreachable code at module level");
    check_for_unreachable_code(file->statements);
    logger::info("Parsed " + std::to_string(file->statements.size()) + " statements");
    return std::move(file->statements);
}