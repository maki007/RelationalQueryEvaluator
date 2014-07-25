#ifndef AlgebraVisitorHPP
#define AlgebraVisitorHPP

#include "Algebra.h"
#include "PhysicalOperator.h"
#include <map>
#include <set>

namespace rafe {

	/**
	* Base class for algebra tree visitors.
	* Every virtual method does nothing only visits node all children.
	*/
	class AlgebraVisitor
	{
	public:

		/**
		* Visits Table element.
		* @param node visited Table.
		*/
		virtual void visitTable(Table * node);

		/**
		* Visits Sort element.
		* @param node visited Sort.
		*/
		virtual void visitSort(Sort * node);

		/**
		* Visits Group element.
		* @param node visited Group.
		*/
		virtual void visitGroup(Group * node);

		/**
		* Visits ColumnOperations element.
		* @param node visited ColumnOperations.
		*/
		virtual void visitColumnOperations(ColumnOperations * node);

		/**
		* Visits Selection element.
		* @param node visited Selection.
		*/
		virtual void visitSelection(Selection * node);

		/**
		* Visits Join element.
		* @param node visited Join.
		*/
		virtual void visitJoin(Join * node);

		/**
		* Visits AntiJoin element.
		* @param node visited AntiJoin.
		*/
		virtual void visitAntiJoin(AntiJoin * node);

		/**
		* Visits Union element.
		* @param node visited Union.
		*/
		virtual void visitUnion(Union * node);

		/**
		* Visits GroupedJoin element.
		* @param node visited GroupedJoin.
		*/
		virtual void visitGroupedJoin(GroupedJoin * node);

		/**
		* Convert expresion from tree to vector of subconditions.
		* Condition will be splited into subconditions which are connected with and.
		* @param condition Expression to convert.
		* @param result vector of Expression. Result of the function will be stored in this variable. Input needs to be empty.
		*/
		static void serializeExpression(std::shared_ptr<Expression> & condition, std::vector<std::shared_ptr<Expression> > & result);

		/**
		* Convert expresion vector of subexpression to expression tree.
		* @param condition vector of Expression
		* @return tree condition
		*/
		static std::shared_ptr<Expression> deserializeExpression(const std::vector<std::shared_ptr<Expression> > & condition);

		/**
		* Remove selection node from tree.
		* @param node Selection to be removed
		*/
		static void removeSelection(Selection * node);

		/**
		* Insert selection into tree.
		* New node will be inserted as parent from first argument.
		* @param node AlgebraNodeBase where new node will be inserted
		* @param selection - Selection to be inserted
		*/
		static void insertSelection(AlgebraNodeBase * node, std::shared_ptr<Selection> & selection);

	};

	/**
	* Algebra visitor, which visits all nodes in the tree and generates text representation in dot for debugging purposes.
	*/
	class GraphDrawingVisitor : public AlgebraVisitor
	{
	public:
		std::string result; /**< Stores final text representation. */
		int nodeCounter; /**< Hellping variable for numbering tree. */

		/**
		* Creates new instance of GraphDrawingVisitor.
		*/
		GraphDrawingVisitor();

	private:
		/**
		* Generates string representation from node and calls iteself on child. Then it connect created nodes.
		* @param label for generated node
		* @param node - on which to call children
		*/
		void generateText(std::string & label, UnaryAlgebraNodeBase * node);

		/**
		* Generates  string representation from node and calls iteself on it's childs. Then it connect created nodes.
		* @param label for generated node
		* @param node - on which to call children
		*/
		void generateText(std::string & label, BinaryAlgebraNodeBase * node);

		/**
		* Generates  string representation from node and calls iteself on it's childs. Then it connect created nodes.
		* @param label for generated node
		* @param node - on which to call children
		*/
		void generateText(std::string & label, GroupedAlgebraNode * node);

	public:
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

	/**
	* This visitor is checking:
	* - if all the requested columns exists
	* - if the columns are not declared multiple times.
	* Then if error is reported, compilation cannot continue.
	* It also gives columns unique identifier, so other vistors don't have to work with string names.
	*/
	class SemanticChecker : public AlgebraVisitor
	{
	private:
		std::map<std::string, ColumnInfo> outputColumns; /**< Structure storing output columns of last visited column. */
		int lastId; /**< Last generated unique id. */

