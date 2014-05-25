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
	virtual void visitExpression(Expression * expression);

	virtual void visitUnaryExpression(UnaryExpression * expression);

	virtual void visitBinaryExpression(BinaryExpression * expression);

	virtual void visitNnaryExpression(NnaryExpression * expression);

	virtual void visitConstant(Constant * expression);

	virtual void visitColumn(Column * expression);

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
	SizeEstimatingExpressionVisitor(const std::map<int, ColumnInfo> * columns)
	{
		size = 1;
		this->columns = columns;
	}

	void visitUnaryExpression(UnaryExpression * expression)
	{
		//NOT operator
		expression->child->accept(*this);
		size = 1 - size;
	}

	void visitBinaryExpression(BinaryExpression * expression)
	{
		switch (expression->operation)
		{
		case BinaryOperator::EQUALS:
		case BinaryOperator::NOT_EQUALS:
			if ((typeid(*((expression->leftChild))) == typeid(Column)))
			{
				if ((typeid(*((expression->rightChild))) == typeid(Column)))
				{
					std::shared_ptr<Column> leftColumn = std::dynamic_pointer_cast<Column>(expression->leftChild);
					std::shared_ptr<Column> rightColumn = std::dynamic_pointer_cast<Column>(expression->rightChild);
					//todo change
					size = 1 / std::max(columns->at(leftColumn->column.id).numberOfUniqueValues, columns->at(rightColumn->column.id).numberOfUniqueValues);
					//, columns->at(rightColumn->name));
				}
				else
				{
					std::shared_ptr<Constant> constant = std::dynamic_pointer_cast<Constant>(expression->rightChild);
					std::shared_ptr<Column> column = std::dynamic_pointer_cast<Column>(expression->leftChild);
					size = 1 / columns->at(column->column.id).numberOfUniqueValues;
				}

			}
			else
			{
				if ((typeid(*((expression->rightChild))) == typeid(Column)))
				{
					std::shared_ptr<Constant> constant = std::dynamic_pointer_cast<Constant>(expression->leftChild);
					std::shared_ptr<Column> column = std::dynamic_pointer_cast<Column>(expression->rightChild);
					size = 1 / columns->at(column->column.id).numberOfUniqueValues;

				}
				else
				{
					std::shared_ptr<Constant> leftConstant = std::dynamic_pointer_cast<Constant>(expression->leftChild);
					std::shared_ptr<Constant> rightConstant = std::dynamic_pointer_cast<Constant>(expression->rightChild);
					if (leftConstant->value == rightConstant->value)
					{
						size = 1;
					}
					else
					{
						size = 0;
					}
				}
			}
			if (expression->operation == BinaryOperator::NOT_EQUALS)
			{
				size = 1 - size;
			}
			break;
		case BinaryOperator::LOWER:
			size = double(1) / 3;
			break;
		case BinaryOperator::LOWER_OR_EQUAL:
			size = double(1) / 3;
			break;
		default:
			break;
		}
	}

	void visitNnaryExpression(NnaryExpression * expression)
	{
		size = 1 / 3;
	}


	void visitGroupedExpression(GroupedExpression * expression)
	{
		double newSize = 1;

		switch (expression->operation)
		{
		case GroupedOperator::AND:
			for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
			{
				(*it)->accept(*this);
				newSize *= size;
			}
			break;
		case GroupedOperator::OR:
			for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
			{
				(*it)->accept(*this);
				newSize *= 1 - size;
			}
			newSize = 1 - newSize;
			break;
		}
		size = newSize;
	}

};


class JoinInfoReadingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::set<uint> * data;
	ConditionType * conditionType;
	JoinInfoReadingExpressionVisitor(std::set<uint> * data, ConditionType * type)
	{
		this->data = data;
		data->clear();
		conditionType = type;
		(*conditionType) = ConditionType::OTHER;
	}

	void visitColumn(Column * expression)
	{
		data->insert(expression->column.id);
	}

	void visitUnaryExpression(UnaryExpression * expression)
	{
		expression->child->accept(*this);
		(*conditionType) = ConditionType::OTHER;
	}

	void visitBinaryExpression(BinaryExpression * expression)
	{
		expression->leftChild->accept(*this);
		expression->rightChild->accept(*this);

		switch (expression->operation)
		{
		case BinaryOperator::EQUALS:
			(*conditionType) = ConditionType::EQUALS;
			break;
		case BinaryOperator::LOWER:
		case BinaryOperator::LOWER_OR_EQUAL:
			(*conditionType) = ConditionType::LOWER;
			break;
		default:
			(*conditionType) = ConditionType::OTHER;
			break;
		}
	}

	void visitNnaryExpression(NnaryExpression * expression)
	{
		for (auto it = expression->arguments.begin(); it != expression->arguments.end(); ++it)
		{
			(*it)->accept(*this);
		}
		(*conditionType) = ConditionType::OTHER;
	}

	void visitConstant(Constant * expression)
	{

	}

	void visitGroupedExpression(GroupedExpression * expression)
	{
		for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
		{
			(*it)->accept(*this);
		}
		(*conditionType) = ConditionType::OTHER;
	}

};


class RenamingJoinConditionExpressionVisitor : public ExpressionVisitorBase
{
public:
	ulong n;
	std::vector<JoinColumnInfo> * inputColumns;
	RenamingJoinConditionExpressionVisitor(ulong i, std::vector<JoinColumnInfo> * inputColumns)
	{
		n = i;
		this->inputColumns = inputColumns;
	}
	void visitColumn(Column * expression)
	{
		if (expression->input == n)
		{
			for (auto it = inputColumns->begin(); it != inputColumns->end(); ++it)
			{
				if (it->column.name != it->newColumn)
				{
					if (it->newColumn == expression->column.name)
					{
						expression->column.name = it->column.name;
					}
				}
			}
		}

	}
};

class MaxOfUniqueValuesExpressionVisitor : public ExpressionVisitorBase
{
public:
	double result;
	std::map<int, ColumnInfo> * columns;
	MaxOfUniqueValuesExpressionVisitor(std::map<int, ColumnInfo> * cols)
	{
		this->result = 1;
		columns = cols;
	}

	void visitColumn(Column * expression)
	{
		this->result = std::max(this->result, columns->at(expression->column.id).numberOfUniqueValues);
	}
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
	RenameColumnsVisitor(std::map<int, int>  * pairs)
	{
		this->pairs = pairs;
	}
	void visitColumn(Column * expression)	
	{
		if (pairs->find(expression->column.id) != pairs->end())
		{
			expression->column.id = pairs->at(expression->column.id);
		}
	}
};

#endif

