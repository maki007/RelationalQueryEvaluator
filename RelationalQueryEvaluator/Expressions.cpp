

#include "Expressions.h"
#include "ExpressionVisitors.h"
/*
void Expression::accept(ExpressionVisitorBase &v)
{

}
*/
Expression * Expression::constructChildren(DOMElement * node)
{
	Expression * child=0;
	XMLCh * notName = XMLString::transcode("not");
	XMLCh * orName = XMLString::transcode("or");
	XMLCh * andName = XMLString::transcode("and");
	XMLCh * equalsName = XMLString::transcode("equals");
	XMLCh * notEqualsName = XMLString::transcode("not_equals");
	XMLCh * lowerName = XMLString::transcode("lower");
	XMLCh * lowerEqualsName = XMLString::transcode("lower_or_equals");
	XMLCh * booleanPredicateName = XMLString::transcode("boolean_predicate");
	XMLCh * plusName = XMLString::transcode("plus");
	XMLCh * minusName = XMLString::transcode("minus");
	XMLCh * timesName = XMLString::transcode("times");
	XMLCh * divideName = XMLString::transcode("divide");
	XMLCh * aritmeticFunctionName = XMLString::transcode("aritmetic_function");
	XMLCh * columnName = XMLString::transcode("column");
	XMLCh * constantName = XMLString::transcode("constant");

	if(XMLString::compareString(node->getNodeName(),notName)==0)
	{
		child = new UnaryExpression((DOMElement *)node,UnaryOperator::NOT);
	}
	else if(XMLString::compareString(node->getNodeName(),orName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::OR);
	}
	else if(XMLString::compareString(node->getNodeName(),andName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::AND);
	}
	else if(XMLString::compareString(node->getNodeName(),equalsName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::EQUALS);
	}
	else if(XMLString::compareString(node->getNodeName(),notEqualsName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::NOT_EQUALS);
	}
	else if(XMLString::compareString(node->getNodeName(),lowerName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::LOWER);
	}
	else if(XMLString::compareString(node->getNodeName(),lowerEqualsName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::LOWER_OR_EQUAL);
	}
	else if(XMLString::compareString(node->getNodeName(),booleanPredicateName)==0)
	{
		child = new NnaryExpression((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),plusName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::PLUS);
	}
	else if(XMLString::compareString(node->getNodeName(),minusName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::MINUS);
	}
	else if(XMLString::compareString(node->getNodeName(),timesName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::TIMES);
	}
	else if(XMLString::compareString(node->getNodeName(),divideName)==0)
	{
		child = new BinaryExpression((DOMElement *)node,BinaryOperator::DIVIDE);
	}
	else if(XMLString::compareString(node->getNodeName(),aritmeticFunctionName)==0)
	{
		child = new NnaryExpression((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),columnName)==0)
	{
		child = new Column((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),constantName)==0)
	{
		child = new Constant((DOMElement *)node);
	}
	else
	{
		throw new std::exception("not valid");
	}


	return child;
}

UnaryExpression::UnaryExpression(DOMElement * node,UnaryOperator op)
{
	DOMElement * childNode =  XmlUtils::GetFirstChildElement(node);
	child = std::shared_ptr<Expression>(constructChildren(childNode));
	child->parent=this;
}

UnaryExpression::UnaryExpression(std::shared_ptr<Expression> node,UnaryOperator op)
{
	child=node;
	child->parent=this;
	operation=op;
}

void UnaryExpression::replaceChild(Expression * oldChild,Expression * newChild)
{
	if(child.get()==oldChild)
	{
		child=std::shared_ptr<Expression>(newChild);
	}
}


void UnaryExpression::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

BinaryExpression::BinaryExpression(DOMElement * node,BinaryOperator op)
{
	std::vector<DOMElement *> childNodes =  XmlUtils::GetChildElements(node);
	leftChild = std::shared_ptr<Expression>(constructChildren(childNodes[0]));
	rightChild = std::shared_ptr<Expression>(constructChildren(childNodes[1]));
	leftChild->parent=this;
	rightChild->parent=this;
	operation=op;
}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> & leftChild, std::shared_ptr<Expression> & rightChild, BinaryOperator op)
{
	this->leftChild=leftChild;
	this->rightChild=rightChild;
	leftChild->parent = this;
	rightChild->parent = this;
	operation=op;
}

void BinaryExpression::replaceChild(Expression * oldChild,Expression * newChild)
{
	if(leftChild.get()==oldChild)
	{
		leftChild=std::shared_ptr<Expression>(newChild);
	}
	if(rightChild.get()==oldChild)
	{
		rightChild=std::shared_ptr<Expression>(newChild);
	}
}

void BinaryExpression::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

NnaryExpression::NnaryExpression(DOMElement * node)
{
	std::vector<DOMElement *> childNodes =  XmlUtils::GetChildElements(node);
	for(auto it=childNodes.begin();it!=childNodes.end();++it)
	{
		arguments.push_back(std::shared_ptr<Expression>(constructChildren(*it)));
		arguments.back()->parent=this;
	}
}

void NnaryExpression::replaceChild(Expression * oldChild,Expression * newChild)
{
	for(auto it=arguments.begin();it!=arguments.end();++it)
	{
		if(it->get()==oldChild)
		{
			*it=std::shared_ptr<Expression>(newChild);
		}
	}
}

void NnaryExpression::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

Constant::Constant(DOMElement * node)
{
	this->value=XmlUtils::ReadAttribute(node,"value");
}

void Constant::replaceChild(Expression * oldChild,Expression * newChild)
{

}

void Constant::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

Column::Column(DOMElement * node)
{
	this->name=XmlUtils::ReadAttribute(node,"name");
	this->input=0;
}

void Column::replaceChild(Expression * oldChild,Expression * newChild)
{

}

void Column::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

GroupedExpression::GroupedExpression()
{
}

GroupedExpression::GroupedExpression(GroupedOperator operation, const std::vector<std::shared_ptr<Expression>> & children)
{
	this->operation = operation;
	this->children = children;

}
void GroupedExpression::replaceChild(Expression * oldChild,Expression * newChild)
{
	for(auto it=children.begin();it!=children.end();++it)
	{
		if(it->get()==oldChild)
		{
			*it=std::shared_ptr<Expression>(newChild);
		}
	}
}

void GroupedExpression::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}