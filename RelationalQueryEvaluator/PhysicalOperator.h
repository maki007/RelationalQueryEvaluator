#ifndef PhysicalOpearatorsHPP
#define PhysicalOpearatorsHPP

#include <memory>
#include <map>
#include "Algebra.h"

class PhysicalOperatorVisitor;

class TimeComplexity
{
public:
	static const double UNCLUSTERED_SCAN;
	static const double CLUSTERED_SCAN;
	static const double SORT;
	static const double SORTED_GROUP;
	static const double HASH;
	static const double FILTER;
	static const double FILTER_KEEPING_ORDER;
	static const double INDEX_SEARCH;
	static const double AGGREGATE;
	static const double READ_HASH_TABLE;
	static double sort(double size);
	static double filter(double size);
	static double filterKeppeingOrder(double size);
	static double clusteredScan(double size);
	static double unClusteredScan(double size);
	static double indexSearch(double size);
	static double hash(double size);
	static double sortedGroup(double size);
	static double aggregate(double size,ulong numberOfagregateFuntions);
	static double hashjoin(double hashSize, double readSize);
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
	std::shared_ptr<Expression> condition;
	Filter(const std::shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}
	void accept(PhysicalOperatorVisitor &v);
};

class FilterKeepingOrder : public UnaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition;
	FilterKeepingOrder(const std::shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}
	void accept(PhysicalOperatorVisitor &v);
};

class SortOperator : public UnaryPhysicalOperator
{
public:
	std::vector<ColumnIdentifier> sortedBy;
	std::vector<SortParameter> sortBy;
	void accept(PhysicalOperatorVisitor &v);
	SortOperator(const std::vector<ColumnIdentifier> &  sortedBy, const std::vector<SortParameter> & sortBy)
	{
		this->sortedBy = sortedBy;
		this->sortBy = sortBy;
	}
};

class MergeJoin : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

class NestedLoopJoin : public BinaryPhysicalOperator
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
	std::shared_ptr<Expression> condition;
	HashJoin(const std::shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}
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
public:
	std::vector<ColumnOperation> operations;
	ColumnsOperationsOperator(const std::vector<ColumnOperation> & operations)
	{
		this->operations = operations;
	}
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
	std::shared_ptr<Expression> condition;
	IndexScan(const std::shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}
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
	PhysicalPlan(NullaryPhysicalOperator * op,double numberOfRows,double time,std::vector<ColumnInfo> & cols)
	{
		
		plan=std::shared_ptr<PhysicalOperator>(op);
		size = numberOfRows;
		plan->size=size;
		timeComplexity=time;
		plan->timeComplexity=time;
		for(auto it2=cols.begin();it2!=cols.end();++it2)
		{
			columns[it2->column.name]=*it2;
		}
	}

	PhysicalPlan(NullaryPhysicalOperator * op, double numberOfRows, double time, const std::map<std::string, ColumnInfo> & newColumns)
	{

		plan = std::shared_ptr<PhysicalOperator>(op);
		size = numberOfRows;
		plan->size = size;
		timeComplexity = time;
		plan->timeComplexity = time;
		columns = newColumns;
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

	static bool Comparator(std::shared_ptr<PhysicalPlan> & i, std::shared_ptr<PhysicalPlan> & j)
	{
		return (i->timeComplexity<j->timeComplexity);
	}

	


};


#endif


