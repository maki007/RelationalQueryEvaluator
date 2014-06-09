#ifndef PhysicalOpearatorVisitorsHPP
#define PhysicalOpearatorVisitorsHPP

#include <memory>
#include "PhysicalOperator.h"

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

class PhysicalOperatorDrawingVisitor : public PhysicalOperatorVisitor
{
public:
	std::string result;
	ulong nodeCounter;
	PhysicalOperatorDrawingVisitor();

	void generateText(std::string & label, NullaryPhysicalOperator * node);

	void generateText(std::string & label, UnaryPhysicalOperator * node);

	void generateText(std::string & label, BinaryPhysicalOperator * node);

	std::string PhysicalOperatorDrawingVisitor::writeGroupParameters(const std::vector<GroupColumn> & groupColumns, const std::vector<AgregateFunction> & agregateFunctions);

	void visitFilter(Filter * node);

	void visitFilterKeepingOrder(FilterKeepingOrder * node);

	void visitSortOperator(SortOperator * node);

	virtual void visitMergeEquiJoin(MergeEquiJoin * node);

	virtual void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

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

class CloningPhysicalOperatorVisitor : public PhysicalOperatorVisitor
{
public:
	std::shared_ptr<PhysicalOperator> result;

	void processUnaryOperator(UnaryPhysicalOperator * res);

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

class SortResolvingPhysicalOperatorVisitor : public PhysicalOperatorVisitor
{
public:
	std::vector<SortParameter> sortParameters;
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

class PhysicalOperatorDrawingVisitorWithouSorts : public PhysicalOperatorDrawingVisitor
{
	void visitSortOperator(SortOperator * node);


};


class BoboxPlanWritingPhysicalOperatorVisitor : public PhysicalOperatorVisitor
{
private:
	ulong numberOfLeafs;

	std::string declarations;

	std::string code;

	ulong lastId;

	std::string lastWritttenNode;

public:

	std::string declaration(const std::string & type, const std::string & inputColumns,
		const std::string & outputColumns, const std::string & name, const std::string & constructParameters);

	std::string connect(const std::string & from, const std::string & to);


	BoboxPlanWritingPhysicalOperatorVisitor();

	std::string writePlan(std::shared_ptr<PhysicalOperator> & plan);

	std::string getColumnTypeOutput(const std::map<int, ColumnInfo> & columns);

	std::string getColumnNameOutput(const std::map<int, ColumnInfo> & columns);

	std::string getId();

	void BoboxPlanWritingPhysicalOperatorVisitor::convertColumns(const std::map<int, ColumnInfo> & columns, std::map<int, int> & result);

	void writeNullaryOperator(const std::string & type, const std::map<int, ColumnInfo> & columns, const std::string & costructorParameters);

	void writeUnaryOperator(const std::string & type, UnaryPhysicalOperator * node, const std::string & costructorParameters);

	void writeBinaryOperator(const std::string & type, BinaryPhysicalOperator * node, const std::string & costructorParameters);

	std::string writeGroupParameters(const std::map<int, ColumnInfo> & outputColumns, const std::map<int, ColumnInfo> & inputColumns,
		const std::vector<GroupColumn> & groupColumns, const std::vector<AgregateFunction> & agregateFunctions);
	std::string BoboxPlanWritingPhysicalOperatorVisitor::writeJoinParameters(BinaryPhysicalOperator * node);

	std::string writeEquiJoinParameters(const std::vector<ColumnIdentifier> & left, const std::vector<ColumnIdentifier> & right, BinaryPhysicalOperator * node);

	std::string  writeMergeEquiJoinParameters(const std::vector<SortParameter> & left, const std::vector<SortParameter> & right, BinaryPhysicalOperator * node);

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