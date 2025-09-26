#pragma once
#include <string>

struct SourceFile;

struct FileLocation
{
    int position = 0;
    int line = 1;
    int column = 0;
    const SourceFile* file;
};

struct FileSpan
{
    FileLocation start;
    FileLocation end;
};

FileLocation get_start_location(const SourceFile*);
FileSpan create_span(const FileLocation&, const FileLocation&);