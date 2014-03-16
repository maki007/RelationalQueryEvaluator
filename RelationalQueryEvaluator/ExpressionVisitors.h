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
	virtual void visit(Expression * expression);

	virtual void visit(UnaryExpression * expression);

	virtual void visit(BinaryExpression * expression);

	virtual void visit(NnaryExpression * expression);

	virtual void visit(Constant * expression);

	virtual void visit(Column * expression);

	virtual void visit(GroupedExpression * expression);
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

	void visit(GroupedExpression * expression);
};


class NumberColumnsInJoinVisitor : public ExpressionVisitorBase
{
public:
	int lastNumberedColumn;
	NumberColumnsInJoinVisitor();
	void visit(Column * expression);
};

class GetColumnsNodesVisitor : public ExpressionVisitorBase
{
public:
	std::vector<Column *> nodes;
	void visit(Column * expression);
};

class GroupingExpressionVisitor : public ExpressionVisitorBase
{
public:
	std::shared_ptr<Expression> * root;
	GroupingExpressionVisitor(std::shared_ptr<Expression> * x);
	void visit(BinaryExpression * expression);

};

class SemanticExpressionVisitor : public ExpressionVisitorBase
{
public:
	bool containsErrors;
	std::map<std::string,ColumnInfo> outputColumns0; 
	std::map<std::string,ColumnInfo> outputColumns1; 
	SemanticExpressionVisitor();
	void visit(Column * expression);
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

	void visit(UnaryExpression * expression)
	{
		//NOT operator
		expression->child->accept(*this);
		size = 1 - size;
	}

	void visit(BinaryExpression * expression)
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
			size = double(1)/ 3;
			break;
		case BinaryOperator::LOWER_OR_EQUAL:
			size = double(1) / 3;
			break;
		default:
			break;
		}
	}

	void visit(NnaryExpression * expression)
	{
		size = 1/3;
	}


	void visit(GroupedExpression * expression)
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
				newSize *= 1-size;
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
	std::set<ulong> * data;
	ConditionType * conditionType;
	JoinInfoReadingExpressionVisitor(std::set<ulong> * data,ConditionType * type)
	{
		this->data = data;
		this->conditionType = type;
	}
	
	void visit(Column * expression)
	{
		data->insert(expression->input);
	}
	
	void visit(UnaryExpression * expression)
	{
		(*conditionType) = ConditionType::OTHER;
		expression->child->accept(*this);
	}

	void visit(BinaryExpression * expression)
	{
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
		expression->leftChild->accept(*this);
		expression->rightChild->accept(*this);
		
	}

	void visit(NnaryExpression * expression)
	{
		(*conditionType) = ConditionType::OTHER;
	}

	void visit(Constant * expression)
	{
		throw new std::exception("constant shouldnt be in joins");
	}

	void visit(GroupedExpression * expression)
	{
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
	void visit(Column * expression)
	{
		if (expression->input == n)
		{
			for (auto it = inputColumns->begin(); it != inputColumns->end();++it)
			{
				if (it->column.name != it->newColumn.name)
				{
					if (it->newColumn.name == expression->column.name)
					{
						expression->column.name = it->column.name;
					}
				}
			}
		}
		
	}
};

#endif