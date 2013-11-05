

#include "Expressions.h"
#include "ExpressionVisitor.h"

void Expression::accept(ExpressionVisitorBase &v){}
Expression * Expression::constructChildren(DOMElement * node){

	/*
	not
	or
	and
	equals
	not_equals
	lower
	lower_or_equal
	boolean_predicate
	plus
	minus
	times
	divide
	aritmetic_function
	column
	costant
	*/
	return 0;
}


void UnaryExpression::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}



void BinaryExpression::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}



void NnaryExpression::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}


void Constant::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}


void Column::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

