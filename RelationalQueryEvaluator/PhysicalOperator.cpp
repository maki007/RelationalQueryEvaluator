#include "PhysicalOperator.h"
#include "PhysicalOperatorVisitor.h"

using namespace std;

const double TimeComplexity::CLUSTERED_SCAN = 1.0;
const double TimeComplexity::UNCLUSTERED_SCAN = 3.0;
const double TimeComplexity::SORT=2.0;
const double TimeComplexity::SORTED_GROUP=1;
const double TimeComplexity::HASH=8;
const double TimeComplexity::FILTER=1;
const double TimeComplexity::FILTER_KEEPING_ORDER=1.3;
const double TimeComplexity::INDEX_SEARCH = 6;
const double TimeComplexity::AGGREGATE = 0.3;
const double TimeComplexity::READ_HASH_TABLE = 2;
const double TimeComplexity::UNION = 0.3;
const double TimeComplexity::CROSS_JOIN = 1;

double TimeComplexity::sort(double size)
{
	return SORT*size*log2(size);
}
double TimeComplexity::filter(double size)
{
	return FILTER*size;
}
double TimeComplexity::filterKeppeingOrder(double size)
{
	return FILTER_KEEPING_ORDER*size;
}
double TimeComplexity::clusteredScan(double size)
{
	return CLUSTERED_SCAN*size;
}
double TimeComplexity::unClusteredScan(double size)
{
	return UNCLUSTERED_SCAN*size;
}
double TimeComplexity::indexSearch(double size)
{
	return INDEX_SEARCH*log2(size);
}
double TimeComplexity::hash(double size)
{
	return (HASH + READ_HASH_TABLE) * size;
}
double TimeComplexity::sortedGroup(double size)
{
	return SORTED_GROUP*size;
}
double TimeComplexity::aggregate(double size, ulong numberOfagregateFuntions)
{
	return AGGREGATE*size;
}
double TimeComplexity::hashjoin(double hashSize, double readSize)
{
	return HASH*hashSize + READ_HASH_TABLE*readSize;
}
double TimeComplexity::Union(double leftSize, double rightSize)
{
	return UNION*leftSize + UNION*rightSize;
}
double TimeComplexity::crossJoin(double leftSize, double rightSize)
{
	return TimeComplexity::CROSS_JOIN*leftSize*rightSize;
}
void Filter::accept(PhysicalOperatorVisitor &v)
{
	v.visitFilter(this);
}

void FilterKeepingOrder::accept(PhysicalOperatorVisitor &v)
{
	v.visitFilterKeepingOrder(this);
}

void SortOperator::accept(PhysicalOperatorVisitor &v)
{
	v.visitSortOperator(this);
}

void MergeEquiJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visitMergeEquiJoin(this);
}

void MergeNonEquiJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visitMergeNonEquiJoin(this);
}

void CrossJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visitCrossJoin(this);
}

void HashJoin::accept(PhysicalOperatorVisitor &v)
{
	v.visitHashJoin(this);
}

void UnionOperator::accept(PhysicalOperatorVisitor &v)
{
	v.visitUnionOperator(this);
}

void HashGroup::accept(PhysicalOperatorVisitor &v)
{
	v.visitHashGroup(this);
}

void SortedGroup::accept(PhysicalOperatorVisitor &v)	
{
	v.visitSortedGroup(this);
}

void ColumnsOperationsOperator::accept(PhysicalOperatorVisitor &v) 
{
	v.visitColumnsOperationsOperator(this);
}

void ScanAndSortByIndex::accept(PhysicalOperatorVisitor &v)
{
	v.visitScanAndSortByIndex(this);
}

void TableScan::accept(PhysicalOperatorVisitor &v)
{
	v.visitTableScan(this);
}

void IndexScan::accept(PhysicalOperatorVisitor &v)
{
	v.visitIndexScan(this);
}

PhysicalPlan::PhysicalPlan()
{
	timeComplexity=0;
	plan=0;
}