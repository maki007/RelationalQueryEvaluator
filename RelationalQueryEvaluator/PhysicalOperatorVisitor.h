#ifndef PhysicalOpearatorVisitorsHPP
#define PhysicalOpearatorVisitorsHPP

#include <memory>
#include "PhysicalOperator.h"

/**
* Base class for physical operator tree visitors.
* Every virtual method does nothing only visits node all children.
*/
class PhysicalOperatorVisitor
{
public:

	/**
	* Visits physical operator Filter.
	* @param node visited Filter.
	*/
	virtual void visitFilter(Filter * node);

	/**
	* Visits physical operator FilterKeepingOrder.
	* @param node visited FilterKeepingOrder.
	*/
	virtual void visitFilterKeepingOrder(FilterKeepingOrder * node);

	/**
	* Visits physical operator SortOperator.
	* @param node visited SortOperator.
	*/
	virtual void visitSortOperator(SortOperator * node);

	/**
	* Visits physical operator MergeEquiJoin.
	* @param node visited MergeEquiJoin.
	*/
	virtual void visitMergeEquiJoin(MergeEquiJoin * node);

	/**
	* Visits physical operator MergeNonEquiJoin.
	* @param node visited MergeNonEquiJoin.
	*/
	virtual void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	/**
	* Visits physical operator CrossJoin.
	* @param node visited CrossJoin.
	*/
	virtual void visitCrossJoin(CrossJoin * node);

	/**
	* Visits physical operator HashJoin.
	* @param node visited HashJoin.
	*/
	virtual void visitHashJoin(HashJoin * node);

	/**
	* Visits physical operator HashAntiJoin.
	* @param node visited HashAntiJoin.
	*/
	virtual void visitHashAntiJoin(HashAntiJoin * node);

	/**
	* Visits physical operator MergeAntiJoin.
	* @param node visited MergeAntiJoin.
	*/
	virtual void visitMergeAntiJoin(MergeAntiJoin * node);

	/**
	* Visits physical operator UnionOperator.
	* @param node visited UnionOperator.
	*/
	virtual void visitUnionOperator(UnionOperator * node);

	/**
	* Visits physical operator HashGroup.
	* @param node visited HashGroup.
	*/
	virtual void visitHashGroup(HashGroup * node);

	/**
	* Visits physical operator SortedGroup.
	* @param node visited SortedGroup.
	*/
	virtual void visitSortedGroup(SortedGroup * node);

	/**
	* Visits physical operator ColumnsOperationsOperator.
	* @param node visited ColumnsOperationsOperator.
	*/
	virtual void visitColumnsOperationsOperator(ColumnsOperationsOperator * node);

	/**
	* Visits physical operator ScanAndSortByIndex.
	* @param node visited ScanAndSortByIndex.
	*/
	virtual void visitScanAndSortByIndex(ScanAndSortByIndex * node);

	/**
	* Visits physical operator TableScan.
	* @param node visited TableScan.
	*/
	virtual void visitTableScan(TableScan * node);

	/**
	* Visits physical operator IndexScan.
	* @param node visited IndexScan.
	*/
	virtual void visitIndexScan(IndexScan * node);
};

/**
* Vsitor generates serialize physical tree to dot code.
*/
class PhysicalOperatorDrawingVisitor : public PhysicalOperatorVisitor
{
public:
	std::string result; /**< Final dot representation. */
	ulong nodeCounter;  /**< Variable that counts nodes. */

	/**
	* Creates new instance of PhysicalOperatorDrawingVisitor.
	*/
	PhysicalOperatorDrawingVisitor();

	/**
	* Generates node representation in dot.
	* @param label - label for newly generated node
	* @param node - processed node
	*/
	void generateText(std::string & label, NullaryPhysicalOperator * node);

	/**
	* Generates node representation in dot.
	* Calls this visitor on child and connects generated nodes.
	* @param label - label for newly generated node
	* @param node  processed node
	*/
	void generateText(std::string & label, UnaryPhysicalOperator * node);

	/**
	* Generates node representation in dot.
	* Calls this visitor on children and connects generated nodes.
	* @param label - label for newly generated node
	* @param node - processed node
	*/
	void generateText(std::string & label, BinaryPhysicalOperator * node);

	/**
	* Generates group parametes as string.
	* @param groupColumns - group by columns
	* @param agregateFunctions - agregate function information
	* @returns string representation of group paramaters
	*/
	std::string PhysicalOperatorDrawingVisitor::writeGroupParameters(const std::vector<GroupColumn> & groupColumns, const std::vector<AgregateFunction> & agregateFunctions);

	void visitFilter(Filter * node);

	void visitFilterKeepingOrder(FilterKeepingOrder * node);

	void visitSortOperator(SortOperator * node);

	void visitMergeEquiJoin(MergeEquiJoin * node);

