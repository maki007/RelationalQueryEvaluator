#ifndef PhysicalOpearatorsHPP
#define PhysicalOpearatorsHPP

#include <memory>
#include <map>
#include "Algebra.h"

class PhysicalOperatorVisitor;

/**
* Static class, which contains only constants and functions for computing time complexity of physical operators.
*/
class TimeComplexity
{
public:
	static const double UNCLUSTERED_SCAN; /**< Constant determining speed of unclustered scan. */
	static const double CLUSTERED_SCAN; /**< Constant determining speed of clustered scan. */
	static const double SORT; /**< Constant determining speed of sort. */
	static const double SORTED_GROUP; /**< Constant determining speed of grouping sorted relation. */
	static const double HASH; /**< Constant determining speed of hashing relation. */
	static const double FILTER; /**< Constant determining speed of filter. */
	static const double FILTER_KEEPING_ORDER; /**< Constant determining speed of filter keeping order. */
	static const double INDEX_SEARCH; /**< Constant determining speed of index search. */
	static const double AGGREGATE; /**< Constant determining speed of computing agregate functions. */
	static const double READ_HASH_TABLE; /**< Constant determining speed of reading hash table. */
	static const double UNION; /**< Constant determining speed of unin. */
	static const double CROSS_JOIN; /**< Constant determining speed of cross join. */
	static const double MERGE_EQUI_JOIN; /**< Constant determining speed of merge equi join. */
	static const double MERGE_NONEQUI_JOIN; /**< Constant determining speed of mergejoin joining by condition a1<b<a2. */

	/**
	* Computes time complexity of sort operator.
	* @param size - size of relation to sort.
	*/
	static double sort(double size);

	/**
	* Computes time complexity of filter operator.
	* @param size - size of relation to filter.
	*/
	static double filter(double size);

	/**
	* Computes time complexity of filterKeepingOrder operator.
	* @param size - size of relation to filter.
	*/
	static double filterKeppeingOrder(double size);

	/**
	* Computes time complexity of scanning table with clusstered index.
	* @param size - size of table or part of table to scan.
	*/
	static double clusteredScan(double size);

	/**
	* Computes time complexity of scanning table with unclusstered index.
	* @param size - size of table or part of table to scan.
	*/
	static double unClusteredScan(double size);

	/**
	* Computes time complexity of finding row with index.
	* @param size - size of table or part of table to search.
	*/
	static double indexSearch(double size);

	/**
	* Computes time complexity of hashing table.
	* @param size - size of relation to hash.
	*/
	static double hash(double size);
	
	/**
	* Computes time complexity of grouping sorted table.
	* @param size - size of intput relation.
	*/
	static double sortedGroup(double size);
	
	/**
	* Computes time complexity of computing agregate functions.
	* @param size - size of relation.
	* @param numberOfagregateFunctions - number of functions to compute.
	*/
	static double aggregate(double size, ulong numberOfagregateFunctions);
	
	/**
	* Computes time complexity of hashequijoin.
	* @param hashSize - size of hashed input. 
	* @param readSize - size of not hashed input.
	*/
	static double hashJoin(double hashSize, double readSize);
	
	/**
	* Computes time complexity of mergeequijoin.
	* @param leftSize - size of first input.
	* @param rightSize - size of second input.
	*/
	static double mergeEquiJoin(double leftSize, double rightSize);

	/**
	* Computes time of merge join evaluation condition a1<b<a2, where a belongs to first input and b to second input.
	* @param leftSize - size of first input.
	* @param rightSize - size of second input.
	* @param finalSize - size of result.
	*/
	static double mergeEquiNonJoin(double leftSize, double rightSize, double finalSize);
	
	/**
	* Computes time complexity of union.
	* @param leftSize - size of first input.
	* @param rightSize - size of second input.
	*/
	static double Union(double leftSize, double rightSize);

	/**
	* Computes time complexity of cross join.
	* @param leftSize - size of first input.
	* @param rightSize - size of second input.
	*/
	static double crossJoin(double leftSize, double rightSize);
};


