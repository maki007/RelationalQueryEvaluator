#include "XmlHandler.h"

namespace rafe {

	void ParserErrorHandler::reportParseException(const SAXParseException& ex)
	{
		errors++;
		char* msg = XMLString::transcode(ex.getMessage());
		fprintf(stderr, "at line %llu column %llu, %s\n",
			ex.getColumnNumber(), ex.getLineNumber(), msg);
		XMLString::release(&msg);
	}

	ParserErrorHandler::ParserErrorHandler()
	{
		errors = 0;
	}

	void ParserErrorHandler::warning(const SAXParseException& ex)
	{
		reportParseException(ex);
	}

	void ParserErrorHandler::error(const SAXParseException& ex)
	{
		reportParseException(ex);
	}

	void ParserErrorHandler::fatalError(const SAXParseException& ex)
	{
		reportParseException(ex);
	}

	void ParserErrorHandler::resetErrors()
	{
		errors = 0;
	}


	std::unique_ptr<AlgebraNodeBase> XmlHandler::ValidateSchema(const char* xmlFilePath)
	{
		DOMParser domParser;

		ParserErrorHandler parserErrorHandler;

		domParser.setErrorHandler(&parserErrorHandler);
		domParser.setValidationScheme(XercesDOMParser::Val_Always);
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
			Sort * root = new Sort(elementRoot);
			return std::unique_ptr<AlgebraNodeBase>(root);
		}
		else
		{
			printf("XML file ");
			printf(xmlFilePath);
			printf("  doesn't conform to the schema\n");
		}
		return 0;
	}

	std::unique_ptr<AlgebraNodeBase> XmlHandler::GenerateRelationalAlgebra(const char *filename)
	{
		XMLPlatformUtils::Initialize();
		std::unique_ptr<AlgebraNodeBase> result = ValidateSchema(filename);
		XMLPlatformUtils::Terminate();
		return result;
	}

	void DOMParser::startElement(const XMLElementDecl& elemDecl, const unsigned int urlId, const XMLCh* const elemPrefix, const RefVectorOf<XMLAttr>& attrList, const XMLSize_t attrCount, const bool isEmpty, const bool isRoot)
	{
		XercesDOMParser::startElement(elemDecl, urlId, elemPrefix, attrList,
			attrCount, isEmpty, isRoot);
		const Locator* locator = getScanner()->getLocator();
		ulong lineNumber = locator->getLineNumber();
		XercesDOMParser::fCurrentNode->setUserData(XMLString::transcode("line"), new ulong(locator->getLineNumber()), 0);
	}

};