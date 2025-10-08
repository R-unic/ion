#include <algorithm>
#include <utility>

#include "ion/logger.h"
#include "ion/diagnostics.h"
#include "ion/source_file.h"
#include "ion/lexer.h"
#include "ion/parsing/parser.h"

std::vector<statement_ptr_t> parse(SourceFile* file)
{
    const auto tokens = tokenize(file);
    logger::info("Parsing file: " + file->path);
    auto state = ParseState { .file = file, .token_stream = tokens };
    while (!is_eof(state))
        file->statements.push_back(parse_statement(state));

    logger::info("Checking for unreachable code at module level");
    check_for_unreachable_code(file->statements);
    logger::info("Parsed " + std::to_string(file->statements.size()) + " statements");
    return std::move(file->statements);
}