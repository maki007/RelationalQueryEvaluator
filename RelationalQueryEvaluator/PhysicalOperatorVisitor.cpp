#include "PhysicalOperatorVisitor.h"
#include "ExpressionVisitors.h"

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

void PhysicalOperatorDrawingVisitor::generateText(std::string & label,NullaryPhysicalOperator * node)
{
	result.append("node");
	result.append(std::to_string(nodeCounter));
	result.append("[label=\""+label+"\n time:"+std::to_string((ulong)node->timeComplexity)+"\n size:"+std::to_string((ulong)node->size)+"\"]\n");
}

void PhysicalOperatorDrawingVisitor::generateText(std::string & label,UnaryPhysicalOperator * node)
{
	std::size_t identifier=nodeCounter;

	result.append("node");
	result.append(std::to_string(nodeCounter));
	result.append("[label=\""+label+"\n time:"+std::to_string((ulong)node->timeComplexity)+"\n size:"+std::to_string((ulong)node->size)+"\"]\n");
	std::size_t childIdentifier=++nodeCounter;
	node->child->accept(*this);

	result.append("node");
	result.append(std::to_string(childIdentifier));
	result.append(" -> node");
	result.append(std::to_string(identifier));
	result.append("[headport=s, tailport=n,label=\"   \"]\n");

}

void PhysicalOperatorDrawingVisitor::generateText(std::string & label,BinaryPhysicalOperator * node)
{
	std::size_t identifier=nodeCounter;

	result.append("node");
	result.append(std::to_string(nodeCounter));
	result.append("[label=\""+label+"\n time:"+std::to_string((ulong)node->timeComplexity)+"\n size:"+std::to_string((ulong)node->size)+"\"]\n");

	std::size_t childIdentifier=++nodeCounter;
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

void PhysicalOperatorDrawingVisitor::visit(Filter * node)
{
	std::string label="Filter\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(FilterKeepingOrder * node)
{
	std::string label="Filter Keeping Order\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(SortOperator * node)
{
	std::string label="Sort";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(MergeJoin * node)
{

}

void PhysicalOperatorDrawingVisitor::visit(NestedLoopJoin * node)
{

}

void PhysicalOperatorDrawingVisitor::visit(CrossJoin * node)
{

}

void PhysicalOperatorDrawingVisitor::visit(HashJoin * node)
{
	
}

void PhysicalOperatorDrawingVisitor::visit(UnionOperator * node)
{
	std::string label="Union";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(HashGroup * node)
{
	std::string label="Hash Group";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(SortedGroup * node)
{
	std::string label="Sorted Group";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(ColumnsOperationsOperator * node)
{
	std::string label="Columns Operations";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(ScanAndSortByIndex * node)
{
	std::string label="Sort by index Scan";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(TableScan * node)
{
	std::string label="Table Scan";
	generateText(label,node);
}

void PhysicalOperatorDrawingVisitor::visit(IndexScan * node)
{
	std::string label="Index Scan\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label,node);
}


