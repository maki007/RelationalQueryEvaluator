#ifndef AlgebraVisitorHPP
#define AlgebraVisitorHPP

#include "Algebra.h"
#include "PhysicalOperator.h"
#include <map>
#include <set>

class AlgebraVisitor
{
public:
	virtual void visit(AlgebraNodeBase * node);

	virtual void visit(UnaryAlgebraNodeBase * node);

	virtual void visit(BinaryAlgebraNodeBase * node);

	virtual void visit(NullaryAlgebraNodeBase * node);

	virtual void visit(GroupedAlgebraNode * node);

	virtual void visit(Table * node);

	virtual void visit(Sort * node);

	virtual void visit(Group * node);

	virtual void visit(ColumnOperations * node);

	virtual void visit(Selection * node);

	virtual void visit(Join * node);

	virtual void visit(AntiJoin * node);
	
	virtual void visit(Union * node);

	virtual void visit(GroupedJoin * node);

};

class GraphDrawingVisitor : public AlgebraVisitor
{
public:
	std::string result;
	int nodeCounter;
	GraphDrawingVisitor();

	void generateText(std::string & label ,UnaryAlgebraNodeBase * node);

	void generateText(std::string & label , BinaryAlgebraNodeBase * node);

	void generateText(std::string & label , GroupedAlgebraNode * node);

	void visit(Sort * node);

	void visit(Group * node);

	void visit(Table * node);

	void visit(ColumnOperations * node);

	void visit(Selection * node);

	void visit(Join * node);

	void visit(AntiJoin * node);

	void visit(Union * node);

	void visit(GroupedJoin * node);

};


class SemanticChecker : public AlgebraVisitor
{
private:
	void ReportError(const char * error);
	std::map<std::string,ColumnInfo> outputColumns; 
public:
	bool containsErrors;
	
	SemanticChecker();
	
	void visit(Table * node);

	void visit(Sort * node);

	void visit(Group * node);

	void visit(ColumnOperations * node);

	void visit(Selection * node);

	void visit(Join * node);

	void visit(AntiJoin * node);
	
	void visit(Union * node);

	void visit(GroupedJoin * node);

};

class GroupingVisitor : public AlgebraVisitor
{
private:

public:
	GroupingVisitor();

	void visit(ColumnOperations * node);

	void visit(Selection * node);

	void visit(Join * node);

	void resolveJoins(BinaryAlgebraNodeBase * node,GroupedJoin * groupedOperator,std::vector<std::shared_ptr<AlgebraNodeBase> > & oldChildren);
};
class JoinInfo
{
public:
	std::vector<std::shared_ptr<PhysicalPlan> > plans;
	std::set<std::size_t> processedPlans;
	std::set<std::size_t> unProcessedPlans;
	static bool compare (const JoinInfo& lhs, const JoinInfo&rhs)
	{
		return (lhs.plans[0]->timeComplexity<rhs.plans[0]->timeComplexity);
	}
};

enum class ConditionType
{
	EQUALS,
	LOWER,
	OTHER

};

class ConditionInfo
{
public:
	std::shared_ptr<Expression> condition;
	std::vector<bool> inputs;
	ConditionType type;
};

class AlgebraCompiler : public AlgebraVisitor
{
public:
	static const ulong NUMBER_OF_PLANS;
	static const ulong  LIMIT_FOR_GREEDY_JOIN_ORDER_ALGORITHM;
	static const ulong AlgebraCompiler::MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM;
	std::vector<std::shared_ptr<PhysicalPlan> > result;

	void visit(Table * node);

	void visit(Sort * node);

	void visit(Group * node);

	void visit(ColumnOperations * node);

	void visit(Selection * node);

	void visit(Join * node);

	void visit(AntiJoin * node);
	
	void visit(Union * node);

	void visit(GroupedJoin * node);


private:
	std::shared_ptr<PhysicalPlan> generateSortParameters(const std::vector<SortParameter> & parameters,const std::shared_ptr<PhysicalPlan> & result);

	std::vector<std::shared_ptr<PhysicalPlan> > getBestPlans(std::vector<std::shared_ptr<PhysicalPlan> > & plans);
	
	std::vector<std::shared_ptr<Expression> > serializeExpression(std::shared_ptr<Expression> condition);

	std::shared_ptr<Expression> deserializeExpression(const std::vector<std::shared_ptr<Expression> > & condition);

	void join(const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan);

	std::vector<std::size_t> getAllSubsets(std::vector<std::size_t> & arr, std::size_t n, std::size_t k) const;

	void greedyJoin(std::vector<JoinInfo>::iterator &it, std::set<std::size_t>::iterator &it2, std::vector<JoinInfo> & plans, std::vector<JoinInfo> & heap);

	template< typename T>
	std::size_t setIndex(const T input) const
	{
		std::size_t result = 0;

		for (auto it = input.begin(); it != input.end(); ++it)
		{
			result |= ulong(1) << (*it);
		}

		return result;
	}
};

#endif