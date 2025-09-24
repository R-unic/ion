#pragma once
#include <string>

#include "ast/source_file.h"

struct FileLocation
{
    int position = 0;
    int line = 1;
    int column = 0;
    SourceFile file;
};

struct FileSpan
{
    FileLocation start;
    FileLocation end;
};

std::string format_location(const FileLocation&, bool = true);
FileLocation get_start_location(const SourceFile&);
FileSpan create_span(const FileLocation&, const FileLocation&);