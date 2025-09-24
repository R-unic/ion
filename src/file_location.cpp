#include "file_location.h"

std::string format_location(const FileLocation& location, const bool include_file_path)
{
    auto numeric_location = std::to_string(location.line) + ':' + std::to_string(location.column);
    if (include_file_path)
        return location.file.path + ':' + numeric_location;
    
    return numeric_location;
}

FileLocation get_start_location(const SourceFile& file)
{
    return FileLocation { .file = file };
}

FileSpan create_span(const FileLocation& start, const FileLocation& end)
{
    return FileSpan { .start = start, .end = end };
}