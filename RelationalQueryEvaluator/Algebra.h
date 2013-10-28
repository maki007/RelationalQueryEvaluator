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

	UnaryAlgebraNodeBase(DOMElement * element)
	{


		XMLCh *input = XMLString::transcode("input");

		DOMNodeList * childs=element->getChildNodes();
		for(XMLSize_t i=0;i<childs->getLength();++i)
		{
			if(childs->item(i)->getNodeType() == DOMElement::ELEMENT_NODE)
			{
				if(XMLString::compareString(childs->item(i)->getNodeName(), input)==0)
				{
					DOMNodeList * inputChildren = childs->item(i)->getChildNodes();
					for(XMLSize_t j=0;j<inputChildren->getLength();++j)
					{
						if(inputChildren->item(j)->getNodeType() == DOMElement::ELEMENT_NODE)
						{
							const XMLCh * elementName = inputChildren->item(j)->getNodeName();
							if(XMLString::compareString(elementName,XMLString::transcode("group"))==0)
							{
								std::cout << "ok" << std::endl;
							}
						}
					}
				}
			}
		}
	}

	~UnaryAlgebraNodeBase()
	{
		delete child;
	}
	
};

class BinaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	AlgebraNodeBase * leftChild;
	AlgebraNodeBase * rightChild;

	BinaryAlgebraNodeBase(DOMElement * element)
	{
	
	}

	~BinaryAlgebraNodeBase()
	{
		delete leftChild;
		delete rightChild;
	}
};

class Table : public AlgebraNodeBase
{
public:
};

class Sort : public UnaryAlgebraNodeBase
{
public:
	Sort(DOMElement * element) : UnaryAlgebraNodeBase(element)
	{

	}
};

class Group : public UnaryAlgebraNodeBase
{
public:
	Group(DOMElement * element) : UnaryAlgebraNodeBase(element)
	{
		
	}
};

class ColumnOperations: public UnaryAlgebraNodeBase
{
public:
	ColumnOperations(DOMElement * element) : UnaryAlgebraNodeBase(element)
	{
		
	}
};

class Join : public BinaryAlgebraNodeBase
{
public:
	Join(DOMElement * element)  : BinaryAlgebraNodeBase(element)
	{
		
	}
};

class AntiJoin : public BinaryAlgebraNodeBase
{
public:
	AntiJoin(DOMElement * element) : BinaryAlgebraNodeBase(element)
	{
		
	}
};

class Selection : public UnaryAlgebraNodeBase
{
public:
	Selection(DOMElement * element) : UnaryAlgebraNodeBase(element)
	{
		
	}
};


#endif