#include "lexer.h"

std::string format_location(const FileLocation& location, const bool include_file_path)
{
    auto numeric_location = std::to_string(location.line) + ':' + std::to_string(location.column);
    if (include_file_path)
        return location.file->path + ':' + numeric_location;
    
    return numeric_location;
}

FileLocation get_start_location(const SourceFile* file)
{
    return FileLocation { .file = file };
}

FileSpan create_span(const FileLocation& start, const FileLocation& end)
{
    return FileSpan { .start = start, .end = end };
}

std::string get_text(const Token& token)
{
    if (token.text.has_value())
        return *token.text;

    if (keyword_lexemes.contains(token.kind))
        return keyword_lexemes.at(token.kind);

    if (single_char_lexemes.contains(token.kind))
        return { 1, single_char_lexemes.at(token.kind) };
    
    const auto start_position = token.span.start.position;
    return token.span.start.file->text.substr(start_position, token.span.end.position - start_position);
}

std::string format_token(const Token& token)
{
    return get_text(token) + " (" + std::to_string(static_cast<int>(token.kind)) + ')';
}