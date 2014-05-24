

#include "Expressions.h"
#include "ExpressionVisitors.h"


using namespace std;

void ExpressionVisitorBase::visitExpression(Expression * expression)
{
	expression->accept(*this);
}

void ExpressionVisitorBase::visitUnaryExpression(UnaryExpression * expression)
{
	expression->child->accept(*this);
}

void ExpressionVisitorBase::visitBinaryExpression(BinaryExpression * expression)
{
	expression->leftChild->accept(*this);
	expression->rightChild->accept(*this);
}

void ExpressionVisitorBase::visitNnaryExpression(NnaryExpression * expression)
{
	for (auto it = expression->arguments.begin(); it != expression->arguments.end(); ++it)
	{
		(*it)->accept(*this);
	}
}

void ExpressionVisitorBase::visitGroupedExpression(GroupedExpression * expression)
{
	for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
	{
		(*it)->accept(*this);
	}
}

void ExpressionVisitorBase::visitConstant(Constant * expression)
{

}

void ExpressionVisitorBase::visitColumn(Column * expression)
{

}

void WritingExpressionVisitor::visitUnaryExpression(UnaryExpression * expression)
{
	result += "!(";
	expression->child->accept(*this);
	result += ")";
}

void WritingExpressionVisitor::visitBinaryExpression(BinaryExpression * expression)
{
	result += "(";
	expression->leftChild->accept(*this);
	switch (expression->operation)
	{
	case BinaryOperator::AND:
		result += " and ";
		break;
	case BinaryOperator::OR:
		result += " or ";
		break;
	case BinaryOperator::PLUS:
		result += " + ";
		break;
	case BinaryOperator::MINUS:
		result += " - ";
		break;
	case BinaryOperator::TIMES:
		result += " * ";
		break;
	case BinaryOperator::DIVIDE:
		result += " / ";
		break;
	case BinaryOperator::EQUALS:
		result += " == ";
		break;
	case BinaryOperator::NOT_EQUALS:
		result += " != ";
		break;
	case BinaryOperator::LOWER:
		result += " < ";
		break;
	case BinaryOperator::LOWER_OR_EQUAL:
		result += " <= ";
		break;
	}
	expression->rightChild->accept(*this);
	result += ")";
}

void WritingExpressionVisitor::visitNnaryExpression(NnaryExpression * expression)
{
	result += expression->name;
	result += "(";
	for (auto it = expression->arguments.begin(); it != expression->arguments.end(); ++it)
	{
		(*it)->accept(*this);
		if (it != expression->arguments.end() - 1)
		{
			result += ",";
		}
	}
	result += ")";
}

void WritingExpressionVisitor::visitConstant(Constant * expression)
{
	result += expression->value;
}

void WritingExpressionVisitor::visitColumn(Column * expression)
{
	result += expression->column.toString();
	if (expression->input >= 0)
	{
		result += "(";
		result += to_string(expression->input);
		result += ")";
	}
}

void WritingExpressionVisitor::visitGroupedExpression(GroupedExpression * expression)
{
	switch (expression->operation)
	{
	case GroupedOperator::AND:
		result += "AND (";
		break;
	case GroupedOperator::OR:
		result += "OR (";
		break;
	}
	for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
	{
		(*it)->accept(*this);
		if (it != expression->children.end() - 1)
		{
			result += ",";
		}
	}
	result += ")";
}

NumberColumnsInJoinVisitor::NumberColumnsInJoinVisitor()
{
	lastNumberedColumn = 1;
}

void NumberColumnsInJoinVisitor::visitColumn(Column * expression)
{
	if (lastNumberedColumn == 1)
	{
		expression->input = 0;
		lastNumberedColumn = 0;
	}
	else
	{
		expression->input = 1;
		lastNumberedColumn = 1;
	}
}

void GetColumnsNodesVisitor::visitColumn(Column * expression)
{
	this->nodes.push_back(expression);
}

