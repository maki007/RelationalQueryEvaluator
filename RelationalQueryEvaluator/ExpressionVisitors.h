#ifndef ExpressionVisitorHPP
#define ExpressionVisitorHPP

#include "Expressions.h"
#include <map>
#include "Algebra.h"
#include "AlgebraVisitors.h"
#include <vector>
#include <algorithm>

namespace rafe {

	/**
	* Base class for expression visitors.
	* Every virtual method does nothing only visits node all children.
	*/
	class ExpressionVisitorBase
	{
	public:

		/**
		* Visits UnaryExpression element.
		* @param expression visited UnaryExpression.
		*/
		virtual void visitUnaryExpression(UnaryExpression * expression);

		/**
		* Visits BinaryExpression element.
		* @param expression visited BinaryExpression.
		*/
		virtual void visitBinaryExpression(BinaryExpression * expression);

		/**
		* Visits NnaryExpression element.
		* @param expression visited NnaryExpression.
		*/
		virtual void visitNnaryExpression(NnaryExpression * expression);

		/**
		* Visits Constant element.
		* @param expression visited Constant.
		*/
		virtual void visitConstant(Constant * expression);

		/**
		* Visits Column element.
		* @param expression visited Column.
		*/
		virtual void visitColumn(Column * expression);

		/**
		* Visits GroupedExpression element.
		* @param expression visited GroupedExpression.
		*/
		virtual void visitGroupedExpression(GroupedExpression * expression);
	};

	/**
	* Writes expression tree into string. Expression is written in infix.
	*/
	class WritingExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		std::string result; /**< Stores tree infix representation. */

		void visitUnaryExpression(UnaryExpression * expression);

		void visitBinaryExpression(BinaryExpression * expression);

		void visitNnaryExpression(NnaryExpression * expression);

		void visitConstant(Constant * expression);

		void visitColumn(Column * expression);

