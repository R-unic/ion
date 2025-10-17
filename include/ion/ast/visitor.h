#pragma once
#include <functional>
#include <stack>

#include "visitor_fwd.h"
#include "ast.h"

template <typename R>
struct AstVisitor : ExpressionVisitor<R>, StatementVisitor<R>, TypeRefVisitor<R>
{
    virtual void visit_ast(const std::vector<statement_ptr_t>& statements)
    {
        visit_statements(statements);
    }

    virtual void visit_statements(const std::vector<statement_ptr_t>& statements)
    {
        for (const auto& statement : statements)
            visit(statement);
    }

    virtual void visit_expressions(const std::vector<expression_ptr_t>& expressions)
    {
        for (const auto& expression : expressions)
            visit(expression);
    }

    virtual void visit_type_refs(const std::vector<type_ref_ptr_t>& type_refs)
    {
        for (const auto& type_ref : type_refs)
            visit(type_ref);
    }

    virtual void visit(const expression_ptr_t& expression)
    {
        expression->accept(*this);
    }

    virtual void visit(const statement_ptr_t& statement)
    {
        statement->accept(*this);
    }

    virtual void visit(const type_ref_ptr_t& type_ref)
    {
        type_ref->accept(*this);
    }

    void visit_type_list_clause(const std::optional<TypeListClause*> type_list)
    {
        if (type_list.has_value())
            visit_type_refs(type_list.value()->list);
    }

    void visit_colon_type_clause(const std::optional<ColonTypeClause*> colon_type)
    {
        if (colon_type.has_value())
            visit(colon_type.value()->type);
    }

    void visit_equals_value_clause(const std::optional<EqualsValueClause*> equals_value)
    {
        if (equals_value.has_value())
            visit(equals_value.value()->value);
    }

    void visit_function_body(const FunctionBody* function_body)
    {
        if (function_body->block.has_value())
            visit(*function_body->block);
        else
            visit(function_body->expression_body.value()->expression);
    }

    void visit_primitive_literal(PrimitiveLiteral&) override
    {
    }

    void visit_array_literal(ArrayLiteral& array_literal) override
    {
        visit_expressions(array_literal.elements);
    }

    void visit_tuple_literal(TupleLiteral& tuple_literal) override
    {
        visit_expressions(tuple_literal.elements);
    }

    void visit_range_literal(RangeLiteral& range_literal) override
    {
        visit(range_literal.minimum);
        visit(range_literal.maximum);
    }

    void visit_rgb_literal(RgbLiteral& rgb_literal) override
    {
        visit(rgb_literal.r);
        visit(rgb_literal.g);
        visit(rgb_literal.b);
    }

    void visit_hsv_literal(HsvLiteral& hsv_literal) override
    {
        visit(hsv_literal.h);
        visit(hsv_literal.s);
        visit(hsv_literal.v);
    }

    void visit_vector_literal(VectorLiteral& vector_literal) override
    {
        visit(vector_literal.x);
        visit(vector_literal.y);
        visit(vector_literal.z);
    }

    void visit_interpolated_string(InterpolatedString& interpolated_string) override
    {
        visit_expressions(interpolated_string.interpolations);
    }

    void visit_identifier(Identifier&) override
    {
    }

    void visit_parenthesized(Parenthesized& parenthesized) override
    {
        visit(parenthesized.expression);
    }

    void visit_binary_op(BinaryOp& binary_op) override
    {
        visit(binary_op.left);
        visit(binary_op.right);
    }

    void visit_unary_op(UnaryOp& unary_op) override
    {
        visit(unary_op.operand);
    }

    void visit_postfix_unary_op(PostfixUnaryOp& postfix_unary_op) override
    {
        visit(postfix_unary_op.operand);
    }

    void visit_assignment_op(AssignmentOp& assignment_op) override
    {
        visit_binary_op(assignment_op);
    }

    void visit_ternary_op(TernaryOp& ternary_op) override
    {
        visit(ternary_op.condition);
        visit(ternary_op.when_true);
        visit(ternary_op.when_false);
    }

    void visit_invocation(Invocation& invocation) override
    {
        visit(invocation.callee);
        visit_type_list_clause(invocation.type_arguments);
        visit_expressions(invocation.arguments);
    }

    void visit_type_of(TypeOf& type_of) override
    {
        visit(type_of.expression);
    }

    void visit_name_of(NameOf&) override
    {
    }

    void visit_await(Await& await) override
    {
        visit(await.expression);
    }

    void visit_member_access(MemberAccess& member_access) override
    {
        visit(member_access.expression);
    }

    void visit_optional_member_access(OptionalMemberAccess& optional_member_access) override
    {
        visit(optional_member_access.expression);
    }

    void visit_element_access(ElementAccess& element_access) override
    {
        visit(element_access.expression);
        visit(element_access.index_expression);
    }

    void visit_expression_statement(ExpressionStatement& expression_statement) override
    {
        visit(expression_statement.expression);
    }

    void visit_block(Block& block) override
    {
        visit_statements(block.braced_statement_list->statements);
    }

    void visit_type_declaration(TypeDeclaration& type_declaration) override
    {
        visit_type_list_clause(type_declaration.type_parameters);
        visit(type_declaration.type);
    }

    void visit_variable_declaration(VariableDeclaration& variable_declaration) override
    {
        visit_colon_type_clause(variable_declaration.colon_type);
        visit_equals_value_clause(variable_declaration.equals_value);
    }

    void visit_event_declaration(EventDeclaration& event_declaration) override
    {
        visit_type_list_clause(event_declaration.type_parameters);
        visit_type_refs(event_declaration.parameter_types);
    }

