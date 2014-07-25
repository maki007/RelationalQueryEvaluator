#include "PhysicalOperator.h"
#include "PhysicalOperatorVisitor.h"


namespace rafe {
	using namespace std;

	const double TimeComplexity::CLUSTERED_SCAN = 1.0;
	const double TimeComplexity::UNCLUSTERED_SCAN = 3.0;
	const double TimeComplexity::SORT = 2.0;
	const double TimeComplexity::SORTED_GROUP = 1;
	const double TimeComplexity::HASH = 10;
	const double TimeComplexity::FILTER = 1;
	const double TimeComplexity::FILTER_KEEPING_ORDER = 1.3;
	const double TimeComplexity::INDEX_SEARCH = 6;
	const double TimeComplexity::AGGREGATE = 0.3;
	const double TimeComplexity::READ_HASH_TABLE = 8;
	const double TimeComplexity::UNION = 0.3;
	const double TimeComplexity::CROSS_JOIN = 1;
	const double TimeComplexity::MERGE_NONEQUI_JOIN = 1;
	const double TimeComplexity::MERGE_EQUI_JOIN = 0.2;

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
	double TimeComplexity::hashJoin(double hashSize, double readSize)
	{
		return HASH*hashSize + READ_HASH_TABLE*readSize;
	}
	double TimeComplexity::mergeEquiJoin(double leftSize, double rightSize)
	{
		return MERGE_EQUI_JOIN*leftSize + rightSize;
	}
	double TimeComplexity::mergeEquiNonJoin(double leftSize, double rightSize, double finalSize)
	{
		return finalSize;
	}
	double TimeComplexity::Union(double leftSize, double rightSize)
	{
		return UNION*leftSize + UNION*rightSize;
	}
	double TimeComplexity::crossJoin(double leftSize, double rightSize)
	{
		return TimeComplexity::CROSS_JOIN*leftSize*rightSize;
	}

	Filter::Filter(const std::shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}

	void Filter::accept(PhysicalOperatorVisitor &v)
	{
		v.visitFilter(this);
	}

	FilterKeepingOrder::FilterKeepingOrder(const std::shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}

	void FilterKeepingOrder::accept(PhysicalOperatorVisitor &v)
	{
		v.visitFilterKeepingOrder(this);
	}

	SortOperator::SortOperator(const PossibleSortParameters &  sortedBy, const PossibleSortParameters & sortBy)
	{
		this->sortedBy = sortedBy;
		this->sortBy = sortBy;
	}

	void SortOperator::accept(PhysicalOperatorVisitor &v)
	{
		v.visitSortOperator(this);
	}

	MergeEquiJoin::MergeEquiJoin(const shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}

	void MergeEquiJoin::accept(PhysicalOperatorVisitor &v)
	{
		v.visitMergeEquiJoin(this);
	}

	MergeNonEquiJoin::MergeNonEquiJoin(const std::shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}

	void MergeNonEquiJoin::accept(PhysicalOperatorVisitor &v)
	{
		v.visitMergeNonEquiJoin(this);
	}

	void CrossJoin::accept(PhysicalOperatorVisitor &v)
	{
		v.visitCrossJoin(this);
	}

	HashJoin::HashJoin(const std::shared_ptr<Expression> & condition, const std::vector<ColumnIdentifier> & leftPartOfCondition, const std::vector<ColumnIdentifier> & rightPartOfCondition)
	{
		this->condition = condition;
		this->leftPartOfCondition = leftPartOfCondition;
		this->rightPartOfCondition = rightPartOfCondition;
	}


	void HashJoin::accept(PhysicalOperatorVisitor &v)
	{
		v.visitHashJoin(this);
	}

	HashAntiJoin::HashAntiJoin(const shared_ptr<Expression> & condition, const std::vector<ColumnIdentifier> & leftPartOfCondition, const std::vector<ColumnIdentifier> & rightPartOfCondition)
	{
		this->condition = condition;
		this->leftPartOfCondition = leftPartOfCondition;
		this->rightPartOfCondition = rightPartOfCondition;
	}

	void HashAntiJoin::accept(PhysicalOperatorVisitor &v)
	{
		v.visitHashAntiJoin(this);
	}

	MergeAntiJoin::MergeAntiJoin(const shared_ptr<Expression> & condition)
	{
		this->condition = condition;
	}

	void MergeAntiJoin::accept(PhysicalOperatorVisitor &v)
	{
		v.visitMergeAntiJoin(this);
	}

