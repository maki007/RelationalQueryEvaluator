#ifndef AlgebraExpressionHPP
#define AlgebraExpressionHPP
#include <memory>
#include <vector>
#include <string>

enum UnaryOperator
{
	NOT
};
enum BinaryOperator
{
	AND,OR,PLUS,MINUS,TIMES,DIVIDE,EQUALS,NOT_EQUALS,LOWER,LOWER_OR_EQUAL
};

class AlgebraExpressionVisitorBase;

class Expression
{
public:
	virtual void accept(AlgebraExpressionVisitorBase &v)=0;
};
class UnaryExpression : public Expression
{
public:
	UnaryOperator operation;
	std::shared_ptr<Expression> child;
	void accept(AlgebraExpressionVisitorBase &v);
};

class BinaryExpression : public Expression
{
public:
	BinaryOperator operation;
	std::shared_ptr<Expression> left;
	std::shared_ptr<Expression> right;
	void accept(AlgebraExpressionVisitorBase &v);
};

class NnaryExpression : public Expression
{
public:
	std::string name;
	std::vector<std::shared_ptr<Expression>> arguments;
	void accept(AlgebraExpressionVisitorBase &v);
};

class Constant : public Expression
{
public:
	std::string value;
	std::string type;
	void accept(AlgebraExpressionVisitorBase &v);
};

class Column : public Expression
{
public:
	std::string name;
	std::string tableName;
	std::string type;
	void accept(AlgebraExpressionVisitorBase &v);
};


class AlgebraExpressionVisitorBase
{
public:
	virtual void visit(Expression * expression)
	{
		expression->accept(*this);
	}

	virtual void visit(UnaryExpression * expression)=0;

	virtual void visit(BinaryExpression * expression)=0;

	virtual void visit(NnaryExpression * expression)=0;

	virtual void visit(Constant * expression)=0;

	virtual void visit(Column * expression)=0;
};

class WritingAlgebraExpressionVisitor
{
public:
	std::string result;
	void visit(UnaryExpression * expression)
	{

	}

	void visit(BinaryExpression * expression)
	{
	
	}

	void visit(NnaryExpression * expression)
	{
		
	}

	void visit(Constant * expression)
	{
		result+=expression->value;
	}

	void visit(Column * expression)
	{
		result+=expression->name;
	}
};

#endif