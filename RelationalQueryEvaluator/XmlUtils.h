#ifndef XmlUtilsHPP
#define XmlUtilsHPP

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
#include <vector>

XERCES_CPP_NAMESPACE_USE

class XmlUtils
{
public:
	static DOMNode * GetChildElementByName(DOMElement * element,const char * elementName)
	{
		XMLCh * name = XMLString::transcode(elementName);
		DOMNodeList * childs=element->getChildNodes();
		for(XMLSize_t i=0;i<childs->getLength();++i)
		{
			if(childs->item(i)->getNodeType() == DOMElement::ELEMENT_NODE)
			{
				if(XMLString::compareString(childs->item(i)->getNodeName(), name)==0)
				{
					return childs->item(i);
				}
			}
		}
		return 0;
	}

	static DOMElement * GetFirstChildElement(DOMElement * element)
	{
		DOMNodeList * childs=element->getChildNodes();
		for(XMLSize_t i=0;i<childs->getLength();++i)
		{
			if(childs->item(i)->getNodeType() == DOMElement::ELEMENT_NODE)
			{
				return (DOMElement *)childs->item(i);
			}
		}
		return 0;
	}

	static std::vector<DOMElement *> GetChildElements(DOMElement * element)
	{
		std::vector<DOMElement *> result;
		DOMNodeList * childs=element->getChildNodes();
		for(XMLSize_t i=0;i<childs->getLength();++i)
		{
			if(childs->item(i)->getNodeType() == DOMElement::ELEMENT_NODE)
			{
				result.push_back((DOMElement *)childs->item(i));
			}
		}
		return result;
	}

	static std::string ReadAttribute(DOMElement * element,const char * atribute)
	{
		return XMLString::transcode(element->getAttribute(XMLString::transcode(atribute)));
	}
};
#endif