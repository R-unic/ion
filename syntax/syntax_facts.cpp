#include "syntax_facts.h"

void initialize_syntax_facts()
{
    keyword_map.insert("let", syntax_kind::let_keyword);
    keyword_map.insert("fn", syntax_kind::fn_keyword);
}

std::optional<syntax_kind> get_keyword_kind(const std::string& keyword)
{
    if (!keyword_map.has_value(keyword))
        return std::nullopt;
    
    return keyword_map.get_value(keyword);
}

std::optional<std::string> get_keyword_lexeme(const syntax_kind kind)
{
    if (!keyword_map.has_key(kind))
        return std::nullopt;
    
    return keyword_map.get_key(kind);
}