#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "ExpressionVisitors.h"

#include <algorithm>


using namespace std;

AlgebraNodeBase::AlgebraNodeBase()
{

}

AlgebraNodeBase *  AlgebraNodeBase::constructChildren(DOMElement * node)
{
	AlgebraNodeBase * child = 0;
	XMLCh * groupName = XMLString::transcode("group");
	XMLCh * joinName = XMLString::transcode("join");
	XMLCh * columnOperationsName = XMLString::transcode("column_operations");
	XMLCh * tableName = XMLString::transcode("table");
	XMLCh * selectionName = XMLString::transcode("selection");
	XMLCh * unionName = XMLString::transcode("union");
	XMLCh * differenceName = XMLString::transcode("difference");
	XMLCh * antijoinName = XMLString::transcode("antijoin");
	XMLCh * intersectionName = XMLString::transcode("intersection");
	if (XMLString::compareString(node->getNodeName(), groupName) == 0)
	{
		child = new Group((DOMElement *)node);
	}
	else if (XMLString::compareString(node->getNodeName(), joinName) == 0)
	{
		child = new Join((DOMElement *)node);
	}
	else if (XMLString::compareString(node->getNodeName(), columnOperationsName) == 0)
	{
		child = new ColumnOperations((DOMElement *)node);
	}
	else if (XMLString::compareString(node->getNodeName(), tableName) == 0)
	{
		child = new Table((DOMElement *)node);
	}
	else if (XMLString::compareString(node->getNodeName(), selectionName) == 0)
	{
		child = new Selection((DOMElement *)node);
	}
	else if (XMLString::compareString(node->getNodeName(), unionName) == 0)
	{
		child = new Union((DOMElement *)node);
	}
	else if (XMLString::compareString(node->getNodeName(), antijoinName) == 0)
	{
		child = new AntiJoin((DOMElement *)node);
	}
	return child;
}

UnaryAlgebraNodeBase::UnaryAlgebraNodeBase()
{
}

shared_ptr<AlgebraNodeBase> UnaryAlgebraNodeBase::replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild)
{
	if (child.get() == oldChild)
	{
		shared_ptr<AlgebraNodeBase> result=child;
		child = newChild;
		return result;
	}
	else
	{
		throw exception("child not found");
	}
}
UnaryAlgebraNodeBase::UnaryAlgebraNodeBase(DOMElement * element)
{
	parent = 0;
	DOMNode * inputNode = XmlUtils::GetChildElementByName(element, "input");
	for (XMLSize_t i = 0; i < inputNode->getChildNodes()->getLength(); ++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			child = shared_ptr<AlgebraNodeBase>(constructChildren((DOMElement*)node));
			child->parent = this;
		}
	}

}


BinaryAlgebraNodeBase::BinaryAlgebraNodeBase()
{

}

shared_ptr<AlgebraNodeBase> BinaryAlgebraNodeBase::replaceChild(AlgebraNodeBase * oldChild, shared_ptr<AlgebraNodeBase> & newChild)
{
	shared_ptr<AlgebraNodeBase> result;
	if (leftChild.get() == oldChild)
	{
		result = leftChild;
		leftChild = newChild;
	}
	else if (rightChild.get() == oldChild)
	{
		result = rightChild;
		rightChild = newChild;
	}
	else
	{
		throw exception("child not found");
	}
	return result;
}

BinaryAlgebraNodeBase::BinaryAlgebraNodeBase(DOMElement * element)
{
	bool leftChildInitialized = false;
	parent = 0;
	DOMNode * inputNode = XmlUtils::GetChildElementByName(element, "input");
	for (XMLSize_t i = 0; i < inputNode->getChildNodes()->getLength(); ++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			if (leftChildInitialized == false)
			{
				leftChild = shared_ptr<AlgebraNodeBase>(constructChildren((DOMElement*)node));
				leftChildInitialized = true;
				leftChild->parent = this;
			}
			else
			{
				rightChild = shared_ptr<AlgebraNodeBase>(constructChildren((DOMElement*)node));
				rightChild->parent = this;
			}
		}
	}
}

shared_ptr<AlgebraNodeBase> NullaryAlgebraNodeBase::replaceChild(AlgebraNodeBase * oldChild, shared_ptr<AlgebraNodeBase> & newChild)
{
	throw exception("not suported");
}

shared_ptr<AlgebraNodeBase> GroupedAlgebraNode::replaceChild(AlgebraNodeBase * oldChild, shared_ptr<AlgebraNodeBase> & newChild)
{
	shared_ptr<AlgebraNodeBase> result;
	for (auto it = children.begin(); it != children.end(); ++it)
	{
		if ((*it).get() == oldChild)
		{
			result = (*it);
			(*it) = newChild;
			return result;
		}
	}
	throw exception("fild not found");
}