	void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	void visitCrossJoin(CrossJoin * node);

	void visitHashJoin(HashJoin * node);

	void visitHashAntiJoin(HashAntiJoin * node);

	void visitMergeAntiJoin(MergeAntiJoin * node);

	void visitUnionOperator(UnionOperator * node);

	void visitHashGroup(HashGroup * node);

	void visitSortedGroup(SortedGroup * node);

	void visitColumnsOperationsOperator(ColumnsOperationsOperator * node);

	void visitScanAndSortByIndex(ScanAndSortByIndex * node);

	void visitTableScan(TableScan * node);

	void visitIndexScan(IndexScan * node);
};

/**
* Clones physial operator tree.
*/
class CloningPhysicalOperatorVisitor : public PhysicalOperatorVisitor
{
public:
	std::shared_ptr<PhysicalOperator> result; /**< Cloned result. */

	/**
	* Calls this visitor on unary operator's child.
	* @param res - unary operator
	*/
	void processUnaryOperator(UnaryPhysicalOperator * res);

	/**
	* Calls this visitor on binary operator's children.
	* @param res - binary operator
	*/
	void processBinaryOperator(BinaryPhysicalOperator * res);

	void visitFilter(Filter * node);

	void visitFilterKeepingOrder(FilterKeepingOrder * node);

	void visitSortOperator(SortOperator * node);

	void visitMergeEquiJoin(MergeEquiJoin * node);

	void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	void visitCrossJoin(CrossJoin * node);

	void visitHashJoin(HashJoin * node);

	void visitHashAntiJoin(HashAntiJoin * node);

	void visitMergeAntiJoin(MergeAntiJoin * node);

	void visitUnionOperator(UnionOperator * node);

	void visitHashGroup(HashGroup * node);

	void visitSortedGroup(SortedGroup * node);

	void visitColumnsOperationsOperator(ColumnsOperationsOperator * node);

	void visitScanAndSortByIndex(ScanAndSortByIndex * node);

	void visitTableScan(TableScan * node);

	void visitIndexScan(IndexScan * node);
};

/**
* After physical plan is generated some sort parameters can be unknown (direciotn or order of columns).
* This visitor visits whole tree from top a set this parameters to known values.
*/
class SortResolvingPhysicalOperatorVisitor : public PhysicalOperatorVisitor
{
public:
	std::vector<SortParameter> sortParameters; /**< Current sorted state. */

	void visitFilter(Filter * node);

	void visitFilterKeepingOrder(FilterKeepingOrder * node);

	void visitSortOperator(SortOperator * node);

	void visitMergeEquiJoin(MergeEquiJoin * node);

	void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	void visitCrossJoin(CrossJoin * node);

	void visitHashJoin(HashJoin * node);

	void visitHashAntiJoin(HashAntiJoin * node);

	void visitMergeAntiJoin(MergeAntiJoin * node);

	void visitUnionOperator(UnionOperator * node);

	void visitHashGroup(HashGroup * node);

	void visitSortedGroup(SortedGroup * node);

	void visitColumnsOperationsOperator(ColumnsOperationsOperator * node);

	void visitScanAndSortByIndex(ScanAndSortByIndex * node);

	void visitTableScan(TableScan * node);

	void visitIndexScan(IndexScan * node);
};

/**
* Vsitor generates serialize physical tree to dot code.
* It is equal to PhysicalOperatorDrawingVisitor except it doens't write sorts, which output and input are sorted same size.
*/
class PhysicalOperatorDrawingVisitorWithouSorts : public PhysicalOperatorDrawingVisitor
{
	void visitSortOperator(SortOperator * node);
};


/**
* Generates bobox string output.
*/
class BoboxPlanWritingPhysicalOperatorVisitor : public PhysicalOperatorVisitor
{
private:
	ulong numberOfLeafs; /**< Counts number of leafs in tree. */

	std::string declarations; /**< Stores declaration of bobox operatos.*/

	std::string code; /**< Stores connections between bobox operators. */

	ulong lastId; /**< Stores last generated uniwue id for node. */

	std::string lastWrittenNode; /**< Stores last written node, so the next node can connect to it. */

public:

	/**
	* Generates physical operator declaration.
	* @param type - type of operator
	* @param inputColumns - input columns of operator
	* @param outputColumns - output columns of operator
	* @param name - new variable name
	* @param constructParameters
	* @returns string representation of physical operator.
	*/
	std::string declaration(const std::string & type, const std::string & inputColumns,
		const std::string & outputColumns, const std::string & name, const std::string & constructParameters);

	/**
	* Generates string representation of connection between operators.
	* @param from - new child of node to
	* @param to - node to connect
	* @returns string representation of connection between nodes.
	*/
	std::string connect(const std::string & from, const std::string & to);

