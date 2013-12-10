#ifndef PhysicalAlgorithmsHPP
#define PhysicalAlgorithmsHPP

#include <memory>
#include <map>
#include "Algebra.h"

class TimeComplexityConstants
{
public:
	static const double TABLE_SCAN;
	static const double SORT_SCAN;
};

class PhysicalOperator
{

};

class NullaryPhysicalOperator : public PhysicalOperator
{

};

class UnaryPhysicalOperator : public PhysicalOperator
{
	std::shared_ptr<PhysicalOperator> child;

};

class BinaryPhysicalOperator : public PhysicalOperator
{
	std::shared_ptr<PhysicalOperator> leftChild;
	std::shared_ptr<PhysicalOperator> rightChild;
};

class Filter : public UnaryPhysicalOperator
{

};

class FilterNotChangingOrder : public UnaryPhysicalOperator
{

};

class SortOperator : public UnaryPhysicalOperator
{

};

class MergeJoin : public BinaryPhysicalOperator
{

};

class IndexJoin : public BinaryPhysicalOperator
{

};

class CrossJoin : public BinaryPhysicalOperator
{

};

class HashJoin : public BinaryPhysicalOperator
{

};

class UnionOperator : public BinaryPhysicalOperator
{

};

class HashGroup : public UnaryPhysicalOperator
{

};

class SortedGroup : public UnaryPhysicalOperator
{
			 
};

class ColumnsOperationsOperator : public UnaryPhysicalOperator
{
			 
};

class ScanAndSortByIndex : public NullaryPhysicalOperator
{

};

class TableScan : public NullaryPhysicalOperator
{

};

class IndexScan : public NullaryPhysicalOperator
{

};

class PhysicalPlan
{
public:
	std::vector<ColumnInfo> columns;
	std::vector<IndexInfo> indices;
	std::vector<std::string> sortedBy;
	double timeComplexity;
	double size;
	std::shared_ptr<PhysicalOperator> plan;
	PhysicalPlan()
	{
		size=0;
		timeComplexity=0;
		plan=0;
	}
};


#endif


