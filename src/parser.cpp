#include <iostream>
#include <algorithm>
#include <functional>
#include <utility>
#include <set>

#include "ion/logger.h"
#include "ion/diagnostics.h"
#include "ion/source_file.h"
#include "ion/lexer.h"
#include "ion/parser.h"

static expression_ptr_t parse_parenthesized(ParseState& state)
{
    const auto l_paren = peek(state, -2);
    auto expression = parse_expression(state);
    const auto r_paren = expect(state, SyntaxKind::RParen);

    return Parenthesized::create(l_paren, r_paren, std::move(expression));
}

static expression_ptr_t parse_rgb_literal(ParseState& state)
{
    const auto keyword = peek(state, -2);
    const auto l_arrow = expect(state, SyntaxKind::LArrow);
    const auto l_paren = expect(state, SyntaxKind::LParen);
    auto r = parse_expression(state);
    expect(state, SyntaxKind::Comma);
    auto g = parse_expression(state);
    expect(state, SyntaxKind::Comma);
    auto b = parse_expression(state);
    const auto r_paren = expect(state, SyntaxKind::RParen);
    const auto r_arrow = expect(state, SyntaxKind::RArrow);

    return RgbLiteral::create(keyword, l_arrow, r_arrow, std::move(r), std::move(g), std::move(b));
}

static expression_ptr_t parse_hsv_literal(ParseState& state)
{
    const auto keyword = peek(state, -2);
    const auto l_arrow = expect(state, SyntaxKind::LArrow);
    const auto l_paren = expect(state, SyntaxKind::LParen);
    auto h = parse_expression(state);
    expect(state, SyntaxKind::Comma);
    auto s = parse_expression(state);
    expect(state, SyntaxKind::Comma);
    auto v = parse_expression(state);
    const auto r_paren = expect(state, SyntaxKind::RParen);
    const auto r_arrow = expect(state, SyntaxKind::RArrow);

    return HsvLiteral::create(keyword, l_arrow, r_arrow, std::move(h), std::move(s), std::move(v));
}

static expression_ptr_t parse_vector_literal(ParseState& state)
{
    const auto l_arrow = *previous_token(state);
    const auto l_paren = expect(state, SyntaxKind::LParen);
    auto x = parse_expression(state);
    expect(state, SyntaxKind::Comma);
    auto y = parse_expression(state);
    expect(state, SyntaxKind::Comma);
    auto z = parse_expression(state);
    const auto r_paren = expect(state, SyntaxKind::RParen);
    const auto r_arrow = expect(state, SyntaxKind::RArrow);

    return VectorLiteral::create(l_arrow, r_arrow, std::move(x), std::move(y), std::move(z));
}

