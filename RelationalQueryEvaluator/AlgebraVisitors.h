#ifndef AlgebraVisitorHPP
#define AlgebraVisitorHPP

#include "Algebra.h"
#include "PhysicalOperator.h"
#include <map>
#include <set>

class AlgebraVisitor
{
public:
	virtual void visitAlgebraNodeBase(AlgebraNodeBase * node);

	virtual void visitUnaryAlgebraNodeBase(UnaryAlgebraNodeBase * node);

	virtual void visitBinaryAlgebraNodeBase(BinaryAlgebraNodeBase * node);

	virtual void visitNullaryAlgebraNodeBase(NullaryAlgebraNodeBase * node);

	virtual void visitGroupedAlgebraNode(GroupedAlgebraNode * node);

	virtual void visitTable(Table * node);

	virtual void visitSort(Sort * node);

	virtual void visitGroup(Group * node);

	virtual void visitColumnOperations(ColumnOperations * node);

	virtual void visitSelection(Selection * node);

	virtual void visitJoin(Join * node);

	virtual void visitAntiJoin(AntiJoin * node);

	virtual void visitUnion(Union * node);

	virtual void visitGroupedJoin(GroupedJoin * node);

	static std::vector<std::shared_ptr<Expression> > serializeExpression(std::shared_ptr<Expression> condition);

	static std::shared_ptr<Expression> deserializeExpression(const std::vector<std::shared_ptr<Expression> > & condition);

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

	void visitSort(Sort * node);

	void visitGroup(Group * node);

	void visitTable(Table * node);

	void visitColumnOperations(ColumnOperations * node);

	void visitSelection(Selection * node);

	void visitJoin(Join * node);

	void visitAntiJoin(AntiJoin * node);

	void visitUnion(Union * node);

