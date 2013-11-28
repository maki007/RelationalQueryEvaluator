

#include "Expressions.h"
#include "ExpressionVisitor.h"


void ExpressionVisitorBase::visit(Expression * expression)
{
	expression->accept(*this);
}

void ExpressionVisitorBase::visit(UnaryExpression * expression)
{
	expression->child->accept(*this); 
}

void ExpressionVisitorBase::visit(BinaryExpression * expression)
{
	expression->leftChild->accept(*this); 
	expression->rightChild->accept(*this); 
}

void ExpressionVisitorBase::visit(NnaryExpression * expression)
{
	for (auto it=expression->arguments.begin();it!=expression->arguments.end();++it)
	{
		(*it)->accept(*this);
	}
}

void ExpressionVisitorBase::visit(Constant * expression)
{

}

void ExpressionVisitorBase::visit(Column * expression)
{

}

void WritingExpressionVisitor::visit(UnaryExpression * expression)
{
	result+="!(";
	expression->child->accept(*this); 
	result+=")";
}

void WritingExpressionVisitor::visit(BinaryExpression * expression)
{
	result+="(";
	expression->leftChild->accept(*this); 
	switch (expression->operation)
	{
	case AND:
		result+=" and ";
		break;
	case OR:
		result+=" or ";
		break;
	case PLUS:
		result+=" + ";
		break;
	case MINUS:
		result+=" - ";
		break;
	case TIMES:
		result+=" * ";
		break;
	case DIVIDE:
		result+=" / ";
		break;
	case EQUALS:
		result+=" == ";
		break;
	case NOT_EQUALS:
		result+=" != ";
		break;
	case LOWER:
		result+=" < ";
		break;
	case LOWER_OR_EQUAL:
		result+=" <= ";
		break;
	}
	expression->rightChild->accept(*this); 
	result+=")";
}

void WritingExpressionVisitor::visit(NnaryExpression * expression)
{
	result+=expression->name;
	result+="(";
	for (auto it=expression->arguments.begin();it!=expression->arguments.end();++it)
	{
		(*it)->accept(*this);
		if(it!=expression->arguments.end()-1)
		{
			result+=",";
		}
	}
	result+=")";
}

void WritingExpressionVisitor::visit(Constant * expression)
{
	result+=expression->value;
}

void WritingExpressionVisitor::visit(Column * expression)
{
	result+=expression->name;
	if(expression->input>=0)
	{
		result+="(";
		result+=std::to_string(expression->input);
		result+=")";
	}
}

NumberColumnsInJoinVisitor::NumberColumnsInJoinVisitor()
{
	lastNumberedColumn=1;
}
void NumberColumnsInJoinVisitor::visit(Column * expression)
{
	if(lastNumberedColumn==1)
	{
		expression->input=0;
		lastNumberedColumn=0;
	}
	else
	{
		expression->input=1;
		lastNumberedColumn=1;
	}
}


void GetColumnsNodesVisitor::visit(Column * expression)
{
	this->nodes.push_back(expression);
}

