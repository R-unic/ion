#include "ion/parsing/parser.h"

ColonTypeClause* parse_colon_type_clause(ParseState& state)
{
    const auto colon_token = expect(state, SyntaxKind::Colon);
    auto type = parse_type(state);
    return new ColonTypeClause(colon_token, std::move(type));
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

type_ref_ptr_t parse_type_name(ParseState& state)
{
    const auto token = previous_token_guaranteed(state);
    if (primitive_type_names.contains(token.get_text()))
        return PrimitiveTypeRef::create(token);

    const auto type_arguments = parse_type_arguments(state);
    return TypeNameRef::create(token, type_arguments);
}

type_ref_ptr_t parse_literal_type(const ParseState& state)
{
    const auto token = previous_token_guaranteed(state);
    const auto value = primitive_from_string(token.get_text());
    return LiteralTypeRef::create(token, value);
}

type_ref_ptr_t parse_tuple_type(ParseState& state)
{
    const auto l_paren = previous_token_guaranteed(state);
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
        const auto pipe_token = previous_token_guaranteed(state);
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
        const auto ampersand_token = previous_token_guaranteed(state);
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
        const auto l_bracket = previous_token_guaranteed(state);
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
               ? NullableTypeRef::create(std::move(non_nullable_type), previous_token_guaranteed(state))
               : std::move(non_nullable_type);
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

type_ref_ptr_t parse_type(ParseState& state)
{
    return parse_nullable_type(state);
}

std::vector<type_ref_ptr_t> parse_type_list(ParseState& state,
                                            const std::function<type_ref_ptr_t (ParseState&)>& subparser)
{
    std::vector<type_ref_ptr_t> list;
    do
        list.push_back(subparser(state));
    while (match(state, SyntaxKind::Comma));

    return list;
}