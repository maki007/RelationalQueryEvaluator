#ifndef PhysicalOpearatorVisitorsHPP
#define PhysicalOpearatorVisitorsHPP

#include "PhysicalOperator.h"

class PhysicalOperatorVisitor
{
public:
	virtual void visit(PhysicalOperator * node);

	virtual void visit(NullaryPhysicalOperator * node);

	virtual void visit(UnaryPhysicalOperator * node);

	virtual void visit(BinaryPhysicalOperator * node);

	virtual void visit(Filter * node);

	virtual void visit(FilterKeepingOrder * node);

	virtual void visit(SortOperator * node);

	virtual void visit(MergeJoin * node);

	virtual void visit(IndexJoin * node);

	virtual void visit(CrossJoin * node);

	virtual void visit(HashJoin * node);

	virtual void visit(UnionOperator * node);

	virtual void visit(HashGroup * node);

	virtual void visit(SortedGroup * node);

	virtual void visit(ColumnsOperationsOperator * node);

	virtual void visit(ScanAndSortByIndex * node);

	virtual void visit(TableScan * node);

	virtual void visit(IndexScan * node);
};

class PhysicalOperatorDrawingVisitor : public PhysicalOperatorVisitor
{
public:
	std::string result;
	std::size_t nodeCounter;
	PhysicalOperatorDrawingVisitor();

	void generateText(std::string & label,NullaryPhysicalOperator * node);

	void generateText(std::string & label,UnaryPhysicalOperator * node);

	void generateText(std::string & label,BinaryPhysicalOperator * node);

	void visit(Filter * node);

	void visit(FilterKeepingOrder * node);

	void visit(SortOperator * node);

	void visit(MergeJoin * node);

	void visit(IndexJoin * node);

	void visit(CrossJoin * node);

	void visit(HashJoin * node);

	void visit(UnionOperator * node);

	void visit(HashGroup * node);

	void visit(SortedGroup * node);

	void visit(ColumnsOperationsOperator * node);

	void visit(ScanAndSortByIndex * node);

	void visit(TableScan * node);

	void visit(IndexScan * node);
};


#endif