/**
* Base class for physical algorithms. 
*/
class PhysicalOperator
{
public:
	double timeComplexity;  /**< Stores information about how much time algorithm will need.  */ 
	double size;  /**< Stores size of output relation. */ 
	std::map<int, ColumnInfo> columns;  /**< Stores information about output columns from this operator. */

	/**
	* Method for calling visit[node] on given PhysicalOperatorVisitor.
	* @param v PhysicalOperatorVisitor, on which to call function.
	*/
	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

/**
* Base class for operators with no input. It is used for leafs like read table, read table by index and index scan.
*/
class NullaryPhysicalOperator : public PhysicalOperator
{
public:
	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

/**
* Base class for operators with one input.
*/
class UnaryPhysicalOperator : public PhysicalOperator
{
public:
	std::shared_ptr<PhysicalOperator> child; /**< Stores input operator. */
	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

/**
* Base class for operators with two inputs.
*/
class BinaryPhysicalOperator : public PhysicalOperator
{
public:
	std::shared_ptr<PhysicalOperator> leftChild; /**< Stores left input operator. */
	std::shared_ptr<PhysicalOperator> rightChild; /**< Stores right input operator. */
	virtual void accept(PhysicalOperatorVisitor &v) = 0;
};

/**
* Class representing filter physical algorithm.
* Operator lters given rows and output only rows satisfying condition. 
* Output doesn't have to be sorted same way as input.
*/
class Filter : public UnaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition; /**< Condition for filtering relation.*/
	
	/**
	* Creates new instance of Filter.
	*  @param condition - filter condition.
	*/
	Filter(const std::shared_ptr<Expression> & condition);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Class representing filter physical algorithm.
* Operator filters given rows and output only rows satisfying condition.
* Output has to be sorted same way as input.
*/
class FilterKeepingOrder : public UnaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition; /**< Condition for filtering relation.*/
	
	/**
	* Creates new instance of FilterKeepingOrder.
	*  @param condition - filter condition.
	*/
	FilterKeepingOrder(const std::shared_ptr<Expression> & condition);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represents physical sort operator.
* Sort can be also partial sort when input is partially sorted by desired columns.
*/
class SortOperator : public UnaryPhysicalOperator
{
public:
	PossibleSortParameters sortedBy; /**< Stores columns, which the relation is sorted by. Can be empty. */
	PossibleSortParameters sortBy; /**< Stores colums, which the relation should be sorted by. */

	/**
	* Creates new instance of SortOperator.
	* @param sortedBy - columns - which the relation is sorted by.
	* @param sortBy - columns - which the relation should be sorted by.
	*/
	SortOperator(const PossibleSortParameters &  sortedBy, const PossibleSortParameters & sortBy);

	void accept(PhysicalOperatorVisitor &v);
};
/**
* Represents physical cross join operator.
* Operator computes Cartesian product from given inputs.
*/
class CrossJoin : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};
/**
* Represents merge join joining by condition a1 < b < a2, where a1 and a2 are columns from 1st input, b is from 2nd input.
*/
class MergeNonEquiJoin : public BinaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition; /**< Join condition. */
	std::vector<SortParameter> sortParameters; /**< Stored information how output is sorted. */
	std::vector<SortParameter> left; /**< Stored information how first input is sorted. */
	std::vector<SortParameter> right; /**< Stored information how second input is sorted. */

	/**
	* Creates new instance of MergeNonEquiJoin.
	* @param condition - join condition.
	*/
	MergeNonEquiJoin(const std::shared_ptr<Expression> & condition);

	void accept(PhysicalOperatorVisitor &v);

};

/**
* Represents merge equi join.
* Operator computes equijoin from given sorted inputs.
*/
class MergeEquiJoin : public BinaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition; /**< Join condition. */
	std::vector<SortParameter> left; /**< Stored information how first input is sorted. */
	std::vector<SortParameter> right; /**< Stored information how second input is sorted. */

