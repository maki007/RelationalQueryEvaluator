#ifndef AlgebraStructureHPP
#define AlgebraStructureHPP

#include <memory>
#include <vector>
#include <string>
#include <list>
#include <set>

namespace rafe {

	class Expression;

	typedef unsigned long long int ulong;

	/**
	* Enum for unary expression operations.
	*/
	enum class UnaryOperator
	{
		NOT
	};

	/**
	* Enum for binary expression operations.
	*/
	enum class BinaryOperator
	{
		AND, OR, PLUS, MINUS, TIMES, DIVIDE, EQUALS, NOT_EQUALS, LOWER, LOWER_OR_EQUAL
	};

	/**
	* Enum for grouped operations.
	*/
	enum class GroupedOperator
	{
		AND, /**< Represents n-nary boolean operation and. */
		OR /**< Represents n-nary boolean operation or. */
	};

	/**
	* Stres unique indetification of a column.
	*/
	class ColumnIdentifier
	{
	public:

		std::string name; /**< Stores the name of the column. */

		int id; /**< Stores the unique identifier of a column. */

		/**
		* Create the instance of ColumnIdentifier.
		* @param name - column name.
		* @param id - column unique identifier.
		*/
		ColumnIdentifier(std::string name, int id);

		/**
		* Creates new instance of ColumnIdentifier.
		* @param name - column name.
		*/
		ColumnIdentifier(std::string name);

		/**
		* Creates new instance of ColumnIdentifier.
		*/
		ColumnIdentifier();

		/**
		* @return String representation of this class.
		*/
		std::string toString() const;

		/**
		* Compares id of ColumnIdentifier.
		* @param other - ColumnIdentifier for comparision.
		*/
		bool operator<(const ColumnIdentifier& other) const;

		/**
		* Overload opeartor == and compares only ids.
		* @param other - ColumnIdentifier for comparision.
		*/
		bool operator ==(const ColumnIdentifier& other) const;

	};

	/**
	* Represents sort order of relation.
	*/
	enum SortOrder
	{
		ASCENDING, /**< Relation is sorted ascending.*/
		DESCENDING, /**< Relation is sorted descending.*/
		UNKNOWN /**< Relation can be sorted in both directions.*/
	};

	/**
	* Represents how the relation is sorted.
	* Stores one column and its sort order.
	*/
	class SortParameter
	{
	public:
		ColumnIdentifier column; /**< Identifies by what column should the relation be sorted by. */

		std::set<ColumnIdentifier> others; /**< Set of identifiers of columns which equals property column. */

		SortOrder order; /**< Order how to sort the column.*/

		/**
		* Creates new instance of SortParameter.
		* @param column - column identifier.
		* @param order - sortOrder.
		*/
		SortParameter(const ColumnIdentifier & column, SortOrder order);

		/**
		* Creates new instance of SortParameter.
		* @param column - column identifier.
		* @param other - order column which equals column.
		* @param order - sortOrder.
		*/
		SortParameter(const ColumnIdentifier & column, const ColumnIdentifier & other, SortOrder order);

		/**
		* Creates new instance of SortParameter.
		*/
		SortParameter();

	};

	/**
	* Represents sort parameters.
	* If there is more that one value in values vector than means that the actual order can change.
	*/
	class SortParameters
	{
	public:

		std::list<SortParameter> values; /**< Vector storing SortParameter. */

		/**
		* Creates new instance of SortParameters.
		* @param value
		*/
		SortParameters(const SortParameter & value);

		/**
		* Creates new instance of SortParameters.
		* @param values
		*/
		SortParameters(const std::vector<SortParameter> & values);

		/**
		* Creates new instance of SortParameters.
		*/
		SortParameters();

		/**
		* Indicates if there is only one sort possibility.
		*/
		bool isKnown() const;

	};

	/**
	* Represents all possibilities how relation can be sorted.
	* It contains vector of list of SortParameter.
	* The list inside of vector means that the order of columns is not determined.
	* The vector order is set and will not change.
	*/
	class PossibleSortParameters
	{
	public:
		std::vector<SortParameters> parameters; /**< Vector of SortParameters. */