	/**
	* Creates new instace of BoboxPlanWritingPhysicalOperatorVisitor.
	*/
	BoboxPlanWritingPhysicalOperatorVisitor();

	/**
	* Generates bobox output of physical plan.
	* @param plan - plan to write
	* @returns bobox plan representation
	*/
	std::string writePlan(std::shared_ptr<PhysicalOperator> & plan);

	/**
	* Generates string output from columns. Output example (columnsName1,columnsName2,columnsName3)
	* @param columns - columns to generate output from
	*/
	std::string getColumnTypeOutput(const std::map<int, ColumnInfo> & columns);

	/**
	* Generates string output from columns. Output example: (columnsType1,columnsType2,columnsType3)
	* @param columns - columns to generate output from
	*/
	std::string getColumnNameOutput(const std::map<int, ColumnInfo> & columns);

	/**
	* Generates new string unique identifier for operator.
	* @return new string unique identifier.
	*/
	std::string getId();

	/**
	* Convert structure map<column id, column info> to map<column id, order nummer in first structure> 
	* This output is used for numbering operator columns.
	* @param columns - in parameter
	* @param result - out parameter
	*/
	void BoboxPlanWritingPhysicalOperatorVisitor::convertColumns(const std::map<int, ColumnInfo> & columns, std::map<int, int> & result);

	/**
	* Writes daclaration of nullary operator to variable declarations.
	* @param type - opearator type
	* @param columns - output columns of operator
	* @param costructorParameters - parameter for constructor like table name
	*/
	void writeNullaryOperator(const std::string & type, const std::map<int, ColumnInfo> & columns, const std::string & costructorParameters);

	/**
	* Writes daclaration of nullary operator to variable declarations.
	* @param type - opearator type
	* @param node - node information
	* @param costructorParameters - parameter for constructor like table name
	*/
	void writeUnaryOperator(const std::string & type, UnaryPhysicalOperator * node, const std::string & costructorParameters);

	/**
	* Writes daclaration of nullary operator to variable declarations.
	* @param type - opearator type
	* @param node - node information
	* @param costructorParameters - parameter for constructor like table name
	*/
	void writeBinaryOperator(const std::string & type, BinaryPhysicalOperator * node, const std::string & costructorParameters);

	/**
	* Generates group parameters.
	* @param outputColumns - output columns of group operator
	* @param inputColumns - input columns of group operator
	* @param groupColumns - information from group by
	* @param agregateFunctions - information about agreagate function 
	* @return string representation of group parameters
	*/
	std::string writeGroupParameters(const std::map<int, ColumnInfo> & outputColumns, const std::map<int, ColumnInfo> & inputColumns,
		const std::vector<GroupColumn> & groupColumns, const std::vector<AgregateFunction> & agregateFunctions);
	
	/**
	* Generates columns from left, right input and from output.
	* Example: left="0,1,2,3,4,5",right="6,7",out="1,2,3,4"
	* @param node - Join node
	* @returns string representation of columns
	*/
	std::string BoboxPlanWritingPhysicalOperatorVisitor::writeJoinParameters(BinaryPhysicalOperator * node);

	/**
	* Generates information about condition in hash equijoin.
	* Example: leftPartOfCondition="0,1",rightPartOfCondition="2,3"
	* @param left - left part of condition
	* @param right - right part of condition
	* @param node - join operator
	* @return string representation of join codition
	*/
	std::string writeEquiJoinParameters(const std::vector<ColumnIdentifier> & left, const std::vector<ColumnIdentifier> & right, BinaryPhysicalOperator * node);

	/**
	* Generates information about condition in merge equijoin.
	* Example: left="0",right="1",out="0,1",leftPartOfCondition="0:D",rightPartOfCondition="1:D"
	* @param left - left part of condition
	* @param right - right part of condition
	* @param node - join operator
	* @return string representation of join codition
	*/
	std::string writeMergeEquiJoinParameters(const std::vector<SortParameter> & left, const std::vector<SortParameter> & right, BinaryPhysicalOperator * node);

	void visitFilter(Filter * node);

	void visitFilterKeepingOrder(FilterKeepingOrder * node);

	void visitSortOperator(SortOperator * node);

	void visitMergeEquiJoin(MergeEquiJoin * node);

	void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	void visitCrossJoin(CrossJoin * node);

	void visitHashJoin(HashJoin * node);

	void visitHashAntiJoin(HashAntiJoin * node);

	void visitMergeAntiJoin(MergeAntiJoin * node);

	void visitUnionOperator(UnionOperator * node);

	void visitHashGroup(HashGroup * node);

	void visitSortedGroup(SortedGroup * node);

	void visitColumnsOperationsOperator(ColumnsOperationsOperator * node);

	void visitScanAndSortByIndex(ScanAndSortByIndex * node);

	void visitTableScan(TableScan * node);

	void visitIndexScan(IndexScan * node);

};
#endif