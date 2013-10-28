#include "Algebra.h"


AlgebraNodeBase::AlgebraNodeBase()
{

}

AlgebraNodeBase *  AlgebraNodeBase::ConstructChildren(DOMElement * node)
{
	AlgebraNodeBase * child=0;
	XMLCh * groupName = XMLString::transcode("group");
	XMLCh * joinName = XMLString::transcode("join");
	XMLCh * columnOperationsName = XMLString::transcode("column_operations");
	XMLCh * tableName = XMLString::transcode("table");
	XMLCh * selectionName = XMLString::transcode("selection");

	if(XMLString::compareString(node->getNodeName(),groupName)==0)
	{
		child = new Group((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),joinName)==0)
	{
		child = new Join((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),columnOperationsName)==0)
	{
		child = new ColumnOperations((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),tableName)==0)
	{
		child = new Table((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),selectionName)==0)
	{
		child = new Selection((DOMElement *)node);
	}
	return child;
}
UnaryAlgebraNodeBase::UnaryAlgebraNodeBase()
{
}

UnaryAlgebraNodeBase::UnaryAlgebraNodeBase(DOMElement * element)
{
	DOMNode * inputNode=XmlUtils::GetChildElementByName(element,"input");
	for(XMLSize_t i=0;i<inputNode->getChildNodes()->getLength();++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if(node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			child=ConstructChildren((DOMElement*)node);
		}
	}
}

UnaryAlgebraNodeBase::~UnaryAlgebraNodeBase()
{
	delete child;
}

BinaryAlgebraNodeBase::BinaryAlgebraNodeBase()
{

}
BinaryAlgebraNodeBase::BinaryAlgebraNodeBase(DOMElement * element)
{
	bool leftChildInitialized=false;

	DOMNode * inputNode=XmlUtils::GetChildElementByName(element,"input");
	for(XMLSize_t i=0;i<inputNode->getChildNodes()->getLength();++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if(node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			if(leftChildInitialized==false)
			{
				leftChild=ConstructChildren((DOMElement*)node);
				leftChildInitialized=true;
			}
			else
			{
				rightChild=ConstructChildren((DOMElement*)node);
			}
		}
	}
}

BinaryAlgebraNodeBase::~BinaryAlgebraNodeBase()
{
	delete leftChild;
	delete rightChild;
}


Table::Table(DOMElement * element)
{
	
}

void Table::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Sort::Sort(DOMElement * element) :UnaryAlgebraNodeBase(element)
{

}

void Sort::accept(AlgebraVisitor &v)
{
	v.visit(this);
}


Group::Group(DOMElement * element) :UnaryAlgebraNodeBase(element)
{

}

void Group::accept(AlgebraVisitor &v)
{
	v.visit(this);
}


ColumnOperations::ColumnOperations(DOMElement * element):UnaryAlgebraNodeBase(element)
{

}

void ColumnOperations::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Join::Join(DOMElement * element) :BinaryAlgebraNodeBase(element)
{

}

void Join::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

CrossJoin::CrossJoin(DOMElement * element) :BinaryAlgebraNodeBase(element)
{

}

void CrossJoin::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

AntiJoin::AntiJoin(DOMElement * element) :BinaryAlgebraNodeBase(element)
{

}

void AntiJoin::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Selection::Selection(DOMElement * element):UnaryAlgebraNodeBase(element)
{

}

void Selection::accept(AlgebraVisitor &v)
{
	v.visit(this);
}