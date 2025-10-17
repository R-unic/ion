#pragma once
#include <memory>
#include <vector>
#include <set>

#include "common.h"
#include "expression_parser.h"
#include "declaration_parser.h"
#include "statement_parser.h"
#include "type_parser.h"
#include "ion/logger.h"
#include "ion/ast/node.h"
#include "ion/ast/ast.h"

const std::set<std::string> primitive_type_names = { "number", "string", "bool", "void" };
const std::set primitive_literal_syntaxes = {
    SyntaxKind::TrueKeyword, SyntaxKind::FalseKeyword, SyntaxKind::NumberLiteral, SyntaxKind::StringLiteral
};

const std::set type_argument_syntaxes = {
    SyntaxKind::Identifier,
    SyntaxKind::LArrow,
    SyntaxKind::RArrow, SyntaxKind::RArrowRArrow, SyntaxKind::RArrowRArrowRArrow,
    SyntaxKind::Comma
};

const std::set member_access_syntaxes = { SyntaxKind::Dot, SyntaxKind::ColonColon, SyntaxKind::At };
const std::set postfix_op_syntaxes = { SyntaxKind::PlusPlus, SyntaxKind::MinusMinus };
const std::set unary_syntaxes = { SyntaxKind::Bang, SyntaxKind::Tilde, SyntaxKind::Minus };
const std::set multiplicative_syntaxes = { SyntaxKind::Star, SyntaxKind::Slash, SyntaxKind::SlashSlash, SyntaxKind::Percent };
const std::set additive_syntaxes = { SyntaxKind::Plus, SyntaxKind::Minus };
const std::set bit_shift_syntaxes = { SyntaxKind::LArrowLArrow, SyntaxKind::RArrowRArrow, SyntaxKind::RArrowRArrowRArrow };
const std::set comparison_syntaxes = {
    SyntaxKind::EqualsEquals,
    SyntaxKind::BangEquals,
    SyntaxKind::LArrow,
    SyntaxKind::LArrowEquals,
    SyntaxKind::RArrow,
    SyntaxKind::RArrowEquals
};

const std::set assignment_syntaxes = {
    SyntaxKind::Equals,
    SyntaxKind::PlusEquals,
    SyntaxKind::MinusEquals,
    SyntaxKind::StarEquals,
    SyntaxKind::SlashEquals,
    SyntaxKind::SlashSlashEquals,
    SyntaxKind::CaretEquals,
    SyntaxKind::PercentEquals,
    SyntaxKind::AmpersandEquals,
    SyntaxKind::PipeEquals,
    SyntaxKind::TildeEquals,
    SyntaxKind::LArrowLArrowEquals,
    SyntaxKind::RArrowRArrowEquals,
    SyntaxKind::RArrowRArrowRArrowEquals,
    SyntaxKind::AmpersandAmpersandEquals,
    SyntaxKind::PipePipeEquals
};

void parse(SourceFile&);