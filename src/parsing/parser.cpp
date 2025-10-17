#include "ion/logger.h"
#include "ion/source_file.h"
#include "ion/lexer.h"
#include "ion/parsing/parser.h"

void parse(SourceFile& file)
{
    const auto tokens = tokenize(file);
    auto state = ParseState { .file = &file, .token_stream = tokens };
    while (!is_eof(state))
        file.statements.push_back(parse_statement(state));

    logger::info("Checking for unreachable code at module level");
    check_for_unreachable_code(file.statements);
}