#ifndef AlgebraVisitorHPP
#define AlgebraVisitorHPP

#include "Algebra.h"

class AlgebraVisitor
{
public:
	virtual void visit(AlgebraNodeBase * node);
	virtual void visit(UnaryAlgebraNodeBase * node);

	virtual void visit(BinaryAlgebraNodeBase * node);

	virtual void visit(Table * node);

	virtual void visit(Sort * node);

	virtual void visit(Group * node);

	virtual void visit(ColumnOperations * node);

	virtual void visit(Selection * node);

	virtual void visit(Join * node);

	virtual void visit(AntiJoin * node);
	
	virtual void visit(Difference * node);

	virtual void visit(Union * node);

	virtual void visit(Intersection * node);

	virtual void visit(GroupedAlgebraNode * node);

	virtual void visit(GroupedIntersection * node);

	virtual void visit(GroupedUnion * node);

	virtual void visit(GroupedDifference * node);

	virtual void visit(GroupedJoin * node);

};

class GraphDrawingVisitor : public AlgebraVisitor
{
public:
	std::string result;
	int nodeCounter;
	GraphDrawingVisitor();

	void generateText(std::string label ,UnaryAlgebraNodeBase * node);

	void generateText(std::string label , BinaryAlgebraNodeBase * node);

	void generateText(std::string label , GroupedAlgebraNode * node);

	void visit(Sort * node);

	void visit(Group * node);

	void visit(Table * node);

	void visit(ColumnOperations * node);

	void visit(Selection * node);

	void visit(Join * node);

	void visit(AntiJoin * node);

	void visit(Difference * node);

	void visit(Union * node);

	void visit(Intersection * node);

	void visit(GroupedIntersection * node);

	void visit(GroupedUnion * node);

	void visit(GroupedDifference * node);

	void visit(GroupedJoin * node);

};


class GroupingVisitor : public AlgebraVisitor
{
public:
	GroupingVisitor();

	void visit(Intersection * node);

	void visit(Union * node);

	void visit(Difference * node);

	void visit(Join * node);

	void visit(AntiJoin * node);
};


#endif