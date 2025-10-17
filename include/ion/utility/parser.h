#pragma once
#include "ion/lexer.h"

struct ParseState
{
    int position = 0,
        stack_position = 0;

    const SourceFile* file;
    std::vector<Token> token_stream, token_stack;
};

inline bool is_eof(const ParseState& state, const int offset = 0)
{
    const auto stream_size = static_cast<int>(state.token_stream.size());
    const auto offset_position = state.position + offset;
    return offset_position < 0 || offset_position >= stream_size;
}

inline std::optional<Token> maybe_peek(const ParseState& state, const int offset)
{
    if (is_eof(state, offset))
        return std::nullopt;

    return state.token_stream.at(state.position + offset);
}

inline Token peek(const ParseState& state, const int offset)
{
    COMPILER_ASSERT(!is_eof(state, offset), "Parser attempted to access an out of bounds token index");
    return *maybe_peek(state, offset);
}

inline Token current_token_guaranteed(const ParseState& state)
{
    return peek(state, 0);
}

inline std::optional<Token> current_token(const ParseState& state)
{
    return maybe_peek(state, 0);
}

inline Token previous_token_guaranteed(const ParseState& state)
{
    return peek(state, -1);
}

inline std::optional<Token> previous_token(const ParseState& state)
{
    return maybe_peek(state, -1);
}

inline int position_with_default(const ParseState& state, const int position)
{
    return position == -1 ? state.stack_position : position;
}

inline bool stack_token_exists(const ParseState& state, const int position = -1)
{
    return !state.token_stack.empty() && position_with_default(state, position) < state.token_stack.size();
}

inline Token stack_token_at(const ParseState& state, const int position = -1)
{
    COMPILER_ASSERT(stack_token_exists(state, position),
                    "Stack position out of bounds indexing current stack token: " + std::to_string(position));

    return state.token_stack.at(position_with_default(state, position));
}

/** Returns the current token before incrementing the position */
inline std::optional<Token> advance(ParseState& state)
{
    if (stack_token_exists(state))
        return stack_token_at(state, state.stack_position++);

    const auto token = current_token(state);
    state.stack_position = 0;
    state.token_stack = {};
    state.position++;

    return token;
}

inline bool check_stack(const ParseState& state, const SyntaxKind kind)
{
    return stack_token_exists(state) && stack_token_at(state).is_kind(kind);
}

inline bool check(const ParseState& state, const SyntaxKind kind, const int offset = 0)
{
    const auto token_opt = maybe_peek(state, offset);
    if (!token_opt.has_value())
        return false;

    return token_opt.value().is_kind(kind);
}

inline bool check_any(const ParseState& state, const std::set<SyntaxKind>& syntaxes, const int offset = 0)
{
    return !is_eof(state) && std::ranges::any_of(syntaxes, [&](const auto syntax)
    {
        return check(state, syntax, offset);
    });
}

inline bool match(ParseState& state, const SyntaxKind kind)
{
    const auto is_match = check(state, kind);
    if (is_match)
        advance(state);

    return is_match;
}

inline bool match_any(ParseState& state, const std::set<SyntaxKind>& syntaxes)
{
    const auto is_match = check_any(state, syntaxes);
    if (is_match)
        advance(state);

    return is_match;
}

inline FileSpan empty_span(const ParseState& state)
{
    return create_span(get_start_location(*state.file), get_start_location(*state.file));
}

/**
 * Finds the last valid source span relative to the current parse position.
 *
 * - If a token exists at `position + offset`, returns its span.
 * - If we are before the beginning or after the end of the file, returns an empty span.
 * - Otherwise, walks backwards until a valid token is found.
 *
 * This is primarily used when reporting errors, so diagnostics can point to the closest meaningful location.
 */
inline FileSpan fallback_span(const ParseState& state, const int offset = 0)
{
    if (const auto token = maybe_peek(state, offset); token.has_value())
        return token.value().span;

    if (state.position + offset < 0)
        return empty_span(state);

    return is_eof(state, offset - 1)
               ? fallback_span(state, offset - 2)
               : fallback_span(state, offset - 1);
}

inline std::optional<Token> try_consume(ParseState& state, const SyntaxKind kind)
{
    if (match(state, kind))
        return previous_token(state);

    return std::nullopt;
}

inline Token expect(ParseState& state,
                    const SyntaxKind kind,
                    const std::string& custom_expected = "",
                    const std::optional<bool> custom_quote_expected = std::nullopt)
{
    const auto span = fallback_span(state);
    const auto token = advance(state);

    if (token.has_value() && token.value().is_kind(kind))
        return *token;

    const auto is_identifier = kind == SyntaxKind::Identifier;
    const auto quote_expected = custom_quote_expected.has_value() ? *custom_quote_expected : !is_identifier;
    const auto expected = !custom_expected.empty()
                              ? custom_expected
                              : is_identifier
                                    ? "identifier"
                                    : syntax_to_string(kind).value_or(std::to_string(static_cast<int>(kind)));

    const auto got = token.has_value() ? token->get_text() : "EOF";
    report_expected_different_syntax(span, expected, got, quote_expected);
}

inline Token expect_r_arrow(ParseState& state)
{
    if (check(state, SyntaxKind::RArrow) || check_stack(state, SyntaxKind::RArrow))
        return expect(state, SyntaxKind::RArrow);

    if (check(state, SyntaxKind::RArrowRArrow) || check_stack(state, SyntaxKind::RArrowRArrow))
    {
        const auto base = expect(state, SyntaxKind::RArrowRArrow);
        const auto arrows = base.split({ 2, SyntaxKind::RArrow });

        state.token_stack.push_back(arrows.back());
        return arrows.front();
    }

    if (check(state, SyntaxKind::RArrowRArrowRArrow))
    {
        const auto base = expect(state, SyntaxKind::RArrowRArrowRArrow);
        const auto arrows = base.split({ 3, SyntaxKind::RArrow });
        const auto& second_last = arrows.at(1);
        const auto& last = arrows.at(2);
        const auto new_span = create_span(second_last.span.start, last.span.end);

        state.token_stack.emplace_back(SyntaxKind::RArrowRArrow, new_span, ">>");
        return arrows.front();
    }

    return expect(state, SyntaxKind::RArrow);
}