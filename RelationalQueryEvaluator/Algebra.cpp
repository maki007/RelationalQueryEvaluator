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

void UnaryAlgebraNodeBase::replaceChild(AlgebraNodeBase * oldChild,AlgebraNodeBase * newChild)
{
	if(child.get()==oldChild)
	{
		child=std::shared_ptr<AlgebraNodeBase>(newChild);
	}
	else
	{
		//exception
	}
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

void BinaryAlgebraNodeBase::replaceChild(AlgebraNodeBase * oldChild,AlgebraNodeBase * newChild)
{
	if(leftChild.get()==oldChild)
	{
		leftChild=std::shared_ptr<AlgebraNodeBase>(newChild);
	}
	else if(rightChild.get()==oldChild)
	{
		rightChild=std::shared_ptr<AlgebraNodeBase>(newChild);
	}
	else
	{
		//exception
	}
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

void NullaryAlgebraNodeBase::replaceChild(AlgebraNodeBase * oldChild,AlgebraNodeBase * newChild)
{
}

void GroupedAlgebraNode::replaceChild(AlgebraNodeBase * oldChild,AlgebraNodeBase * newChild)
{

}

Table::Table(DOMElement * element)
{
	/*
	<table name="users" numberOfRows="10000">
	<column name="a" type="int" number_of_unique_values="1000" />
	<index>
	<column name="a"/>
	</index>
	</table>
	*/
	name=XmlUtils::ReadAttribute(element,"name");
	if(XmlUtils::ReadAttribute(element,"numberOfRows")=="")
	{
		numberOfRows=1000;
	}
	else
	{
		std::istringstream ( XmlUtils::ReadAttribute(element,"numberOfRows") ) >> numberOfRows;
	}
	std::vector<DOMElement *> parameters=XmlUtils::GetChildElements(element);
	for(auto it=parameters.begin();it!=parameters.end();++it)
	{
		if(XmlUtils::GetElementName(*it)=="column")
		{
			ColumnInfo info;
			info.type=XmlUtils::ReadAttribute(*it,"type");
			info.name=XmlUtils::ReadAttribute(*it,"name");
			if(XmlUtils::ReadAttribute(*it,"number_of_unique_values")=="")
			{
				info.numberOfUniqueValues=numberOfRows/3;
			}
			else
			{
				std::istringstream ( XmlUtils::ReadAttribute(*it,"number_of_unique_values") ) >> info.numberOfUniqueValues;
				info.numberOfUniqueValues = std::min(info.numberOfUniqueValues,numberOfRows);
			}
			columns.push_back(info);
		}
		else if(XmlUtils::GetElementName(*it)=="index")
		{
			IndexInfo index;
			if(XmlUtils::ReadAttribute(*it,"type")=="clustered")
			{
				index.type=IndexType::CLUSTERED;
			}
			else
			{
				index.type=IndexType::UNCLUSTERED;
			}
			std::vector<DOMElement *> columnsElement=XmlUtils::GetChildElements(*it);
			for(auto it2=columnsElement.begin();it2!=columnsElement.end();++it2)
			{
				index.columns.push_back(std::string(XmlUtils::ReadAttribute(*it2,"name")));
			}
			indices.push_back(index);
		}
	}
	//todo indices
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
			std::string direction=XmlUtils::ReadAttribute(parameterElement,"direction");
			if(direction=="asc")
			{
				parameter.ascending=true;
			}
			else
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
	DOMElement * parametersNode=XmlUtils::GetChildElementByName(element,"parameters");
	std::vector<DOMElement *> columns=XmlUtils::GetChildElements(parametersNode);
	for(auto it=columns.begin();it!=columns.end();++it)
	{
		ColumnOperation op;
		op.result=XmlUtils::ReadAttribute(*it,"name");
		if(XmlUtils::GetFirstChildElement(*it)!=0)
		{
			op.expression=std::shared_ptr<Expression>(Expression::constructChildren(XmlUtils::GetFirstChildElement(XmlUtils::GetFirstChildElement(*it))));
		}
		else
		{
			op.expression=std::shared_ptr<Expression>(0);
		}
		operations.push_back(op);
	}
}

void ColumnOperations::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Selection::Selection(DOMElement * element):UnaryAlgebraNodeBase(element)
{
	DOMElement * parametersNode=XmlUtils::GetChildElementByName(element,"parameters");
	DOMElement * conditionNode=XmlUtils::GetChildElementByName(parametersNode,"condition");
	condition = std::shared_ptr<Expression>(Expression::constructChildren(XmlUtils::GetFirstChildElement(conditionNode)));

}

void Selection::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

Join::Join(DOMElement * element) :BinaryAlgebraNodeBase(element)
{
	DOMElement * parametersNode=XmlUtils::GetChildElementByName(element,"parameters");
	DOMElement * conditionNode=XmlUtils::GetFirstChildElement(parametersNode);
	int start=0;
	if(XmlUtils::GetFirstChildElement(conditionNode)!=0)
	{
		start=1;
		condition = std::shared_ptr<Expression>(Expression::constructChildren(XmlUtils::GetFirstChildElement(conditionNode)));
		std::vector<DOMElement *> conditions=XmlUtils::GetChildElements(conditionNode);
		for(auto it=conditions.begin()+1;it!=conditions.end();++it)
		{
			std::shared_ptr<Expression> newCondition(Expression::constructChildren(*it));
			condition = std::shared_ptr<Expression>(new BinaryExpression(condition,newCondition,BinaryOperator::AND));
		}
	}
	else
	{
		condition=0;
	}
	std::vector<DOMElement *> columns=XmlUtils::GetChildElements(parametersNode);
	for(auto it=columns.begin()+start;it!=columns.end();++it)
	{
		JoinColumnInfo info;
		info.newName=XmlUtils::ReadAttribute(*it,"newName");
		info.name=XmlUtils::ReadAttribute(*it,"name");
		if(info.newName=="")
		{
			info.newName=info.name;
		}
		if(XmlUtils::ReadAttribute(*it,"input")=="first")
		{
			info.input=0;
		}
		else
		{
			info.input=1;
		}
		outputColumns.push_back(info);
	}
}

void Join::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

AntiJoin::AntiJoin(DOMElement * element) :BinaryAlgebraNodeBase(element)
{
	DOMElement * parametersNode=XmlUtils::GetChildElementByName(element,"parameters");
	DOMElement * conditionNode=XmlUtils::GetFirstChildElement(parametersNode);
	condition = std::shared_ptr<Expression>(Expression::constructChildren(XmlUtils::GetFirstChildElement(conditionNode)));
	std::vector<DOMElement *> conditions=XmlUtils::GetChildElements(conditionNode);
	for(auto it=conditions.begin()+1;it!=conditions.end();++it)
	{
		std::shared_ptr<Expression> newCondition(Expression::constructChildren(*it));
		condition = std::shared_ptr<Expression>(new BinaryExpression(condition,newCondition,BinaryOperator::AND));
	}

	std::vector<DOMElement *> columns=XmlUtils::GetChildElements(parametersNode);
	for(auto it=columns.begin()+1;it!=columns.end();++it)
	{
		JoinColumnInfo info;
		info.newName=XmlUtils::ReadAttribute(*it,"newName");
		info.name=XmlUtils::ReadAttribute(*it,"name");
		if(info.newName=="")
		{
			info.newName=info.name;
		}
		if(XmlUtils::ReadAttribute(*it,"input")=="first")
		{
			info.input=0;
		}
		else
		{
			info.input=1;
		}
		outputColumns.push_back(info);
	}

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

void GroupedJoin::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

void GroupedIntersection::accept(AlgebraVisitor &v)
{
	v.visit(this);
}

