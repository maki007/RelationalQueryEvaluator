#ifndef ExpressionVisitorHPP
#define ExpressionVisitorHPP

#include "Expressions.h"

class ExpressionVisitorBase
{
public:
	virtual void visit(Expression * expression);

	virtual void visit(UnaryExpression * expression);

	virtual void visit(BinaryExpression * expression);

	virtual void visit(NnaryExpression * expression);

	virtual void visit(Constant * expression);

	virtual void visit(Column * expression);
};

class WritingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::string result;
	void visit(UnaryExpression * expression);

	void visit(BinaryExpression * expression);

	void visit(NnaryExpression * expression);

	void visit(Constant * expression);

	void visit(Column * expression);
};

#endif