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

XERCES_CPP_NAMESPACE_USE

#ifndef AlgebraHPP
#define AlgebraHPP

class AlgebraNodeBase
{
public:
	AlgebraNodeBase()
	{
	
	}
};

class UnaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	AlgebraNodeBase * child;
	~UnaryAlgebraNodeBase()
	{
		
	}

};

class BinaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	AlgebraNodeBase * leftChild;
	AlgebraNodeBase * rightChild;
	~BinaryAlgebraNodeBase()
	{
		
	}
};

class Table : public AlgebraNodeBase
{
public:
};

class Sort : public UnaryAlgebraNodeBase
{
public:
	Sort(DOMElement * element)
	{
	
	}
};

class Group : public UnaryAlgebraNodeBase
{
public:
};

class ColumnOperations: public UnaryAlgebraNodeBase
{
public:
};

class Join : public BinaryAlgebraNodeBase
{
public:
};

class AntiJoin : public BinaryAlgebraNodeBase
{
public:
};

class Selection : public UnaryAlgebraNodeBase
{
public:
};


#endif