		/**
		* Creates the instance of PossibleSortParameters.
		* @param parameters - vector<SortParameters>.
		*/
		PossibleSortParameters(const std::vector<SortParameters> & parameters);

		/**
		* Creates the instance of PossibleSortParameters.
		* @param parameters - vector<SortParameter>.
		*/
		PossibleSortParameters(const std::vector<SortParameter> & parameters);

		/**
		* Creates the instance of PossibleSortParameters.
		*/
		PossibleSortParameters();

	};

	/**
	* Enum for agregation function types.
	*/
	enum AgregateFunctionType
	{
		SUM, MIN, MAX, COUNT
	};

	/**
	* Stores information about agregation.
	*/
	class AgregateFunction
	{
	public:
		AgregateFunctionType function; /**< Agregate function. */
		std::string functionName; /**< Function name, stored only for debuging purposes. */
		ColumnIdentifier parameter; /**< Column identifier of parameter fo function. Count function does not have any parameter. */
		ColumnIdentifier output;  /**< Identifier of the output column storing function result. */
	};

	/**
	* Enum for index type.
	*/
	enum IndexType
	{
		CLUSTERED, /**< Clustered index. */
		UNCLUSTERED  /**< Unclustered index. */
	};

	/**
	* Structure storing information about index on table.
	*/
	class Index
	{
	public:
		IndexType type; /**< Type of index: clustered or unclustered. */
		std::string name; /**< Index name.*/
		std::vector<SortParameter> columns; /**< Columns on wwhich is index created. It also stores sort order of every column in index.*/

		/**
		* @return String representation of this class.
		*/
		std::string toString();
	};

	class JoinColumnInfo;

	/**
	* Stored information about column, like identifier, number of unique values and type.
	*/
	class ColumnInfo
	{
	public:
		ColumnIdentifier column;  /**< Columns identifier. */
		std::string type; /**< Type of the column. */
		double numberOfUniqueValues; /**< Stores number of unique values for joining purposes. */

		/**
		* Create the instance of ColumnInfo.
		* @param name - column name
		* @param type - column type
		*/
		ColumnInfo(std::string name, std::string type);

		/**
		* Creates new instance of ColumnInfo.
		*/
		ColumnInfo();

		/**
		* Creates new instance of ColumnInfo.
		* @param name - column name
		* @param numberOfUniqueValues
		* @param type - type of column
		*/
		ColumnInfo(std::string name, double numberOfUniqueValues, std::string type);

		/**
		* Creates new instance of ColumnInfo.
		* @param column - ColumnIdentifier of the column
		* @param numberOfUniqueValues
		* @param type - type of column
		*/
		ColumnInfo(const ColumnIdentifier & column, double numberOfUniqueValues, std::string type);

		/**
		* Creates new instance of ColumnInfo.
		* @param info - JoinColumnInfo
		*/
		ColumnInfo(const JoinColumnInfo & info);

	};

	/**
	* Inherits from ColumnInfo and stores information which input is column from and new column name.
	*/
	class JoinColumnInfo : public ColumnInfo
	{
	public:
		ulong input; /**< Input, which column is from. Is is numbered from 0. */
		std::string newColumn; /**< New column name. */

		/**
		* Creates new instance of JoinColumnInfo.
		* @param col - ColumnInfo
		*/
		JoinColumnInfo(const ColumnInfo & col);

		/**
		* Creates new the instance of JoinColumnInfo.
		*/
		JoinColumnInfo();

	};

	/**
	* Stores information about operation on columns from extended projection operator.
	*/
	class ColumnOperation
	{
	public:
		ColumnIdentifier result; /**< Identifier of output column. */
		std::shared_ptr <Expression> expression; /**< Expression to compute new column. If it is null, then the column is only copied to output. */
		std::string type; /**< Type of the column. */
	};

	/**
	* Stores information about grouping column.
	*/
	class GroupColumn
	{
	public:
		ColumnIdentifier input; /**< Identifier of input column. */
		ColumnIdentifier output; /**< Identifier of output column. Input and output are same but they have different id. */

		/**
		* Creates new instance of GroupColumn using ColumnIdentifier.
		* @param input - ColumnIdentifier of the column
		*/
		GroupColumn(const ColumnIdentifier & input);
	};
};
#endif