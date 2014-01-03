#include "PhysicalOperator.h"
#include "PhysicalOperatorVisitor.h"

const double TimeComplexityConstants::TABLE_SCAN=1.0;

const double TimeComplexityConstants::SORT_SCAN=3.0;

const double TimeComplexityConstants::SORT=2.0;
const double TimeComplexityConstants::SORTED_GROUP=1;
const double TimeComplexityConstants::HASHED_GROUP=6;
const double TimeComplexityConstants::FILTER=1;
const double TimeComplexityConstants::FILTER_KEEPING_ORDER=1.3;
const double TimeComplexityConstants::INDEX_SCAN = 6;

void Filter::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void FilterKeepingOrder::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void SortOperator::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void MergeJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void NestedLoopJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void CrossJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void HashJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void UnionOperator::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void HashGroup::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void SortedGroup::accept(PhysicalOperatorVisitor &v)	
{
	v.visit(this);
}

void ColumnsOperationsOperator::accept(PhysicalOperatorVisitor &v) 
{
	v.visit(this);
}

void ScanAndSortByIndex::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void TableScan::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

void IndexScan::accept(PhysicalOperatorVisitor &v)
{
	v.visit(this);
}

PhysicalPlan::PhysicalPlan()
{
	size=0;
	timeComplexity=0;
	plan=0;
}