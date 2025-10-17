#pragma once
#include <functional>

#include "visitor.h"
#include "ion/logger.h"

class AstViewer final : public AstVisitor<void>
{
    unsigned int indent_ = 0;

    static void write(const std::string&);
    static void write(const char*);
    void write_indent() const;
    void write_line() const;
    void write_line(const std::string&) const;
    void write_line(const char*) const;

public:
    AstViewer()
    {
        logger::info("Created AstViewer");
    }

    void visit_statements(const std::vector<statement_ptr_t>& statements) override
    {
        size_t i = 0;
        for (const auto& statement : statements)
        {
            visit(statement);
            if (i++ < statements.size())
                write_line();
        }
    }

    void write_binary_op_contents(const BinaryOp&);
    void write_closing_paren();
    template <typename T>
    void write_list(const std::vector<T>&, const std::function<void (const T&)>&);
    void write_type_list_clause(const std::optional<TypeListClause*>&);

    void visit_primitive_literal(PrimitiveLiteral&) override;
    void visit_array_literal(ArrayLiteral&) override;
    void visit_tuple_literal(TupleLiteral&) override;
    void visit_range_literal(RangeLiteral&) override;
    void visit_rgb_literal(RgbLiteral&) override;
    void visit_hsv_literal(HsvLiteral&) override;
    void visit_vector_literal(VectorLiteral&) override;
    void visit_interpolated_string(InterpolatedString&) override;
    void visit_identifier(Identifier&) override;
    void visit_parenthesized(Parenthesized&) override;
    void visit_binary_op(BinaryOp&) override;
    void visit_unary_op(UnaryOp&) override;
    void visit_postfix_unary_op(PostfixUnaryOp&) override;
    void visit_assignment_op(AssignmentOp&) override;
    void visit_ternary_op(TernaryOp&) override;
    void visit_invocation(Invocation&) override;
    void visit_type_of(TypeOf&) override;
    void visit_name_of(NameOf&) override;
    void visit_await(Await&) override;
    void visit_member_access(MemberAccess&) override;
    void visit_optional_member_access(OptionalMemberAccess&) override;
    void visit_element_access(ElementAccess&) override;

    void visit_expression_statement(ExpressionStatement&) override;
    void visit_block(Block&) override;
    void visit_type_declaration(TypeDeclaration&) override;
    void visit_variable_declaration(VariableDeclaration&) override;
    void visit_event_declaration(EventDeclaration&) override;
    void visit_interface_declaration(InterfaceDeclaration&) override;
    void visit_interface_field(InterfaceField&) override;
    void visit_interface_method(InterfaceMethod&) override;
    void visit_enum_declaration(EnumDeclaration&) override;
    void visit_enum_member(EnumMember&) override;
    void visit_function_declaration(FunctionDeclaration&) override;
    void visit_parameter(Parameter&) override;
    void visit_instance_constructor(InstanceConstructor&) override;
    void visit_instance_property_declarator(InstancePropertyDeclarator&) override;
    void visit_instance_name_declarator(InstanceNameDeclarator&) override;
    void visit_instance_attribute_declarator(InstanceAttributeDeclarator&) override;
    void visit_instance_tag_declarator(InstanceTagDeclarator&) override;
    void visit_break(Break&) override;
    void visit_continue(Continue&) override;
    void visit_return(Return&) override;
    void visit_if(If&) override;
    void visit_while(While&) override;
    void visit_repeat(Repeat&) override;
    void visit_for(For&) override;
    void visit_after(After&) override;
    void visit_every(Every&) override;
    void visit_match(Match&) override;
    void visit_match_case(MatchCase&) override;
    void visit_match_else_case(MatchElseCase&) override;
    void visit_import(Import&) override;
    void visit_export(Export&) override;
    void visit_decorator(Decorator&) override;

    void visit_primitive_type(PrimitiveTypeRef&) override;
    void visit_literal_type(LiteralTypeRef&) override;
    void visit_type_name(TypeNameRef&) override;
    void visit_nullable_type(NullableTypeRef&) override;
    void visit_array_type(ArrayTypeRef&) override;
    void visit_tuple_type(TupleTypeRef&) override;
    void visit_function_type(FunctionTypeRef&) override;
    void visit_union_type(UnionTypeRef&) override;
    void visit_intersection_type(IntersectionTypeRef&) override;
    void visit_type_parameter(TypeParameterRef&) override;
};