	/**
	* Creates new instance of MergeEquiJoin.
	* @param condition - join condition.
	*/
	MergeEquiJoin(const std::shared_ptr<Expression> & condition);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represents hash join physical operator.
* Operator computes equijoin using hash table.
* First input will be stored in hash table.
*/
class HashJoin : public BinaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition; /**< Join condition. */
	std::vector<ColumnIdentifier> leftPartOfCondition; /**< Stores columns from left input, which belongs to condition. */
	std::vector<ColumnIdentifier> rightPartOfCondition; /**< Stores columns from right input, which belongs to condition. */
	
	/**
	* Creates new instance of HashJoin.
	* @param condition - join condition.
	* @param leftPartOfCondition - condition columns from first input
	* @param rightPartOfCondition - condition columns from second input
	*/
	HashJoin(const std::shared_ptr<Expression> & condition, const std::vector<ColumnIdentifier> & leftPartOfCondition, const std::vector<ColumnIdentifier> & rightPartOfCondition);
	
	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represents hash antijoin physical operator.
* Operator computes equijoin using hash table.
* First input will be stored in hash table.
*/
class HashAntiJoin : public BinaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition; /**< Anti join condition. */
	std::vector<ColumnIdentifier> leftPartOfCondition; /**< Stores columns from left input, which belongs to condition. */
	std::vector<ColumnIdentifier> rightPartOfCondition; /**< Stores columns from right input, which belongs to condition. */

	/**
	* Creates new instance of HashAntiJoin.
	* @param condition - join condition.
	* @param leftPartOfCondition - condition columns from first input
	* @param rightPartOfCondition - condition columns from second input
	*/
	HashAntiJoin(const std::shared_ptr<Expression> & condition, const std::vector<ColumnIdentifier> & leftPartOfCondition, const std::vector<ColumnIdentifier> & rightPartOfCondition);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represents Merge equijoin.
* Operator computes equiantijoin from given sorted inputs.
*/
class MergeAntiJoin : public BinaryPhysicalOperator
{
public:
	std::shared_ptr<Expression> condition; /**< Anti join condition. */
	std::vector<SortParameter> left; /**< Stored information how first input is sorted. */
	std::vector<SortParameter> right; /**< Stored information how second input is sorted. */

	/**
	* Creates new instance of MergeAntiJoin.
	* @param condition - join condition.
	*/
	MergeAntiJoin(const std::shared_ptr<Expression> & condition);

	void accept(PhysicalOperatorVisitor &v);
};


/**
* Represets set union.
* Operator copies the rst input to output and append the data from second input. 
*/
class UnionOperator : public BinaryPhysicalOperator
{
public:
	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represent hash group algorithm.
*/
class HashGroup : public UnaryPhysicalOperator
{
public:
	std::vector<GroupColumn> groupColumns; /**< Group by columns. */
	std::vector<AgregateFunction> agregateFunctions; /**< Information about agreggate functions. */
	
	/**
	* Creates new instance of HashGroup.
	* @param groupColumns - group by columns.
	* @param agregateFunctions - information about agreggate functions.
	*/
	HashGroup(const std::vector<GroupColumn> & groupColumns, const std::vector<AgregateFunction> & agregateFunctions);
	
	void accept(PhysicalOperatorVisitor &v);

};

/**
* Represents group algorithm, which groups input sorted by group columns.
*/
class SortedGroup : public UnaryPhysicalOperator
{
public:
	std::vector<GroupColumn> groupColumns; /**< Group by columns. */
	std::vector<AgregateFunction> agregateFunctions; /**< Information about agreggate functions. */
	
	/**
	* Creates new instance of HashGroup.
	* @param groupColumns - group by columns.
	* @param agregateFunctions - information about agreggate functions.
	*/
	SortedGroup(const std::vector<GroupColumn> & groupColumns, const std::vector<AgregateFunction> & agregateFunctions);

	void accept(PhysicalOperatorVisitor &v);

};
/**
* This is extended projection operator, it eliminates certain columns and computes new ones.
*/
class ColumnsOperationsOperator : public UnaryPhysicalOperator
{
public:
	std::vector<ColumnOperation> operations; /**< Information about output and new computed columns. */

