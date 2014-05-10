#include "PhysicalOperatorVisitor.h"
#include "ExpressionVisitors.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <map>

using namespace std;

BoboxPlanWritingPhysicalOperatorVisitor::BoboxPlanWritingPhysicalOperatorVisitor()
{
	numberOfLeafs = 0;
	declarations = "";
	code = "";
	lastId = 0;
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

string BoboxPlanWritingPhysicalOperatorVisitor::getColumnNameOutput(const map<int, ColumnInfo> & columns)
{
	string res = "";
	ulong i = 0;
	for (auto it = columns.begin(); it != columns.end(); ++it)
	{
		res += it->second.column.name;
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
	return type + "(" + inputColumns + ")->(" + outputColumns + ") " + name + "(" + constructParameters + "); \n";
}

string BoboxPlanWritingPhysicalOperatorVisitor::connect(const string & from, const string & to)
{
	return from + " -> " + to + ";\n";
}

string BoboxPlanWritingPhysicalOperatorVisitor::getId()
{
	return to_string(lastId++);
}

void BoboxPlanWritingPhysicalOperatorVisitor::convertColumns(const std::map<int, ColumnInfo> & columns, std::map<int, int> & result)
{
	int i = 0;
	for (auto it = columns.begin(); it != columns.end(); ++it)
	{
		result[it->first] = i++;
	}
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
	result += connect(lastWritttenNode, "storeResult");
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
	declarations += declaration(type, getColumnTypeOutput(node->child->columns), getColumnTypeOutput(node->columns), newlastWritttenNode, costructorParameters);
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


string BoboxPlanWritingPhysicalOperatorVisitor::writeGroupParameters(const map<int, ColumnInfo> & outputColumns, const map<int, ColumnInfo> & inputColumns,
	const vector<GroupColumn> & groupColumns, const vector<AgregateFunction> & agregateFunctions)
{
	string groupBy = "groupBy=\"";
	map<int, int> cols, inputCols;
	convertColumns(outputColumns, cols);
	convertColumns(inputColumns, inputCols);

	ulong i = 0;
	for (auto it = groupColumns.begin(); it != groupColumns.end(); ++it)
	{
		groupBy += to_string(inputCols[it->input.id]);
		i++;
		if (i != groupColumns.size())
		{
			groupBy += ",";
		}
	}
	groupBy += "\",";
	string functions = "functions=\"";
	i = 0;
	for (auto it = agregateFunctions.begin(); it != agregateFunctions.end(); ++it)
	{
		functions += it->functionName;
		if (it->function == AgregateFunctionType::COUNT)
		{
			functions += "()";
		}
		else
		{
			functions += "(" + to_string(inputCols[it->parameter.id]) + ")";
		}
		i++;
		if (i != agregateFunctions.size())
		{
			functions += ",";
		}
	}
	functions += "\"";
	return groupBy + functions;
}


string BoboxPlanWritingPhysicalOperatorVisitor::writeJoinParameters(BinaryPhysicalOperator * node)
{
	map<int, int> cols;
	std::map<int, ColumnInfo> allColumns = node->leftChild->columns;
	allColumns.insert(node->rightChild->columns.begin(), node->rightChild->columns.end());
	convertColumns(allColumns, cols);
	string left = "left=\"";
	string right = "right=\"";
	string out = "out=\"";
	ulong i = 0;
	for (auto it = cols.begin(); it != cols.end(); ++it)
	{
		if (node->columns.find(it->first) != node->columns.end())
		{
			out += to_string(i) + ",";
		}
		
		if (node->leftChild->columns.find(it->first) != node->leftChild->columns.end())
		{
			left += to_string(i) + ",";
		}
		
		if (node->rightChild->columns.find(it->first) != node->rightChild->columns.end())
		{
			right += to_string(i) + ",";
		}

		++i;
	}

	left[left.size()-1] = '\"';
	right[right.size() - 1] = '\"';
	out[out.size() - 1] = '\"';

	return left+","+right+","+out;

}

void BoboxPlanWritingPhysicalOperatorVisitor::visitMergeEquiJoin(MergeEquiJoin * node)
{
	writeBinaryOperator("MergeEquiJoin", node, writeJoinParameters(node));
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitMergeNonEquiJoin(MergeNonEquiJoin * node)
{
	writeBinaryOperator("MergeNonEquiJoin", node, writeJoinParameters(node));
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitCrossJoin(CrossJoin * node)
{
	writeBinaryOperator("CrossJoin", node, writeJoinParameters(node));
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitHashJoin(HashJoin * node)
{
	writeBinaryOperator("HashJoin", node, writeJoinParameters(node));
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitUnionOperator(UnionOperator * node)
{
	string left = "left=\"";
	string right = "right=\"";
	string out = "out=\"";

	map<string, ulong> names;
	ulong i = 0;
	for (auto it = node->rightChild->columns.begin(); it != node->rightChild->columns.end(); ++it)
	{
		names.insert(make_pair(it->second.column.name, i++));
	}
	i = 0;
	for (auto it = node->columns.begin(); it != node->columns.end(); ++it)
	{
		left += to_string(i) + ",";
		right += to_string(names[it->second.column.name]) + ",";
		out += to_string(i++) + ",";
	}
	left.at(left.size() - 1) = '\"';
	right.at(right.size() - 1) = '\"';
	out.at(out.size() - 1) = '\"';

	writeBinaryOperator("Union", node, left + "," + right + "," + out);
}


void BoboxPlanWritingPhysicalOperatorVisitor::visitFilter(Filter * node)
{
	map<int, int> cols;
	convertColumns(node->child->columns, cols);
	BoboxWritingExpressionVisitor writer(cols);
	node->condition->accept(writer);
	writeUnaryOperator("Filter", node, "condition=\"" + writer.result + "\"");

}

void BoboxPlanWritingPhysicalOperatorVisitor::visitFilterKeepingOrder(FilterKeepingOrder * node)
{
	map<int, int> cols;
	convertColumns(node->child->columns, cols);
	BoboxWritingExpressionVisitor writer(cols);
	node->condition->accept(writer);
	writeUnaryOperator("FilterKeepingOrder", node, "condition=\"" + writer.result + "\"");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitSortOperator(SortOperator * node)
{
	if (node->sortBy.parameters.size() > 0)
	{
		map<int, int> cols;
		convertColumns(node->columns, cols);
		string sortedby = "sortedBy=\"";
		ulong i = 0;
		for (auto it = node->sortedBy.parameters.begin(); it != node->sortedBy.parameters.end(); ++it)
		{
			sortedby += to_string(cols[(it->values.begin())->column.id]);
			++i;
			if (node->sortedBy.parameters.size() != i)
			{
				sortedby += ",";
			}
		}
		sortedby += "\",";
		string sortby = "sortBy=\"";
		i = 0;
		for (auto it = node->sortBy.parameters.begin(); it != node->sortBy.parameters.end(); ++it)
		{
			string direction = "A";
			if (it->values.begin()->order == SortOrder::DESCENDING)
			{
				direction = "D";
			}
			sortby += to_string(cols[(it->values.begin())->column.id]) + ":" + direction;
			++i;
			if (node->sortBy.parameters.size() != i)
			{
				sortby += ",";
			}
		}
		sortby += "\"";


		writeUnaryOperator("SortOperator", node, sortedby + sortby);
	}
	else
	{
		node->child->accept(*this);
	}
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitHashGroup(HashGroup * node)
{
	writeUnaryOperator("HashGroup", node, writeGroupParameters(node->columns, node->child->columns, node->groupColumns, node->agregateFunctions));
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitSortedGroup(SortedGroup * node)
{
	writeUnaryOperator("SortedGroup", node, writeGroupParameters(node->columns, node->child->columns, node->groupColumns, node->agregateFunctions));
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitColumnsOperationsOperator(ColumnsOperationsOperator * node)
{
	sort(node->operations.begin(), node->operations.end(),
		[](const ColumnOperation & a, const ColumnOperation & b)
	{
		return a.result.id < b.result.id;
	});

	map<int, int> cols;
	convertColumns(node->child->columns, cols);

	string parameters = "out=\"";
	ulong i = 0;
	for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
	{
		if (it->expression != 0)
		{
			BoboxWritingExpressionVisitor writer(cols);
			it->expression->accept(writer);
			parameters += writer.result;
		}
		else
		{
			parameters += to_string(cols[it->result.id]);
		}

		++i;
		if (i != node->operations.size())
		{
			parameters += ",";
		}
	}

	parameters += "\"";
	writeUnaryOperator("ColumnsOperations", node, parameters);
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitScanAndSortByIndex(ScanAndSortByIndex * node)
{
	string cols = "";
	writeNullaryOperator("ScanAndSortByIndexScan", node->columns, "name=\""+node->tableName+"\",index=\""+node->index.name+"\",columns=\""+getColumnNameOutput(node->columns)+"\"");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitTableScan(TableScan * node)
{
	writeNullaryOperator("TableScan", node->columns, "name=\"" + node->tableName + "\",columns=\"" + getColumnNameOutput(node->columns) + "\"");
}

void BoboxPlanWritingPhysicalOperatorVisitor::visitIndexScan(IndexScan * node)
{
	map<int, int> cols;
	convertColumns(node->columns, cols);
	BoboxWritingExpressionVisitor writer(cols);
	node->condition->accept(writer);
	writeNullaryOperator("IndexScan", node->columns, "name=\"" + node->tableName + "\",index=\"" + node->index.name + "\",columns=\"" + getColumnNameOutput(node->columns) + "\",condition=\"" + writer .result+ "\"");
}

BoboxWritingExpressionVisitor::BoboxWritingExpressionVisitor(map<int, int> & cols)
{
	this->cols = &cols;
}

void BoboxWritingExpressionVisitor::visitUnaryExpression(UnaryExpression * expression)
{
	switch (expression->operation)
	{
	case UnaryOperator::NOT:
		result += "OP_NOT(";
		break;
	default:
		throw new exception("operator not found");
	}
	expression->child->accept(*this);
	result += ")";

}

void BoboxWritingExpressionVisitor::visitBinaryExpression(BinaryExpression * expression)
{
	string opName = "";
	switch (expression->operation)
	{
	case BinaryOperator::AND:
		opName += "OP_AND";
		break;
	case BinaryOperator::OR:
		opName += "OP_OR";
		break;
	case BinaryOperator::PLUS:
		opName += "OP_PLUS";
		break;
	case BinaryOperator::MINUS:
		opName += "OP_MINUS";
		break;
	case BinaryOperator::TIMES:
		opName += "OP_TIMES";
		break;
	case BinaryOperator::DIVIDE:
		opName += "OP_DIVIDE";
		break;
	case BinaryOperator::EQUALS:
		opName += "OP_EQUALS";
		break;
	case BinaryOperator::NOT_EQUALS:
		opName += "OP_NOT_EQUALS";
		break;
	case BinaryOperator::LOWER:
		opName += "OP_LOWER";
		break;
	case BinaryOperator::LOWER_OR_EQUAL:
		opName += "OP_LOWER_OR_EQUAL";
		break;
	default:
		throw new exception("operator not found");
	}
	result += opName + "(";
	expression->leftChild->accept(*this);
	result += ",";
	expression->rightChild->accept(*this);
	result += ")";

}

void BoboxWritingExpressionVisitor::visitNnaryExpression(NnaryExpression * expression)
{
	result += "OP_"+expression->name + "(";
	ulong i = 0;
	for (auto it = expression->arguments.begin(); it != expression->arguments.end(); ++it)
	{
		(*it)->accept(*this);
		++i;
		if (expression->arguments.size() != i)
		{
			result += ",";
		}
	}
}

void BoboxWritingExpressionVisitor::visitConstant(Constant * expression)
{
	result += "OP_" + expression->type + "_CONSTANT(" + expression->value + ")";
}

void BoboxWritingExpressionVisitor::visitColumn(Column * expression)
{
	result += to_string(cols->at(expression->column.id));
}

void BoboxWritingExpressionVisitor::visitGroupedExpression(GroupedExpression * expression)
{
	string opName = "";
	switch (expression->operation)
	{
	case GroupedOperator::AND:
		opName += "OP_AND";
		break;
	case GroupedOperator::OR:
		opName += "OP_OR";
		break;
	default:
		throw new exception("operator not found");
	}
	result += opName + "(";
	ulong i = 0;
	for (auto it = expression->children.begin(); it != expression->children.end(); ++it)
	{
		(*it)->accept(*this);
		++i;
		if (expression->children.size() != i)
		{
			result += ",";
		}
	}
	result += ")";
}

