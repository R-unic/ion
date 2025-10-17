#pragma once
#include "common.h"
#include "ion/ast/containers/type_list_clause.h"

ColonTypeClause* parse_colon_type_clause(ParseState&);
std::optional<TypeListClause*> parse_type_arguments(ParseState&);
std::optional<TypeListClause*> parse_type_parameters(ParseState&);
type_ref_ptr_t parse_type_name(ParseState&);
type_ref_ptr_t parse_type_parameter(ParseState&);
type_ref_ptr_t parse_type(ParseState&);
std::vector<type_ref_ptr_t> parse_type_list(ParseState&, const std::function<type_ref_ptr_t (ParseState&)>& = parse_type);
