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

	void generateText(std::string & label, UnaryAlgebraNodeBase * node);

	void generateText(std::string & label, BinaryAlgebraNodeBase * node);

	void generateText(std::string & label, GroupedAlgebraNode * node);

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
	std::map<std::string, ColumnInfo> outputColumns;
	int lastId;

public:

	bool containsErrors;

	SemanticChecker();

	int nextId();

	template <typename T>
	void checkJoinOutPutParameters(std::map<std::string, ColumnInfo> & outputColumns0, std::map<std::string, ColumnInfo> & outputColumns1, T * node)
	{
		outputColumns.clear();
		for (auto it = node->outputColumns.begin(); it != node->outputColumns.end(); ++it)
		{
			if (outputColumns.find(it->newColumn.name) == outputColumns.end())
			{
				if (it->input == 0)
				{
					if (outputColumns0.find(it->column.name) == outputColumns0.end())
					{
						ReportError("Column not found in input 0");
					}
					else
					{
						outputColumns[it->newColumn.name] = ColumnInfo(it->newColumn.name, "");
						outputColumns[it->newColumn.name].column.id = outputColumns0[it->column.name].column.id;

					}
				}
				else if (it->input == 1)
				{
					if (outputColumns1.find(it->column.name) == outputColumns1.end())
					{
						ReportError("Column not found in input 1");
					}
					else
					{
						outputColumns[it->newColumn.name] = ColumnInfo(it->newColumn.name, "");
						outputColumns[it->newColumn.name].column.id = outputColumns1[it->column.name].column.id;
					}
				}
				else
				{
					throw new std::exception("Error");
				}
			}
			else
			{
				ReportError("Cannot use same column twice");
			}
		}

	}

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

	void resolveJoins(BinaryAlgebraNodeBase * node, GroupedJoin * groupedOperator, std::vector<std::shared_ptr<AlgebraNodeBase> > & oldChildren);
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
	std::set<ulong> inputs;
	ConditionType type;
};

class JoinInfo
{
public:
	std::vector<std::shared_ptr<PhysicalPlan> > plans;
	std::set<ulong> processedPlans;
	std::set<ulong> unProcessedPlans;
	std::vector<std::shared_ptr<ConditionInfo>> condition;
	std::map <int,JoinColumnInfo> columns;
	static bool Comparator(const JoinInfo& lhs, const JoinInfo&rhs)
	{
		return (lhs.plans[0]->timeComplexity < rhs.plans[0]->timeComplexity);
	}
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
	void insertPlan(std::vector<std::shared_ptr<PhysicalPlan> > & plans, std::shared_ptr<PhysicalPlan> & plan);

	std::shared_ptr<PhysicalPlan> generateSortParameters(const std::vector<SortParameter> & parameters, const std::shared_ptr<PhysicalPlan> & result);

	std::vector<std::shared_ptr<Expression> > serializeExpression(std::shared_ptr<Expression> condition);

	std::shared_ptr<Expression> deserializeExpression(const std::vector<std::shared_ptr<Expression> > & condition);

	void join(const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan);

	std::vector<ulong> getAllSubsets(std::vector<ulong> & arr, ulong n, ulong k) const;

	void greedyJoin(std::vector<JoinInfo>::iterator &it, std::set<ulong>::iterator &it2, std::vector<JoinInfo> & plans, std::vector<JoinInfo> & heap);

	std::shared_ptr<Expression> deserializeConditionInfo(const std::vector<std::shared_ptr<ConditionInfo>> & a, const std::vector<std::shared_ptr<ConditionInfo>> & b)
	{
		std::vector<std::shared_ptr<Expression> >  data;
		for (auto it = a.begin(); it != a.end();++it)
		{
			data.push_back((*it)->condition);
		}
		for (auto it = b.begin(); it != b.end(); ++it)
		{
			data.push_back((*it)->condition);
		}
		return deserializeExpression(data);
	}

	template< typename T>
	ulong setIndex(const T input) const
	{
		ulong result = 0;

		for (auto it = input.begin(); it != input.end(); ++it)
		{
			result |= ulong(1) << (*it);
		}

		return result;
	}
};

#endif