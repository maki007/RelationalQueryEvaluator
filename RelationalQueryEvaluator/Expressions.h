#ifndef ExpressionHPP
#define ExpressionHPP
#include <memory>
#include <vector>
#include <string>

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

#include "XmlUtils.h"


XERCES_CPP_NAMESPACE_USE

typedef unsigned long long int ulong;

enum class UnaryOperator
{
	NOT
};
enum class BinaryOperator
{
	AND,OR,PLUS,MINUS,TIMES,DIVIDE,EQUALS,NOT_EQUALS,LOWER,LOWER_OR_EQUAL
};
enum class GroupedOperator
{
	AND,OR
};

class ColumnIdentifier
{
public:
	std::string name;
	int id;
	ColumnIdentifier(std::string name, int id)
	{
		this->name = name;
		this->id = id;
	}

	ColumnIdentifier(std::string name)
	{
		this->name = name;
		this->id = -1;
	}

	ColumnIdentifier()
	{
		this->name = "";
		this->id = -1;
	}
	std::string toString()
	{
		return name + "_" + std::to_string(id);
	}
};
class ExpressionVisitorBase;

class Expression
{
public:
	Expression * parent;
	virtual void accept(ExpressionVisitorBase &v)=0;
	static Expression * constructChildren(DOMElement * node);
	virtual void replaceChild(Expression * oldChild,Expression * newChild) = 0;
};
class UnaryExpression : public Expression
{
public:
	UnaryOperator operation;
	std::shared_ptr<Expression> child;
	UnaryExpression(DOMElement * node,UnaryOperator op);
	UnaryExpression(std::shared_ptr<Expression> node,UnaryOperator op);
	void accept(ExpressionVisitorBase &v);
	void replaceChild(Expression * oldChild,Expression * newChild);
};

class BinaryExpression : public Expression
{
public:
	BinaryOperator operation;
	std::shared_ptr<Expression> leftChild;
	std::shared_ptr<Expression> rightChild;
	BinaryExpression(DOMElement * node,BinaryOperator op);
	BinaryExpression(std::shared_ptr<Expression> & leftChild, std::shared_ptr<Expression> & rightChild, BinaryOperator op);
	void accept(ExpressionVisitorBase &v);
	void replaceChild(Expression * oldChild,Expression * newChild);
};

class NnaryExpression : public Expression
{
public:
	std::string name;
	std::vector<std::shared_ptr<Expression>> arguments;
	NnaryExpression(DOMElement * node);
	void accept(ExpressionVisitorBase &v);
	void replaceChild(Expression * oldChild,Expression * newChild);
};


class Constant : public Expression
{
public:
	std::string value;
	std::string type;
	Constant(DOMElement * node);
	void accept(ExpressionVisitorBase &v);
	void replaceChild(Expression * oldChild,Expression * newChild);
};

class Column : public Expression
{
public:
	ColumnIdentifier column;
	std::string tableName;
	std::string type;
	ulong input;
	Column(DOMElement * node);
	void accept(ExpressionVisitorBase &v);
	void replaceChild(Expression * oldChild,Expression * newChild);
};

class GroupedExpression : public Expression
{
public:
	GroupedOperator operation;
	std::vector<std::shared_ptr<Expression>> children;
	GroupedExpression();
	GroupedExpression(GroupedOperator operation, const std::vector<std::shared_ptr<Expression>> & children);
	void accept(ExpressionVisitorBase &v);
	void replaceChild(Expression * oldChild,Expression * newChild);
};

#endif