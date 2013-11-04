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

#include <iostream>
#include <stdio.h>



#include "Algebra.h"

XERCES_CPP_NAMESPACE_USE

class WStr
{
private:
	XMLCh*  wStr;

public:
	WStr(const char* str)
	{
		wStr = XMLString::transcode(str);
	}

	~WStr()
	{
		XMLString::release(&wStr);
	}

	operator const XMLCh*() const
	{
		return wStr;
	}
};

class ParserErrorHandler : public ErrorHandler
{
private:
	void reportParseException(const SAXParseException& ex)
	{
		errors++;
		char* msg = XMLString::transcode(ex.getMessage());
		fprintf(stderr, "at line %llu column %llu, %s\n", 
			ex.getColumnNumber(), ex.getLineNumber(), msg);
		XMLString::release(&msg);
	}

public:
	int errors;

	ParserErrorHandler()
	{
		errors = 0;
	}

	void warning(const SAXParseException& ex)
	{
		reportParseException(ex);
	}

	void error(const SAXParseException& ex)
	{
		reportParseException(ex);
	}

	void fatalError(const SAXParseException& ex)
	{
		reportParseException(ex);
	}

	void resetErrors()
	{
		errors = 0;
	}
};
class XmlHandler
{
public:
	static std::unique_ptr<AlgebraNodeBase>   ValidateSchema(const char* xmlFilePath)
	{
		XercesDOMParser domParser;

		ParserErrorHandler parserErrorHandler;

		domParser.setErrorHandler(&parserErrorHandler);
		domParser.setValidationScheme(XercesDOMParser::Val_Auto);
		domParser.setDoNamespaces(true);
		domParser.setDoSchema(true);
		domParser.setValidationConstraintFatal(true);

		domParser.parse(xmlFilePath);
		if (domParser.getErrorCount() == 0 && parserErrorHandler.errors == 0)
		{
			printf("XML file ");
			printf(xmlFilePath);
			printf(" validated against the schema successfully\n");
			
			DOMDocument * xmlDoc = domParser.getDocument();
			DOMElement* elementRoot = xmlDoc->getDocumentElement();
			return std::unique_ptr<AlgebraNodeBase>(new Sort(elementRoot));
		}
		else
		{
			printf("XML file ");
			printf(xmlFilePath);
			printf("  doesn't conform to the schema\n");
		}
		return 0;
	}

	static std::unique_ptr<AlgebraNodeBase> GenerateRelationalAlgebra(const char *filename)
	{
		XMLPlatformUtils::Initialize();
		return ValidateSchema(filename);
		XMLPlatformUtils::Terminate();
	}
};
#endif