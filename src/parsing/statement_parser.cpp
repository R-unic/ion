#include "ion/parsing/parser.h"

BracedStatementList* parse_braced_statement_list(ParseState& state,
                                                 const std::function<statement_ptr_t (ParseState&)>& parse_list_statement,
                                                 const bool comma_allowed)
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

BracedStatementList* parse_braced_statement_list_comma_separated(ParseState& state,
                                                                 const std::function<statement_ptr_t (ParseState&)>& parse_list_statement)
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

statement_ptr_t parse_block(ParseState& state)
{
    const auto braced_statement_list = parse_braced_statement_list(state, parse_statement, false);
    logger::info("Checking for unreachable code at block level");
    check_for_unreachable_code(braced_statement_list->statements);

    return Block::create(braced_statement_list);
}

static statement_ptr_t parse_parameter(ParseState& state)
{
    const auto name = expect(state, SyntaxKind::Identifier);
    const auto colon_type_clause = parse_optional(state, SyntaxKind::Colon, parse_colon_type_clause);
    const auto equals_value_clause = parse_equals_value_clause(state);

    return Parameter::create(name, colon_type_clause, equals_value_clause);
}

ParameterListClause* parse_parameter_list(ParseState& state)
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
    const auto at_token = previous_token_guaranteed(state);
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

std::vector<statement_ptr_t> parse_decorator_list(ParseState& state)
{
    std::vector<statement_ptr_t> list;
    while (match(state, SyntaxKind::At))
        list.push_back(parse_decorator(state));

    return list;
}

static statement_ptr_t parse_if(ParseState& state)
{
    const auto if_keyword = previous_token_guaranteed(state);
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
    const auto keyword = previous_token_guaranteed(state);
    auto condition = parse_expression(state);
    auto statement = parse_statement(state);
    check_for_ambiguous_equals(condition);

    return While::create(keyword, std::move(condition), std::move(statement));
}

static statement_ptr_t parse_repeat(ParseState& state)
{
    const auto repeat_keyword = previous_token_guaranteed(state);
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
    const auto keyword = previous_token_guaranteed(state);
    const auto names = parse_name_list(state);
    const auto colon_token = expect(state, SyntaxKind::Colon);
    auto iterable = parse_expression(state);
    auto statement = parse_statement(state);

    return For::create(keyword, names, colon_token, std::move(iterable), std::move(statement));
}

static statement_ptr_t parse_after(ParseState& state)
{
    const auto keyword = previous_token_guaranteed(state);
    auto time_expression = parse_expression(state);
    auto statement = parse_statement(state);

    return After::create(keyword, std::move(time_expression), std::move(statement));
}

static statement_ptr_t parse_every(ParseState& state)
{
    const auto keyword = previous_token_guaranteed(state);
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
    const auto keyword = previous_token_guaranteed(state);
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
    const auto keyword = previous_token_guaranteed(state);
    auto expression = parse_expression(state);
    const auto cases = parse_braced_statement_list_comma_separated(state, [&](const auto& _)
    {
        return parse_match_case(state, parse_match_case_block);
    });

    return Match::create(keyword, std::move(expression), cases);
}

static statement_ptr_t parse_import(ParseState& state)
{
    const auto import_keyword = previous_token_guaranteed(state);
    auto names = parse_name_list(state);

    if (names.empty())
    {
        if (match(state, SyntaxKind::Star))
        {
            const auto star_token = previous_token_guaranteed(state);
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
    const auto keyword = previous_token_guaranteed(state);
    std::optional<expression_ptr_t> expression = std::nullopt;

    const auto has_line_break_between = keyword.span.has_line_break_between(current_token_guaranteed(state));
    if (!is_eof(state) && !check(state, SyntaxKind::Semicolon) && !has_line_break_between)
        expression = parse_expression(state);

    return Return::create(keyword, std::move(expression));
}

statement_ptr_t parse_expression_statement(ParseState& state)
{
    auto expression = parse_expression(state);
    return ExpressionStatement::create(std::move(expression));
}

static statement_ptr_t parse_break(const ParseState& state)
{
    const auto keyword = previous_token_guaranteed(state);
    return Break::create(keyword);
}

static statement_ptr_t parse_continue(const ParseState& state)
{
    const auto keyword = previous_token_guaranteed(state);
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