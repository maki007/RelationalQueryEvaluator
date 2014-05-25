#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "Expressions.h"
#include "ExpressionVisitors.h"
#include <exception>
#include <vector>

using namespace std;

void SelectionSpitingVisitor::visitSelection(Selection * node)
{
	vector<shared_ptr<Expression> > condition;
	serializeExpression(node->condition, condition);
	vector<Selection *> newSelections;
	for (auto it = condition.begin(); it != condition.end(); ++it)
	{
		newSelections.push_back(new Selection(*it));
	}
	if (condition.size() < 2)
	{
		node->child->accept(*this);
		return;
	}
	for (ulong i = 0; i < condition.size() - 1; ++i)
	{
		newSelections[i]->child = shared_ptr<AlgebraNodeBase>(newSelections[i + 1]);
		newSelections[i + 1]->parent = newSelections[i];
	}
	newSelections[0]->parent = node->parent;
	newSelections.back()->child = node->child;
	node->child->parent = newSelections.back();
	node->parent->replaceChild(node, shared_ptr<AlgebraNodeBase>(newSelections[0]));
	newSelections.back()->child->accept(*this);
}

void SelectionFusingVisitor::visitSelection(Selection * node)
{
	vector<Selection *> selections;
	AlgebraNodeBase * current = node;
	while (typeid(*current) == typeid(Selection))
	{
		selections.push_back((Selection *)current);
		current = ((Selection *)current)->child.get();
	}

	if (selections.size() < 2)
	{
		node->child->accept(*this);
	}
	else
	{
		vector<shared_ptr<Expression> > newCondition;
		for (auto it = selections.begin(); it != selections.end(); ++it)
		{
			newCondition.push_back((*it)->condition);
		}
		Selection * newNode = new Selection(deserializeExpression(newCondition));

		newNode->child = shared_ptr<AlgebraNodeBase>(current);
		current->parent = newNode;
		newNode->parent = node->parent;
		node->parent->replaceChild(node, shared_ptr<AlgebraNodeBase>(newNode));
		newNode->child->accept(*this);
	}
}


void SelectionColectingVisitor::visitSelection(Selection * node)
{
	selections.push_back(node);
	node->child->accept(*this);
}


PushSelectionDownVisitor::PushSelectionDownVisitor(Selection * node)
{
	nodePointer = node;
	condition = node->condition;
}

void PushSelectionDownVisitor::pushDown()
{
	JoinInfoReadingExpressionVisitor expresionVisitor(&columns, &conditionType);
	condition->accept(expresionVisitor);
	AlgebraNodeBase * start = nodePointer->child.get();
	removeSelection(nodePointer);
	start->accept(*this);
}

void PushSelectionDownVisitor::visitTable(Table * node)
{
	insertSelection(node, shared_ptr<Selection>(new Selection(condition)));
}

void PushSelectionDownVisitor::visitSort(Sort * node)
{
	throw new exception("should not happen");
}

void PushSelectionDownVisitor::visitGroup(Group * node)
{
	map<int, int> equalpairs;
	ulong matchedColumns = 0;
	for (auto it = node->groupColumns.begin(); it != node->groupColumns.end(); ++it)
	{
		equalpairs[it->output.id] = it->input.id;
		if (columns.find(it->output.id) != columns.end())
		{
			++matchedColumns;
		}
	}
	
	if (matchedColumns == columns.size())
	{
		condition->accept(RenameColumnsVisitor(&equalpairs));
		condition->accept(JoinInfoReadingExpressionVisitor(&columns, &conditionType));
		node->child->accept(*this);
	}
	else
	{
		insertSelection(node, shared_ptr<Selection>(new Selection(condition)));
	}

}

void PushSelectionDownVisitor::visitColumnOperations(ColumnOperations * node)
{
	ulong matchedColumns = 0;
	map<int, int> equalpairs;
	for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
	{
		if (it->expression == 0)
		{
			if (columns.find(it->result.id) != columns.end())
			{
				++matchedColumns;
			}
		}
		else
		{
			if (typeid(*(it->expression)) == typeid(Column))
			{
				Column * column = (Column *)(it->expression.get());
				if (columns.find(it->result.id) != columns.end())
				{
					++matchedColumns;
					equalpairs[it->result.id] = column->column.id;
				}
			}
		}
	}

	if (matchedColumns == columns.size())
	{
		condition->accept(RenameColumnsVisitor(&equalpairs));
		condition->accept(JoinInfoReadingExpressionVisitor(&columns, &conditionType));
		node->child->accept(*this);
	}
	else
	{
		insertSelection(node, shared_ptr<Selection>(new Selection(condition)));
	}
}

void PushSelectionDownVisitor::visitSelection(Selection * node)
{
	node->child->accept(*this);
}

void PushSelectionDownVisitor::visitJoin(Join * node)
{
	throw new exception("not supported");
}

void PushSelectionDownVisitor::visitAntiJoin(AntiJoin * node)
{
	node->leftChild->accept(*this);
}

void PushSelectionDownVisitor::visitUnion(Union * node)
{
	CloningExpressionVisitor cloner;
	condition->accept(cloner);

	shared_ptr<Expression> savedCondition = cloner.result;
	node->leftChild->accept(*this);
	
	
	map<int, int> equalpairs;
	map<string, int> leftInput;
	for (auto it = node->leftChild->outputColumns.begin(); it != node->leftChild->outputColumns.end(); ++it)
	{
		leftInput[it->second.column.name] = it->first;
	}
	for (auto it = node->rightChild->outputColumns.begin(); it != node->rightChild->outputColumns.end(); ++it)
	{
		equalpairs[leftInput[it->second.column.name]] = it->first;
	}
	savedCondition->accept(RenameColumnsVisitor(&equalpairs));

	savedCondition->accept(JoinInfoReadingExpressionVisitor(&columns, &conditionType));
	condition = savedCondition;
	
	node->rightChild->accept(*this);
}

void PushSelectionDownVisitor::visitGroupedJoin(GroupedJoin * node)
{

}



void PushSelectionUpVisitor::visitTable(Table * node)
{
	throw new exception("should not happen");
}

void PushSelectionUpVisitor::visitSort(Sort * node)
{

}

void PushSelectionUpVisitor::visitGroup(Group * node)
{

}

void PushSelectionUpVisitor::visitColumnOperations(ColumnOperations * node)
{

}

void PushSelectionUpVisitor::visitSelection(Selection * node)
{

}

void PushSelectionUpVisitor::visitJoin(Join * node)
{
	throw new exception("not supported");
}

void PushSelectionUpVisitor::visitAntiJoin(AntiJoin * node)
{

}

void PushSelectionUpVisitor::visitUnion(Union * node)
{

}

void PushSelectionUpVisitor::visitGroupedJoin(GroupedJoin * node)
{

}

