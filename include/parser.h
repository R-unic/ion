#pragma once
#include <memory>
#include <vector>

#include "lexer.h"
#include "source_file.h"
#include "ast/node.h"
#include "ast/ast.h"

struct ParseState
{
    int position = 0;
    const SourceFile* file;
    std::vector<Token> tokens;
};

static expression_ptr_t parse_expression(ParseState& state);
static statement_ptr_t parse_statement(ParseState& state);
static type_ref_ptr_t parse_type(ParseState& state);
std::vector<statement_ptr_t>* parse(const SourceFile*);