    void visit_interface_declaration(InterfaceDeclaration& interface_declaration) override
    {
        visit_statements(interface_declaration.members->statements);
    }

    void visit_interface_field(InterfaceField&) override
    {
    }

    void visit_interface_method(InterfaceMethod&) override
    {
    }

    void visit_enum_declaration(EnumDeclaration& enum_declaration) override
    {
        visit_statements(enum_declaration.members->statements);
    }

    void visit_enum_member(EnumMember&) override
    {
    }

    void visit_function_declaration(FunctionDeclaration& function_declaration) override
    {
        visit_type_list_clause(function_declaration.type_parameters);
        if (function_declaration.parameters.has_value())
            visit_statements(function_declaration.parameters.value()->list);

        visit_colon_type_clause(function_declaration.return_type);
        visit_function_body(function_declaration.body);
    }

    void visit_parameter(Parameter& parameter) override
    {
        visit_colon_type_clause(parameter.colon_type);
        visit_equals_value_clause(parameter.equals_value);
    }

    void visit_instance_constructor(InstanceConstructor& instance_constructor) override
    {
        visit_colon_type_clause(instance_constructor.colon_type);
        if (instance_constructor.declarators.has_value())
            visit_statements(instance_constructor.declarators.value()->statements);
        if (instance_constructor.parent.has_value())
            visit(*instance_constructor.parent);
    }

    void visit_instance_property_declarator(InstancePropertyDeclarator& instance_property_declarator) override
    {
        visit(instance_property_declarator.value);
    }

    void visit_instance_name_declarator(InstanceNameDeclarator&) override
    {
    }

    void visit_instance_attribute_declarator(InstanceAttributeDeclarator& instance_attribute_declarator) override
    {
        visit(instance_attribute_declarator.value);
    }

    void visit_instance_tag_declarator(InstanceTagDeclarator&) override
    {
    }

    void visit_break(Break&) override
    {
    }

    void visit_continue(Continue&) override
    {
    }

    void visit_return(Return& return_statement) override
    {
        if (return_statement.expression.has_value())
            visit(*return_statement.expression);
    }

    void visit_if(If& if_statement) override
    {
        visit(if_statement.condition);
        visit(if_statement.then_branch);
        if (if_statement.else_branch.has_value())
            visit(*if_statement.else_branch);
    }

    void visit_while(While& while_statement) override
    {
        visit(while_statement.condition);
        visit(while_statement.statement);
    }

    void visit_repeat(Repeat& repeat_statement) override
    {
        visit(repeat_statement.statement);
        visit(repeat_statement.condition);
    }

    void visit_for(For& for_statement) override
    {
        visit(for_statement.iterable);
        visit(for_statement.statement);
    }

    void visit_after(After& after_statement) override
    {
        visit(after_statement.time_expression);
        visit(after_statement.statement);
    }

    void visit_every(Every& every_statement) override
    {
        visit(every_statement.time_expression);
        if (every_statement.condition.has_value())
            visit(*every_statement.condition);

        visit(every_statement.statement);
    }

    void visit_match(Match& match_statement) override
    {
        visit(match_statement.expression);
        visit_statements(match_statement.cases->statements);
    }

    void visit_match_case(MatchCase& match_case) override
    {
        visit_expressions(match_case.comparands);
        visit(match_case.statement);
    }

    void visit_match_else_case(MatchElseCase& match_else_case) override
    {
        visit(match_else_case.statement);
    }

    void visit_import(Import& import_statement) override
    {
    }

    void visit_export(Export& export_statement) override
    {
        visit(export_statement.statement);
    }

    void visit_decorator(Decorator& decorator) override
    {
        visit_expressions(decorator.arguments);
    }

    void visit_primitive_type(PrimitiveTypeRef&) override
    {
    }

    void visit_literal_type(LiteralTypeRef&) override
    {
    }

    void visit_type_name(TypeNameRef&) override
    {
    }

    void visit_nullable_type(NullableTypeRef& nullable_type) override
    {
        visit(nullable_type.non_nullable_type);
    }

    void visit_array_type(ArrayTypeRef& array_type) override
    {
        visit(array_type.element_type);
    }

    void visit_tuple_type(TupleTypeRef& tuple_type) override
    {
        visit_type_refs(tuple_type.elements);
    }

    void visit_function_type(FunctionTypeRef& function_type) override
    {
        visit_type_list_clause(function_type.type_parameters);
        visit_type_refs(function_type.parameter_types);
        visit(function_type.return_type);
    }

    void visit_union_type(UnionTypeRef& union_type) override
    {
        visit_type_refs(union_type.types);
    }

    void visit_intersection_type(IntersectionTypeRef& intersection_type) override
    {
        visit_type_refs(intersection_type.types);
    }

    void visit_type_parameter(TypeParameterRef& type_parameter) override
    {
        if (type_parameter.base_type.has_value())
            visit(*type_parameter.base_type);
        if (type_parameter.default_type.has_value())
            visit(*type_parameter.default_type);
    }
};

template <typename R, typename ScopeElement>
struct ScopedAstVisitor : AstVisitor<R>
{
    std::vector<ScopeElement> scopes_;

    virtual void push_scope()
    {
        scopes_.emplace_back();
    }

    virtual void pop_scope()
    {
        scopes_.pop_back();
    }

    void visit_ast(const std::vector<statement_ptr_t>& statements,
                   const std::optional<std::function<void ()>>& before_statements = std::nullopt)
    {
        push_scope();
        if (before_statements.has_value())
            (*before_statements)();

        AstVisitor<R>::visit_ast(statements);
        pop_scope();
    }
};