#include "PhysicalOperatorVisitor.h"
#include "ExpressionVisitors.h"

using namespace std;

void PhysicalOperatorVisitor::visit(PhysicalOperator * node)
{
	node->accept(*this);
}
void PhysicalOperatorVisitor::visit(NullaryPhysicalOperator * node)
{
	node->accept(*this);
}

void PhysicalOperatorVisitor::visit(UnaryPhysicalOperator * node)
{
	node->accept(*this);
}

void PhysicalOperatorVisitor::visit(BinaryPhysicalOperator * node)
{
	node->accept(*this);
}

void PhysicalOperatorVisitor::visit(Filter * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visit(FilterKeepingOrder * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visit(SortOperator * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visit(MergeJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visit(NestedLoopJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visit(CrossJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visit(HashJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visit(UnionOperator * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visit(HashGroup * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visit(SortedGroup * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visit(ColumnsOperationsOperator * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visit(ScanAndSortByIndex * node)
{

}

void PhysicalOperatorVisitor::visit(TableScan * node)
{

}

void PhysicalOperatorVisitor::visit(IndexScan * node)
{

}



PhysicalOperatorDrawingVisitor::PhysicalOperatorDrawingVisitor()
{
	result="digraph g {node [shape=box]\n graph[rankdir=\"BT\", concentrate=true];\n";
	nodeCounter=0;
}

void PhysicalOperatorDrawingVisitor::generateText(string & label,NullaryPhysicalOperator * node)
{
	result.append("node");
	result.append(to_string(nodeCounter));
	result.append("[label=\""+label+"\n time:"+to_string((ulong)node->timeComplexity)+"\n size:"+to_string((ulong)node->size)+"\"]\n");
}

void PhysicalOperatorDrawingVisitor::generateText(string & label,UnaryPhysicalOperator * node)
{
	ulong identifier=nodeCounter;

	result.append("node");
	result.append(to_string(nodeCounter));
	result.append("[label=\""+label+"\n time:"+to_string((ulong)node->timeComplexity)+"\n size:"+to_string((ulong)node->size)+"\"]\n");
	ulong childIdentifier=++nodeCounter;
	node->child->accept(*this);

	result.append("node");
	result.append(to_string(childIdentifier));
	result.append(" -> node");
	result.append(to_string(identifier));
	result.append("[headport=s, tailport=n,label=\"   \"]\n");

}

void PhysicalOperatorDrawingVisitor::generateText(string & label,BinaryPhysicalOperator * node)
{
	ulong identifier=nodeCounter;

	result.append("node");
	result.append(to_string(nodeCounter));
	result.append("[label=\""+label+"\n time:"+to_string((ulong)node->timeComplexity)+"\n size:"+to_string((ulong)node->size)+"\"]\n");

	ulong childIdentifier=++nodeCounter;
	node->leftChild->accept(*this);
	result.append("node");
	result.append(to_string(childIdentifier));
	result.append(" -> node");
	result.append(to_string(identifier));
	result.append("[headport=s, tailport=n,label=\"   \"]\n");

	childIdentifier=++nodeCounter;
	node->rightChild->accept(*this);
	result.append("node");
	result.append(to_string(childIdentifier));
	result.append(" -> node");
	result.append(to_string(identifier));
	result.append("[headport=s, tailport=n,label=\"   \"]\n");

}

void PhysicalOperatorDrawingVisitor::visit(Filter * node)
{
	string label="Filter\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(FilterKeepingOrder * node)
{
	string label="Filter Keeping Order\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(SortOperator * node)
{
	string label="Sort";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(MergeJoin * node)
{
	string label = "Merge Join";
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visit(NestedLoopJoin * node)
{

}

void PhysicalOperatorDrawingVisitor::visit(CrossJoin * node)
{
	string label = "Cross Join";
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visit(HashJoin * node)
{
	string label = "Hash Join\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visit(UnionOperator * node)
{
	string label="Union";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(HashGroup * node)
{
	string label="Hash Group";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(SortedGroup * node)
{
	string label="Sorted Group";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(ColumnsOperationsOperator * node)
{
	string label="Columns Operations\n";
	for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
	{
		label += it->result.name;
		if (it->expression != 0)
		{
			shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
			it->expression->accept(*visitor);
			label += " = ";
			label += visitor->result;
		}
		if (it != node->operations.end() - 1)
		{
			label += ", ";
		}
	}
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(ScanAndSortByIndex * node)
{
	string label="Sort by index Scan";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(TableScan * node)
{
	string label="Table Scan";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(IndexScan * node)
{
	string label="Index Scan\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}


