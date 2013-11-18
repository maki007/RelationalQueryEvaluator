

#include "Expressions.h"
#include "ExpressionVisitor.h"
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
	XMLCh * lowerEqualsName = XMLString::transcode("lower_or_equal");
	XMLCh * booleanPredicateName = XMLString::transcode("boolean_predicate");
	XMLCh * plusName = XMLString::transcode("plus");
	XMLCh * minusName = XMLString::transcode("minus");
	XMLCh * timesName = XMLString::transcode("times");
	XMLCh * divideName = XMLString::transcode("divide");
	XMLCh * aritmeticFunctionName = XMLString::transcode("aritmetic_function");
	XMLCh * columnName = XMLString::transcode("column");
	XMLCh * costantName = XMLString::transcode("costant");

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
	else if(XMLString::compareString(node->getNodeName(),costantName)==0)
	{
		child = new Constant((DOMElement *)node);
	}


	return child;
}

UnaryExpression::UnaryExpression(DOMElement * node,UnaryOperator op)
{
	DOMElement * childNode =  XmlUtils::GetFirstChildElement(node);
	child = std::shared_ptr<Expression>(constructChildren(childNode));
}

UnaryExpression::UnaryExpression(std::shared_ptr<Expression> node,UnaryOperator op)
{
	child=node;
	operation=op;
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
	operation=op;
}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> leftChild,std::shared_ptr<Expression> rightChild,BinaryOperator op)
{
	this->leftChild=leftChild;
	this->rightChild=rightChild;
	this->operation=op;
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

void Constant::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

Column::Column(DOMElement * node)
{
	this->name=XmlUtils::ReadAttribute(node,"name");
}

void Column::accept(ExpressionVisitorBase &v)
{
	v.visit(this);
}