		void visitGroupedExpression(GroupedExpression * expression);
	};

	/**
	* Visitor which assign inout number to every column node.
	* It is used in join and antijoin for identifying columns.
	*/
	class NumberColumnsInJoinVisitor : public ExpressionVisitorBase
	{
	public:
		int lastNumberedColumn; /**< Contains only 0or 1. Columns from left join input are numbered 0, and from right join input are numbered 1. */

		/**
		* Creates new instance of NumberColumnsInJoinVisitor
		*/
		NumberColumnsInJoinVisitor();
		void visitColumn(Column * expression);
		void visitBinaryExpression(BinaryExpression * expression);
	};

	/**
	* Visitor, which stores pointer of every column into vector. This visitor doesn't change expression tree.
	*/
	class GetColumnsNodesVisitor : public ExpressionVisitorBase
	{
	public:
		std::vector<Column *> nodes; /**< Vector for saving column pointers. */
		void visitColumn(Column * expression);
	};

	/**
	* This visitor merges all neighbouring AND and OR binary expression into Grouped expression.
	*/
	class GroupingExpressionVisitor : public ExpressionVisitorBase
	{
	public:

		std::shared_ptr<Expression> * root; /**< Root of expression tree. If the root is changed it root has to be rewritten. */

		/**
		* Creates new instance of GroupingExpressionVisitor.
		* @param x - root of visited expression tree.
		*/
		GroupingExpressionVisitor(std::shared_ptr<Expression> * x);
		void visitBinaryExpression(BinaryExpression * expression);

	};

	/**
	* Checks if columns in expression exists. If not, it stores this information and missing column name.
	*/
	class SemanticExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		bool containsErrors; /**< Idicates if visitor found an error. */

		std::string missingColumn; /**< Stores name of first missing column.*/

		std::map<std::string, ColumnInfo> outputColumns0; /**< Stores columns, which came from first childnode. Key is column name. */

		std::map<std::string, ColumnInfo> outputColumns1; /**< Stores columns, which came from second childnode if it has one. If not it stays empty. Key is column name. */

		/**
		* Creates new instance of SemanticExpressionVisitor.
		*/
		SemanticExpressionVisitor();
		void visitColumn(Column * expression);
	};

	/**
	* Visitor coputes new size of relation after applying current condition.
	* Estimation is made based on number of unique values in columns.
	*/
	class SizeEstimatingExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		const std::map<int, ColumnInfo> * columns; /**< Map of columns in this expression. Key is unique column identifier. */

		double size; /**< Relative size of relation. In constructor it is set to 1. After visit of expression it is a fraction representing relative size of relation. */

		/**
		* Creates new instance of SizeEstimatingExpressionVisitor
		* @param columns - columns, which are used in visited expression.
		*/
		SizeEstimatingExpressionVisitor(const std::map<int, ColumnInfo> * columns);

		void visitUnaryExpression(UnaryExpression * expression);

		void visitBinaryExpression(BinaryExpression * expression);

		void visitNnaryExpression(NnaryExpression * expression);

		void visitGroupedExpression(GroupedExpression * expression);

	};

	/**
	* Reads all columns identifiers from expression and determing what kind of condition is expression representing.
	* It can be equal condition a=b or le condition a1<b<a2. Other conditions should not be in joins.
	*/
	class JoinInfoReadingExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		std::set<int> * data; /**< Set of unique columns identifiers.*/
		ConditionType * conditionType; /**< Type of join condition.*/

		/**
		* Creates new instance of JoinInfoReadingExpressionVisitor.
		* @param data - visitor will store here unique column identifiers.
		* @param type - visitor will store here condition type.
		*/
		JoinInfoReadingExpressionVisitor(std::set<int> * data, ConditionType * type);

		void visitColumn(Column * expression);

		void visitUnaryExpression(UnaryExpression * expression);

		void visitBinaryExpression(BinaryExpression * expression);

		void visitNnaryExpression(NnaryExpression * expression);

		void visitConstant(Constant * expression);

		void visitGroupedExpression(GroupedExpression * expression);

	};

	/**
	* Visitor is used for renaming join condition while grouping joins.
	* Join can rename column and in this case can happen, that condition in groupped join doesn't have correct column name.
	*/
	class RenamingJoinConditionExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		ulong input; /**< Information which input is beeing processed. */
		std::vector<JoinColumnInfo> * inputColumns; /**< Possible columns to rename. */

		/**
		* Creates new instance of RenamingJoinConditionExpressionVisitor.
		* @param i - input number.
		* @param inputColumns - Possible columns to rename.
		*/
		RenamingJoinConditionExpressionVisitor(ulong i, std::vector<JoinColumnInfo> * inputColumns);

		void visitColumn(Column * expression);

	};

	/**
	* Visitor is used in join calculation to estimate size of relation after join.
	* It computes maximum of unique values in each colum and stores the result in result variable.
	*/
	class MaxOfUniqueValuesExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		double result; /**< Visitor stores here maximum of unique values of all columns in visited expression.*/
		std::map<int, ColumnInfo> * columns;  /**< Map of columns in this expression. Key is unique column identifier. */

		/**
		* Creates new instance of MaxOfUniqueValuesExpressionVisitor.
		* @param cols - structure mapping column unique identifier to number of column in processed bobox operator
		*/
		MaxOfUniqueValuesExpressionVisitor(std::map<int, ColumnInfo> * cols);

		void visitColumn(Column * expression);

	};

	/**
	* Visitor make simple type analysis. It assumes that all types are correct and after expression visit,
	* there will be stored type of whole expression in variable resultType.
	*/
	class TypeResolvingExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		std::string resultType; /**< Visited expression type.*/

		void visitUnaryExpression(UnaryExpression * expression);

		void visitBinaryExpression(BinaryExpression * expression);

		void visitNnaryExpression(NnaryExpression * expression);

		void visitConstant(Constant * expression);

		void visitColumn(Column * expression);

		void visitGroupedExpression(GroupedExpression * expression);
	};

	/**
	* Visitor writes expression in infix for bobox output.
	*/
	class BoboxWritingExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		std::string result;  /**< Stores tree infix representation. */
		std::map<int, int> * cols; /**< Structure mapping column unique identifier to number of column in processed bobox operator.*/

		/**
		* Creates new instance of BoboxWritingExpressionVisitor.
		* @param cols - structure mapping column unique identifier to number of column in processed bobox operator
		*/
		BoboxWritingExpressionVisitor(std::map<int, int> & cols);

		void visitUnaryExpression(UnaryExpression * expression);

		void visitBinaryExpression(BinaryExpression * expression);

		void visitNnaryExpression(NnaryExpression * expression);

		void visitConstant(Constant * expression);

		void visitColumn(Column * expression);

		void visitGroupedExpression(GroupedExpression * expression);
	};

	/**
	* Visitor maked a copy from visited tree. All nodes are copied.
	*/
	class CloningExpressionVisitor : public ExpressionVisitorBase
	{
	public:
		std::shared_ptr<Expression> result; /**< For storing cloned expressoin*/

		void visitUnaryExpression(UnaryExpression * expression);

		void visitBinaryExpression(BinaryExpression * expression);

		void visitNnaryExpression(NnaryExpression * expression);

		void visitConstant(Constant * expression);

		void visitColumn(Column * expression);

		void visitGroupedExpression(GroupedExpression * expression);

	};

	/**
	* Visitor changes column identifiers in Column nodes.
	*/
	class RenameColumnsVisitor : public ExpressionVisitorBase
	{
	public:
		std::map<int, int>  * pairs; /**< Structure mapping old id to new id. */
		/**
		* Creates new instance of RenameColumnsVisitor.
		* @param pairs - structure mapping old column id to new id.
		*/
		RenameColumnsVisitor(std::map<int, int>  * pairs);

		void visitColumn(Column * expression);

	};
};
#endif

