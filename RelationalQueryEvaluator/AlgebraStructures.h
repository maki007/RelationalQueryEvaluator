#ifndef AlgebraStructureHPP
#define AlgebraStructureHPP

#include <memory>
#include <vector>
#include <string>
#include <list>
#include <set>

class Expression;

typedef unsigned long long int ulong;

enum class UnaryOperator
{
	NOT
};
enum class BinaryOperator
{
	AND, OR, PLUS, MINUS, TIMES, DIVIDE, EQUALS, NOT_EQUALS, LOWER, LOWER_OR_EQUAL
};
enum class GroupedOperator
{
	AND, OR
};

class ColumnIdentifier
{
public:
	std::string name;
	int id;
	ColumnIdentifier(std::string name, int id);
	
	ColumnIdentifier(std::string name);

	ColumnIdentifier();
	
	std::string toString() const;

	bool operator<(const ColumnIdentifier& other) const
	{
		return this->id < other.id;
	}
	bool operator ==(const ColumnIdentifier& other) const
	{
		return this->id == other.id;
	}
};



enum SortOrder
{
	ASCENDING, DESCENDING, UNKNOWN
};

class SortParameter
{
public:
	ColumnIdentifier column;
	std::set<ColumnIdentifier> others;
	SortOrder order;
	SortParameter(const ColumnIdentifier & column,SortOrder order)
	{
		this->column = column;
		this->order = order;
	}
	
	SortParameter(const ColumnIdentifier & column, const ColumnIdentifier & other, SortOrder order)
	{
		this->column = column;
		this->order = order;
		this->others.insert(other);
	}


	SortParameter()
	{
		this->order = SortOrder::UNKNOWN;
	}
};

class SortParameters
{
public:
	std::list<SortParameter> values;
	bool isKnown() const
	{
		return values.size() <= 1;
	}

	SortParameters(const SortParameter & value)
	{
		values.push_back(value);
	}

	SortParameters(const std::vector<SortParameter> & values)
	{
		this->values = std::list<SortParameter>(values.begin(), values.end());
	}
	SortParameters()
	{
	
	}

};

class PossibleSortParameters
{
public:
	std::vector<SortParameters> parameters;
	PossibleSortParameters(const std::vector<SortParameters> & parameters)
	{
		this->parameters = parameters;
	}

	PossibleSortParameters(const std::vector<SortParameter> & parameters)
	{
		for (auto it = parameters.begin(); it != parameters.end();++it)
		{
			this->parameters.push_back(*it);
		}
	}

	PossibleSortParameters()
	{
	}
};

enum AgregateFunctionType
{
	SUM, MIN, MAX, COUNT
};

class AgregateFunction
{
public:
	AgregateFunctionType function;
	std::string functionName;
	ColumnIdentifier parameter;
	ColumnIdentifier output;
};


enum IndexType
{
	CLUSTERED, UNCLUSTERED
};

class Index
{
public:
	IndexType type;
	std::string name;
	std::vector<SortParameter> columns;
	std::string toString()
	{
		std::string column = "";
		ulong i = 0;
		for (auto it = columns.begin(); it != columns.end(); ++it)
		{
			column += it->column.toString();
			column += ":";
			if (it->order == SortOrder::ASCENDING)
			{
				column += "ascending";
			}
			else
			{
				column += "descending";
			}
			++i;
			if (i != columns.size())
			{
				column += ",";
			}
		}
		
		return name + "(" + column + ")";
	}
};

class JoinColumnInfo;

class ColumnInfo
{
public:
	ColumnIdentifier column;
	std::string type;
	double numberOfUniqueValues;
	ColumnInfo(std::string name, std::string type);

	ColumnInfo();

	ColumnInfo(std::string name, double numberOfUniqueValues, std::string type);

	ColumnInfo(const ColumnIdentifier & column, double numberOfUniqueValues, std::string type);

	ColumnInfo(const JoinColumnInfo & info);

};



class JoinColumnInfo : public ColumnInfo
{
public:
	ulong input;
	std::string newColumn;
	JoinColumnInfo(const ColumnInfo & col);

	JoinColumnInfo();
		
};

class ColumnOperation
{
public:
	ColumnIdentifier result;
	std::shared_ptr <Expression> expression;
	std::string type;
};

class GroupColumn
{
public:
	ColumnIdentifier input;
	ColumnIdentifier output;
	GroupColumn(const ColumnIdentifier & input)
	{
		this->input = input;
	}
};

#endif