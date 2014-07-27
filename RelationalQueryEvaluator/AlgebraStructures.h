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
	* Stores unique indetification of a column.
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
		* @returns String representation of this class.
		*/
		std::string toString() const;

		/**
		* Compares ids of ColumnIdentifier class.
		* @param other - ColumnIdentifier for comparision.
		*/
		bool operator<(const ColumnIdentifier& other) const;

		/**
		* Overload of opeartor ==, function compares only ids.
		* @param other - ColumnIdentifier for comparision.
		*/
		bool operator ==(const ColumnIdentifier& other) const;

	};

	/**
	* Represents sort order of the relation.
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

		std::set<ColumnIdentifier> others; /**< Set of column identifiers which are equal to the property column. */

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
	* If there is more that one value in vector values, then the actual order of columns can change.
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
	* The list inside of vector represents group of columns with arbitrary order.
	* The order in the vector is set and will not change.
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
	* Enum for aggregation types of functions.
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
		ColumnIdentifier parameter; /**< Column identifier of parameter for function. The function Count does not have any parameters. */
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
		std::vector<SortParameter> columns; /**< Index columns with their sort order.*/

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
		ColumnIdentifier column;  /**< Identifier of the column. */
		std::string type; /**< Type of the column. */
		double numberOfUniqueValues; /**< Stores number of unique values for joining purposes. */

		/**
		* Creates new instance of ColumnInfo.
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
		* @param numberOfUniqueValues - number of unique values in this column
		* @param type - type of column
		*/
		ColumnInfo(std::string name, double numberOfUniqueValues, std::string type);

		/**
		* Creates new instance of ColumnInfo.
		* @param column - ColumnIdentifier of the column
		* @param numberOfUniqueValues - number of unique values in this column
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
	* Inherits from ColumnInfo and stores additional information about optional column renaming.
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
	* Stores information about operations on the columns. It is used in extended projection operator.
	*/
	class ColumnOperation
	{
	public:
		ColumnIdentifier result; /**< Identifier of an output column. */
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
		ColumnIdentifier output; /**< Identifier of output column. Input and output are the same but they have different id. */

		/**
		* Creates new instance of GroupColumn using ColumnIdentifier.
		* @param input - ColumnIdentifier of the column
		*/
		GroupColumn(const ColumnIdentifier & input);
	};
};
#endif