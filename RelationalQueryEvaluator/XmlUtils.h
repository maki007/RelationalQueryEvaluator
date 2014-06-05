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
	static DOMElement * GetChildElementByName(DOMElement * element, const char * elementName);

	static DOMElement * GetFirstChildElement(DOMElement * element);

	static std::vector<DOMElement *> GetChildElements(DOMElement * element);

	static std::string ReadAttribute(DOMElement * element, const char * atribute);

	static std::string GetElementName(DOMElement * element);

};
#endif