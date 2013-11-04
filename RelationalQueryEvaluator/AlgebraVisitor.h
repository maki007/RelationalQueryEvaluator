#ifndef AlgebraVisitorHPP
#define AlgebraVisitorHPP

#include "Algebra.h"

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

	virtual void visit(Table * node)=0;

	virtual void visit(Sort * node)=0;

	virtual void visit(Group * node)=0;

	virtual void visit(ColumnOperations * node)=0;

	virtual void visit(Selection * node)=0;

	virtual void visit(Join * node)=0;

	virtual void visit(AntiJoin * node)=0;

	virtual void visit(Difference * node)=0;

	virtual void visit(Union * node)=0;

	virtual void visit(Intersection * node)=0;


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
		std::string label="Sort";
		if(node->parameters.size()!=0)
		{
			label+="\n";
		}
		for(auto it=node->parameters.begin();it!=node->parameters.end();++it)
		{
			label+=it->column;
			if(it->ascending)
			{
				label+=" asc";
			}
			else
			{
				label+=" desc";
			}
			if(it!=node->parameters.end()-1)
				label+=", ";
		}
		generateText(label.c_str(),node);

		result+="\n}";
	}

	void visit(Group * node)
	{
		std::string label="Group\n";
		label+="groupBy ";
		for(auto it=node->groupColumns.begin();it!=node->groupColumns.end();++it)
		{
			label+=*it;
			label+=", ";
		}
		for(auto it=node->agregateFunctions.begin();it!=node->agregateFunctions.end();++it)
		{
			label+=it->output;
			label+="=";
			label+=it->functionName;
			label+="("+it->parameter+")";
			label+=";";
		}

		generateText(label,node);
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

	void visit(Selection * node)
	{
		generateText("Selection",node);
	}

	void visit(Join * node)
	{
		generateText("Join",node);
	}

	void visit(AntiJoin * node)
	{
		generateText("AntiJoin",node);
	}
	void visit(Difference * node)
	{
		generateText("Difference",node);
	}

	void visit(Union * node)
	{
		generateText("Union",node);
	}

	void visit(Intersection * node)
	{
		generateText("Intersection",node);
	}

};

#endif