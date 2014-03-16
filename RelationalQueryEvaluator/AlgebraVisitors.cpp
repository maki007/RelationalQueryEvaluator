

#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "Expressions.h"
#include "ExpressionVisitors.h"
#include <exception>


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

void  AlgebraVisitor::visit(Union * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
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

void GraphDrawingVisitor::generateText(std::string & label ,UnaryAlgebraNodeBase * node)
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

void GraphDrawingVisitor::generateText(std::string & label , BinaryAlgebraNodeBase * node)
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
		label += it->column.toString();
		if(it->order==ASCENDING)
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
	generateText(label,node);

	result+="\n}";
}

void GraphDrawingVisitor::visit(Group * node)
{
	std::string label="Group\n";
	label+="groupBy ";
	for(auto it=node->groupColumns.begin();it!=node->groupColumns.end();++it)
	{
		label+=it->toString();
		label+=", ";
	}
	for(auto it=node->agregateFunctions.begin();it!=node->agregateFunctions.end();++it)
	{
		label+=it->output.toString();
		label+="=";
		label+=it->functionName;
		if (it->parameter.name == "")
		{
			label += "()";
		}
		else
		{
			label += "(" + it->parameter.toString() + ")";
		}
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
		label += it->column.toString();
		label+= "(";
		label+=it->type;
		label+=",";
		label+=std::to_string((ulong)it->numberOfUniqueValues);
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
			label+=it2->name;
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
		label+=it->result.toString();
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

void GraphDrawingVisitor::visit(Union * node)
{
	generateText(std::string("Union"),node);
}

void GraphDrawingVisitor::generateText(std::string & label , GroupedAlgebraNode * node)
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


void GroupingVisitor::visit(Join * node)
{
	if(node->condition.get()!=0)
	{
		node->condition->accept(GroupingExpressionVisitor(&(node->condition)));
	}
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

void GroupingVisitor::resolveJoins(BinaryAlgebraNodeBase * node,GroupedJoin * groupedOperator,std::vector<std::shared_ptr<AlgebraNodeBase> > & oldChildren)
{

	std::shared_ptr<Expression> newCondition=0;
	ulong numberOfChildreninFirstChild=0;
	for(ulong i=0;i<2;++i)
	{
		if(typeid(*(oldChildren[i])) == typeid(GroupedJoin))
		{
			std::shared_ptr<GroupedJoin> newNode = std::dynamic_pointer_cast<GroupedJoin>(oldChildren[i]);
			if (groupedOperator->condition != 0)
			{
				groupedOperator->condition->accept(RenamingJoinConditionExpressionVisitor(i, &newNode->outputColumns));
			}
			
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
		if(i==0)
		{
			numberOfChildreninFirstChild=groupedOperator->children.size();
		}
	}


	if(groupedOperator->condition!=0)
	{
		GetColumnsNodesVisitor visitor;
		groupedOperator->condition->accept(visitor);
		for(long long int i=1;i>=0;--i)
		{
			if(typeid(*(oldChildren[i])) == typeid(GroupedJoin))
			{
				std::shared_ptr<GroupedJoin> join=std::dynamic_pointer_cast<GroupedJoin>(oldChildren[i]);
				for(auto it=join->outputColumns.begin();it!=join->outputColumns.end();++it)
				{
					int columnId = it->column.id;
					for(auto it2=visitor.nodes.begin();it2!=visitor.nodes.end();++it2)
					{
						if ((*it2)->column.id == columnId)
						{
							(*it2)->input=it->input+numberOfChildreninFirstChild;
						}
					}
					for(auto it2=groupedOperator->outputColumns.begin();it2!=groupedOperator->outputColumns.end();++it2)
					{
						if ((it2)->column.id == columnId)
						{
							(it2)->input=it->input+numberOfChildreninFirstChild;;
						}
					}
				}
			}
			else
			{
				for(auto it2=visitor.nodes.begin();it2!=visitor.nodes.end();++it2)
				{
					if((*it2)->input==i)
					{
						(*it2)->input=numberOfChildreninFirstChild;
					}
				}
				for(auto it2=groupedOperator->outputColumns.begin();it2!=groupedOperator->outputColumns.end();++it2)
				{
					if((it2)->input==i)
					{
						(it2)->input=numberOfChildreninFirstChild;
					}
				}

			}
			numberOfChildreninFirstChild=0;

		}

	}

	if(newCondition.get()==0)
	{
		newCondition=groupedOperator->condition;
	}
	else
	{
		if(groupedOperator->condition.get()!=0)
		{
			newCondition=std::shared_ptr<Expression>(new BinaryExpression(groupedOperator->condition,newCondition,BinaryOperator::AND));
		}
	}
	groupedOperator->condition=newCondition;
	groupedOperator->parent=node->parent;
	node->parent->replaceChild(node,groupedOperator);
	if(groupedOperator->condition.get()!=0)
	{
		groupedOperator->condition->accept(GroupingExpressionVisitor(&(groupedOperator->condition)));
	}
}

void GroupingVisitor::visit(ColumnOperations * node)
{
	for(auto it=node->operations.begin();it!=node->operations.end();++it)
	{
		if(it->expression.get()!=0)
		{
			it->expression->accept(GroupingExpressionVisitor(&it->expression));
		}
	}
	node->child->accept(*this);
}

void GroupingVisitor::visit(Selection * node)
{
	node->condition->accept(GroupingExpressionVisitor(&(node->condition)));
	node->child->accept(*this);
}



