#include "ion/parsing/parser.h"

static statement_ptr_t parse_variable_declaration(ParseState& state)
{
    const auto let_keyword = previous_token_guaranteed(state);
    const auto const_keyword = try_consume(state, SyntaxKind::ConstKeyword);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_optional(state, SyntaxKind::Colon, parse_colon_type_clause);
    const auto equals_value_clause = parse_equals_value_clause(state);

    return VariableDeclaration::create(let_keyword, const_keyword, name, colon_type_clause, equals_value_clause);
}

static statement_ptr_t parse_type_declaration(ParseState& state)
{
    const auto type_keyword = previous_token_guaranteed(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto type_parameters = parse_type_parameters(state);
    const auto equals_token = expect(state, SyntaxKind::Equals);
    auto type = parse_type(state);

    return TypeDeclaration::create(type_keyword, name, type_parameters, equals_token, std::move(type));
}

static statement_ptr_t parse_event_declaration(ParseState& state)
{
    const auto event_keyword = previous_token_guaranteed(state);
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
    const auto keyword = previous_token_guaranteed(state);
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
        consume_semicolons(state);

        return InterfaceMethod::create(*fn_keyword, name, type_parameters, l_paren, std::move(parameter_types), r_paren,
                                       colon_token, std::move(return_type));
    }

    const auto const_keyword = try_consume(state, SyntaxKind::ConstKeyword);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_token = expect(state, SyntaxKind::Colon);
    auto type = parse_type(state);
    consume_semicolons(state);

    return InterfaceField::create(const_keyword, name, colon_token, std::move(type));
}

static statement_ptr_t parse_interface_declaration(ParseState& state)
{
    const auto keyword = previous_token_guaranteed(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto braced_statement_list = parse_braced_statement_list(state, parse_interface_member);

    return InterfaceDeclaration::create(keyword, name, braced_statement_list);
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

static statement_ptr_t parse_function_declaration(ParseState& state, const std::optional<Token>& async_keyword,
                                                  std::vector<statement_ptr_t> decorator_list)
{
    const auto fn_keyword = previous_token_guaranteed(state);
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
    const auto instance_keyword = previous_token_guaranteed(state);
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_colon_type_clause(state);
    const auto clone_keyword = try_consume(state, SyntaxKind::CloneKeyword);
    std::optional<expression_ptr_t> clone_target = std::nullopt;
    if (clone_keyword.has_value())
        clone_target = parse_expression(state);

    const auto declarators = parse_optional(state, SyntaxKind::LBrace, [&](ParseState&)
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

statement_ptr_t parse_declaration(ParseState& state)
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

        report_invalid_decorator_target(current_token(state).value_or(previous_token_guaranteed(state)));
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