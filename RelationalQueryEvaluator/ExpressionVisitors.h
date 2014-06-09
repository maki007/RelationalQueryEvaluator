#ifndef ExpressionVisitorHPP
#define ExpressionVisitorHPP

#include "Expressions.h"
#include <map>
#include "Algebra.h"
#include "AlgebraVisitors.h"
#include <vector>
#include <algorithm>

class ExpressionVisitorBase
{
public:

	/**
	* Visits UnaryExpression element.
	* @param expression visited UnaryExpression.
	*/
	virtual void visitUnaryExpression(UnaryExpression * expression);

	/**
	* Visits BinaryExpression element.
	* @param expression visited BinaryExpression.
	*/
	virtual void visitBinaryExpression(BinaryExpression * expression);

	/**
	* Visits NnaryExpression element.
	* @param expression visited NnaryExpression.
	*/
	virtual void visitNnaryExpression(NnaryExpression * expression);

	/**
	* Visits Constant element.
	* @param expression visited Constant.
	*/
	virtual void visitConstant(Constant * expression);

	/**
	* Visits Column element.
	* @param expression visited Column.
	*/
	virtual void visitColumn(Column * expression);

	/**
	* Visits GroupedExpression element.
	* @param expression visited GroupedExpression.
	*/
	virtual void visitGroupedExpression(GroupedExpression * expression);
};

class WritingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::string result;
	void visitUnaryExpression(UnaryExpression * expression);

	void visitBinaryExpression(BinaryExpression * expression);

	void visitNnaryExpression(NnaryExpression * expression);

	void visitConstant(Constant * expression);

	void visitColumn(Column * expression);

	void visitGroupedExpression(GroupedExpression * expression);
};


class NumberColumnsInJoinVisitor : public ExpressionVisitorBase
{
public:
	int lastNumberedColumn;
	NumberColumnsInJoinVisitor();
	void visitColumn(Column * expression);
	void visitBinaryExpression(BinaryExpression * expression);
};

class GetColumnsNodesVisitor : public ExpressionVisitorBase
{
public:
	std::vector<Column *> nodes;
	void visitColumn(Column * expression);
};

class GroupingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::shared_ptr<Expression> * root;
	GroupingExpressionVisitor(std::shared_ptr<Expression> * x);
	void visitBinaryExpression(BinaryExpression * expression);

};

class SemanticExpressionVisitor : public ExpressionVisitorBase
{
public:
	bool containsErrors;
	std::string missingColumn;
	std::map<std::string, ColumnInfo> outputColumns0;
	std::map<std::string, ColumnInfo> outputColumns1;
	SemanticExpressionVisitor();
	void visitColumn(Column * expression);
};

class SizeEstimatingExpressionVisitor : public ExpressionVisitorBase
{
public:
	const std::map<int, ColumnInfo> * columns;
	double size;
	SizeEstimatingExpressionVisitor(const std::map<int, ColumnInfo> * columns);

	void visitUnaryExpression(UnaryExpression * expression);

	void visitBinaryExpression(BinaryExpression * expression);
	
	void visitNnaryExpression(NnaryExpression * expression);
	
	void visitGroupedExpression(GroupedExpression * expression);

};


class JoinInfoReadingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::set<int> * data;
	ConditionType * conditionType;
	JoinInfoReadingExpressionVisitor(std::set<int> * data, ConditionType * type);

	void visitColumn(Column * expression);

	void visitUnaryExpression(UnaryExpression * expression);

	void visitBinaryExpression(BinaryExpression * expression);

	void visitNnaryExpression(NnaryExpression * expression);

	void visitConstant(Constant * expression);

	void visitGroupedExpression(GroupedExpression * expression);

};


class RenamingJoinConditionExpressionVisitor : public ExpressionVisitorBase
{
public:
	ulong n;
	std::vector<JoinColumnInfo> * inputColumns;

	RenamingJoinConditionExpressionVisitor(ulong i, std::vector<JoinColumnInfo> * inputColumns);

	void visitColumn(Column * expression);
	
};

class MaxOfUniqueValuesExpressionVisitor : public ExpressionVisitorBase
{
public:
	double result;
	std::map<int, ColumnInfo> * columns;
	MaxOfUniqueValuesExpressionVisitor(std::map<int, ColumnInfo> * cols);

	void visitColumn(Column * expression);

};


class TypeResolvingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::string resultType;

	void visitUnaryExpression(UnaryExpression * expression);

	void visitBinaryExpression(BinaryExpression * expression);

	void visitNnaryExpression(NnaryExpression * expression);

	void visitConstant(Constant * expression);

	void visitColumn(Column * expression);

	void visitGroupedExpression(GroupedExpression * expression);
};

class BoboxWritingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::string result;
	std::map<int, int> * cols;

	BoboxWritingExpressionVisitor(std::map<int, int> & cols);

	void visitUnaryExpression(UnaryExpression * expression);

	void visitBinaryExpression(BinaryExpression * expression);

	void visitNnaryExpression(NnaryExpression * expression);

	void visitConstant(Constant * expression);

	void visitColumn(Column * expression);

	void visitGroupedExpression(GroupedExpression * expression);
};

class CloningExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::shared_ptr<Expression> result;

	void visitUnaryExpression(UnaryExpression * expression);

	void visitBinaryExpression(BinaryExpression * expression);

	void visitNnaryExpression(NnaryExpression * expression);

	void visitConstant(Constant * expression);

	void visitColumn(Column * expression);

	void visitGroupedExpression(GroupedExpression * expression);

};


class RenameColumnsVisitor : public ExpressionVisitorBase
{
public:
	std::map<int, int>  * pairs;
	RenameColumnsVisitor(std::map<int, int>  * pairs);

	void visitColumn(Column * expression);

};

#endif

