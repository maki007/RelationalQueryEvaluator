#include "AlgebraVisitors.h"
#include <set>
#include "ExpressionVisitors.h"
#include <exception>

using namespace std;

SemanticChecker::SemanticChecker()
{
	containsErrors = false;
	lastId = 1;
}

int SemanticChecker::nextId()
{
	return lastId++;
}

void SemanticChecker::convertColumns(const map<string, ColumnInfo> & outputColumns, AlgebraNodeBase * node)
{
	node->outputColumns.clear();
	for (auto it = outputColumns.begin(); it != outputColumns.end(); ++it)
	{
		node->outputColumns.insert(make_pair(it->second.column.id, it->second));
	}
}

void SemanticChecker::visitTable(Table * node)
{
	outputColumns.clear();

	set<string> hashSet;
	for (auto it = node->columns.begin(); it != node->columns.end(); ++it)
	{
		if (hashSet.find(it->column.name) == hashSet.end())
		{
			hashSet.insert(it->column.name);
			outputColumns[it->column.name] = ColumnInfo(it->column.name, it->type);
			outputColumns[it->column.name].column.id = nextId();
			it->column = outputColumns[it->column.name].column;
		}
		else
		{
			ReportError("Column names should be unique","table",node->lineNumber);
		}
	}

	bool containsClusteredIndex = false;
	for (auto it = node->indices.begin(); it != node->indices.end(); ++it)
	{
		if (it->type == CLUSTERED)
		{
			if (containsClusteredIndex == false)
			{
				containsClusteredIndex = true;
			}
			else
			{
				ReportError("Table can contain only one clustered index", "table", node->lineNumber);
			}
		}
		set<string>  indexSet;
		for (auto it2 = it->columns.begin(); it2 != it->columns.end(); ++it2)
		{
			if (hashSet.find(it2->column.name) == hashSet.end())
			{
				ReportError("Index cannot be on non-existing column", "table", node->lineNumber);
			}
			else
			{
				it2->column.id = outputColumns[it2->column.name].column.id;
			}
			if (indexSet.find(it2->column.name) == indexSet.end())
			{
				indexSet.insert(it2->column.name);
			}
			else
			{
				ReportError("Index contains same column multiple times", "table", node->lineNumber);
			}
		}
	}
	convertColumns(outputColumns, node);
}

void SemanticChecker::visitSort(Sort * node)
{
	node->child->accept(*this);
	for (auto it = node->parameters.begin(); it != node->parameters.end(); ++it)
	{
		if (outputColumns.find(it->column.name) == outputColumns.end())
		{
			ReportError("Column doesn't exist","sort", node->lineNumber);
		}
		else
		{
			it->column.id = outputColumns[it->column.name].column.id;
		}
	}
	convertColumns(outputColumns, node);
}

void SemanticChecker::visitGroup(Group * node)
{
	node->child->accept(*this);
	for (auto it = node->agregateFunctions.begin(); it != node->agregateFunctions.end(); ++it)
	{
		if (it->function != COUNT)
		{
			if (outputColumns.find(it->parameter.name) == outputColumns.end())
			{
				ReportError("Column doesn't exist", "group", node->lineNumber);
			}
			else
			{
				it->parameter.id = outputColumns[it->parameter.name].column.id;
			}
		}
	}
	map<string, ColumnInfo> groupColumns;
	for (auto it = node->groupColumns.begin(); it != node->groupColumns.end(); ++it)
	{
		if (outputColumns.find(it->input.name) == outputColumns.end())
		{
			ReportError("Column doesn't exist", "group", node->lineNumber);
		}
		else
		{
			if (groupColumns.find(it->input.name) == groupColumns.end())
			{
				groupColumns[it->input.name] = ColumnInfo(it->input.name, "");
				groupColumns[it->input.name].column = outputColumns[it->input.name].column;
				it->input.id = outputColumns[it->input.name].column.id;
				it->output = it->input;
				it->output.id = nextId();
				groupColumns[it->input.name].column.id = it->output.id;
			}
			else
			{
				ReportError("Cannot group by same column twice", "group", node->lineNumber);
			}
		}

	}
	outputColumns = groupColumns;
	for (auto it = node->agregateFunctions.begin(); it != node->agregateFunctions.end(); ++it)
	{
		if (outputColumns.find(it->output.name) == outputColumns.end())
		{
			outputColumns[it->output.name] = ColumnInfo(it->output.name, "");
			outputColumns[it->output.name].column.id = nextId();
			it->output.id = outputColumns[it->output.name].column.id;
		}
		else
		{
			ReportError("Output columns must have different name", "group", node->lineNumber);
		}
	}
	if (outputColumns.size() == 0)
	{
		ReportError("Group must group by one column or must contain at least one aggregate function", "group", node->lineNumber);
	}
	convertColumns(outputColumns, node);
}

