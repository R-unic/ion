#pragma once
#include <optional>
#include "syntax_kind.h"
#include "../bi_map.h"

inline bi_map<std::string, syntax_kind> keyword_map = {};

void initialize_syntax_facts();
std::optional<syntax_kind> get_keyword_kind(const std::string& keyword);
std::optional<std::string> get_keyword_lexeme(syntax_kind kind);