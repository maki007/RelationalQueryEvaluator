

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

void NumberColumnsInJoinVisitor::visitBinaryExpression(BinaryExpression * expression)
{
	expression->leftChild->accept(*this);
	expression->rightChild->accept(*this);
	if (expression->operation == BinaryOperator::LOWER || expression->operation == BinaryOperator::LOWER_OR_EQUAL)
	{
		if (lastNumberedColumn == 1)
		{
			lastNumberedColumn = 0;
		}
		else
		{
			lastNumberedColumn = 1;
		}
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
	missingColumn = "";
}

void SemanticExpressionVisitor::visitColumn(Column * expression)
{
	if (expression->input == 0)
	{
		if (outputColumns0.find(expression->column.name) == outputColumns0.end())
		{
			containsErrors = true;
			if (missingColumn == "")
			{
				missingColumn = expression->column.name;
			}
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
			if (missingColumn == "")
			{
				missingColumn = expression->column.name;
			}
		}
		else
		{
			expression->column.id = outputColumns1[expression->column.name].column.id;
		}
	}
}

SizeEstimatingExpressionVisitor::SizeEstimatingExpressionVisitor(const std::map<int, ColumnInfo> * columns)
{
	size = 1;
	this->columns = columns;
}

void SizeEstimatingExpressionVisitor::visitUnaryExpression(UnaryExpression * expression)
{
	//NOT operator
	expression->child->accept(*this);
	size = 1 - size;
}

void SizeEstimatingExpressionVisitor::visitBinaryExpression(BinaryExpression * expression)
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

void SizeEstimatingExpressionVisitor::visitNnaryExpression(NnaryExpression * expression)
{
	size = double(1) / 3;
}


void SizeEstimatingExpressionVisitor::visitGroupedExpression(GroupedExpression * expression)
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

JoinInfoReadingExpressionVisitor::JoinInfoReadingExpressionVisitor(std::set<int> * data, ConditionType * type)
{
	this->data = data;
	data->clear();
	conditionType = type;
	(*conditionType) = ConditionType::OTHER;
}

void JoinInfoReadingExpressionVisitor::visitColumn(Column * expression)
{
	data->insert(expression->column.id);
}

void JoinInfoReadingExpressionVisitor::visitUnaryExpression(UnaryExpression * expression)
{
	expression->child->accept(*this);
	(*conditionType) = ConditionType::OTHER;
}

void JoinInfoReadingExpressionVisitor::visitBinaryExpression(BinaryExpression * expression)
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

void JoinInfoReadingExpressionVisitor::visitNnaryExpression(NnaryExpression * expression)
{
	for (auto it = expression->arguments.begin(); it != expression->arguments.end(); ++it)
	{
		(*it)->accept(*this);
	}
	(*conditionType) = ConditionType::OTHER;
}

void JoinInfoReadingExpressionVisitor::visitConstant(Constant * expression)
{

}

void JoinInfoReadingExpressionVisitor::visitGroupedExpression(GroupedExpression * expression)
{
	for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
	{
		(*it)->accept(*this);
	}
	(*conditionType) = ConditionType::OTHER;
}

RenamingJoinConditionExpressionVisitor::RenamingJoinConditionExpressionVisitor(ulong i, std::vector<JoinColumnInfo> * inputColumns)
{
	n = i;
	this->inputColumns = inputColumns;
}
void RenamingJoinConditionExpressionVisitor::visitColumn(Column * expression)
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

MaxOfUniqueValuesExpressionVisitor::MaxOfUniqueValuesExpressionVisitor(std::map<int, ColumnInfo> * cols)
{
	this->result = 1;
	columns = cols;
}

void MaxOfUniqueValuesExpressionVisitor::visitColumn(Column * expression)
{
	this->result = std::max(this->result, columns->at(expression->column.id).numberOfUniqueValues);
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

void CloningExpressionVisitor::visitUnaryExpression(UnaryExpression * expression)
{
	auto node = new UnaryExpression(*expression);
	node->child->accept(*this);
	node->child = result;
	result = shared_ptr<Expression>(node);
}

void CloningExpressionVisitor::visitBinaryExpression(BinaryExpression * expression)
{
	auto node = new BinaryExpression(*expression);
	node->leftChild->accept(*this);
	node->leftChild = result;
	node->rightChild->accept(*this);
	node->rightChild = result;
	result = shared_ptr<Expression>(node);

}

void CloningExpressionVisitor::visitNnaryExpression(NnaryExpression * expression)
{
	auto node = new NnaryExpression(*expression);
	for (auto it = node->arguments.begin(); it != node->arguments.end(); ++it)
	{
		(*it)->accept(*this);
		(*it) = result;
	}
	result = shared_ptr<Expression>(node);
}

void CloningExpressionVisitor::visitConstant(Constant * expression)
{
	auto node = new Constant(*expression);
	result = shared_ptr<Expression>(node);
}


void CloningExpressionVisitor::visitColumn(Column * expression)
{
	auto node = new Column(*expression);
	result = shared_ptr<Expression>(node);
}

void CloningExpressionVisitor::visitGroupedExpression(GroupedExpression * expression)
{
	auto node = new GroupedExpression(*expression);
	for (auto it = node->children.begin(); it != node->children.end(); ++it)
	{
		(*it)->accept(*this);
		(*it) = result;
	}
	result = shared_ptr<Expression>(node);
}

RenameColumnsVisitor::RenameColumnsVisitor(std::map<int, int>  * pairs)
{
	this->pairs = pairs;
}

void RenameColumnsVisitor::visitColumn(Column * expression)
{
	if (pairs->find(expression->column.id) != pairs->end())
	{
		expression->column.id = pairs->at(expression->column.id);
	}
}