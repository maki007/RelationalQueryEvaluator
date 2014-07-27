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

namespace rafe {

	/**
	* Static class providing methods for helping reading DOM tree.
	*/
	class XmlUtils
	{
	public:

		/**
		* Returns elemenet child, which name matches given value.
		* @param element - element which child will be returned 
		* @param elementName - child element name
		* @returns child element matching elementName
		*/
		static DOMElement * GetChildElementByName(DOMElement * element, const char * elementName);

		/**
		* Returns first child element from given element.
		* @param element - element node
		* @returns first child element from given element
		*/
		static DOMElement * GetFirstChildElement(DOMElement * element);

		/**
		* Return all child elements of element node.
		* @param element - element node
		* @returns all child elements
		*/
		static std::vector<DOMElement *> GetChildElements(DOMElement * element);

		/**
		* Reads atrribute and returns attribute values as string.
		* @param element - dom element
		* @param attribute - attribute name
		* @returns attribude value.
		*/
		static std::string ReadAttribute(DOMElement * element, const char * attribute);

		/**
		* Return element name as string.
		* @param element
		* @returns element name as string.
		*/
		static std::string GetElementName(DOMElement * element);

	};
};
#endif