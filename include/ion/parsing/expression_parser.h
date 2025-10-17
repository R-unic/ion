#pragma once
#include "common.h"

std::optional<EqualsValueClause*> parse_equals_value_clause(ParseState&);
expression_ptr_t parse_expression(ParseState&);