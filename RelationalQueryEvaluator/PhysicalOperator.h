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

class IndexScan : UnaryPhysicalOperator
{

};

class Sort : UnaryPhysicalOperator
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

class Union : BinaryPhysicalOperator
{

};

class HashGroup : UnaryPhysicalOperator
{

};

class SortedGroup :UnaryPhysicalOperator
{
			 
};
//columns operations
//scans
//groups



#endif