GroupingExpressionVisitor::GroupingExpressionVisitor(shared_ptr<Expression> * x)
{
	root = x;
}

void GroupingExpressionVisitor::visitBinaryExpression(BinaryExpression * expression)
{
	expression->leftChild->accept(*this);
	expression->rightChild->accept(*this);
	if ((expression->operation == BinaryOperator::AND) || (expression->operation == BinaryOperator::OR))
	{
		vector<shared_ptr<Expression> > oldChildren;
		oldChildren.resize(2);
		oldChildren[0] = expression->leftChild;
		oldChildren[1] = expression->rightChild;
		GroupedExpression * newNode = new GroupedExpression();
		newNode->parent = expression->parent;

		if (expression->operation == BinaryOperator::AND)
		{
			newNode->operation = GroupedOperator::AND;
		}
		else if (expression->operation == BinaryOperator::OR)
		{
			newNode->operation = GroupedOperator::OR;
		}

		if (newNode->parent == 0)
		{
			*root = shared_ptr<Expression>(newNode);
		}
		else
		{
			newNode->parent->replaceChild(expression, shared_ptr<Expression>(newNode));
		}

		for (ulong i = 0; i < 2; ++i)
		{
			if (typeid(*(oldChildren[i])) == typeid(GroupedExpression))
			{
				shared_ptr<GroupedExpression> expression = dynamic_pointer_cast<GroupedExpression>(oldChildren[i]);
				if (newNode->operation == expression->operation)
				{
					for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
					{
						newNode->children.push_back(*it);
					}
				}
				else
				{
					newNode->children.push_back(oldChildren[i]);
				}
			}
			else
			{
				newNode->children.push_back(oldChildren[i]);
			}
		}
	}
}

SemanticExpressionVisitor::SemanticExpressionVisitor()
{
	containsErrors = false;
}

void SemanticExpressionVisitor::visitColumn(Column * expression)
{
	if (expression->input == 0)
	{
		if (outputColumns0.find(expression->column.name) == outputColumns0.end())
		{
			containsErrors = true;
		}
		else
		{
			expression->column.id = outputColumns0[expression->column.name].column.id;
		}
	}
	if (expression->input == 1)
	{
		if (outputColumns1.find(expression->column.name) == outputColumns1.end())
		{
			containsErrors = true;
		}
		else
		{
			expression->column.id = outputColumns1[expression->column.name].column.id;
		}
	}
}


void TypeResolvingExpressionVisitor::visitUnaryExpression(UnaryExpression * expression)
{
	resultType = "bool";
}

void TypeResolvingExpressionVisitor::visitBinaryExpression(BinaryExpression * expression)
{
	expression->leftChild->accept(*this);
	string leftType = resultType;
	expression->rightChild->accept(*this);
	string rightType = resultType;

	switch (expression->operation)
	{
	case BinaryOperator::AND:
	case BinaryOperator::OR:
	case BinaryOperator::EQUALS:
	case BinaryOperator::NOT_EQUALS:
	case BinaryOperator::LOWER:
	case BinaryOperator::LOWER_OR_EQUAL:
		resultType = "bool";
		break;

	case BinaryOperator::PLUS:
	case BinaryOperator::MINUS:
	case BinaryOperator::TIMES:
	case BinaryOperator::DIVIDE:
		resultType = "double";
		break;

	default:

		break;
	}

}

void TypeResolvingExpressionVisitor::visitNnaryExpression(NnaryExpression * expression)
{
	resultType = expression->returnType;
}

void TypeResolvingExpressionVisitor::visitConstant(Constant * expression)
{
	resultType = expression->type;
}

void TypeResolvingExpressionVisitor::visitColumn(Column * expression)
{
	resultType = expression->type;
}

void TypeResolvingExpressionVisitor::visitGroupedExpression(GroupedExpression * expression)
{
	resultType = "bool";
}
