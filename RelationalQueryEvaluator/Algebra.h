#ifndef AlgebraHPP
#define AlgebraHPP

#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/validators/common/Grammar.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include "XmlUtils.h"
#include "Expressions.h"
#include <memory>

XERCES_CPP_NAMESPACE_USE

typedef unsigned long long int ulong;
typedef unsigned int uint;

class AlgebraVisitor;


//nodes
class AlgebraNodeBase
{
public:
	AlgebraNodeBase * parent;
	AlgebraNodeBase();
	AlgebraNodeBase * constructChildren(DOMElement * node);
	virtual void accept(AlgebraVisitor &v) = 0;
	virtual void replaceChild(AlgebraNodeBase * oldChild, AlgebraNodeBase * newChild) = 0;
};

class UnaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	std::shared_ptr <AlgebraNodeBase> child;
	UnaryAlgebraNodeBase(DOMElement * element);
	UnaryAlgebraNodeBase();
	virtual void accept(AlgebraVisitor &v) = 0;
	void replaceChild(AlgebraNodeBase * oldChild, AlgebraNodeBase * newChild);
};

class BinaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	std::shared_ptr <AlgebraNodeBase> leftChild;
	std::shared_ptr <AlgebraNodeBase> rightChild;

	BinaryAlgebraNodeBase(DOMElement * element);
	BinaryAlgebraNodeBase();
	virtual void accept(AlgebraVisitor &v) = 0;
	void constructJoinParameters(DOMElement * node, std::shared_ptr<Expression> & condition, std::vector<JoinColumnInfo> & outputColumns);
	void replaceChild(AlgebraNodeBase * oldChild, AlgebraNodeBase * newChild);
};

class GroupedAlgebraNode : public AlgebraNodeBase
{
public:
	std::vector<std::shared_ptr<AlgebraNodeBase>> children;
	virtual void accept(AlgebraVisitor &v) = 0;
	void replaceChild(AlgebraNodeBase * oldChild, AlgebraNodeBase * newChild);
};

class NullaryAlgebraNodeBase : public AlgebraNodeBase
{
public:

	virtual void accept(AlgebraVisitor &v) = 0;
	void replaceChild(AlgebraNodeBase * oldChild, AlgebraNodeBase * newChild);
};

class Table : public NullaryAlgebraNodeBase
{
public:
	std::string name;
	std::vector<ColumnInfo> columns;
	double numberOfRows;
	std::vector<Index> indices;
	Table(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Sort : public UnaryAlgebraNodeBase
{
public:
	std::vector<SortParameter> parameters;
	Sort(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Group : public UnaryAlgebraNodeBase
{
public:
	std::vector<GroupColumn> groupColumns;
	std::vector<AgregateFunction> agregateFunctions;
	Group(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class ColumnOperations : public UnaryAlgebraNodeBase
{
public:
	std::vector<ColumnOperation> operations;
	ColumnOperations(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Selection : public UnaryAlgebraNodeBase
{
public:
	std::shared_ptr<Expression> condition;
	Selection(DOMElement * element);
	Selection(std::shared_ptr<Expression> & cond);
	void accept(AlgebraVisitor &v);
};

class Join : public BinaryAlgebraNodeBase
{
public:
	std::shared_ptr<Expression> condition;
	std::vector<JoinColumnInfo> outputColumns;
	Join(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class AntiJoin : public BinaryAlgebraNodeBase
{
public:
	std::shared_ptr<Expression> condition;
	std::vector<JoinColumnInfo> outputColumns;
	AntiJoin(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Union : public BinaryAlgebraNodeBase
{
public:
	Union(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class GroupedJoin : public GroupedAlgebraNode
{
public:
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Expression> nonJoincondition;
	std::vector<JoinColumnInfo> outputColumns;
	void accept(AlgebraVisitor &v);
};

#endif


