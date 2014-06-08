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

class WStr
{
private:
	XMLCh*  wStr;

public:
	WStr(const char* str);

	~WStr();

	operator const XMLCh*() const;
};

class ParserErrorHandler : public ErrorHandler
{
public:
	int errors;

	ParserErrorHandler();

	void reportParseException(const SAXParseException& ex);

	void warning(const SAXParseException& ex);

	void error(const SAXParseException& ex);

	void fatalError(const SAXParseException& ex);

	void resetErrors();

	void setDocumentLocator(const Locator *const locator);
};

class XmlHandler
{
public:
	static std::unique_ptr<AlgebraNodeBase> ValidateSchema(const char* xmlFilePath);

	static std::unique_ptr<AlgebraNodeBase> GenerateRelationalAlgebra(const char *filename);

};





class DOMParser : public XercesDOMParser
{
public:
	void startElement(const XMLElementDecl& elemDecl, const unsigned int urlId, const XMLCh* const elemPrefix, const RefVectorOf<XMLAttr>& attrList, const XMLSize_t attrCount, const bool isEmpty, const bool isRoot)
	{
		XercesDOMParser::startElement(elemDecl, urlId, elemPrefix, attrList,
			attrCount, isEmpty, isRoot);
		const Locator* locator = getScanner()->getLocator();
		int lineNumber = locator->getLineNumber();
		XercesDOMParser::fCurrentNode->setUserData(XMLString::transcode("line"), new int(locator->getLineNumber()),0);
	}

};

#endif

