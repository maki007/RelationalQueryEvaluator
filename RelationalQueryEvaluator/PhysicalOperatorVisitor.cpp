#include "PhysicalOperatorVisitor.h"
#include "ExpressionVisitors.h"

using namespace std;

void PhysicalOperatorVisitor::visitPhysicalOperator(PhysicalOperator * node)
{
	node->accept(*this);
}
void PhysicalOperatorVisitor::visitNullaryPhysicalOperator(NullaryPhysicalOperator * node)
{
	node->accept(*this);
}

void PhysicalOperatorVisitor::visitUnaryPhysicalOperator(UnaryPhysicalOperator * node)
{
	node->accept(*this);
}

void PhysicalOperatorVisitor::visitBinaryPhysicalOperator(BinaryPhysicalOperator * node)
{
	node->accept(*this);
}

void PhysicalOperatorVisitor::visitFilter(Filter * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visitFilterKeepingOrder(FilterKeepingOrder * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visitSortOperator(SortOperator * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visitMergeJoin(MergeJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visitNestedLoopJoin(NestedLoopJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visitCrossJoin(CrossJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visitHashJoin(HashJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visitUnionOperator(UnionOperator * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visitHashGroup(HashGroup * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visitSortedGroup(SortedGroup * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visitColumnsOperationsOperator(ColumnsOperationsOperator * node)
{
	node->child->accept(*this);
}

void PhysicalOperatorVisitor::visitScanAndSortByIndex(ScanAndSortByIndex * node)
{

}

void PhysicalOperatorVisitor::visitTableScan(TableScan * node)
{

}

void PhysicalOperatorVisitor::visitIndexScan(IndexScan * node)
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

void PhysicalOperatorDrawingVisitor::visitFilter(Filter * node)
{
	string label="Filter\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitFilterKeepingOrder(FilterKeepingOrder * node)
{
	string label="Filter Keeping Order\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitSortOperator(SortOperator * node)
{
	string label="Sort";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitMergeJoin(MergeJoin * node)
{
	string label = "Merge Join";
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitNestedLoopJoin(NestedLoopJoin * node)
{

}

void PhysicalOperatorDrawingVisitor::visitCrossJoin(CrossJoin * node)
{
	string label = "Cross Join";
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitHashJoin(HashJoin * node)
{
	string label = "Hash Join\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitUnionOperator(UnionOperator * node)
{
	string label="Union";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitHashGroup(HashGroup * node)
{
	string label="Hash Group";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitSortedGroup(SortedGroup * node)
{
	string label="Sorted Group";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitColumnsOperationsOperator(ColumnsOperationsOperator * node)
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

void PhysicalOperatorDrawingVisitor::visitScanAndSortByIndex(ScanAndSortByIndex * node)
{
	string label="Sort by index Scan";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitTableScan(TableScan * node)
{
	string label="Table Scan";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visitIndexScan(IndexScan * node)
{
	string label="Index Scan\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}


