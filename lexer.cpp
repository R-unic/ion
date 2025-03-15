#include <stdexcept>
#include "lexer.h"

#include <iostream>

#include "syntax/syntax_facts.h"

lexer::lexer(const source_file& file)
    : source(file.source), lexeme_start_location(location::empty(file.path))
{
    initialize_syntax_facts();
}

token_stream lexer::tokenize()
{
    while (!is_eof())
        lex();

    return token_stream(tokens);
}

void lexer::lex()
{
    const char character = current_char();
    lexeme_start_location = current_location();
    if (character == '\n')
        advance_new_line();
    else
        advance();

    switch (character)
    {
    case '+':
        {
            auto kind = syntax_kind::plus;
            if (match_char('='))
                kind = syntax_kind::plus_equals;

            return push_token(kind);
        }
    case '-':
        {
            auto kind = syntax_kind::minus;
            if (match_char('='))
                kind = syntax_kind::minus_equals;

            return push_token(kind);
        }
    case '*':
        {
            auto kind = syntax_kind::star;
            if (match_char('='))
                kind = syntax_kind::star_equals;

            return push_token(kind);
        }
    case '/':
        {
            auto kind = syntax_kind::slash;
            if (match_char('/'))
            {
                kind = syntax_kind::slash_slash;
                if (match_char('='))
                    kind = syntax_kind::slash_slash_equals;
            }
            else if (match_char('='))
                kind = syntax_kind::slash_equals;

            return push_token(kind);
        }
    case '^':
        {
            auto kind = syntax_kind::star;
            if (match_char('='))
                kind = syntax_kind::star_equals;

            return push_token(kind);
        }
    case '%':
        {
            auto kind = syntax_kind::percent;
            if (match_char('='))
                kind = syntax_kind::percent_equals;

            return push_token(kind);
        }
    case '~':
        {
            auto kind = syntax_kind::tilde;
            if (match_char('='))
                kind = syntax_kind::tilde_equals;

            return push_token(kind);
        }
    case '&':
        {
            auto kind = syntax_kind::ampersand;
            if (match_char('&'))
            {
                kind = syntax_kind::ampersand_ampersand;
                if (match_char('='))
                    kind = syntax_kind::ampersand_ampersand_equals;
            }
            else if (match_char('='))
                kind = syntax_kind::ampersand_equals;

            return push_token(kind);
        }
    case '|':
        {
            auto kind = syntax_kind::pipe;
            if (match_char('|'))
            {
                kind = syntax_kind::pipe_pipe;
                if (match_char('='))
                    kind = syntax_kind::pipe_pipe_equals;
            }
            else if (match_char('|'))
                kind = syntax_kind::ampersand_equals;

            return push_token(kind);
        }
    case '=':
        {
            auto kind = syntax_kind::equals;
            if (match_char('='))
                kind = syntax_kind::equals_equals;

            return push_token(kind);
        }
    case '!':
        {
            auto kind = syntax_kind::bang;
            if (match_char('='))
                kind = syntax_kind::bang_equals;

            return push_token(kind);
        }
    case '<':
        {
            auto kind = syntax_kind::lt;
            if (match_char('='))
                kind = syntax_kind::lte;

            return push_token(kind);
        }
    case '>':
        {
            auto kind = syntax_kind::gt;
            if (match_char('='))
                kind = syntax_kind::gte;

            return push_token(kind);
        }
    case ':': return push_token(syntax_kind::colon);
    case '(': return push_token(syntax_kind::l_paren);
    case ')': return push_token(syntax_kind::r_paren);
    case '{': return push_token(syntax_kind::l_brace);
    case '}': return push_token(syntax_kind::r_brace);
    case '[': return push_token(syntax_kind::l_bracket);
    case ']': return push_token(syntax_kind::r_bracket);

    case '\'':
    case '"': return read_string(character);

    default:
        {
            if (std::isspace(character))
                return skip_whitespace();

            if (std::isalpha(character))
                return read_identifier_or_keyword();

            if (std::isdigit(character) || character == '.')
                return read_number();

            throw std::runtime_error(std::format("Unexpected character {}", character));
        }
    }
}

