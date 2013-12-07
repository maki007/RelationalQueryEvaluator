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


//columns operations
//scans
//groups

#endif


