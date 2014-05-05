#ifndef PhysicalOpearatorVisitorsHPP
#define PhysicalOpearatorVisitorsHPP

#include <memory>
#include "PhysicalOperator.h"

class PhysicalOperatorVisitor
{
public:
	virtual void visitPhysicalOperator(PhysicalOperator * node);

	virtual void visitNullaryPhysicalOperator(NullaryPhysicalOperator * node);

	virtual void visitUnaryPhysicalOperator(UnaryPhysicalOperator * node);

	virtual void visitBinaryPhysicalOperator(BinaryPhysicalOperator * node);

	virtual void visitFilter(Filter * node);

	virtual void visitFilterKeepingOrder(FilterKeepingOrder * node);

	virtual void visitSortOperator(SortOperator * node);

	virtual void visitMergeEquiJoin(MergeEquiJoin * node);

	virtual void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	virtual void visitCrossJoin(CrossJoin * node);

	virtual void visitHashJoin(HashJoin * node);

	virtual void visitUnionOperator(UnionOperator * node);

	virtual void visitHashGroup(HashGroup * node);

	virtual void visitSortedGroup(SortedGroup * node);

	virtual void visitColumnsOperationsOperator(ColumnsOperationsOperator * node);

	virtual void visitScanAndSortByIndex(ScanAndSortByIndex * node);

	virtual void visitTableScan(TableScan * node);

	virtual void visitIndexScan(IndexScan * node);
};

class PhysicalOperatorDrawingVisitor : public PhysicalOperatorVisitor
{
public:
	std::string result;
	ulong nodeCounter;
	PhysicalOperatorDrawingVisitor();

	void generateText(std::string & label,NullaryPhysicalOperator * node);

	void generateText(std::string & label,UnaryPhysicalOperator * node);

	void generateText(std::string & label,BinaryPhysicalOperator * node);

	void visitFilter(Filter * node);

	void visitFilterKeepingOrder(FilterKeepingOrder * node);

	void visitSortOperator(SortOperator * node);

	virtual void visitMergeEquiJoin(MergeEquiJoin * node);

	virtual void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	void visitCrossJoin(CrossJoin * node);

	void visitHashJoin(HashJoin * node);

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

	std::string writePlan(std::shared_ptr<PhysicalOperator> plan);
	
	std::string getColumnNumberOutput(const std::map<int, ColumnInfo> & columns);

	std::string getColumnTypeOutput(const std::map<int, ColumnInfo> & columns);

	std::string getId();

	void BoboxPlanWritingPhysicalOperatorVisitor::convertColumns(const std::map<int, ColumnInfo> & columns, std::map<int, int> & result);

	void writeNullaryOperator(const std::string & type, const std::map<int, ColumnInfo> & columns, const std::string & costructorParameters);

	void writeUnaryOperator(const std::string & type, UnaryPhysicalOperator * node, const std::string & costructorParameters);

	void writeBinaryOperator(const std::string & type, BinaryPhysicalOperator * node, const std::string & costructorParameters);

	std::string writeGroupParameters(const std::map<int, ColumnInfo> & outputColumns, const std::map<int, ColumnInfo> & inputColumns,
		const std::vector<GroupColumn> & groupColumns, const std::vector<AgregateFunction> & agregateFunctions);
	

	void visitFilter(Filter * node);

	void visitFilterKeepingOrder(FilterKeepingOrder * node);

	void visitSortOperator(SortOperator * node);

	void visitMergeEquiJoin(MergeEquiJoin * node);

	void visitMergeNonEquiJoin(MergeNonEquiJoin * node);

	void visitCrossJoin(CrossJoin * node);

	void visitHashJoin(HashJoin * node);

	void visitUnionOperator(UnionOperator * node);

	void visitHashGroup(HashGroup * node);

	void visitSortedGroup(SortedGroup * node);

	void visitColumnsOperationsOperator(ColumnsOperationsOperator * node);

	void visitScanAndSortByIndex(ScanAndSortByIndex * node);

	void visitTableScan(TableScan * node);

	void visitIndexScan(IndexScan * node);

};
#endif