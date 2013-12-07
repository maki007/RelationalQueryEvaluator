#ifndef PhysicalAlgorithmsHPP
#define PhysicalAlgorithmsHPP

class PhysicalOperator
{

};

class NullaryPhysicalOperator : PhysicalOperator
{

};

class UnaryPhysicalOperator : PhysicalOperator
{

};

class BinaryPhysicalOperator : PhysicalOperator
{

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
	//columns
	//indices
	//sorted by
	//time
	//size
	//std::sharedptr<PhysicalOperator>
};


#endif


