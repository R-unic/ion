#include "ion/file_location.h"

FileLocation get_start_location(const SourceFile* file)
{
    return FileLocation { .file = file };
}

FileSpan create_span(const FileLocation& start, const FileLocation& end)
{
    return FileSpan { .start = start, .end = end };
}