void SemanticChecker::visitColumnOperations(ColumnOperations * node)
{
	node->child->accept(*this);

	for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
	{
		if (it->expression != 0)
		{
			SemanticExpressionVisitor expresionVisitor;
			expresionVisitor.outputColumns0 = outputColumns;
			it->expression->accept(expresionVisitor);
			if (expresionVisitor.containsErrors)
			{
				ReportError("Column expression doesn't exists", "columnoperations", node->lineNumber);
			}
		}
		else
		{
			if (outputColumns.find(it->result.name) == outputColumns.end())
			{
				ReportError("Column allready used", "columnoperations", node->lineNumber);
			}
			else
			{
				it->result.id = outputColumns[it->result.name].column.id;
			}
		}
	}

	outputColumns.clear();
	for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
	{
		if (outputColumns.find(it->result.name) == outputColumns.end())
		{
			outputColumns[it->result.name] = ColumnInfo(it->result.name, "");
			if (it->expression != 0)
			{
				outputColumns[it->result.name].column.id = nextId();
				it->result.id = outputColumns[it->result.name].column.id;
			}
			else
			{
				outputColumns[it->result.name].column.id = it->result.id;
			}
		}
		else
		{
			ReportError("Columns must have different name", "columnoperations", node->lineNumber);
		}
	}
	convertColumns(outputColumns, node);
}

void SemanticChecker::visitSelection(Selection * node)
{
	node->child->accept(*this);
	SemanticExpressionVisitor expresionVisitor;
	expresionVisitor.outputColumns0 = outputColumns;
	node->condition->accept(expresionVisitor);
	if (expresionVisitor.containsErrors)
	{
		ReportError("Column expression doesn't exists", "selection", node->lineNumber);
	}
	convertColumns(outputColumns, node);
}

void SemanticChecker::visitJoin(Join * node)
{
	map<string, ColumnInfo> outputColumns0, outputColumns1;
	node->leftChild->accept(*this);
	outputColumns0 = outputColumns;
	node->rightChild->accept(*this);
	outputColumns1 = outputColumns;

	if (node->condition != 0)
	{
		SemanticExpressionVisitor expresionVisitor;
		expresionVisitor.outputColumns0 = outputColumns0;
		expresionVisitor.outputColumns1 = outputColumns1;
		node->condition->accept(expresionVisitor);
		containsErrors |= expresionVisitor.containsErrors;
	}

	checkJoinOutPutParameters(outputColumns0, outputColumns1, node);
	convertColumns(outputColumns, node);
}

void SemanticChecker::visitAntiJoin(AntiJoin * node)
{
	map<string, ColumnInfo> outputColumns0, outputColumns1;
	node->leftChild->accept(*this);
	outputColumns0 = outputColumns;
	node->rightChild->accept(*this);
	outputColumns1 = outputColumns;

	SemanticExpressionVisitor expresionVisitor;
	expresionVisitor.outputColumns0 = outputColumns0;
	expresionVisitor.outputColumns1 = outputColumns1;
	node->condition->accept(expresionVisitor);
	containsErrors |= expresionVisitor.containsErrors;

	checkJoinOutPutParameters(outputColumns0, outputColumns1, node);
	convertColumns(outputColumns, node);
}



void SemanticChecker::visitUnion(Union * node)
{
	map<string, ColumnInfo> outputColumns0, outputColumns1;
	node->leftChild->accept(*this);
	outputColumns0 = outputColumns;
	node->rightChild->accept(*this);
	outputColumns1 = outputColumns;

	if (outputColumns0.size() != outputColumns1.size())
	{
		ReportError("Union requires inputs to have same size","union",node->lineNumber);
	}

	for (auto it = outputColumns0.begin(); it != outputColumns0.end(); ++it)
	{
		if (outputColumns1.find(it->first) == outputColumns1.end())
		{
			ReportError("Union requires inputs to have same columns", "union", node->lineNumber);
		}
	}
	outputColumns = outputColumns0;
	convertColumns(outputColumns, node);
}

void SemanticChecker::visitGroupedJoin(GroupedJoin * node)
{
	throw exception("Not Suported: GroupedJoin cannot be in input algebra");
}

void SemanticChecker::ReportError(const char * error, string nodeName, const int lineNumber)
{
	containsErrors = true;
	cout << "Error in operator " << nodeName << " at line " << lineNumber << ":" << error << endl;
}