void lexer::read_identifier_or_keyword()
{
    while (!is_eof() && (std::isalnum(current_char()) || current_char() == '_'))
        advance();

    const std::string lexeme = current_lexeme();
    if (lexeme == "true" || lexeme == "false")
        return push_token(syntax_kind::bool_literal, lexeme == "true");

    if (lexeme == "null")
        return push_token(syntax_kind::null_literal);

    const auto keyword_kind = get_keyword_kind(lexeme);
    const auto kind = keyword_kind.has_value() ? keyword_kind.value() : syntax_kind::identifier;
    push_token(kind);
}

void lexer::read_string(const char terminator)
{
    while (!is_eof() && current_char() != terminator)
        advance();

    if (const bool terminated = match_char(terminator); !terminated)
        throw std::runtime_error("Unterminated string literal");

    const std::string lexeme = current_lexeme();
    std::string value = lexeme.substr(1, lexeme.length() - 2);
    push_token(syntax_kind::string_literal, value);
}

void lexer::read_number()
{
    const auto malformed_message = "Malformed number literal";
    bool decimal_used = peek_char(-1) == '.';
    while (!is_eof() && (std::isdigit(current_char()) || current_char() == '.'))
    {
        const bool current_is_decimal = current_char() == '.';
        if (decimal_used && current_is_decimal)
            throw std::runtime_error(malformed_message);

        decimal_used |= current_is_decimal;
        advance();
    }

    const std::string lexeme = current_lexeme();
    if (lexeme == ".")
        throw std::runtime_error(malformed_message);

    const auto kind = decimal_used ? syntax_kind::float_literal : syntax_kind::int_literal;
    try
    {
        const double value = std::stod(lexeme);
        push_token(kind, value);
    }
    catch (const std::invalid_argument& e)
    {
        throw std::runtime_error("Invalid number (compiler error): " + std::string(e.what()));
    } catch (const std::out_of_range& e)
    {
        throw std::runtime_error("Number literal out of range: " + std::string(e.what()));
    }
}

static std::string escape_string(const std::string& input) {
    std::string output;
    for (const char c : input) {
        switch (c) {
        case '\"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\n': output += "\\n"; break;
        case '\t': output += "\\t"; break;
        default: output += c;
        }
    }
    return output;
}


void lexer::skip_whitespace()
{
    while (!is_eof() && std::isspace(current_char()))
    {
        if (const char current = current_char(); current == '\n')
            advance_new_line();
        else
            advance();
    }

    lexeme_start_location = current_location();
}

void lexer::push_token(syntax_kind kind, const token::value_t& value)
{
    tokens.emplace_back(kind, current_span(), current_lexeme(), value);
}

std::string lexer::current_lexeme() const
{
    const span span = current_span();
    return source.substr(span.start.position, span.end.position - span.start.position);
}

span lexer::current_span() const
{
    const location end_location = current_location();
    return {lexeme_start_location, end_location};
}

location lexer::current_location() const
{
    return {lexeme_start_location.file_name, line, column, position};
}

char lexer::current_char() const
{
    return peek_char(0);
}

char lexer::peek_char(const size_t offset) const
{
    if (is_eof(offset))
        return '\0';

    return source[position + offset];
}

bool lexer::match_char(const char expected)
{
    const bool is_match = !is_eof() && current_char() == expected;
    if (is_match)
        advance();

    return is_match;
}

void lexer::advance(const size_t amount)
{
    position += amount;
    column += amount;
}

void lexer::advance_new_line()
{
    position += 1;
    line += 1;
    column = 0;
}

bool lexer::is_eof(const size_t offset) const
{
    return position + offset >= source.length();
}
