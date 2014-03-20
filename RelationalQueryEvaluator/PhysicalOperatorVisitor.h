#ifndef PhysicalOpearatorVisitorsHPP
#define PhysicalOpearatorVisitorsHPP

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

	virtual void visitMergeJoin(MergeJoin * node);

	virtual void visitNestedLoopJoin(NestedLoopJoin * node);

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

	void visitMergeJoin(MergeJoin * node);

	void visitNestedLoopJoin(NestedLoopJoin * node);

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