	void visitGroupedJoin(GroupedJoin * node);

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
			if (outputColumns.find(it->newColumn) == outputColumns.end())
			{
				if (it->input == 0)
				{
					if (outputColumns0.find(it->column.name) == outputColumns0.end())
					{
						ReportError("Column not found in input 0");
					}
					else
					{
						outputColumns[it->newColumn] = ColumnInfo(it->newColumn, outputColumns0[it->column.name].type);
						outputColumns[it->newColumn].column.id = outputColumns0[it->column.name].column.id;
						(it)->column.id = outputColumns[it->newColumn].column.id;
						(it)->type = outputColumns[it->newColumn].type;
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
						outputColumns[it->newColumn] = ColumnInfo(it->newColumn, outputColumns1[it->column.name].type);
						outputColumns[it->newColumn].column.id = outputColumns1[it->column.name].column.id;
						(it)->column.id = outputColumns[it->newColumn].column.id;
						(it)->type = outputColumns[it->newColumn].type;
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

	void visitTable(Table * node);

	void visitSort(Sort * node);

	void visitGroup(Group * node);

	void visitColumnOperations(ColumnOperations * node);

	void visitSelection(Selection * node);

	void visitJoin(Join * node);

	void visitAntiJoin(AntiJoin * node);

	void visitUnion(Union * node);

	void visitGroupedJoin(GroupedJoin * node);

};

class GroupingVisitor : public AlgebraVisitor
{
private:

public:
	GroupingVisitor();

	void visitColumnOperations(ColumnOperations * node);

	void visitSelection(Selection * node);

	void visitJoin(Join * node);

	void visitAntiJoin(AntiJoin * node);

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
	std::set<uint> inputs;
	ConditionType type;
};

class JoinInfo
{
public:
	std::vector<std::shared_ptr<PhysicalPlan> > plans;
	std::set<ulong> processedPlans;
	std::set<ulong> unProcessedPlans;
	std::vector<std::shared_ptr<ConditionInfo>> condition;
	std::map <ulong,JoinColumnInfo> columns;
	double size;
	static bool Comparator(const JoinInfo& lhs, const JoinInfo&rhs)
	{
		return (lhs.plans[0]->timeComplexity < rhs.plans[0]->timeComplexity);
	}
	void RemoveUnnecessaryColumns(std::vector<JoinColumnInfo> & outputColumns)
	{
		std::set<ulong> allColumns;
		for (auto it = outputColumns.begin(); it != outputColumns.end(); ++it)
		{
			allColumns.insert(it->column.id);
		}

		for (auto it = condition.begin(); it != condition.end(); ++it)
		{
			for (auto it2 = (*it)->inputs.begin(); it2 != (*it)->inputs.end(); ++it2)
			{
				allColumns.insert(*it2);
			}
		}

		for (auto it = columns.begin(); it != columns.end(); )
		{
			if (allColumns.find(it->first) == allColumns.end())
			{
				columns.erase(it++);
				continue;
			}
			++it;
		}

		for (auto it = plans.begin(); it != plans.end();++it)
		{
			for (auto it2 = (*it)->plan->columns.begin(); it2 != (*it)->plan->columns.end();)
			{
				if (allColumns.find(it2->first) == allColumns.end())
				{
					(*it)->plan->columns.erase(it2++);
					continue;
				}
				++it2;
			}
		}

	}
};

class AlgebraCompiler : public AlgebraVisitor
{
public:
	static const ulong NUMBER_OF_PLANS;
	static const ulong  LIMIT_FOR_GREEDY_JOIN_ORDER_ALGORITHM;
	static const ulong AlgebraCompiler::MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM;
	
	std::vector<std::shared_ptr<PhysicalPlan> > result;
	std::map<int, ColumnInfo> columns;
	double size;
	void visitTable(Table * node);

	void visitSort(Sort * node);

	void visitGroup(Group * node);

	void visitColumnOperations(ColumnOperations * node);

	void visitSelection(Selection * node);

	void visitJoin(Join * node);

	void visitAntiJoin(AntiJoin * node);

	void visitUnion(Union * node);

	void visitGroupedJoin(GroupedJoin * node);
	
private:
	void insertPlan(std::vector<std::shared_ptr<PhysicalPlan> > & plans, std::shared_ptr<PhysicalPlan> & plan);

	std::shared_ptr<PhysicalPlan> generateSortParameters(const PossibleSortParameters & parameters, const std::shared_ptr<PhysicalPlan> & result);

	void generateIndexScan(const std::string & tableName,std::vector<std::shared_ptr<PhysicalPlan> >::iterator plan, std::vector<std::shared_ptr<Expression> > & condition, std::vector<std::shared_ptr<PhysicalPlan>> & newResult);

	void join(GroupedJoin * node, const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan);

	std::vector<ulong> getAllSubsets(std::vector<ulong> & arr, ulong n, ulong k) const;

	void greedyJoin(GroupedJoin * node, std::vector<JoinInfo>::iterator &it, std::set<ulong>::iterator &it2, std::vector<JoinInfo> & plans, std::vector<JoinInfo> & heap);


	void generateSortParametersForMergeJoin(PossibleSortParameters & sortParameters, const std::vector<std::shared_ptr<ConditionInfo>> & conditions, const std::map<int, ColumnInfo> & columns);

	void getEqualPairsFromCondition(const std::vector<std::shared_ptr<ConditionInfo>> & conditions, std::map<int, int> & equalPairs, std::map<int, int> & equalPairsReverse, const std::map<int, ColumnInfo> & leftColumns, const std::map<int, ColumnInfo> & rightColumns);

	void generateSortParametersForOtherPlanInMergeJoin(PossibleSortParameters & rightSortParameters, std::shared_ptr<PhysicalPlan> plan, const std::map<int, ColumnInfo> & columns, std::map<int, int> & equalPairs);

	void getMergeJoinSortedParametes(PossibleSortParameters & resultParameters, std::map<int, int> & equalPairsReverse, std::map<int, ColumnInfo> & otherColumns);
	
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


class SelectionSpitingVisitor : public AlgebraVisitor
{
public:

	void visitSelection(Selection * node);

};

class SelectionFusingVisitor : public AlgebraVisitor
{
public:

	void visitSelection(Selection * node);

};


#endif