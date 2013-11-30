

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

void ExpressionVisitorBase::visit(GroupedExpression * expression)
{
	for (auto it=expression->children.begin();it!=expression->children.end();++it)
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

void WritingExpressionVisitor::visit(GroupedExpression * expression)
{
	switch (expression->operation)
	{
	case GROUPED_AND:
		result+="AND (";
		break;
	case GROUPED_OR:
		result+="OR (";
		break;
	}
	for (auto it=expression->children.begin();it!=expression->children.end();++it)
	{
		(*it)->accept(*this);
		if(it!=expression->children.end()-1)
		{
			result+=",";
		}
	}
	result+=")";
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

GroupingExpressionVisitor::GroupingExpressionVisitor(std::shared_ptr<Expression> * x)
{
	root=x;
}

void GroupingExpressionVisitor::visit(BinaryExpression * expression)
{
	expression->leftChild->accept(*this); 
	expression->rightChild->accept(*this);
	if((expression->operation == AND) || (expression->operation==OR))
	{
		std::vector<std::shared_ptr<Expression> > oldChildren;
		oldChildren.resize(2);
		oldChildren[0]=expression->leftChild;
		oldChildren[1]=expression->rightChild;
		GroupedExpression * newNode=new GroupedExpression();
		newNode->parent=expression->parent;
		
		if(expression->operation == AND)
		{
			newNode->operation=GroupedOperator::GROUPED_AND;
		}
		else if(expression->operation == OR)
		{
			newNode->operation=GroupedOperator::GROUPED_OR;
		}
		
		if(newNode->parent==0)
		{
			*root=std::shared_ptr<Expression>(newNode);
		}
		else
		{
			newNode->parent->replaceChild(expression,newNode);
		}

		for(std::size_t i=0;i<2;++i)
		{
			if(typeid(*(oldChildren[i])) == typeid(GroupedExpression))
			{
				std::shared_ptr<GroupedExpression> expression=std::dynamic_pointer_cast<GroupedExpression>(oldChildren[i]);
				if(newNode->operation==expression->operation)
				{
					for(auto it=expression->children.begin();it!=expression->children.end();++it)
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
