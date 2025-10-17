#pragma once
#include "common.h"

BracedStatementList* parse_braced_statement_list(ParseState&, const std::function<statement_ptr_t (ParseState&)>&, bool = true);
BracedStatementList* parse_braced_statement_list_comma_separated(ParseState&, const std::function<statement_ptr_t (ParseState&)>&);
ParameterListClause* parse_parameter_list(ParseState&);
std::vector<statement_ptr_t> parse_decorator_list(ParseState&);
statement_ptr_t parse_expression_statement(ParseState&);
statement_ptr_t parse_block(ParseState&);
statement_ptr_t parse_statement(ParseState&);
