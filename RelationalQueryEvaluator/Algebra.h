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

class SortParameter
{
public: 
	std::string column;
	bool ascending;
};
enum AgregateFunction
{
	SUM,MIN,MAX,COUNT
};

class AgregateFunctionInfo
{
public:
	AgregateFunction function;
	std::string functionName;
	std::string parameter;
	std::string output;
};

class ColumnInfo
{
	std::string name;
	ulong numberOfUniqueValues;
};

//todo indexes
class IndexInfo
{

};

class ColumnOperation
{
	std::string result;
	std::weak_ptr <Expression> expression;
	std::string type;
};



//nodes
class AlgebraNodeBase
{
public:
	std::shared_ptr <AlgebraNodeBase> parent;
	AlgebraNodeBase();
	AlgebraNodeBase * constructChildren(DOMElement * node);
	virtual void accept(AlgebraVisitor &v) = 0;
};

class UnaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	std::shared_ptr <AlgebraNodeBase> child;
	UnaryAlgebraNodeBase(DOMElement * element);
	UnaryAlgebraNodeBase();	
	virtual void accept(AlgebraVisitor &v) = 0;
};

class BinaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	std::shared_ptr <AlgebraNodeBase> leftChild;
	std::shared_ptr <AlgebraNodeBase> rightChild;

	BinaryAlgebraNodeBase(DOMElement * element);
	BinaryAlgebraNodeBase();
	virtual void accept(AlgebraVisitor &v) = 0;
};

class Table : public AlgebraNodeBase
{
public:
	std::string name;
	std::vector<ColumnInfo> columns;
	ulong numberOfRows;
	std::vector<IndexInfo> indices;
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
	std::vector<std::string> groupColumns;
	std::vector<AgregateFunctionInfo> agregateFunctions;
	Group(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class ColumnOperations: public UnaryAlgebraNodeBase
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
	void accept(AlgebraVisitor &v);
};

class Join : public BinaryAlgebraNodeBase
{
public:
	std::shared_ptr<Expression> condition;
	Join(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class AntiJoin : public BinaryAlgebraNodeBase
{
public:
	std::shared_ptr<Expression> condition;
	AntiJoin(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Union : public BinaryAlgebraNodeBase
{
public:
	Union(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Difference : public BinaryAlgebraNodeBase
{
public:
	Difference(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Intersection : public BinaryAlgebraNodeBase
{
public:
	Intersection(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

#endif


