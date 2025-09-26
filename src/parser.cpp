#include <iostream>
#include <memory>
#include <algorithm>

#include "diagnostics.h"
#include "parser.h"

static bool is_eof(const ParseState& state, const int offset = 0)
{
    const auto offset_position = state.position + offset;
    return offset_position < 0 || static_cast<int>(state.tokens.size()) <= offset_position;
}

static std::optional<Token> maybe_peek(const ParseState& state, const int offset)
{
    if (is_eof(state, offset))
        return std::nullopt;
    
    return state.tokens.at(state.position + offset);
}

static Token peek(const ParseState& state, const int offset)
{
    if (const auto token = maybe_peek(state, offset); token.has_value())
        return *token;
    
    report_compiler_error("Parser attempted to access an out of bounds token index");
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
    auto token = current_token(state);
    state.position++;
    
    return token;
}

static bool check(const ParseState& state, const SyntaxKind kind, const int offset = 0)
{
    const auto token = maybe_peek(state, offset);
    return token.has_value() && token.value().kind == kind;
}

static bool match(ParseState& state, const SyntaxKind kind)
{
    const auto is_match = check(state, kind);
    if (is_match)
        advance(state);
    
    return is_match;
}

static bool match_any(ParseState& state, const std::vector<SyntaxKind>& characters)
{
    const auto is_match = !is_eof(state) && std::ranges::any_of(characters, [&](const auto syntax)
    {
        return check(state, syntax);
    });
    
    if (is_match)
        advance(state);
    
    return is_match;
}

