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

	static void serializeExpression(std::shared_ptr<Expression> & condition, std::vector<std::shared_ptr<Expression> > & result);

	static std::shared_ptr<Expression> deserializeExpression(const std::vector<std::shared_ptr<Expression> > & condition);

	static void removeSelection(Selection * node);

	static void insertSelection(AlgebraNodeBase * node, std::shared_ptr<Selection> & selection);

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

	void convertColumns(const std::map<std::string, ColumnInfo> & outputColumns, AlgebraNodeBase * node);

public:

	bool containsErrors;

	SemanticChecker();

	int nextId();
	
	template <typename T>
	void checkJoinOutPutParameters(std::map<std::string, ColumnInfo> & outputColumns0, std::map<std::string, ColumnInfo> & outputColumns1, T * node)
	{
		outputColumns.clear();
		for (auto it = node->outputJoinColumns.begin(); it != node->outputJoinColumns.end(); ++it)
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
					throw std::exception("Error");
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
	std::map <ulong, JoinColumnInfo> columns;
	double size;
	static bool Comparator(const JoinInfo& lhs, const JoinInfo&rhs);

	static bool PointerComparator(const std::shared_ptr<JoinInfo> & lhs, const std::shared_ptr<JoinInfo> &rhs);
	
	void RemoveUnnecessaryColumns(std::vector<JoinColumnInfo> & outputColumns);
	
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
	static void updateSortParameters(const PossibleSortParameters & possibleSortParameters, std::shared_ptr<PhysicalPlan> & newPlan, std::map<int, ColumnInfo> & newColumns);
private:
	void insertPlan(std::vector<std::shared_ptr<PhysicalPlan> > & plans, std::shared_ptr<PhysicalPlan> & plan);

	std::shared_ptr<PhysicalPlan> generateSortParameters(const PossibleSortParameters & parameters, const std::shared_ptr<PhysicalPlan> & result);

	void generateIndexScan(const std::string & tableName, std::vector<std::shared_ptr<PhysicalPlan> >::iterator plan, std::vector<std::shared_ptr<Expression> > & condition, std::vector<std::shared_ptr<PhysicalPlan>> & newResult);

	void join(GroupedJoin * node, const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan);

	std::vector<ulong> getAllSubsets(std::vector<ulong> & arr, ulong n, ulong k) const;

	void greedyJoin(GroupedJoin * node, JoinInfo &it, std::set<ulong>::iterator &it2, std::vector<JoinInfo> & plans, std::vector<std::shared_ptr<JoinInfo >> & heap);


	void generateSortParametersForMergeJoin(PossibleSortParameters & sortParameters, const std::vector<std::shared_ptr<ConditionInfo>> & conditions, const std::map<int, ColumnInfo> & columns);

	void getEqualPairsFromCondition(const std::vector<std::shared_ptr<ConditionInfo>> & conditions, std::map<int, int> & equalPairs, std::map<int, int> & equalPairsReverse, const std::map<int, ColumnInfo> & leftColumns, const std::map<int, ColumnInfo> & rightColumns);

	void generateSortParametersForOtherPlanInMergeJoin(PossibleSortParameters & rightSortParameters, std::shared_ptr<PhysicalPlan> plan, const std::map<int, ColumnInfo> & columns, std::map<int, int> & equalPairs);

	void getMergeJoinSortedParametes(PossibleSortParameters & resultParameters, std::map<int, int> & equalPairsReverse, std::map<int, ColumnInfo> & otherColumns);

	std::shared_ptr<Expression> deserializeConditionInfo(const std::vector<std::shared_ptr<ConditionInfo>> & a, const std::vector<std::shared_ptr<ConditionInfo>> & b);

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

	std::shared_ptr<PhysicalPlan> AlgebraCompiler::generateFilterAfterJoin(const std::shared_ptr<PhysicalPlan> & plan, std::shared_ptr<Expression> & condition);
	
	std::shared_ptr<PhysicalPlan> AlgebraCompiler::generateFilterAfterMergeJoin(const std::shared_ptr<PhysicalPlan> & plan, std::shared_ptr<Expression> & condition);
	
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

class SelectionColectingVisitor : public AlgebraVisitor
{
public:
	std::vector<Selection * > selections;
	void visitSelection(Selection * node);
};

class PushSelectionDownVisitor : public AlgebraVisitor
{
private:
	std::set<uint> columns;
	ConditionType conditionType;

	Selection * nodePointer;
	std::shared_ptr<Expression> condition;
public:
	PushSelectionDownVisitor(Selection * node);

	void pushDown();

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

class PushSelectionUpVisitor : public AlgebraVisitor
{
public:
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

#endif