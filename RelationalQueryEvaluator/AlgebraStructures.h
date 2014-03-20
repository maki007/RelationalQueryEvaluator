#ifndef AlgebraStructureHPP
#define AlgebraStructureHPP

#include <memory>
#include <vector>
#include <string>


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
	ColumnIdentifier(std::string name, int id)
	{
		this->name = name;
		this->id = id;
	}

	ColumnIdentifier(std::string name)
	{
		this->name = name;
		this->id = -1;
	}

	ColumnIdentifier()
	{
		this->name = "";
		this->id = -1;
	}
	std::string toString()
	{
		return name + "_" + std::to_string(id);
	}
};



enum SortOrder
{
	ASCENDING, DESCENDING
};

class SortParameter
{
public:
	ColumnIdentifier column;
	SortOrder order;
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
	std::vector<ColumnIdentifier> columns;
};


class ColumnInfo
{
public:
	ColumnIdentifier column;
	std::string type;
	double numberOfUniqueValues;
	ColumnInfo(std::string name, std::string type)
	{
		this->column = ColumnIdentifier(name);
		this->type = type;
	}
	ColumnInfo()
	{
		this->column = ColumnIdentifier("");
		type = "";
		numberOfUniqueValues = 0;
	}
	ColumnInfo(std::string name, double numberOfUniqueValues)
	{
		this->column = ColumnIdentifier(name);
		this->numberOfUniqueValues = numberOfUniqueValues;
	}
	ColumnInfo(const ColumnIdentifier & column, double numberOfUniqueValues)
	{
		this->column = column;
		this->numberOfUniqueValues = numberOfUniqueValues;
	}
};



class JoinColumnInfo
{
public:
	ulong input;
	ColumnIdentifier column;
	ColumnIdentifier newColumn;
};

class ColumnOperation
{
public:
	ColumnIdentifier result;
	std::shared_ptr <Expression> expression;
	std::string type;
};



#endif