static bool is_assignment_target(const expression_ptr_t& expression)
{
    // TODO: implement
    return true;
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

static Token consume(ParseState& state, const SyntaxKind kind, const std::string& custom_expected = "")
{
    const auto token = current_token(state);
    const auto span = get_current_optional_span(state);
    advance(state);
    
    if (token.has_value() && token.value().kind == kind)
        return *token;

    const auto quote_expected = kind != SyntaxKind::Identifier;
    const auto expected = !quote_expected ? "identifier" : std::to_string(static_cast<int>(kind));
    const auto got = token.has_value() ? get_text(*token) : "EOF";
    report_expected_different_syntax(span, custom_expected.empty() ? expected : custom_expected, got, quote_expected);
}

static double to_number(const std::string& s) {
    if (s.starts_with("0x") || s.starts_with("0X"))
        return static_cast<double>(std::stoll(s, nullptr, 16));
    if (s.starts_with("0o") || s.starts_with("0O"))
        return static_cast<double>(std::stoll(s.substr(2), nullptr, 8));
    if (s.starts_with("0b") || s.starts_with("0B"))
        return static_cast<double>(std::stoll(s.substr(2), nullptr, 2));
        
    return std::stod(s);
}

static expression_ptr_t parse_primary(ParseState& state)
{
    const auto span = get_current_optional_span(state);
    const auto token_opt = advance(state);
    if (!token_opt.has_value())
    {
        const auto last_token = previous_token(state);
        report_unexpected_eof(span);
    }
    
    const auto& token = token_opt.value();
    const auto text = get_text(token);
    switch (token.kind)
    {
    case SyntaxKind::Identifier:
        return Identifier::create(text);
        
    case SyntaxKind::TrueKeyword:
        return Literal::create(true);
    case SyntaxKind::FalseKeyword:
        return Literal::create(false);
    case SyntaxKind::NullKeyword:
        return Literal::create(std::nullopt);
    case SyntaxKind::StringLiteral:
        return Literal::create(text.substr(1, text.size() - 2));
    case SyntaxKind::NumberLiteral:
        return Literal::create(to_number(text));
        
    default:
        report_unexpected_syntax(token);
    }
}

static expression_ptr_t parse_invocation(ParseState& state, expression_ptr_t callee, const std::optional<Token>& bang_token)
{
    const auto l_paren = *previous_token(state);
    const auto arguments = new std::vector<expression_ptr_t>;
    if (!check(state, SyntaxKind::RParen))
    {
        do {
            arguments->push_back(parse_expression(state));
        } while (match(state, SyntaxKind::Comma));
    }

    const auto r_paren = consume(state, SyntaxKind::RParen);
    return Invocation::create(l_paren, r_paren, std::move(callee), bang_token, arguments);
}

static expression_ptr_t parse_member_access(ParseState& state, expression_ptr_t expression, const Token& dot_token)
{
    const auto name = consume(state, SyntaxKind::Identifier);
    return MemberAccess::create(dot_token, std::move(expression), name);
}

static expression_ptr_t parse_postfix(ParseState& state)
{
    auto expression = parse_primary(state);
    while (true)
    {
        if ((check(state, SyntaxKind::Bang) && check(state, SyntaxKind::LParen, 1)) || check(state, SyntaxKind::LParen))
        {
            const auto is_special = match(state, SyntaxKind::Bang);
            const auto bang_token = is_special ? previous_token(state) : std::nullopt;
            consume(state, SyntaxKind::LParen);
            
            expression = parse_invocation(state, std::move(expression), bang_token);
        }
        else if (match(state, SyntaxKind::Dot))
        {
            const auto dot_token = *previous_token(state);
            expression = parse_member_access(state, std::move(expression), dot_token);
        }
        else
            break;
    }
    
    return expression;
}

const std::vector unary_syntaxes = {SyntaxKind::Bang, SyntaxKind::Tilde, SyntaxKind::Minus};
static expression_ptr_t parse_unary(ParseState& state)
{
    while (match_any(state, unary_syntaxes))
    {
        const auto operator_token = *previous_token(state);
        auto operand = parse_unary(state);
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

const std::vector multiplicative_syntaxes = {SyntaxKind::Star, SyntaxKind::Slash, SyntaxKind::Percent};
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

const std::vector additive_syntaxes = {SyntaxKind::Plus, SyntaxKind::Minus};
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

const std::vector bit_shift_syntaxes = {SyntaxKind::LArrowLArrow, SyntaxKind::RArrowRArrow, SyntaxKind::RArrowRArrowRArrow};
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

static expression_ptr_t parse_bitwise_or(ParseState& state)
{
    auto left = parse_bitwise_and(state);
    while (match(state, SyntaxKind::Pipe))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_bitwise_and(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

static expression_ptr_t parse_bitwise_xor(ParseState& state)
{
    auto left = parse_bitwise_or(state);
    while (match(state, SyntaxKind::Tilde))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_bitwise_or(state);
        left = BinaryOp::create(operator_token, std::move(left), std::move(right));
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
    auto left = parse_bitwise_xor(state);
    while (match_any(state, comparison_syntaxes))
    {
        const auto operator_token = *previous_token(state);
        auto right = parse_bitwise_xor(state);
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
        if (!is_assignment_target(left))
        {
            // TODO: error
        }
        
        const auto operator_token = *previous_token(state);
        auto right = parse_assignment(state);
        left = AssignmentOp::create(operator_token, std::move(left), std::move(right));
    }

    return left;
}

expression_ptr_t parse_expression(ParseState& state)
{
    return parse_assignment(state);
}

static statement_ptr_t parse_variable_declaration(ParseState& state)
{
    const auto keyword = *previous_token(state);
    const auto name = consume(state, SyntaxKind::Identifier);
    std::optional<Token> equals_token = std::nullopt;
    std::optional<expression_ptr_t> initializer = std::nullopt;
    if (match(state, SyntaxKind::Equals))
    {
        equals_token = *previous_token(state);
        initializer = parse_expression(state);
    }

    return VariableDeclaration::create(keyword, name, std::move(equals_token), std::move(initializer));
}

static statement_ptr_t parse_if(ParseState& state)
{
    const auto if_keyword = *previous_token(state);
    auto condition = parse_expression(state);
    auto then_branch = parse_statement(state);
    std::optional<Token> else_keyword = std::nullopt;
    std::optional<statement_ptr_t> else_branch = std::nullopt;
    if (match(state, SyntaxKind::ElseKeyword))
    {
        else_keyword = *previous_token(state);
        else_branch = parse_statement(state);
    }

    return If::create(if_keyword, std::move(condition), std::move(then_branch), else_keyword, std::move(else_branch));
}

static statement_ptr_t parse_while(ParseState& state)
{
    const auto keyword = *previous_token(state);
    auto condition = parse_expression(state);
    auto body = parse_statement(state);
   
    return While::create(keyword, std::move(condition), std::move(body));
}

static statement_ptr_t parse_import(ParseState& state)
{
    const auto import_keyword = *previous_token(state);
    std::vector<Token> names = {};
    while (match(state, SyntaxKind::Identifier))
    {
        const auto name = *previous_token(state);
        names.push_back(name);
        match(state, SyntaxKind::Comma);
    }

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
            report_expected_different_syntax(import_keyword.span, "import name", get_text(token), false);
        }
    }
    
    const auto from_keyword = consume(state, SyntaxKind::FromKeyword);
    const auto module_name = consume(state, SyntaxKind::Identifier, "module name");
    return Import::create(import_keyword, names, from_keyword, module_name);
}

statement_ptr_t parse_statement(ParseState& state)
{
    if (match(state, SyntaxKind::LetKeyword))
        return parse_variable_declaration(state);
    if (match(state, SyntaxKind::IfKeyword))
        return parse_if(state);
    if (match(state, SyntaxKind::WhileKeyword))
        return parse_while(state);
    if (match(state, SyntaxKind::ImportKeyword))
        return parse_import(state);

    auto expression = parse_expression(state);
    return ExpressionStatement::create(std::move(expression));
}

std::vector<statement_ptr_t>* parse(const SourceFile* file)
{
    const auto tokens = tokenize(file);
    auto state = ParseState { .file = file, .tokens = tokens };
    while (!is_eof(state))
        file->statements->push_back(parse_statement(state));
    
    return file->statements;
}