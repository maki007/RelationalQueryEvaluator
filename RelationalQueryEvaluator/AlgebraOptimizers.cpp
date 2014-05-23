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
		selections.push_back((Selection *) current);
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

