#include "AlgebraVisitors.h"



SemanticChecker::SemanticChecker()
{
	containsErrors=false;
}

void SemanticChecker::visit(Table * node)
{

}

void SemanticChecker::visit(Sort * node)
{
	node->child->accept(*this);
}

void SemanticChecker::visit(Group * node)
{
	node->child->accept(*this);
}

void SemanticChecker::visit(ColumnOperations * node)
{
	node->child->accept(*this);
}

void SemanticChecker::visit(Selection * node)
{
	node->child->accept(*this);
}

void SemanticChecker::visit(Join * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void SemanticChecker::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}
	
void SemanticChecker::visit(Union * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void SemanticChecker::visit(GroupedJoin * node)
{
	throw new std::exception("Not Suported: GroupedJoin cannot be in input algebra");
}