static expression_ptr_t parse_primary(ParseState& state)
{
    const auto span = fallback_span(state);
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
    const auto l_arrow = try_consume(state, SyntaxKind::LArrow);
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

static expression_ptr_t parse_member_access(ParseState& state, expression_ptr_t expression)
{
    const auto token = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    return MemberAccess::create(token, std::move(expression), name);
}

static expression_ptr_t parse_element_access(ParseState& state, expression_ptr_t expression)
{
    const auto l_bracket = *previous_token(state);
    auto index_expression = parse_expression(state);
    const auto r_bracket = expect(state, SyntaxKind::RBracket);

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
        const auto identifier = expect(state, SyntaxKind::Identifier);
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
                                                const std::vector<SyntaxKind>& operator_kinds)
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

const std::vector multiplicative_syntaxes = { SyntaxKind::Star, SyntaxKind::Slash, SyntaxKind::Percent };

static expression_ptr_t parse_multiplication(ParseState& state)
{
    return parse_binary_expression(state, parse_exponentiation, multiplicative_syntaxes);
}

const std::vector additive_syntaxes = { SyntaxKind::Plus, SyntaxKind::Minus };

static expression_ptr_t parse_addition(ParseState& state)
{
    return parse_binary_expression(state, parse_multiplication, additive_syntaxes);
}

const std::vector bit_shift_syntaxes = {
    SyntaxKind::LArrowLArrow, SyntaxKind::RArrowRArrow, SyntaxKind::RArrowRArrowRArrow
};

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

const std::vector assignment_syntaxes = {
    SyntaxKind::Equals,
    SyntaxKind::PlusEquals,
    SyntaxKind::MinusEquals,
    SyntaxKind::StarEquals,
    SyntaxKind::SlashEquals,
    SyntaxKind::CaretEquals,
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
        const auto colon_token = expect(state, SyntaxKind::Colon);
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

static statement_ptr_t parse_block(ParseState& state)
{
    const auto braced_statement_list = parse_braced_statement_list(state, parse_statement, false);
    return Block::create(braced_statement_list);
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
    const auto name = expect(state, SyntaxKind::Identifier);
    std::optional<ColonTypeClause*> colon_type_clause;
    if (check(state, SyntaxKind::Colon))
        colon_type_clause = parse_colon_type_clause(state);

    const auto equals_value_clause = parse_equals_value_clause(state);
    return VariableDeclaration::create(let_keyword, name, colon_type_clause, equals_value_clause);
}

static std::optional<TypeListClause*> parse_type_parameters(ParseState& state)
{
    const auto l_arrow = try_consume(state, SyntaxKind::LArrow);
    if (!l_arrow.has_value())
        return std::nullopt;

    std::vector<type_ref_ptr_t> list;
    do
        list.push_back(parse_type_parameter(state));
    while (match(state, SyntaxKind::Comma));

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
        do
            parameter_types.push_back(parse_type(state));
        while (match(state, SyntaxKind::Comma));

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
    const auto enum_keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto braced_statement_list = parse_braced_statement_list(state, parse_enum_member);

    return EnumDeclaration::create(enum_keyword, name, braced_statement_list);
}

static statement_ptr_t parse_parameter(ParseState& state)
{
    const auto name = expect(state, SyntaxKind::Identifier);
    std::optional<ColonTypeClause*> colon_type_clause;
    if (check(state, SyntaxKind::Colon))
        colon_type_clause = parse_colon_type_clause(state);

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
    do
        list.push_back(parse_parameter(state));
    while (match(state, SyntaxKind::Comma));

    const auto r_paren = expect(state, SyntaxKind::RParen);
    return new ParameterListClause(l_paren, std::move(list), r_paren);
}

static statement_ptr_t parse_function_declaration(ParseState& state, const std::optional<Token>& async_keyword)
{
    const auto fn_keyword = *previous_token(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    std::optional<ParameterListClause*> parameters = std::nullopt;
    if (check(state, SyntaxKind::LParen))
        parameters = parse_parameter_list(state);

    std::optional<ColonTypeClause*> return_type;
    if (check(state, SyntaxKind::Colon))
        return_type = parse_colon_type_clause(state);

    const auto body = parse_function_body(state);
    return FunctionDeclaration::create(async_keyword, fn_keyword, name, type_parameters, parameters, return_type, body);
}

const std::set<std::string> primitive_type_names = { "number", "string", "bool", "void" };

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

    const auto l_brace = try_consume(state, SyntaxKind::LBrace);
    std::optional<BracedStatementList*> declarators = std::nullopt;
    if (l_brace.has_value())
        declarators = parse_braced_statement_list(state, parse_instance_declarator);

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

    return While::create(keyword, std::move(condition), std::move(statement));
}

static statement_ptr_t parse_repeat(ParseState& state)
{
    const auto repeat_keyword = *previous_token(state);
    auto statement = parse_statement(state);
    const auto while_keyword = expect(state, SyntaxKind::WhileKeyword);
    auto condition = parse_expression(state);

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
    if (check(state, SyntaxKind::LBrace))
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
            report_unexpected_eof(fallback_span(state, -2));

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
    const auto first_pipe_token = try_consume(state, SyntaxKind::Pipe);
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
    const auto name = expect(state, SyntaxKind::Identifier, "type parameter");
    const auto colon_token = try_consume(state, SyntaxKind::Colon);
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