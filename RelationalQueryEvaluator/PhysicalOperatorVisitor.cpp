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

void PhysicalOperatorVisitor::visitMergeEquiJoin(MergeEquiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visitMergeNonEquiJoin(MergeNonEquiJoin * node)
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

void PhysicalOperatorVisitor::visitHashAntiJoin(HashAntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void PhysicalOperatorVisitor::visitMergeAntiJoin(MergeAntiJoin * node)
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
	result = "digraph g {node [shape=box]\n graph[rankdir=\"BT\", concentrate=true];\n";
	nodeCounter = 0;
}

string columns(std::map<int, ColumnInfo> & columns)
{
	string result = "";
	for (auto it = columns.begin(); it != columns.end(); ++it)
	{
		result += it->second.column.toString() + " " + to_string(int(it->second.numberOfUniqueValues)) + ",";
	}
	return result;
}

void PhysicalOperatorDrawingVisitor::generateText(string & label, NullaryPhysicalOperator * node)
{
	result.append("node");
	result.append(to_string(nodeCounter));
	result.append("[label=\"" + label + "\n time:" + to_string((ulong)node->timeComplexity) + "\n size:" + to_string((ulong)node->size) + "\n" + columns(node->columns) + "\n\"]");
}

void PhysicalOperatorDrawingVisitor::generateText(string & label, UnaryPhysicalOperator * node)
{
	ulong identifier = nodeCounter;

	result.append("node");
	result.append(to_string(nodeCounter));
	result.append("[label=\"" + label + "\n time:" + to_string((ulong)node->timeComplexity) + "\n size:" + to_string((ulong)node->size) + "\n" + columns(node->columns) + "\n\"]");
	ulong childIdentifier = ++nodeCounter;
	node->child->accept(*this);

	result.append("node");
	result.append(to_string(childIdentifier));
	result.append(" -> node");
	result.append(to_string(identifier));
	result.append("[headport=s, tailport=n,label=\"   \"]\n");

}

void PhysicalOperatorDrawingVisitor::generateText(string & label, BinaryPhysicalOperator * node)
{
	ulong identifier = nodeCounter;

	result.append("node");
	result.append(to_string(nodeCounter));
	result.append("[label=\"" + label + "\n time:" + to_string((ulong)node->timeComplexity) + "\n size:" + to_string((ulong)node->size) + "\n" + columns(node->columns) + "\n\"]");

	ulong childIdentifier = ++nodeCounter;
	node->leftChild->accept(*this);
	result.append("node");
	result.append(to_string(childIdentifier));
	result.append(" -> node");
	result.append(to_string(identifier));
	result.append("[headport=s, tailport=n,label=\"   \"]\n");

	childIdentifier = ++nodeCounter;
	node->rightChild->accept(*this);
	result.append("node");
	result.append(to_string(childIdentifier));
	result.append(" -> node");
	result.append(to_string(identifier));
	result.append("[headport=s, tailport=n,label=\"   \"]\n");

}

void PhysicalOperatorDrawingVisitor::visitFilter(Filter * node)
{
	string label = "Filter\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitFilterKeepingOrder(FilterKeepingOrder * node)
{
	string label = "Filter Keeping Order\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
}

void writeSortParameters(const PossibleSortParameters & sort, string & label)
{
	for (auto it = sort.parameters.begin(); it != sort.parameters.end(); ++it)
	{
		if (!it->isKnown())
		{
			label += "(";
		}
		for (auto it2 = it->values.begin(); it2 != it->values.end(); ++it2)
		{
			string order = "";
			switch (it2->order)
			{
			case SortOrder::ASCENDING:
				order = " asc";
				break;
			case SortOrder::DESCENDING:
				order = " desc";
				break;
			case SortOrder::UNKNOWN:
				order = " both";
				break;
			}
			label += (it2->column.toString()) + order;
			auto copyIt = it2;
			++copyIt;
			if (copyIt != it->values.end())
			{
				label += ", ";
			}
		}
		if (!it->isKnown())
		{
			label += ")";
		}
		if (it != sort.parameters.end() - 1)
		{
			label += ", ";
		}
	}
}

void PhysicalOperatorDrawingVisitor::visitSortOperator(SortOperator * node)
{
	string label = "";
	if (node->sortedBy.parameters.size() == 0)
	{
		label += "Sort";
	}
	else
	{
		label += "Partial Sort\n";
		label += "Sorted by ";
		writeSortParameters(node->sortedBy, label);
	}
	label += "\nSort by: ";

	writeSortParameters(node->sortBy, label);

	generateText(label, node);
}

void PhysicalOperatorDrawingVisitorWithouSorts::visitSortOperator(SortOperator * node)
{
	if (node->sortBy.parameters.size() > 0)
	{
		PhysicalOperatorDrawingVisitor::visitSortOperator(node);
	}
	else
	{
		node->child->accept(*this);
	}

}
void PhysicalOperatorDrawingVisitor::visitMergeEquiJoin(MergeEquiJoin * node)
{
	string label = "Merge Join\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitMergeNonEquiJoin(MergeNonEquiJoin * node)
{
	string label = "Merge Join\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
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

void PhysicalOperatorDrawingVisitor::visitHashAntiJoin(HashAntiJoin * node)
{
	string label = "Hash Antijoin\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitMergeAntiJoin(MergeAntiJoin * node)
{
	string label = "Merge Antijoin\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitUnionOperator(UnionOperator * node)
{
	string label = "Union";
	generateText(label, node);
}

string PhysicalOperatorDrawingVisitor::writeGroupParameters(const vector<GroupColumn> & groupColumns, const vector<AgregateFunction> & agregateFunctions)
{
	string label = "";
	for (auto it = groupColumns.begin(); it != groupColumns.end(); ++it)
	{
		label += it->input.toString();
		label += ", ";
	}
	if (groupColumns.size() == 0)
	{
		label += "nothing,";
	}

	for (auto it = agregateFunctions.begin(); it != agregateFunctions.end(); ++it)
	{
		label += it->output.toString();
		label += "=";
		label += it->functionName;
		if (it->parameter.name == "")
		{
			label += "()";
		}
		else
		{
			label += "(" + it->parameter.toString() + ")";
		}
		label += ";";
	}
	return label;
}

void PhysicalOperatorDrawingVisitor::visitHashGroup(HashGroup * node)
{
	string label = "Hash Group\n";
	label += writeGroupParameters(node->groupColumns, node->agregateFunctions);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitSortedGroup(SortedGroup * node)
{
	string label = "Sorted Group\n";
	label += writeGroupParameters(node->groupColumns, node->agregateFunctions);
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitColumnsOperationsOperator(ColumnsOperationsOperator * node)
{
	string label = "Columns Operations\n";
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
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitScanAndSortByIndex(ScanAndSortByIndex * node)
{
	string label = "Sort by index Scan\n" +node->tableName + "\n";
	label.append(node->index.toString());
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitTableScan(TableScan * node)
{
	string label = "Table Scan\n"+ node->tableName;
	generateText(label, node);
}

void PhysicalOperatorDrawingVisitor::visitIndexScan(IndexScan * node)
{
	string label = "Index Scan\n" + node->tableName + "\n";
	WritingExpressionVisitor expresionWriter;
	node->condition->accept(expresionWriter);
	label.append(expresionWriter.result+"\n");
	label.append(node->index.toString());
	generateText(label, node);
}


void CloningPhysicalOperatorVisitor::processUnaryOperator(UnaryPhysicalOperator * res)
{
	res->child->accept(*this);
	res->child = result;
	result = shared_ptr<PhysicalOperator>(res);
}

void CloningPhysicalOperatorVisitor::processBinaryOperator(BinaryPhysicalOperator * res)
{
	res->leftChild->accept(*this);
	res->leftChild = result;
	res->rightChild->accept(*this);
	res->rightChild = result;
	result = shared_ptr<PhysicalOperator>(res);
}

void CloningPhysicalOperatorVisitor::visitFilter(Filter * node)
{
	auto res = new Filter(*node);
	processUnaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitFilterKeepingOrder(FilterKeepingOrder * node)
{
	auto res = new FilterKeepingOrder(*node);
	processUnaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitSortOperator(SortOperator * node)
{
	auto res = new SortOperator(*node);
	processUnaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitMergeEquiJoin(MergeEquiJoin * node)
{
	auto res = new MergeEquiJoin(*node);
	processBinaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitMergeNonEquiJoin(MergeNonEquiJoin * node)
{
	auto res = new MergeNonEquiJoin(*node);
	processBinaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitCrossJoin(CrossJoin * node)
{
	auto res = new CrossJoin(*node);
	processBinaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitHashJoin(HashJoin * node)
{
	auto res = new HashJoin(*node);
	processBinaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitHashAntiJoin(HashAntiJoin * node)
{
	auto res = new HashAntiJoin(*node);
	processBinaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitMergeAntiJoin(MergeAntiJoin * node)
{
	auto res = new MergeAntiJoin(*node);
	processBinaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitUnionOperator(UnionOperator * node)
{
	auto res = new UnionOperator(*node);
	processBinaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitHashGroup(HashGroup * node)
{
	auto res = new HashGroup(*node);
	processUnaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitSortedGroup(SortedGroup * node)
{
	auto res = new SortedGroup(*node);
	processUnaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitColumnsOperationsOperator(ColumnsOperationsOperator * node)
{
	auto res = new ColumnsOperationsOperator(*node);
	processUnaryOperator(res);
}

void CloningPhysicalOperatorVisitor::visitScanAndSortByIndex(ScanAndSortByIndex * node)
{
	result = shared_ptr<PhysicalOperator>(new ScanAndSortByIndex(*node));
}

void CloningPhysicalOperatorVisitor::visitTableScan(TableScan * node)
{
	result = shared_ptr<PhysicalOperator>(new TableScan(*node));
}

void CloningPhysicalOperatorVisitor::visitIndexScan(IndexScan * node)
{
	result = shared_ptr<PhysicalOperator>(new IndexScan(*node));
}




void SortResolvingPhysicalOperatorVisitor::visitFilter(Filter * node)
{
	sortParameters.clear();
	node->child->accept(*this);

}

void SortResolvingPhysicalOperatorVisitor::visitFilterKeepingOrder(FilterKeepingOrder * node)
{
	node->child->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitSortOperator(SortOperator * node)
{
	ulong index = 0;

	PossibleSortParameters sortBy = node->sortedBy;
	ulong sortedBySize = 0;
	for (auto it = node->sortedBy.parameters.begin(); it != node->sortedBy.parameters.end(); ++it)
	{
		sortedBySize += it->values.size();
	}

	for (auto it = node->sortBy.parameters.begin(); it != node->sortBy.parameters.end(); ++it)
	{
		sortBy.parameters.push_back(*it);
	}

	std::vector<SortParameter> newSortParameters;
	ulong i = 0;
	for (; i < sortBy.parameters.size(); ++i)
	{
		while (true)
		{
			bool found = false;
			for (auto it = sortBy.parameters[i].values.begin(); it != sortBy.parameters[i].values.end(); ++it)
			{
				if (index >= sortParameters.size())
				{
					goto endOfOuterCycle;
				}
				std::set<ColumnIdentifier> first = it->others;
				first.insert(it->column);
				std::set<ColumnIdentifier> second = sortParameters[index].others;
				second.insert(sortParameters[index].column);
				for (auto it2 = first.begin(); it2 != first.end(); ++it2)
				{
					for (auto it3 = second.begin(); it3 != second.end(); ++it3)
					{
						if (it2->id == it3->id)
						{
							sortBy.parameters[i].values.erase(it);
							SortParameter newParameter = sortParameters[index];
							++index;
							newParameter.others.insert(first.begin(), first.end());
							newParameter.others.erase(newParameter.others.find(newParameter.column));
							newSortParameters.push_back(newParameter);
							found = true;
							goto endOfInnerCycle;
						}
					}
				}
			
			}
		endOfInnerCycle:
			if (found == false)
			{
				break;
			}
		}
	}
endOfOuterCycle:

	for (; i < sortBy.parameters.size(); ++i)
	{
		SortParameters param = sortBy.parameters[i];
		for (auto it = param.values.begin(); it != param.values.end(); ++it)
		{
			newSortParameters.push_back(*it);
			if (newSortParameters[newSortParameters.size() - 1].order == SortOrder::UNKNOWN)
			{
				newSortParameters[newSortParameters.size() - 1].order = SortOrder::ASCENDING;
			}
		}
	}
	sortParameters.clear();
	node->sortedBy.parameters.clear();
	node->sortBy.parameters.clear();
	for (ulong j = 0; j < sortedBySize; ++j)
	{
		node->sortedBy.parameters.push_back(SortParameters(newSortParameters[j]));
		sortParameters.push_back(newSortParameters[j]);
	}
	for (ulong j = sortedBySize; j < newSortParameters.size(); ++j)
	{
		node->sortBy.parameters.push_back(SortParameters(newSortParameters[j]));
	}

	node->child->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitMergeEquiJoin(MergeEquiJoin * node)
{
	map<ulong, ulong> pairs;
	vector<shared_ptr<Expression> > condition = AlgebraCompiler::serializeExpression(node->condition);
	for (auto it = condition.begin(); it != condition.end(); ++it)
	{
		BinaryExpression * expr = (BinaryExpression *)((*it).get());
		Column * leftColumn = (Column *)(expr->leftChild.get());
		Column * rightColumn = (Column *)(expr->rightChild.get());
		pairs[leftColumn->column.id] = rightColumn->column.id;
		pairs[rightColumn->column.id] = leftColumn->column.id;

	}
	std::map<int, ColumnInfo> allColumns = node->leftChild->columns;
	allColumns.insert(node->rightChild->columns.begin(), node->rightChild->columns.end());

	for (auto it = sortParameters.begin(); it != sortParameters.end(); ++it)
	{
		if (pairs.find(it->column.id) != pairs.end())
		{
			it->others.insert(allColumns[pairs[it->column.id]].column);
		}
		for (auto it2 = it->others.begin(); it2 != it->others.end(); ++it2)
		{
			if (pairs.find(it2->id) != pairs.end())
			{
				it->others.insert(allColumns[pairs[it2->id]].column);
			}
		}
		if (it->others.find(it->column) != it->others.end())
		{
			it->others.erase(it->others.find(it->column));
		}
	}
	std::vector<SortParameter> leftSortParameters;
	std::vector<SortParameter> rightSortParameters;
	for (auto it = sortParameters.begin(); it != sortParameters.end(); ++it)
	{
		it->others.insert(it->column);
		SortParameter newParameter;
		newParameter.order = it->order;
		for (auto it2 = it->others.begin(); it2 != it->others.end(); ++it2)
		{
			if (node->leftChild->columns.find(it2->id) != node->leftChild->columns.end())
			{
				newParameter.others.insert(*it2);
			}
		}
		if (newParameter.others.size() > 0)
		{
			newParameter.column = *(newParameter.others.begin());
			newParameter.others.erase(newParameter.others.begin());
			leftSortParameters.push_back(newParameter);
		}
		else
		{
			break;
		}
	}
	for (auto it = sortParameters.begin(); it != sortParameters.end(); ++it)
	{
		it->others.insert(it->column);
		SortParameter newParameter;
		newParameter.order = it->order;
		for (auto it2 = it->others.begin(); it2 != it->others.end(); ++it2)
		{
			if (node->rightChild->columns.find(it2->id) != node->rightChild->columns.end())
			{
				newParameter.others.insert(*it2);
			}
		}
		if (newParameter.others.size() > 0)
		{
			newParameter.column = *(newParameter.others.begin());
			newParameter.others.erase(newParameter.others.begin());
			rightSortParameters.push_back(newParameter);
		}
		else
		{
			break;
		}
	}

	sortParameters = leftSortParameters;
	node->leftChild->accept(*this);
	sortParameters = rightSortParameters;
	node->rightChild->accept(*this);

}

void SortResolvingPhysicalOperatorVisitor::visitMergeNonEquiJoin(MergeNonEquiJoin * node)
{
	//TODO
}

void SortResolvingPhysicalOperatorVisitor::visitHashAntiJoin(HashAntiJoin * node)
{
	sortParameters.clear();
	node->leftChild->accept(*this);
	sortParameters.clear();
	node->rightChild->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitMergeAntiJoin(MergeAntiJoin * node)
{
	map<ulong, ulong> pairs;
	vector<shared_ptr<Expression> > condition = AlgebraCompiler::serializeExpression(node->condition);
	for (auto it = condition.begin(); it != condition.end(); ++it)
	{
		BinaryExpression * expr = (BinaryExpression *)((*it).get());
		Column * leftColumn = (Column *)(expr->leftChild.get());
		Column * rightColumn = (Column *)(expr->rightChild.get());
		pairs[leftColumn->column.id] = rightColumn->column.id;
		pairs[rightColumn->column.id] = leftColumn->column.id;

	}
	std::map<int, ColumnInfo> allColumns = node->leftChild->columns;
	allColumns.insert(node->rightChild->columns.begin(), node->rightChild->columns.end());
	for (auto it = sortParameters.begin(); it != sortParameters.end(); ++it)
	{
		if (pairs.find(it->column.id) != pairs.end())
		{
			it->others.insert(allColumns[pairs[it->column.id]].column);
		}
		for (auto it2 = it->others.begin(); it2 != it->others.end(); ++it2)
		{
			if (pairs.find(it2->id) != pairs.end())
			{
				it->others.insert(allColumns[pairs[it2->id]].column);
			}
		}
		if (it->others.find(it->column) != it->others.end())
		{
			it->others.erase(it->others.find(it->column));
		}
	}
	std::vector<SortParameter> leftSortParameters;
	std::vector<SortParameter> rightSortParameters;
	for (auto it = sortParameters.begin(); it != sortParameters.end(); ++it)
	{
		it->others.insert(it->column);
		SortParameter newParameter;
		newParameter.order = it->order;
		for (auto it2 = it->others.begin(); it2 != it->others.end(); ++it2)
		{
			if (node->leftChild->columns.find(it2->id) != node->leftChild->columns.end())
			{
				newParameter.others.insert(*it2);
			}
		}
		if (newParameter.others.size() > 0)
		{
			newParameter.column = *(newParameter.others.begin());
			newParameter.others.erase(newParameter.others.begin());
			leftSortParameters.push_back(newParameter);
		}
		else
		{
			break;
		}
	}
	for (auto it = sortParameters.begin(); it != sortParameters.end(); ++it)
	{
		it->others.insert(it->column);
		SortParameter newParameter;
		newParameter.order = it->order;
		for (auto it2 = it->others.begin(); it2 != it->others.end(); ++it2)
		{
			if (node->rightChild->columns.find(it2->id) != node->rightChild->columns.end())
			{
				newParameter.others.insert(*it2);
			}
		}
		if (newParameter.others.size() > 0)
		{
			newParameter.column = *(newParameter.others.begin());
			newParameter.others.erase(newParameter.others.begin());
			rightSortParameters.push_back(newParameter);
		}
		else
		{
			break;
		}
	}

	sortParameters = leftSortParameters;
	node->leftChild->accept(*this);

	sortParameters = rightSortParameters;
	node->rightChild->accept(*this);

}

void SortResolvingPhysicalOperatorVisitor::visitCrossJoin(CrossJoin * node)
{
	sortParameters.clear();
	node->leftChild->accept(*this);
	sortParameters.clear();
	node->rightChild->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitHashJoin(HashJoin * node)
{
	sortParameters.clear();
	node->leftChild->accept(*this);
	sortParameters.clear();
	node->rightChild->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitUnionOperator(UnionOperator * node)
{
	sortParameters.clear();
	node->leftChild->accept(*this);
	sortParameters.clear();
	node->rightChild->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitHashGroup(HashGroup * node)
{
	sortParameters.clear();
	node->child->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitSortedGroup(SortedGroup * node)
{
	map<int, int> mapedColumns;
	for (auto it = node->groupColumns.begin(); it != node->groupColumns.end(); ++it)
	{
		mapedColumns[it->output.id] = it->input.id;
	}
	for (auto it = sortParameters.begin(); it != sortParameters.end(); ++it)
	{
		if (mapedColumns.find(it->column.id) != mapedColumns.end())
		{
			it->column.id = mapedColumns[it->column.id];
		}
		std::set<ColumnIdentifier> others;
		for (auto it2 = it->others.begin(); it2 != it->others.end(); ++it2)
		{
			if (mapedColumns.find(it2->id) != mapedColumns.end())
			{
				auto copy = *it2;
				copy.id = mapedColumns[it2->id];
				others.insert(copy);
			}
		}
		it->others = others;
	}
	node->child->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitColumnsOperationsOperator(ColumnsOperationsOperator * node)
{
	node->child->accept(*this);
}

void SortResolvingPhysicalOperatorVisitor::visitScanAndSortByIndex(ScanAndSortByIndex * node)
{
	//empty
}

void SortResolvingPhysicalOperatorVisitor::visitTableScan(TableScan * node)
{
	//empty
}

void SortResolvingPhysicalOperatorVisitor::visitIndexScan(IndexScan * node)
{
	//empty
}

