#pragma once
#include "ion/ast/visitor.h"
#include "ion/logger.h"

class TypeSolver final : public AstVisitor<void>
{
public:
    TypeSolver()
    {
        logger::info("Created type solver");
    }

    static void bind_type(const SyntaxNode&, const type_ptr_t&);

    void visit_primitive_literal(PrimitiveLiteral&) override;
    void visit_array_literal(ArrayLiteral&) override;
    void visit_range_literal(RangeLiteral&) override;
    void visit_tuple_literal(TupleLiteral&) override;
    void visit_rgb_literal(RgbLiteral&) override;
    void visit_hsv_literal(HsvLiteral&) override;
    void visit_vector_literal(VectorLiteral&) override;
    void visit_identifier(Identifier&) override;

    void visit_expression_statement(ExpressionStatement&) override;
    void visit_variable_declaration(VariableDeclaration&) override;
};