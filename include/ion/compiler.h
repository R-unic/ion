#pragma once
#include <vector>

#include "source_file.h"

struct Compiler
{
    std::vector<SourceFile> files;

    explicit Compiler(std::vector<SourceFile> files)
        : files(std::move(files))
    {
    }

    explicit Compiler(SourceFile file)
    {
        files.push_back(std::move(file));
    }

    void emit();

private:
    static void pre_emit(SourceFile&);
    static void emit(SourceFile&);
};

void compile_files(std::vector<SourceFile>&);
void compile_file(const std::string&);
void compile_file(SourceFile&);