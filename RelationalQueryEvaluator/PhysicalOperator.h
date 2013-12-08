#include <memory>

#ifndef PhysicalAlgorithmsHPP
#define PhysicalAlgorithmsHPP

static class TimeComplexityConstants
{

};

class PhysicalOperator
{

};

class NullaryPhysicalOperator : PhysicalOperator
{

};

class UnaryPhysicalOperator : PhysicalOperator
{
	std::shared_ptr<PhysicalOperator> child;

};

class BinaryPhysicalOperator : PhysicalOperator
{
	std::shared_ptr<PhysicalOperator> leftChild;
	std::shared_ptr<PhysicalOperator> rightChild;
};

class Filter : UnaryPhysicalOperator
{

};

class FilterNotChangingOrder : UnaryPhysicalOperator
{

};

class SortOperator : UnaryPhysicalOperator
{

};

class MergeJoin : BinaryPhysicalOperator
{

};

class IndexJoin : BinaryPhysicalOperator
{

};

class CrossJoin : BinaryPhysicalOperator
{

};

class HashJoin : BinaryPhysicalOperator
{

};

class UnionOperator : BinaryPhysicalOperator
{

};

class HashGroup : UnaryPhysicalOperator
{

};

class SortedGroup : UnaryPhysicalOperator
{
			 
};

class ColumnsOperationsOperator : UnaryPhysicalOperator
{
			 
};

class ScanAndSortByIndex : NullaryPhysicalOperator
{

};

class TableScan : NullaryPhysicalOperator
{

};

class IndexScan : NullaryPhysicalOperator
{

};

class PhysicalPlan
{
public:
	//columns
	//indices
	//sorted by
	double timeComplexity;
	double size;
	std::shared_ptr<PhysicalOperator> plan;
};


#endif


