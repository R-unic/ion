#pragma once
#include "common.h"

static std::vector<Token> get_file_tokens(const std::string& path)
{
    return tokenize(load_data_file(path));
}

static std::vector<Token> get_tokens(const std::string& text)
{
    return tokenize(fake_file(text));
}

using syntax_pair_t = std::pair<const std::string, const SyntaxKind>;

THEORY(
    (
        syntax_pair_t("+", SyntaxKind::Plus),
        syntax_pair_t("-", SyntaxKind::Minus),
        syntax_pair_t("*", SyntaxKind::Star),
        syntax_pair_t("/", SyntaxKind::Slash),
        syntax_pair_t("%", SyntaxKind::Percent),
        syntax_pair_t("^", SyntaxKind::Carat)
    ),
    lexes_basic_arithmetic_ops
)(const syntax_pair_t& pair)
{
    const auto token = get_tokens(pair.first).front();
    // TODO: assertion
}