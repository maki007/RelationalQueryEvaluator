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
#include <string>
#include <sstream>
#include "XmlUtils.h"


XERCES_CPP_NAMESPACE_USE

#ifndef AlgebraHPP
#define AlgebraHPP

class AlgebraVisitor;

class AlgebraNodeBase
{
public:
	AlgebraNodeBase();
	AlgebraNodeBase * ConstructChildren(DOMElement * node);
	virtual void accept(AlgebraVisitor &v) = 0;
};

class UnaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	AlgebraNodeBase * child;

	UnaryAlgebraNodeBase(DOMElement * element);
	UnaryAlgebraNodeBase();
	~UnaryAlgebraNodeBase();	
	virtual void accept(AlgebraVisitor &v) = 0;
};

class BinaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	AlgebraNodeBase * leftChild;
	AlgebraNodeBase * rightChild;

	BinaryAlgebraNodeBase(DOMElement * element);
	BinaryAlgebraNodeBase();
	~BinaryAlgebraNodeBase();
	virtual void accept(AlgebraVisitor &v) = 0;
};

class Table : public AlgebraNodeBase
{
public:
public:
	Table(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Sort : public UnaryAlgebraNodeBase
{
public:
	Sort(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Group : public UnaryAlgebraNodeBase
{
public:
	Group(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class ColumnOperations: public UnaryAlgebraNodeBase
{
public:
	ColumnOperations(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Join : public BinaryAlgebraNodeBase
{
public:
	Join(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class CrossJoin : public BinaryAlgebraNodeBase
{
public:
	CrossJoin(DOMElement * element);
	void accept(AlgebraVisitor &v);
};



class AntiJoin : public BinaryAlgebraNodeBase
{
public:
	AntiJoin(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

class Selection : public UnaryAlgebraNodeBase
{
public:
	Selection(DOMElement * element);
	void accept(AlgebraVisitor &v);
};



class AlgebraVisitor
{
public:
	virtual void visit(AlgebraNodeBase * node)
	{
		node->accept(*this);
	}

	virtual void visit(UnaryAlgebraNodeBase * node)
	{
		node->accept(*this);
	}

	virtual void visit(BinaryAlgebraNodeBase * node)
	{
		node->accept(*this);
	}

	virtual void visit(Table * node)
	{

	}

	virtual void visit(Sort * node)
	{
		node->child->accept(*this);
	}

	virtual void visit(Group * node)
	{
		node->child->accept(*this);
	}

	virtual void visit(ColumnOperations * node)
	{
		node->child->accept(*this);
	}

	virtual void visit(Join * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	virtual void visit(CrossJoin * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	virtual void visit(AntiJoin * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	virtual void visit(Selection * node)
	{
		node->child->accept(*this);
	}



};

class GraphDrawingVisitor : public AlgebraVisitor
{
public:
	std::string result;
	int nodeCounter;
	GraphDrawingVisitor()
	{
		result="";
		nodeCounter=0;
	}

	void generateText(std::string label ,UnaryAlgebraNodeBase * node)
	{
		int identifier=nodeCounter;

		result.append("node");
		result.append(std::to_string(nodeCounter));
		result.append("[label=\""+label+"\"]\n");
		int childIdentifier=++nodeCounter;
		node->child->accept(*this);

		result.append("node");
		result.append(std::to_string(childIdentifier));
		result.append(" -> node");
		result.append(std::to_string(identifier));
		result.append("[headport=s, tailport=n,label=\"   \"]\n");

	}

	void generateText(std::string label , BinaryAlgebraNodeBase * node)
	{
		int identifier=nodeCounter;

		result.append("node");
		result.append(std::to_string(nodeCounter));
		result.append("[label=\""+label+"\"]\n");

		int childIdentifier=++nodeCounter;
		node->leftChild->accept(*this);
		result.append("node");
		result.append(std::to_string(childIdentifier));
		result.append(" -> node");
		result.append(std::to_string(identifier));
		result.append("[headport=s, tailport=n,label=\"   \"]\n");

		childIdentifier=++nodeCounter;
		node->rightChild->accept(*this);
		result.append("node");
		result.append(std::to_string(childIdentifier));
		result.append(" -> node");
		result.append(std::to_string(identifier));
		result.append("[headport=s, tailport=n,label=\"   \"]\n");

	}

	void visit(Sort * node)
	{
		result="digraph g {node [shape=box]\n graph[rankdir=\"BT\", concentrate=true];\n";
		generateText("Sort",node);
		result+="\n}";
	}

	void visit(Group * node)
	{
		generateText("Group",node);
	}

	void visit(Table * node)
	{
		result.append("node");
		result.append(std::to_string(nodeCounter));
		result.append("[label=\"Table\"]\n");
	}

	void visit(ColumnOperations * node)
	{
		generateText("ColumnOperations",node);
	}
	void visit(Join * node)
	{
		generateText("Join",node);
	}

	void visit(AntiJoin * node)
	{
		generateText("AntiJoin",node);
	}

	void visit(Selection * node)
	{
		generateText("Selection",node);
	}

	void visit(CrossJoin * node)
	{
		generateText("Cross Join",node);
	}

};
#endif


