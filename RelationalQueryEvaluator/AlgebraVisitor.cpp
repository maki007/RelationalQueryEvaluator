

#include "Algebra.h"
#include "AlgebraVisitor.h"


	void AlgebraVisitor::visit(AlgebraNodeBase * node)
	{
		node->accept(*this);
	}

	void  AlgebraVisitor::visit(UnaryAlgebraNodeBase * node)
	{
		node->accept(*this);
	}

	void  AlgebraVisitor::visit(BinaryAlgebraNodeBase * node)
	{
		node->accept(*this);
	}

	void  AlgebraVisitor::visit(Table * node){}

	void  AlgebraVisitor::visit(Sort * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visit(Group * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visit(ColumnOperations * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visit(Selection * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visit(Join * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	void  AlgebraVisitor::visit(AntiJoin * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}
	
	void  AlgebraVisitor::visit(Difference * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	void  AlgebraVisitor::visit(Union * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	void  AlgebraVisitor::visit(Intersection * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}


	GraphDrawingVisitor::GraphDrawingVisitor()
	{
		result="";
		nodeCounter=0;
	}

	void GraphDrawingVisitor::generateText(std::string label ,UnaryAlgebraNodeBase * node)
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

	void GraphDrawingVisitor::generateText(std::string label , BinaryAlgebraNodeBase * node)
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

	void GraphDrawingVisitor::visit(Sort * node)
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

	void GraphDrawingVisitor::visit(Group * node)
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

	void GraphDrawingVisitor::visit(Table * node)
	{
		result.append("node");
		result.append(std::to_string(nodeCounter));
		result.append("[label=\"Table\"]\n");
	}

	void GraphDrawingVisitor::visit(ColumnOperations * node)
	{
		generateText("ColumnOperations",node);
	}

	void GraphDrawingVisitor::visit(Selection * node)
	{
		generateText("Selection",node);
	}

	void GraphDrawingVisitor::visit(Join * node)
	{
		generateText("Join",node);
	}

	void GraphDrawingVisitor::visit(AntiJoin * node)
	{
		generateText("AntiJoin",node);
	}
	void GraphDrawingVisitor::visit(Difference * node)
	{
		generateText("Difference",node);
	}

	void GraphDrawingVisitor::visit(Union * node)
	{
		generateText("Union",node);
	}

	void GraphDrawingVisitor::visit(Intersection * node)
	{
		generateText("Intersection",node);
	}


