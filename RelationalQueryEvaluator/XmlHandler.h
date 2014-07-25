#ifndef XmlHandlerHPP
#define XmlHandlerHPP

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
#include <xercesc/internal/XMLScanner.hpp>

#include <iostream>
#include <stdio.h>



#include "Algebra.h"

XERCES_CPP_NAMESPACE_USE

namespace rafe {

	/**
	* Dom parser error handler.
	*/
	class ParserErrorHandler : public ErrorHandler
	{
	public:
		int errors; /**< Stores number of errors. */

		/**
		* Creates new instance of ParserErrorHandler.
		*/
		ParserErrorHandler();

		/**
		* Reports SAXParseException.
		* @param ex - reported exception
		*/
		void reportParseException(const SAXParseException& ex);

		/**
		* Reports warning.
		* @param ex - reported warning.
		*/
		void warning(const SAXParseException& ex);

		/**
		* Reports error.
		* @param ex - reported error.
		*/
		void error(const SAXParseException& ex);

		/**
		* Reports fatal error.
		* @param ex - reported error.
		*/
		void fatalError(const SAXParseException& ex);

		/**
		* Resets reported errors.
		*/
		void resetErrors();

	};

	/**
	* Static class for generating relation algebra from DOM XML.
	*/
	class XmlHandler
	{
	public:

		/**
		* Generates relation algebra tree from dom xml. Xml will be valiadated.
		* Does not initlialize or cleans dom parser. This is done in GenerateRelationalAlgebra.
		* @param xmlFilePath - name of file containing xml
		* @return root of algebra tree
		*/
		static std::unique_ptr<AlgebraNodeBase> ValidateSchema(const char* xmlFilePath);

		/**
		* Generates relation algebra tree from dom xml. Xml will be valiadated.
		* @param filename - name of file containing xml
		* @return root of algebra tree
		*/
		static std::unique_ptr<AlgebraNodeBase> GenerateRelationalAlgebra(const char *filename);
	};

	/**
	* Custom DOM parser which creates dom and adds information about line number to every element.
	*/
	class DOMParser : public XercesDOMParser
	{
	public:
		/**
		* Overriden method startElement, where parser adds information about line number to processed element.
		*/
		void startElement(const XMLElementDecl& elemDecl, const unsigned int urlId, const XMLCh* const elemPrefix, const RefVectorOf<XMLAttr>& attrList, const XMLSize_t attrCount, const bool isEmpty, const bool isRoot);


	};
};
#endif

