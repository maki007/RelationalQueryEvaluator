#include "XmlUtils.h"



DOMElement * XmlUtils::GetChildElementByName(DOMElement * element, const char * elementName)
{
	XMLCh * name = XMLString::transcode(elementName);
	DOMNodeList * childs = element->getChildNodes();
	for (XMLSize_t i = 0; i < childs->getLength(); ++i)
	{
		if (childs->item(i)->getNodeType() == DOMElement::ELEMENT_NODE)
		{
			if (XMLString::compareString(childs->item(i)->getNodeName(), name) == 0)
			{
				return (DOMElement *)(childs->item(i));
			}
		}
	}
	return 0;
}

DOMElement * XmlUtils::GetFirstChildElement(DOMElement * element)
{
	DOMNodeList * childs = element->getChildNodes();
	for (XMLSize_t i = 0; i < childs->getLength(); ++i)
	{
		if (childs->item(i)->getNodeType() == DOMElement::ELEMENT_NODE)
		{
			return (DOMElement *)childs->item(i);
		}
	}
	return 0;
}

std::vector<DOMElement *> XmlUtils::GetChildElements(DOMElement * element)
{
	std::vector<DOMElement *> result;
	DOMNodeList * childs = element->getChildNodes();
	for (XMLSize_t i = 0; i < childs->getLength(); ++i)
	{
		if (childs->item(i)->getNodeType() == DOMElement::ELEMENT_NODE)
		{
			result.push_back((DOMElement *)childs->item(i));
		}
	}
	return result;
}

std::string XmlUtils::ReadAttribute(DOMElement * element, const char * attribute)
{
	return XMLString::transcode(element->getAttribute(XMLString::transcode(attribute)));
}

std::string XmlUtils::GetElementName(DOMElement * element)
{
	return std::string(XMLString::transcode(element->getNodeName()));
}