	/**
	* Creates new instance of ColumnsOperationsOperator.
	* @param operations - information about output and new computed columns.
	*/
	ColumnsOperationsOperator::ColumnsOperationsOperator(const std::vector<ColumnOperation> & operations);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represents operator, which reads whole table using index.
*/
class ScanAndSortByIndex : public NullaryPhysicalOperator
{
public:
	std::string tableName; /**< Table to read. */
	Index index;  /**< Table index to use. */

	/**
	* Creates new instance of ScanAndSortByIndex.
	* @param name of table to read. 
	* @param index - index to user for reading.
	*/
	ScanAndSortByIndex::ScanAndSortByIndex(const std::string & name, const Index & index);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represents operator, which reads whole table.
*/
class TableScan : public NullaryPhysicalOperator
{
public:
	std::string tableName; /**< Table to read. */

	/**
	* Creates new instance of TableScan.
	* @param name of table to read.
	*/
	TableScan(const std::string & name);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Represents operator, which reads part of the table using index.
*/
class IndexScan : public NullaryPhysicalOperator
{
public:
	std::string tableName;   /**< Table to read. */
	std::shared_ptr<Expression> condition; /**< Condition to apply while reading. */
	Index index; /**< Table index to use. */

	/**
	* Creates new instance of IndexScan.
	* @param name of table to read.
	* @param condition to apply while reading.
	* @param index - index to user for reading.
	*/
	IndexScan(const std::string & name, const std::shared_ptr<Expression> & condition, const Index & index);

	void accept(PhysicalOperatorVisitor &v);
};

/**
* Class representing whole physical plan, that means operator tree and aditional informations.
*/
class PhysicalPlan
{
public:
	std::vector<Index> indices; /**< Information if current table has iny indices. */
	PossibleSortParameters sortedBy; /**< Information how is the output sorted by. */
	double timeComplexity; /**< Time, how much will it take to evaluate plan. */
	std::shared_ptr<PhysicalOperator> plan; /**< Pointer on the root of operator tree. */
	
	/**
	* Creates new instance of PhysicalPlan.
	*/
	PhysicalPlan();

	/**
	* Creates new instance of PhysicalPlan.
	* @param op - new tree root
	* @param numberOfRows - new relation size
	* @param time - time to execute op
	* @param cols - new columns
	*/
	PhysicalPlan(NullaryPhysicalOperator * op, double numberOfRows, double time, std::vector<ColumnInfo> & cols);

	/**
	* Creates new instance of PhysicalPlan.
	* @param op - new tree root
	* @param numberOfRows - new relation size
	* @param time - time to execute op
	* @param newColumns - new columns
	*/
	PhysicalPlan(NullaryPhysicalOperator * op, double numberOfRows, double time, const std::map<int, ColumnInfo> & newColumns);

	/**
	* Creates new instance of PhysicalPlan. It connects op to operator tree.
	* @param op - new tree root
	* @param newSize - new relation size
	* @param time - time to execute op
	* @param newColumns - new columns
	* @param oldPlan - new child for op
	*/
	PhysicalPlan(UnaryPhysicalOperator * op, double newSize, double time, const std::map<int, ColumnInfo> & newColumns, const std::shared_ptr<PhysicalPlan> & oldPlan);

	/**
	* Creates new instance of PhysicalPlan. It connects op to operator tree.
	* @param op - new tree root
	* @param newSize - new relation size
	* @param time - time to execute op
	* @param newColumns - new columns
	* @param oldPlan1 - new left child for op
	* @param oldPlan2 - new right child for op
	*/
	PhysicalPlan(BinaryPhysicalOperator * op, double newSize, double time, const std::map<int, ColumnInfo> & newColumns, const std::shared_ptr<PhysicalPlan> & oldPlan1, const std::shared_ptr<PhysicalPlan> & oldPlan2);

	/**
	* Compares two plans. Operation less is defined: i < j <=> i.timeComplexity<j.timeComplexity
	* @param i - first plan
	* @param j - second plan
	* @returns true if i<j
	*/
	static bool Comparator(std::shared_ptr<PhysicalPlan> & i, std::shared_ptr<PhysicalPlan> & j);

};


#endif


