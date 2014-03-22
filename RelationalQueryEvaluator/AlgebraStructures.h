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
	ColumnIdentifier(std::string name, int id);
	
	ColumnIdentifier(std::string name);

	ColumnIdentifier();
	
	std::string toString();
};



enum SortOrder
{
	ASCENDING, DESCENDING, UNKNOWN
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
	std::vector<SortParameter> columns;
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

	ColumnInfo(std::string name, double numberOfUniqueValues);

	ColumnInfo(const ColumnIdentifier & column, double numberOfUniqueValues);

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



#endif