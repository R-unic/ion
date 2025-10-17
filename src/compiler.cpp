#include <iostream>

#include "ion/compiler.h"
#include "ion/parsing/parser.h"
#include "ion/binder.h"
#include "ion/resolver.h"
#include "ion/type_solver.h"
#include "ion/ast/viewer.h"

void Compiler::emit()
{
    for (auto& file : files)
        pre_emit(file);
    for (auto& file : files)
        emit(file);
}

void Compiler::pre_emit(SourceFile& file)
{
    logger::info("Parsing file: " + file.path);
    parse(file);
    logger::info("Successfully parsed " + std::to_string(file.statements.size()) + " statements");

    const auto viewer = new AstViewer;
    logger::info("Running AstViewer on parsed statements...");
    viewer->visit_statements(file.statements);

    const auto resolver = new Resolver;
    resolver->visit_ast(file.statements);
    logger::info("Successfully resolved AST");

    const auto binder = new Binder;
    binder->visit_ast(file.statements);
    logger::info("Successfully bound AST");

    const auto type_solver = new TypeSolver;
    type_solver->visit_ast(file.statements);
    logger::info("Successfully solved types for AST");

    for (const auto& statement : file.statements)
        if (statement->symbol.has_value())
            std::cout << typeid(*statement).name() << ": " << statement->symbol.value()->to_string() << '\n';
}

void Compiler::emit(SourceFile& file)
{
    // TODO: transpilation
}

void compile_files(std::vector<SourceFile>& files)
{
    auto compiler = Compiler(std::move(files));
    compiler.emit();
}

void compile_file(const std::string& path)
{
    auto file = create_file(path);
    compile_file(file);
}

void compile_file(SourceFile& file)
{
    auto compiler = Compiler(std::move(file));
    compiler.emit();
}