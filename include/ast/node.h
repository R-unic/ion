#pragma once
#include <variant>

#include "source_file.h"

using expression_t = std::variant<>;
using statement_t = std::variant<SourceFile>;
using node_t = std::variant<expression_t, statement_t>;