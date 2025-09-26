#pragma once
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "diagnostics.h"
#include "ast/node.h"

inline bool is_block(const statement_ptr_t& statement)
{
    using type_t = std::decay_t<decltype(statement)>;
    if constexpr (std::is_same_v<type_t, Block>)
        return true;
    
    return false;
}

inline bool is_assignment_target(const expression_ptr_t& expression)
{
    using type_t = std::decay_t<decltype(expression)>;
    if constexpr (std::is_same_v<type_t, Identifier> || std::is_same_v<type_t, MemberAccess> || std::is_same_v<type_t, Identifier>)
        return true;
    
    return false;
}

inline void assert_assignment_target(const expression_ptr_t& expression)
{
    if (is_assignment_target(expression)) return;
    report_invalid_assignment(expression->get_span(), expression->get_text());
}

inline std::string join_strings_by(const std::vector<std::string>& texts, const std::string& separator)
{
    std::ostringstream oss;
    bool first = true;
    for (auto& s : texts) {
        if (!first) oss << separator;
        oss << s;
        first = false;
    }

    return oss.str();
}

inline std::string join_by(const std::vector<expression_ptr_t>& expressions, const std::string& separator)
{
    std::vector<std::string> texts = {};
    texts.reserve(texts.size());
    for (const auto& statement : expressions)
        texts.push_back(statement->get_text());

    return join_strings_by(texts, separator);
}

inline std::string join_by(const std::vector<statement_ptr_t>& expressions, const std::string& separator)
{
    std::vector<std::string> texts = {};
    texts.reserve(texts.size());
    for (const auto& expression : expressions)
        texts.push_back(expression->get_text());

    return join_strings_by(texts, separator);
}

inline std::string join_tokens_by(const std::vector<Token>& tokens, const std::string& separator)
{
    std::vector<std::string> texts = {};
    texts.reserve(texts.size());
    for (const auto& token : tokens)
        texts.push_back(token.get_text());

    return join_strings_by(texts, separator);
}

template <typename K, typename V>
std::unordered_map<V, K> inverse_map(const std::unordered_map<K, V>& forward_map)
{
    std::unordered_map<V, K> map;
    for (auto& [lexeme, kind] : forward_map)
        map.emplace(kind, lexeme);
    
    return map;
}