

#include "Algebra.h"
#include "AlgebraVisitor.h"
#include "Expressions.h"
#include "ExpressionVisitor.h"

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
void AlgebraVisitor::visit(GroupedAlgebraNode * node)
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

void AlgebraVisitor::visit(GroupedIntersection * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);
	}
}
void AlgebraVisitor::visit(GroupedUnion * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);
	}
}
void AlgebraVisitor::visit(GroupedDifference * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);
	}
}
void AlgebraVisitor::visit(GroupedJoin * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);
	}
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
	std::string label="[label=\"Table ";
	label+=node->name;
	label+="\n number of rows: ";
	label+=std::to_string(node->numberOfRows);
	label+="\n columns: ";
	for(auto it=node->columns.begin();it!=node->columns.end();++it)
	{
		label+=it->name;
		label+= "(";
		label+=it->type;
		label+=",";
		label+=std::to_string(it->numberOfUniqueValues);
		label+= ")";

		if(it!=node->columns.end()-1)
		{
			label+=", ";
		}
	}
	label+="\n indices: ";

	for(auto it=node->indices.begin();it!=node->indices.end();++it)
	{
		if(it->type==IndexType::CLUSTERED)
		{
			label+="CLUSTERED";
		}
		else
		{
			label+="UNCLUSTERED";
		}
		label+= "(";
		for(auto it2=it->columns.begin();it2!=it->columns.end();++it2)
		{
			label+=*it2;
			if(it2!=it->columns.end()-1)
			{
				label+=", ";
			}
		}
		label+= ")";
		if(it!=node->indices.end()-1)
		{
			label+=", ";
		}
	}

	label+="\"]\n";
	result.append(label);
}

void GraphDrawingVisitor::visit(ColumnOperations * node)
{
	std::string label="ColumnOperations\n";
	for(auto it=node->operations.begin();it!=node->operations.end();++it)
	{
		label+=it->result;
		if(it->expression!=0)
		{
			std::shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
			it->expression->accept(*visitor);
			label+=" = ";
			label+=visitor->result;
		}
		if(it!=node->operations.end()-1)
		{
			label+=", ";
		}
	}
	generateText(label,node);

}

void GraphDrawingVisitor::visit(Selection * node)
{
	std::string label="Selection\n";
	std::shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
	node->condition->accept(*visitor);
	label+=visitor->result;
	generateText(label,node);
}

void GraphDrawingVisitor::visit(Join * node)
{
	std::string label="Join\n";
	//crossjoin
	if(node->condition!=0)
	{
		std::shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
		node->condition->accept(*visitor);
		label+=visitor->result;
	}
	generateText(label,node);
}

void GraphDrawingVisitor::visit(AntiJoin * node)
{
	std::string label="AntiJoin\n";
	std::shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
	node->condition->accept(*visitor);
	label+=visitor->result;
	generateText(label,node);
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

void GraphDrawingVisitor::generateText(std::string label , GroupedAlgebraNode * node)
{
	int identifier=nodeCounter;

	result.append("node");
	result.append(std::to_string(nodeCounter));
	result.append("[label=\""+label+"\"]\n");


	int childIdentifier;
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		childIdentifier=++nodeCounter;
		(*it)->accept(*this);
		result.append("node");
		result.append(std::to_string(childIdentifier));
		result.append(" -> node");
		result.append(std::to_string(identifier));
		result.append("[headport=s, tailport=n,label=\"   \"]\n");
	}

}

void GraphDrawingVisitor::visit(GroupedIntersection * node)
{
	generateText("GroupedIntersection",node);
}
void GraphDrawingVisitor::visit(GroupedUnion * node)
{
	generateText("GroupedUnion",node);
}
void GraphDrawingVisitor::visit(GroupedDifference * node)
{
	generateText("GroupedDifference",node);
}
void GraphDrawingVisitor::visit(GroupedJoin * node)
{
	std::string label="GroupedJoin\n";
	std::shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
	node->condition->accept(*visitor);
	label+=visitor->result;
	generateText(label,node);
}

GroupingVisitor::GroupingVisitor()
{

}

void GroupingVisitor::visit(Intersection * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	GroupedIntersection * groupedOperator = new GroupedIntersection(node);
}

void GroupingVisitor::visit(Union * node)
{
	GroupedUnion * groupedOperator = new GroupedUnion(node);
}

void GroupingVisitor::visit(Difference * node)
{
	GroupedDifference * groupedOperator = new GroupedDifference(node);
}

void GroupingVisitor::visit(Join * node)
{

}

void GroupingVisitor::visit(AntiJoin * node)
{

}