		/**
		* Converts columns from map<column name,ColumnInfo> to map<unique identifier,ColumnInfo> and stores result info given algebra node.
		* @param outputColumns, structure to be converted
		* @param node, AlgebraNodeBase for storing resultant structure.
		*/
		void convertColumns(const std::map<std::string, ColumnInfo> & outputColumns, AlgebraNodeBase * node);

		/**
		* Reports found errors.
		* @param error found error
		* @param nodeName name of algebra node, where the error was found.
		* @param lineNumber number of line, where error was found.
		*/
		void ReportError(const std::string error, std::string nodeName, ulong lineNumber);

		/**
		* Generates new unique column identifier.
		* @return new unique identifier
		*/
		int nextId();

		/**
		* Checks join condition and output columns and reports semantic errors.
		* @param outputColumns0 - output columns from first join input
		* @param outputColumns1 - output columns from second join input
		* @param node - node join or antijoin node
		*/
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
							ReportError("Column " + it->column.name + " not found in first input", "join", node->lineNumber);
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
							ReportError("Column " + it->column.name + " not found in second input", "join", node->lineNumber);
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
					ReportError("Output column " + it->newColumn + " is allready used", "join", node->lineNumber);
				}
			}

		}

	public:

		bool containsErrors; /**< Indicates if semantic checker has found any errors. */

		/**
		* Creates new instance of SemanticChecker.
		*/
		SemanticChecker();

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

	/**
	* This algebra visitor groupes neighbour join into one groupped join.
	* It also calls GroupingExpressionVisitor on every condition.
	*/
	class GroupingVisitor : public AlgebraVisitor
	{
	public:

		/**
		* Creates new instance of GroupingVisitor.
		*/
		GroupingVisitor();

		void visitColumnOperations(ColumnOperations * node);

		void visitSelection(Selection * node);

		void visitJoin(Join * node);

		void visitAntiJoin(AntiJoin * node);

	private:
		/**
		* Hepler method, which merges join node to GroupedJoin and update all nedded parameters like condition and outputcolumns.
		* @param node - node to merge
		* @param groupedOperator - node to be merge to
		*/
		void resolveJoins(Join * node, GroupedJoin * groupedOperator);
	};

	/**
	* Type of join condition.
	*/
	enum class ConditionType
	{
		EQUALS,
		LOWER,
		OTHER
	};

	/**
	* Stores information about condition, its columns and condition type.
	*/
	class ConditionInfo
	{
	public:
		std::shared_ptr<Expression> condition; /**< Given condition. */
		std::set<int> inputs; /**< Set of column identifiers. */
		ConditionType type; /**< Condition type. */
	};

	/**
	* Structures storing information in selecting join order algoritm.
	*/
	class JoinInfo
	{
	public:
		std::vector<std::shared_ptr<PhysicalPlan> > plans; /**< Generated plans. */
		std::set<ulong> processedPlans; /**< Numbers of processed inputs. */
		std::set<ulong> unProcessedPlans; /**< Numbers of unprocessed inputs. */
		std::vector<std::shared_ptr<ConditionInfo>> condition;  /**< Join condition to be processed. */
		std::map <int, JoinColumnInfo> columns; /**< Output columns from current plans. */
		double size; /**< Size of the processed relation. */


		/**
		* Comparer for head in greedy join order algorirthm.
		* Compares plans bases on timecomplexity of plans[0]. Greedy join order algorirthm uses this structure with only one plan.
		* @param lhs - first plan to compare
		* @param rhs - second plan to compare
		* @return true if lsf is faster then rhs
		*/
		static bool Comparator(const std::shared_ptr<JoinInfo> & lhs, const std::shared_ptr<JoinInfo> &rhs);

		/**
		* Remove unecessary columns from current plans.
		* Unnecessary columns are the one, which are not in condition out in output of procesed GroupJoin.
		* @param outputColumns columns which are necessary.
		*/
		void RemoveUnnecessaryColumns(std::vector<JoinColumnInfo> & outputColumns);

	};

	/**
	* AlgebraVisitor which compiles relational algebra into physical plan.
	* It uses from down to up method and it saves costant number of best plans for current subtree.
	*/
	class AlgebraCompiler : public AlgebraVisitor
	{
	public:
		static const ulong NUMBER_OF_PLANS; /**< Determines what number of plans are used from visited subtree. */
		static const ulong LIMIT_FOR_GREEDY_JOIN_ORDER_ALGORITHM;  /**< Determines maximal number of inputs in group join to uses brute force join order algorithm. Otherwise greedy algorithm is used. */
		static const ulong MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM;  /**< Determines how many plans can be stored in greddy join algorithm. */

		std::vector<std::shared_ptr<PhysicalPlan> > result; /**< Stores plans after processing subtree. */



		void visitTable(Table * node);

		void visitSort(Sort * node);

		void visitGroup(Group * node);

		void visitColumnOperations(ColumnOperations * node);

		void visitSelection(Selection * node);

		void visitJoin(Join * node);

		void visitAntiJoin(AntiJoin * node);

		void visitUnion(Union * node);

		void visitGroupedJoin(GroupedJoin * node);

		/**
		* From possible paramers remove columns, which aren't in newColumns and stores them into newPlan.
		* @param possibleSortParameters - parameters to update
		* @param newPlan - PhysicalPlan, where to store result
		* @param newColumns - columns which stays in parameters
		*/
		static void updateSortParameters(const PossibleSortParameters & possibleSortParameters, std::shared_ptr<PhysicalPlan> & newPlan, std::map<int, ColumnInfo> & newColumns);

	private:

		std::map<int, ColumnInfo> columns; /**< Columns containing plans after each subtree. */

		double size; /**< Size of relation computed after each subtree. */

		/**
		* Inserts physical plan into heap of physical plans and removes the slowest, if the heap size > NUMBER_OF_PLANS
		* @param plans - heap of plans
		* @param plan - new inserted plan
		*/
		void insertPlan(std::vector<std::shared_ptr<PhysicalPlan> > & plans, std::shared_ptr<PhysicalPlan> & plan);

		/**
		* Generates new plan from old one and Sort, PartalSort or no sort plan.
		* @param parameters - sort parameters
		* @param result - processed plan
		* @return new plan
		*/
		std::shared_ptr<PhysicalPlan> generateSortParameters(const PossibleSortParameters & parameters, const std::shared_ptr<PhysicalPlan> & result);

		/**
		* Generates indexScan from current plan.
		* @param name - name of the table
		* @param plan - PhysicalPlan from which to generate indexScan
		* @param condition - Expression what to use
		* @param newResult - vector for storing possible plan
		*/
		void generateIndexScan(const std::string & tableName, std::vector<std::shared_ptr<PhysicalPlan> >::iterator plan, std::vector<std::shared_ptr<Expression> > & condition, std::vector<std::shared_ptr<PhysicalPlan>> & newResult);

		/**
		* Generates possible join plans. It is used in Greedy and Brute force algorithm.
		* @param node - GroupedJoin beeing processed
		* @param left - first inputs into join
		* @param right - second inputs into join
		* @param newPlan - for storing results
		*/
		void join(GroupedJoin * node, const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan);

		/**
		* Generates vector containg all subsets. Subsests are binary coded in output.
		* @param arr - input numbers
		* @param n - size of input vector
		* @param k - size of generated subsets
		* @return vector containg all subsets. Subsests are binary coded in output.
		*/
		std::vector<ulong> getAllSubsets(std::vector<ulong> & arr, ulong n, ulong k) const;

		/**
		* Performs greedy join algorithm.
		* @param node - GroupedJoin beeing processed
		* @param it - first input
		* @param it2 - second input
		* @param plans - vector for second input
		* @param heap for storing results
		*/
		void greedyJoin(GroupedJoin * node, JoinInfo &it, std::set<ulong>::iterator &it2, std::vector<JoinInfo> & plans, std::vector<std::shared_ptr<JoinInfo >> & heap);

		/**
		* Creates PossibleSortParameters for sort before merge join algorithm
		* @param sortParameters - for storing result
		* @param conditions - join condition
		* @param columns - columns from current input
		*/
		void generateSortParametersForMergeJoin(PossibleSortParameters & sortParameters, const std::vector<std::shared_ptr<ConditionInfo>> & conditions, const std::map<int, ColumnInfo> & columns);

		/**
		* For equal condition, it generates pairs, which columns are equal.
		* @param conditions - condition to generate from
		* equalPairs - result of the function
		* equalPairsReverse - same structure like equalPairs, but key and value are reversed
		* leftColumns - columns from left input
		* rightColumns - columns from right input
		*/
		void getEqualPairsFromCondition(const std::vector<std::shared_ptr<ConditionInfo>> & conditions, std::map<int, int> & equalPairs, std::map<int, int> & equalPairsReverse, const std::map<int, ColumnInfo> & leftColumns, const std::map<int, ColumnInfo> & rightColumns);

		/**
		* Creates PossibleSortParameters for sort before merge join algorithm, it used PossibleSortParameters from other input so the columns are sorted correct way.
		* @param rightSortParameters - for storing result
		* @param plan - processed plan
		* @param columns - columns from current input
		* @param equalPairs - other respresentation of equals conditions
		*/
		void generateSortParametersForOtherPlanInMergeJoin(PossibleSortParameters & rightSortParameters, std::shared_ptr<PhysicalPlan> plan, const std::map<int, ColumnInfo> & columns, std::map<int, int> & equalPairs);

		/**
		* Determines how relation after merge join is sorted.
		* @param rightSortParameters - for storing result
		* @param equalPairs - other respresentation of equals conditions
		* @param columns -columns from first or second output
		*/
		void getMergeJoinSortedParametes(PossibleSortParameters & resultParameters, std::map<int, int> & equalPairs, std::map<int, ColumnInfo> & columns);

		/**
		* Converts conditions from both input and merges them into one with and operator condition.
		* @param a - first input
		* @param b - second input
		*/
		std::shared_ptr<Expression> deserializeConditionInfo(const std::vector<std::shared_ptr<ConditionInfo>> & a, const std::vector<std::shared_ptr<ConditionInfo>> & b);

		/**
		* Converts vector of number representing set to binary representation of set in ulong.
		* It will only work for sets representing intergers >=0 and <65
		* @param input vector of numbers
		*/
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

		/**
		* Generates Filte operator after mergeJoin
		* @param plan for processing
		* @param condition for filter
		*/
		std::shared_ptr<PhysicalPlan> AlgebraCompiler::generateFilterAfterJoin(const std::shared_ptr<PhysicalPlan> & plan, std::shared_ptr<Expression> & condition);

		/**
		* Generates FilteKeepingOrder operator after mergeJoin
		* @param plan for processing
		* @param condition for filter
		*/
		std::shared_ptr<PhysicalPlan> AlgebraCompiler::generateFilterAfterMergeJoin(const std::shared_ptr<PhysicalPlan> & plan, std::shared_ptr<Expression> & condition);

	};

	/**
	* Splits selections into partial condition and insert them into tree at place where the original selection was.
	*/
	class SelectionSpitingVisitor : public AlgebraVisitor
	{
	public:
		void visitSelection(Selection * node);

	};

	/**
	* Merges neighbouring selections into one selection and insert it into tree instred of original chain of selections.
	*/
	class SelectionFusingVisitor : public AlgebraVisitor
	{
	public:
		void visitSelection(Selection * node);

	};

	/**
	* Collects all selections from tree.
	*/
	class SelectionColectingVisitor : public AlgebraVisitor
	{
	public:

		std::vector<Selection * > selections; /**< Resulting vector of selection pointers. */

		void visitSelection(Selection * node);
	};

	/**
	* Visitor takes selection out of the tree and pushes it down the tree as far as possible and inserts it there.
	*/
	class PushSelectionDownVisitor : public AlgebraVisitor
	{
	private:
		std::set<int> columns; /**< Set of column identifiers of pushed condition. */
		ConditionType conditionType; /**< Condition type of pushed condition. */

		Selection * nodePointer; /**< Pointer to pushed down selection. */
		std::shared_ptr<Expression> condition; /**< Pointer to pushed down condition. */
	public:

		/**
		* Creates new instance of PushSelectionDownVisitor.
		* @param node - node to bu pushed down
		*/
		PushSelectionDownVisitor(Selection * node);

		/**
		* Removes selection and push it down the tree.
		*/
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
};

#endif