	void UnionOperator::accept(PhysicalOperatorVisitor &v)
	{
		v.visitUnionOperator(this);
	}

	HashGroup::HashGroup(const vector<GroupColumn> & groupColumns, const vector<AgregateFunction> & agregateFunctions)
	{
		this->groupColumns = groupColumns;
		this->agregateFunctions = agregateFunctions;
	}

	void HashGroup::accept(PhysicalOperatorVisitor &v)
	{
		v.visitHashGroup(this);
	}

	SortedGroup::SortedGroup(const vector<GroupColumn> & groupColumns, const vector<AgregateFunction> & agregateFunctions)
	{
		this->groupColumns = groupColumns;
		this->agregateFunctions = agregateFunctions;
	}

	void SortedGroup::accept(PhysicalOperatorVisitor &v)
	{
		v.visitSortedGroup(this);
	}

	ColumnsOperationsOperator::ColumnsOperationsOperator(const std::vector<ColumnOperation> & operations)
	{
		this->operations = operations;
	}

	void ColumnsOperationsOperator::accept(PhysicalOperatorVisitor &v)
	{
		v.visitColumnsOperationsOperator(this);
	}

	ScanAndSortByIndex::ScanAndSortByIndex(const std::string & name, const Index & index)
	{
		tableName = name;
		this->index = index;
	}

	void ScanAndSortByIndex::accept(PhysicalOperatorVisitor &v)
	{
		v.visitScanAndSortByIndex(this);
	}

	TableScan::TableScan(const std::string & name)
	{
		tableName = name;
	}

	void TableScan::accept(PhysicalOperatorVisitor &v)
	{
		v.visitTableScan(this);
	}

	IndexScan::IndexScan(const string & name, const std::shared_ptr<Expression> & condition, const Index & index)
	{
		tableName = name;
		this->condition = condition;
		this->index = index;
	}

	void IndexScan::accept(PhysicalOperatorVisitor &v)
	{
		v.visitIndexScan(this);
	}

	PhysicalPlan::PhysicalPlan()
	{
		timeComplexity = 0;
		plan = 0;
	}

	PhysicalPlan::PhysicalPlan(NullaryPhysicalOperator * op, double numberOfRows, double time, std::vector<ColumnInfo> & cols)
	{

		plan = std::shared_ptr<PhysicalOperator>(op);
		plan->size = numberOfRows;
		timeComplexity = time;
		plan->timeComplexity = time;
		for (auto it2 = cols.begin(); it2 != cols.end(); ++it2)
		{
			op->columns[it2->column.id] = *it2;
		}
	}

	PhysicalPlan::PhysicalPlan(NullaryPhysicalOperator * op, double numberOfRows, double time, const std::map<int, ColumnInfo> & newColumns)
	{

		plan = std::shared_ptr<PhysicalOperator>(op);
		plan->size = numberOfRows;
		timeComplexity = time;
		plan->timeComplexity = time;
		op->columns = newColumns;
	}


	PhysicalPlan::PhysicalPlan(UnaryPhysicalOperator * op, double newSize, double time, const std::map<int, ColumnInfo> & newColumns, const std::shared_ptr<PhysicalPlan> & oldPlan)
	{
		op->child = oldPlan->plan;
		plan = std::shared_ptr<PhysicalOperator>(op);
		plan->size = newSize;
		plan->timeComplexity = time;
		timeComplexity = oldPlan->timeComplexity + plan->timeComplexity;
		op->columns = newColumns;
	}



	PhysicalPlan::PhysicalPlan(BinaryPhysicalOperator * op, double newSize, double time, const std::map<int, ColumnInfo> & newColumns, const std::shared_ptr<PhysicalPlan> & oldPlan1, const std::shared_ptr<PhysicalPlan> & oldPlan2)
	{
		op->leftChild = oldPlan1->plan;
		op->rightChild = oldPlan2->plan;
		plan = std::shared_ptr<PhysicalOperator>(op);
		plan->size = newSize;
		plan->timeComplexity = time;
		timeComplexity = oldPlan1->timeComplexity + oldPlan2->timeComplexity + plan->timeComplexity;
		op->columns = newColumns;

	}

	bool PhysicalPlan::Comparator(std::shared_ptr<PhysicalPlan> & i, std::shared_ptr<PhysicalPlan> & j)
	{
		return (i->timeComplexity < j->timeComplexity);
	}
}