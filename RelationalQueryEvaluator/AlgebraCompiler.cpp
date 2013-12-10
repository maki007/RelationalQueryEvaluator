#include "AlgebraVisitors.h"


void AlgebraCompiler::visit(Table * node)
{
	result.clear();
	PhysicalPlan * physicalPlan=new PhysicalPlan();
	physicalPlan->plan=std::shared_ptr<PhysicalOperator>(new TableScan());
	physicalPlan->size=(double)node->numberOfRows;
	physicalPlan->timeComplexity= TimeComplexityConstants::TABLE_SCAN*node->numberOfRows;
	physicalPlan->columns=node->columns;
	physicalPlan->indices=node->indices;
	for(auto it=node->indices.begin();it!=node->indices.end();++it)
	{
		if(it->type==CLUSTERED)
		{
			physicalPlan->sortedBy=it->columns;
		}
	}
	result.push_back(std::shared_ptr<PhysicalPlan>(physicalPlan));

	for(auto it=node->indices.begin();it!=node->indices.end();++it)
	{
		if(it->type==UNCLUSTERED)
		{
			PhysicalPlan * physicalPlan=new PhysicalPlan();
			physicalPlan->plan=std::shared_ptr<PhysicalOperator>(new ScanAndSortByIndex());
			physicalPlan->size=(double)node->numberOfRows;
			physicalPlan->timeComplexity= TimeComplexityConstants::SORT_SCAN*node->numberOfRows;
			physicalPlan->columns=node->columns;
			physicalPlan->sortedBy=it->columns;
			result.push_back(std::shared_ptr<PhysicalPlan>(physicalPlan));
		}	
	}
}

void AlgebraCompiler::visit(Sort * node)
{
	node->child->accept(*this);
}

void AlgebraCompiler::visit(Group * node)
{
	node->child->accept(*this);
}

void AlgebraCompiler::visit(ColumnOperations * node)
{
	node->child->accept(*this);
}

void AlgebraCompiler::visit(Selection * node)
{
	node->child->accept(*this);
}

void AlgebraCompiler::visit(Join * node)
{
	throw new std::exception("Not Suported: join should be replaced with groupedJoin");
}

void AlgebraCompiler::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}
	
void AlgebraCompiler::visit(Union * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void AlgebraCompiler::visit(GroupedJoin * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);	
	}
}
