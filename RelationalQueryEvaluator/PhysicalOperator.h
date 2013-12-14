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
	static const double FILTER;
	static const double FILTER_KEEPING_ORDER;

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

class FilterKeepingOrder : public UnaryPhysicalOperator
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
	PhysicalPlan(NullaryPhysicalOperator * op,double numberOfColumns,double time,std::vector<ColumnInfo> & cols)
	{
		
		plan=std::shared_ptr<PhysicalOperator>(op);
		size=numberOfColumns;
		plan->size=size;
		timeComplexity=time;
		plan->timeComplexity=time;
		for(auto it2=cols.begin();it2!=cols.end();++it2)
		{
			columns[it2->name]=*it2;
		}
	}

	PhysicalPlan(UnaryPhysicalOperator * op,double newSize,double time,const std::map<std::string,ColumnInfo> & newColumns,const std::shared_ptr<PhysicalPlan> & oldPlan)
	{
		op->child=oldPlan->plan;
		columns=newColumns;
		plan=std::shared_ptr<PhysicalOperator>(op);
		size=newSize;
		plan->size=size;	
		plan->timeComplexity=time;
		timeComplexity=oldPlan->timeComplexity+ plan->timeComplexity;
	}

	PhysicalPlan(BinaryPhysicalOperator * op,double newSize,double time,const std::map<std::string,ColumnInfo> & newColumns,const std::shared_ptr<PhysicalPlan> & oldPlan1,const std::shared_ptr<PhysicalPlan> & oldPlan2)
	{
		op->leftChild=oldPlan1->plan;
		op->rightChild=oldPlan2->plan;
		columns=newColumns;
		plan=std::shared_ptr<PhysicalOperator>(op);
		size=newSize;
		plan->size=size;	
		plan->timeComplexity=time;
		timeComplexity=oldPlan1->timeComplexity+oldPlan2->timeComplexity+ plan->timeComplexity;

	}

};


#endif