Table::Table(DOMElement * element)
{
	name = XmlUtils::ReadAttribute(element, "name");
	if (XmlUtils::ReadAttribute(element, "numberOfRows") == "")
	{
		numberOfRows = 1000;
	}
	else
	{
		istringstream(XmlUtils::ReadAttribute(element, "numberOfRows")) >> numberOfRows;
	}
	vector<DOMElement *> parameters = XmlUtils::GetChildElements(element);
	for (auto it = parameters.begin(); it != parameters.end(); ++it)
	{
		if (XmlUtils::GetElementName(*it) == "column")
		{
			ColumnInfo info;
			info.type = XmlUtils::ReadAttribute(*it, "type");
			info.column = ColumnIdentifier(XmlUtils::ReadAttribute(*it, "name"));
			if (XmlUtils::ReadAttribute(*it, "number_of_unique_values") == "")
			{
				info.numberOfUniqueValues = pow(numberOfRows, 0.8);
			}
			else
			{
				istringstream(XmlUtils::ReadAttribute(*it, "number_of_unique_values")) >> info.numberOfUniqueValues;
				info.numberOfUniqueValues = min(info.numberOfUniqueValues, double(numberOfRows));
			}
			columns.push_back(info);
		}
		else if (XmlUtils::GetElementName(*it) == "index")
		{
			Index index;
			if (XmlUtils::ReadAttribute(*it, "type") == "clustered")
			{
				index.type = IndexType::CLUSTERED;
			}
			else
			{
				index.type = IndexType::UNCLUSTERED;
			}
			index.name = XmlUtils::ReadAttribute(*it, "name");
			vector<DOMElement *> columnsElement = XmlUtils::GetChildElements(*it);
			for (auto it2 = columnsElement.begin(); it2 != columnsElement.end(); ++it2)
			{
				SortParameter parameter;
				parameter.column = ColumnIdentifier(string(XmlUtils::ReadAttribute(*it2, "name")));
				if (XmlUtils::ReadAttribute(*it2, "order") == "asc")
					parameter.order = SortOrder::ASCENDING;
				else
					parameter.order = SortOrder::DESCENDING;

				index.columns.push_back(parameter);
			}

			indices.push_back(index);
		}
	}
}

void Table::accept(AlgebraVisitor &v)
{
	v.visitTable(this);
}

Sort::Sort(DOMElement * element) :UnaryAlgebraNodeBase(element)
{
	DOMNode * inputNode = XmlUtils::GetChildElementByName(element, "parameters");
	for (XMLSize_t i = 0; i < inputNode->getChildNodes()->getLength(); ++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			SortParameter parameter;
			DOMElement * parameterElement = (DOMElement *)node;
			parameter.column = ColumnIdentifier(XmlUtils::ReadAttribute(parameterElement, "column"));
			string direction = XmlUtils::ReadAttribute(parameterElement, "direction");
			if (direction == "asc")
			{
				parameter.order = ASCENDING;
			}
			else
			{
				parameter.order = DESCENDING;
			}
			parameters.push_back(parameter);
		}
	}
}

void Sort::accept(AlgebraVisitor &v)
{
	v.visitSort(this);
}

Group::Group(DOMElement * element) :UnaryAlgebraNodeBase(element)
{

	DOMNode * inputNode = XmlUtils::GetChildElementByName(element, "parameters");
	for (XMLSize_t i = 0; i < inputNode->getChildNodes()->getLength(); ++i)
	{
		DOMNode * node = inputNode->getChildNodes()->item(i);
		if (node->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			DOMElement * parameterElement = (DOMElement *)node;
			string elementName = XMLString::transcode(parameterElement->getLocalName());

			if (elementName == "group_by")
			{
				groupColumns.push_back(GroupColumn(ColumnIdentifier(XmlUtils::ReadAttribute(parameterElement, "column"))));
			}
			else
			{
				AgregateFunction function;
				function.functionName = elementName;
				if (elementName == "max")
				{
					function.parameter = ColumnIdentifier(XmlUtils::ReadAttribute(parameterElement, "argument"));
					function.function = AgregateFunctionType::MAX;
				}
				if (elementName == "min")
				{
					function.parameter = ColumnIdentifier(XmlUtils::ReadAttribute(parameterElement, "argument"));
					function.function = AgregateFunctionType::MIN;
				}
				if (elementName == "sum")
				{
					function.parameter = ColumnIdentifier(XmlUtils::ReadAttribute(parameterElement, "argument"));
					function.function = AgregateFunctionType::SUM;
				}
				if (elementName == "count")
				{
					function.function = AgregateFunctionType::COUNT;
				}
				function.output = ColumnIdentifier(XmlUtils::ReadAttribute(parameterElement, "output"));
				agregateFunctions.push_back(function);
			}
		}
	}
}

