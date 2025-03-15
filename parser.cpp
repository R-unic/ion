#include "parser.h"

expression parser::parse()
{
    return parse_expression();
}

expression parser::parse_expression()
{
    return parse_primary();
}

expression parser::parse_primary()
{
    if (tokens.check_set(std::set({
        syntax_kind::int_literal,
        syntax_kind::float_literal,
        syntax_kind::string_literal,
        syntax_kind::bool_literal,
        syntax_kind::null_literal
    })))
    {
        const token token = tokens.advance();
        return literal(token);
    }

    throw std::runtime_error(std::format("unexpected token {}", tokens.current().kind));
}