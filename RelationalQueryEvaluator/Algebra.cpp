

#include "Algebra.h"
#include "AlgebraVisitor.h"

AlgebraNodeBase::AlgebraNodeBase()
{

}

AlgebraNodeBase *  AlgebraNodeBase::constructChildren(DOMElement * node)
{
	AlgebraNodeBase * child=0;
	XMLCh * groupName = XMLString::transcode("group");
	XMLCh * joinName = XMLString::transcode("join");
	XMLCh * columnOperationsName = XMLString::transcode("column_operations");
	XMLCh * tableName = XMLString::transcode("table");
	XMLCh * selectionName = XMLString::transcode("selection");
	XMLCh * unionName = XMLString::transcode("union");
	XMLCh * differenceName = XMLString::transcode("difference");
	XMLCh * antijoinName = XMLString::transcode("antijoin");
	XMLCh * intersectionName = XMLString::transcode("intersection");
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
	else if(XMLString::compareString(node->getNodeName(),unionName)==0)
	{
		child = new Union((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),differenceName)==0)
	{
		child = new Difference((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),antijoinName)==0)
	{
		child = new AntiJoin((DOMElement *)node);
	}
	else if(XMLString::compareString(node->getNodeName(),intersectionName)==0)
	{
		child = new Intersection((DOMElement *)node);
	}
	return child;
}
UnaryAlgebraNodeBase::UnaryAlgebraNodeBase()
{
}

UnaryAlgebraNodeBase::UnaryAlgebraNodeBase(DOMElement * element)
{
	parent=0;
	DOMNode * inputNode=XmlUtils::GetChildElementByName(element,"input");
	for(XMLSize_t i=0;i<inputNode->getChildNodes()->getLength();++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if(node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			child=std::shared_ptr<AlgebraNodeBase>(constructChildren((DOMElement*)node));
			child->parent=std::shared_ptr<AlgebraNodeBase>(this);
		}
	}

}


BinaryAlgebraNodeBase::BinaryAlgebraNodeBase()
{

}
BinaryAlgebraNodeBase::BinaryAlgebraNodeBase(DOMElement * element)
{
	bool leftChildInitialized=false;
	parent=0;
	DOMNode * inputNode=XmlUtils::GetChildElementByName(element,"input");
	for(XMLSize_t i=0;i<inputNode->getChildNodes()->getLength();++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if(node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			if(leftChildInitialized==false)
			{
				leftChild=std::shared_ptr<AlgebraNodeBase>(constructChildren((DOMElement*)node));
				leftChildInitialized=true;
				leftChild->parent=std::shared_ptr<AlgebraNodeBase>(this);
			}
			else
			{
				rightChild=std::shared_ptr<AlgebraNodeBase>(constructChildren((DOMElement*)node));
				rightChild->parent=std::shared_ptr<AlgebraNodeBase>(this);
			}
		}
	}
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
	DOMNode * inputNode=XmlUtils::GetChildElementByName(element,"parameters");
	for(XMLSize_t i=0;i<inputNode->getChildNodes()->getLength();++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if(node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			SortParameter parameter;
			DOMElement * parameterElement=(DOMElement *)node;
			parameter.column=XmlUtils::ReadAttribute(parameterElement,"column");
			std::string direction=XmlUtils::ReadAttribute(parameterElement,"column");
			if(direction=="asc")
			{
				parameter.ascending=true;
			}
			else if(direction=="desc")
			{
				parameter.ascending=false;
			}
			parameters.push_back(parameter);
		}
	}
}

void Sort::accept(AlgebraVisitor &v)
{
	v.visit(this);
}


Group::Group(DOMElement * element) :UnaryAlgebraNodeBase(element)
{
	
	DOMNode * inputNode=XmlUtils::GetChildElementByName(element,"parameters");
	for(XMLSize_t i=0;i<inputNode->getChildNodes()->getLength();++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if(node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			DOMElement * parameterElement=(DOMElement *)node;
			std::string elementName=XMLString::transcode(parameterElement->getLocalName());
			
			if(elementName=="group_by")
			{
				groupColumns.push_back(XmlUtils::ReadAttribute(parameterElement,"column"));
			}
			else
			{
				AgregateFunctionInfo function;
				function.functionName=elementName;
				if(elementName == "max")
				{
					function.parameter=XmlUtils::ReadAttribute(parameterElement,"argument");
					function.function=AgregateFunction::MAX;
				}
				if(elementName == "min")
				{
					function.parameter=XmlUtils::ReadAttribute(parameterElement,"argument");
					function.function=AgregateFunction::MIN;
				}
				if(elementName == "sum")
				{
					function.parameter=XmlUtils::ReadAttribute(parameterElement,"argument");
					function.function=AgregateFunction::SUM;
				}
				if(elementName == "count")
				{
					function.function=AgregateFunction::COUNT;
				}
				function.output=XmlUtils::ReadAttribute(parameterElement,"output");
				agregateFunctions.push_back(function);
			}
		}
	}
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

Selection::Selection(DOMElement * element):UnaryAlgebraNodeBase(element)
{

}

void Selection::accept(AlgebraVisitor &v)
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

AntiJoin::AntiJoin(DOMElement * element) :BinaryAlgebraNodeBase(element)
{

}

void AntiJoin::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Union::Union(DOMElement * element) :BinaryAlgebraNodeBase(element)
{

}

void Union::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Difference::Difference(DOMElement * element) :BinaryAlgebraNodeBase(element)
{

}

void Difference::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Intersection::Intersection(DOMElement * element) : BinaryAlgebraNodeBase(element)
{

}

void Intersection::accept(AlgebraVisitor &v)
{
	v.visit(this);
}



