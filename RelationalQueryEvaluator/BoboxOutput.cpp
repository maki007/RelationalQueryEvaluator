#include "PhysicalOperatorVisitor.h"

using namespace std;

BoboxPlanWritingPhysicalOperatorVisitor::BoboxPlanWritingPhysicalOperatorVisitor()
{
	numberOfLeafs = 0;
	declarations = "";
	code = ""; 
	lastId = 0;
}

string BoboxPlanWritingPhysicalOperatorVisitor::getColumnNumberOutput(const map<int, ColumnInfo> & columns)
{
	string res = "";
	ulong i = 0;
	for (auto it = columns.begin(); it != columns.end(); ++it)
	{
		res += to_string(it->second.column.id);
		++i;
		if (i != columns.size())
		{
			res += ",";
		}
	}
	return res;
}

string BoboxPlanWritingPhysicalOperatorVisitor::getColumnTypeOutput(const map<int, ColumnInfo> & columns)
{
	string res = "";
	ulong i = 0;
	for (auto it = columns.begin(); it != columns.end(); ++it)
	{
		res += it->second.type;
		++i;
		if (i != columns.size())
		{
			res += ",";
		}
	}
	return res;
}

string BoboxPlanWritingPhysicalOperatorVisitor::declaration(const string & type, const string & inputColumns, const string & outputColumns, const string & name, const string & constructParameters)
{
	return type + "(" + inputColumns + ")->(" + outputColumns + ") " + name + "("+constructParameters+"); \n";
}

string BoboxPlanWritingPhysicalOperatorVisitor::connect(const string & from, const string & to)
{
	return from  + " -> " + to + ";\n";
}



string BoboxPlanWritingPhysicalOperatorVisitor::getId()
{
	return to_string(lastId++);
}
string BoboxPlanWritingPhysicalOperatorVisitor::writePlan(std::shared_ptr<PhysicalOperator> plan)
{
	plan->accept(*this);
	string result = "operator main()->()\n";
	result += "{\n";
	result += "broadcast()->(){" + to_string(numberOfLeafs) + "} start;\n";
	result += declarations;
	result += declaration("Store", getColumnTypeOutput(plan->columns), "", "storeResult", "") + "\n";
	result += "source -> start\n";
	result += code;
	result += connect(lastWritttenNode,"storeResult");
	result += connect("storeResult", "output");
	result += "}";
	return result;
}


void BoboxPlanWritingPhysicalOperatorVisitor::writeNullaryOperator(const string & type, const std::map<int, ColumnInfo> & columns, const string & costructorParameters)
{
	lastWritttenNode = type + getId();
	declarations += declaration(type, "", getColumnTypeOutput(columns), lastWritttenNode, costructorParameters);
	code += connect("start[" + to_string(numberOfLeafs) + "]", lastWritttenNode);
	++numberOfLeafs;
}

void BoboxPlanWritingPhysicalOperatorVisitor::writeUnaryOperator(const string & type, UnaryPhysicalOperator * node, const string & costructorParameters)
{
	node->child->accept(*this);
	string newlastWritttenNode = type + getId();
	declarations += declaration(type, getColumnTypeOutput(node->columns), getColumnTypeOutput(node->columns), newlastWritttenNode, costructorParameters);
	code += connect(lastWritttenNode, newlastWritttenNode);
	lastWritttenNode = newlastWritttenNode;
}



void BoboxPlanWritingPhysicalOperatorVisitor::writeBinaryOperator(const string & type, BinaryPhysicalOperator * node, const string & costructorParameters)
{
	string leftNode, rightNode;
	node->leftChild->accept(*this);
	leftNode = lastWritttenNode;
	node->rightChild->accept(*this);
	rightNode = lastWritttenNode;
	string newlastWritttenNode = type + getId();
	declarations += declaration(type, getColumnTypeOutput(node->leftChild->columns) + ")(" + getColumnTypeOutput(node->rightChild->columns), getColumnTypeOutput(node->columns), newlastWritttenNode, costructorParameters);
	code += connect(leftNode, "[0]" + newlastWritttenNode);
	code += connect(rightNode, "[1]" + newlastWritttenNode);
	lastWritttenNode = newlastWritttenNode;

}

void BoboxPlanWritingPhysicalOperatorVisitor::visitMergeEquiJoin(MergeEquiJoin * node)
{
	writeBinaryOperator("MergeEquiJoin", node, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitMergeNonEquiJoin(MergeNonEquiJoin * node)
{
	writeBinaryOperator("MergeNonEquiJoin", node, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitCrossJoin(CrossJoin * node)
{
	writeBinaryOperator("CrossJoin", node, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitHashJoin(HashJoin * node)
{
	writeBinaryOperator("HashJoin", node, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitUnionOperator(UnionOperator * node)
{
	writeBinaryOperator("Union", node, "");
}


void BoboxPlanWritingPhysicalOperatorVisitor::visitFilter(Filter * node)
{
	writeUnaryOperator("Filter", node, "");

}

void BoboxPlanWritingPhysicalOperatorVisitor::visitFilterKeepingOrder(FilterKeepingOrder * node)
{
	writeUnaryOperator("FilterKeepingOrder", node, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitSortOperator(SortOperator * node)
{
	if (node->sortBy.parameters.size() > 0)
	{
		writeUnaryOperator("SortOperator", node, "");
	}
	else
	{
		node->child->accept(*this);
	}
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitHashGroup(HashGroup * node)
{
	writeUnaryOperator("HashGroup", node, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitSortedGroup(SortedGroup * node)
{
	writeUnaryOperator("SortedGroup", node, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitColumnsOperationsOperator(ColumnsOperationsOperator * node)
{
	writeUnaryOperator("ColumnsOperations", node, "");
}




void BoboxPlanWritingPhysicalOperatorVisitor::visitScanAndSortByIndex(ScanAndSortByIndex * node)
{
	writeNullaryOperator("ScanAndSortByIndexScan", node->columns,"");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitTableScan(TableScan * node)
{
	writeNullaryOperator("TableScan", node->columns, "");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitIndexScan(IndexScan * node)
{
	writeNullaryOperator("IndexScan", node->columns, "");
}

