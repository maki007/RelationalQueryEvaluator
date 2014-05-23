#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "Expressions.h"
#include "ExpressionVisitors.h"
#include <exception>
#include <vector>

using namespace std;

void SelectionSpitingVisitor::visitSelection(Selection * node)
{
	vector<shared_ptr<Expression> > condition = serializeExpression(node->condition);
	vector<Selection *> newSelections;
	for (auto it = condition.begin(); it != condition.end(); ++it)
	{
		newSelections.push_back(new Selection(*it));
	}

	for (ulong i = 0; i < condition.size() - 1; ++i)
	{
		newSelections[i]->child = shared_ptr<AlgebraNodeBase>(newSelections[i + 1]);
		newSelections[i + 1]->parent = newSelections[i];
	}
	node->replaceChild(node, newSelections[0]);
	newSelections[0]->parent = node->parent;
	newSelections.back()->child = node->child;
	node->child->parent = newSelections.back();
}

void SelectionFusingVisitor::visitSelection(Selection * node)
{

}

