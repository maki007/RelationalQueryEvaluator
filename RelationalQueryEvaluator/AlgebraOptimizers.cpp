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
	node->parent->replaceChild(node, newSelections[0]);
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
		node->parent->replaceChild(node, newNode);
		newNode->child->accept(*this);
	}
}


void SelectionColectingVisitor::visitSelection(Selection * node)
{
	selections.push_back(shared_ptr<Selection>(node));
	node->child->accept(*this);
}


PushSelectionDownVisitor::PushSelectionDownVisitor(Selection * node)
{
	this->node = shared_ptr<Selection>(node);
}

void PushSelectionDownVisitor::pushDown()
{
	AlgebraNodeBase * start = node->child.get();
	removeSelection(node.get());
//	start->accept(*this);
}

void PushSelectionDownVisitor::visitTable(Table * node)
{

}

void PushSelectionDownVisitor::visitSort(Sort * node)
{
	throw new exception("should not happen");
}

void PushSelectionDownVisitor::visitGroup(Group * node)
{

}

void PushSelectionDownVisitor::visitColumnOperations(ColumnOperations * node)
{

}

void PushSelectionDownVisitor::visitSelection(Selection * node)
{

}

void PushSelectionDownVisitor::visitJoin(Join * node)
{
	throw new exception("not supported");
}

void PushSelectionDownVisitor::visitAntiJoin(AntiJoin * node)
{

}

void PushSelectionDownVisitor::visitUnion(Union * node)
{

}

void PushSelectionDownVisitor::visitGroupedJoin(GroupedJoin * node)
{

}



void PushSelectionUpVisitor::visitTable(Table * node)
{

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

