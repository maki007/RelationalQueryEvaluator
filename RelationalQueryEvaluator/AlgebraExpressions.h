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

class Expression
{

};
class UnaryExpression : public Expression
{
	UnaryOperator operation;
	std::shared_ptr<Expression> child;
};

class BinaryExpression : public Expression
{
	BinaryOperator operation;
	std::shared_ptr<Expression> left;
	std::shared_ptr<Expression> right;
};

class NnaryExpression : public Expression
{
	std::string name;
	std::vector<std::shared_ptr<Expression>> arguments;
};

template< class T>
class Constant : public Expression
{
public:
	T value;
	std::string type;
};

class Column : public Expression
{
public:
	std::string name;
	std::string tableName;
	std::string type;
};

#endif