void Group::accept(AlgebraVisitor &v)
{
	v.visitGroup(this);
}

ColumnOperations::ColumnOperations(DOMElement * element) :UnaryAlgebraNodeBase(element)
{
	DOMElement * parametersNode = XmlUtils::GetChildElementByName(element, "parameters");
	vector<DOMElement *> columns = XmlUtils::GetChildElements(parametersNode);
	for (auto it = columns.begin(); it != columns.end(); ++it)
	{
		ColumnOperation op;
		op.result = ColumnIdentifier(XmlUtils::ReadAttribute(*it, "name"));
		if (XmlUtils::GetFirstChildElement(*it) != 0)
		{
			op.expression = shared_ptr<Expression>(Expression::constructChildren(XmlUtils::GetFirstChildElement(XmlUtils::GetFirstChildElement(*it))));
		}
		else
		{
			op.expression = shared_ptr<Expression>(0);
		}
		operations.push_back(op);
	}
}

void ColumnOperations::accept(AlgebraVisitor &v)
{
	v.visitColumnOperations(this);
}

Selection::Selection(DOMElement * element) :UnaryAlgebraNodeBase(element)
{
	DOMElement * parametersNode = XmlUtils::GetChildElementByName(element, "parameters");
	DOMElement * conditionNode = XmlUtils::GetChildElementByName(parametersNode, "condition");
	condition = shared_ptr<Expression>(Expression::constructChildren(XmlUtils::GetFirstChildElement(conditionNode)));

}

Selection::Selection(shared_ptr<Expression> & cond)
{
	condition = cond;

}

void Selection::accept(AlgebraVisitor &v)
{
	v.visitSelection(this);
}

void BinaryAlgebraNodeBase::constructJoinParameters(DOMElement * element, shared_ptr<Expression> & condition, vector<JoinColumnInfo> & outputColumns)
{
	DOMElement * parametersNode = XmlUtils::GetChildElementByName(element, "parameters");
	DOMElement * conditionNode = XmlUtils::GetFirstChildElement(parametersNode);
	int start = 0;
	if (XmlUtils::GetFirstChildElement(conditionNode) != 0)
	{
		start = 1;
		condition = shared_ptr<Expression>(Expression::constructChildren(XmlUtils::GetFirstChildElement(conditionNode)));
		condition->accept(NumberColumnsInJoinVisitor());
		vector<DOMElement *> conditions = XmlUtils::GetChildElements(conditionNode);
		for (auto it = conditions.begin() + 1; it != conditions.end(); ++it)
		{
			shared_ptr<Expression> newCondition(Expression::constructChildren(*it));
			newCondition->accept(NumberColumnsInJoinVisitor());
			condition = shared_ptr<Expression>(new BinaryExpression(condition, newCondition, BinaryOperator::AND));
		}
	}
	else
	{
		condition = 0;
	}
	vector<DOMElement *> columns = XmlUtils::GetChildElements(parametersNode);
	for (auto it = columns.begin() + start; it != columns.end(); ++it)
	{
		JoinColumnInfo info;
		info.newColumn = XmlUtils::ReadAttribute(*it, "newName");
		info.column = ColumnIdentifier(XmlUtils::ReadAttribute(*it, "name"));
		if (info.newColumn == "")
		{
			info.newColumn = info.column.name;
		}
		if (XmlUtils::ReadAttribute(*it, "input") == "second")
		{
			info.input = 1;
		}
		else
		{
			info.input = 0;
		}
		outputColumns.push_back(info);
	}

}
Join::Join(DOMElement * element) :BinaryAlgebraNodeBase(element)
{
	constructJoinParameters(element, condition, outputJoinColumns);
}

void Join::accept(AlgebraVisitor &v)
{
	v.visitJoin(this);
}

AntiJoin::AntiJoin(DOMElement * element) :BinaryAlgebraNodeBase(element)
{
	constructJoinParameters(element, condition, outputJoinColumns);
}

void AntiJoin::accept(AlgebraVisitor &v)
{
	v.visitAntiJoin(this);
}

Union::Union(DOMElement * element) :BinaryAlgebraNodeBase(element)
{

}

void Union::accept(AlgebraVisitor &v)
{
	v.visitUnion(this);
}

void GroupedJoin::accept(AlgebraVisitor &v)
{
	v.visitGroupedJoin(this);
}

