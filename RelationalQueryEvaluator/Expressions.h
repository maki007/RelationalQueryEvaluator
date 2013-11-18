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

enum UnaryOperator
{
	NOT
};
enum BinaryOperator
{
	AND,OR,PLUS,MINUS,TIMES,DIVIDE,EQUALS,NOT_EQUALS,LOWER,LOWER_OR_EQUAL
};

class ExpressionVisitorBase;

class Expression
{
public:
	virtual void accept(ExpressionVisitorBase &v)=0;
	static Expression * constructChildren(DOMElement * node);
};
class UnaryExpression : public Expression
{
public:
	UnaryOperator operation;
	std::shared_ptr<Expression> child;
	UnaryExpression(DOMElement * node,UnaryOperator op);
	UnaryExpression(std::shared_ptr<Expression> node,UnaryOperator op);
	void accept(ExpressionVisitorBase &v);
};

class BinaryExpression : public Expression
{
public:
	BinaryOperator operation;
	std::shared_ptr<Expression> leftChild;
	std::shared_ptr<Expression> rightChild;
	BinaryExpression(DOMElement * node,BinaryOperator op);
	BinaryExpression(std::shared_ptr<Expression> leftChild,std::shared_ptr<Expression> rightChild,BinaryOperator op);
	void accept(ExpressionVisitorBase &v);
};

class NnaryExpression : public Expression
{
public:
	std::string name;
	std::vector<std::shared_ptr<Expression>> arguments;
	NnaryExpression(DOMElement * node);
	void accept(ExpressionVisitorBase &v);
};

class Constant : public Expression
{
public:
	std::string value;
	std::string type;
	Constant(DOMElement * node);
	void accept(ExpressionVisitorBase &v);
};

class Column : public Expression
{
public:
	std::string name;
	std::string tableName;
	std::string type;
	Column(DOMElement * node);
	void accept(ExpressionVisitorBase &v);
};

#endif