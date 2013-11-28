

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
void AlgebraVisitor::visit(NullaryAlgebraNodeBase * node)
{
	node->accept(*this);
}
void  AlgebraVisitor::visit(Table * node)
{

}

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

void GraphDrawingVisitor::visit(GroupedJoin * node)
{
	std::string label="GroupedJoin\n";
	std::shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
	if(node->condition!=0)
	{
		node->condition->accept(*visitor);
	label+=visitor->result;
	}
	generateText(label,node);
}

GroupingVisitor::GroupingVisitor()
{

}

void GroupingVisitor::visit(Intersection * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	GroupedIntersection * groupedOperator = new GroupedIntersection();
	std::vector<std::shared_ptr<AlgebraNodeBase> > oldChildren;
	oldChildren.resize(2);
	oldChildren[0]=node->leftChild;
	oldChildren[1]=node->rightChild;

	for(std::size_t i=0;i<2;++i)
	{
		if(typeid(*(oldChildren[i])) == typeid(GroupedIntersection))
		{
		
			std::vector<std::shared_ptr<AlgebraNodeBase>> children=std::dynamic_pointer_cast<GroupedIntersection>(oldChildren[i])->children;
			for(auto it=children.begin();it!=children.end();++it)
			{
				groupedOperator->children.push_back(*it);
			}
		}
		else
		{
			groupedOperator->children.push_back(oldChildren[i]);
		}
	}
	groupedOperator->parent=node->parent;
	node->parent->replaceChild(node,groupedOperator);
}

void GroupingVisitor::visit(Join * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	GroupedJoin * groupedOperator = new GroupedJoin();
	groupedOperator->outputColumns=node->outputColumns;
	groupedOperator->condition=node->condition;
	std::vector<std::shared_ptr<AlgebraNodeBase> > oldChildren;
	oldChildren.resize(2);
	oldChildren[0]=node->leftChild;
	oldChildren[1]=node->rightChild;
	resolveJoins(node,groupedOperator,oldChildren);
}

void GroupingVisitor::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	GroupedJoin * groupedOperator = new GroupedJoin();
	groupedOperator->outputColumns=node->outputColumns;
	groupedOperator->condition=std::shared_ptr<Expression>(new UnaryExpression(node->condition,UnaryOperator::NOT));

	std::vector<std::shared_ptr<AlgebraNodeBase> > oldChildren;
	oldChildren.resize(2);
	oldChildren[0]=node->leftChild;
	oldChildren[1]=node->rightChild;
	resolveJoins(node,groupedOperator,oldChildren);

}

void GroupingVisitor::resolveJoins(BinaryAlgebraNodeBase * node,GroupedJoin * groupedOperator,std::vector<std::shared_ptr<AlgebraNodeBase> > & oldChildren)
{

	std::shared_ptr<Expression> newCondition=0;
	for(std::size_t i=0;i<2;++i)
	{
		if(typeid(*(oldChildren[i])) == typeid(GroupedJoin))
		{
		
			std::shared_ptr<GroupedJoin> newNode= std::dynamic_pointer_cast<GroupedJoin>(oldChildren[i]);
			if(i==1)
			{
				if(newNode->condition!=0)
				{
					GetColumnsNodesVisitor visitor;
					newNode->condition->accept(visitor);
					for(auto it=visitor.nodes.begin();it!=visitor.nodes.end();++it)
					{
						(*it)->input+=groupedOperator->children.size();
					}
				}
			}
			
			std::vector<std::shared_ptr<AlgebraNodeBase>> children=newNode->children;
			if(newCondition==0)
			{
				newCondition=newNode->condition;
			}
			else
			{
				if(newNode->condition!=0)
				{
					newCondition=std::shared_ptr<Expression>(new BinaryExpression(newNode->condition,newCondition,BinaryOperator::AND));
				}
			}
			
			for(auto it=children.begin();it!=children.end();++it)
			{
				groupedOperator->children.push_back(*it);
			}
		}
		else
		{
			groupedOperator->children.push_back(oldChildren[i]);
		}
	}
	if(groupedOperator->condition!=0)
	{
		std::size_t existingChildren=0;
		GetColumnsNodesVisitor visitor;
		groupedOperator->condition->accept(visitor);
		for(std::size_t i=0;i<2;++i)
		{
			if(typeid(*(oldChildren[i])) == typeid(GroupedJoin))
			{
				std::shared_ptr<GroupedJoin> join=std::dynamic_pointer_cast<GroupedJoin>(oldChildren[i]);
				for(auto it=join->outputColumns.begin();it!=join->outputColumns.end();++it)
				{
					std::string name=it->newName;
					for(auto it2=visitor.nodes.begin();it2!=visitor.nodes.end();++it2)
					{
						if((*it2)->name==name)
						{
							(*it2)->input+=it->input+existingChildren;
						}
					}
				}
				existingChildren+=join->children.size();
			}
			else
			{
				existingChildren++;
			}
		}
	}

	if(newCondition==0)
	{
		newCondition=groupedOperator->condition;
	}
	else
	{
		if(groupedOperator->condition!=0)
		{
			newCondition=std::shared_ptr<Expression>(new BinaryExpression(groupedOperator->condition,newCondition,BinaryOperator::AND));
		}
	}
	groupedOperator->condition=newCondition;
	groupedOperator->parent=node->parent;
	node->parent->replaceChild(node,groupedOperator);
}

