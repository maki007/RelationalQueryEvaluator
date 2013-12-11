#ifndef PhysicalOpearatorsHPP
#define PhysicalOpearatorsHPP

#include <memory>
#include <map>
#include "Algebra.h"

class PhysicalOperatorVisitor;

class TimeComplexityConstants
{
public:
	static const double TABLE_SCAN;
	static const double SORT_SCAN;
	static const double SORT;
	static const double SORTED_GROUP;
	static const double HASHED_GROUP;

};

class PhysicalOperator
{
public:
	double timeComplexity;
	double size;

	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

class NullaryPhysicalOperator : public PhysicalOperator
{
public:
	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

class UnaryPhysicalOperator : public PhysicalOperator
{
public:
	std::shared_ptr<PhysicalOperator> child;
	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

class BinaryPhysicalOperator : public PhysicalOperator
{
public:
	std::shared_ptr<PhysicalOperator> leftChild;
	std::shared_ptr<PhysicalOperator> rightChild;
	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

class Filter : public UnaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class FilterNotChangingOrder : public UnaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class SortOperator : public UnaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class MergeJoin : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class IndexJoin : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class CrossJoin : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class HashJoin : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class UnionOperator : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class HashGroup : public UnaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class SortedGroup : public UnaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);		 
};

class ColumnsOperationsOperator : public UnaryPhysicalOperator
{
	void accept(PhysicalOperatorVisitor &v);		 
};

class ScanAndSortByIndex : public NullaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class TableScan : public NullaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class IndexScan : public NullaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class PhysicalPlan
{
public:
	std::map<std::string,ColumnInfo> columns;
	std::vector<IndexInfo> indices;
	std::vector<SortParameter> sortedBy;
	double timeComplexity;
	double size;
	std::shared_ptr<PhysicalOperator> plan;
	PhysicalPlan();
};


#endif


