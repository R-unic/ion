#pragma once

struct SourceFile
{
    std::string path;
    std::string text;
};

SourceFile create_file(const std::string& path);