#pragma once
#include <filesystem>
#include <string>

#include "ion/source_file.h"
#include "ion/parser.h"
#include "testing/runner.h"

#define SPEC_RUN
#define TEST_FILE "spec.ion"

inline SourceFile* load_data_file(const std::string& path)
{
    const std::filesystem::path file_path(__FILE__);
    const auto spec_folder_path = file_path.parent_path().string();

    return create_file(spec_folder_path + '/' + path);
}

inline SourceFile* fake_file(const std::string& text, std::vector<statement_ptr_t> statements = {})
{
    return new SourceFile(TEST_FILE, text, std::move(statements));
}