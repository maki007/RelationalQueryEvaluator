#include "AlgebraStructures.h"

using namespace std;


ColumnIdentifier::ColumnIdentifier(std::string name, int id)
{
	this->name = name;
	this->id = id;
}

ColumnIdentifier::ColumnIdentifier(std::string name)
{
	this->name = name;
	this->id = -1;
}

ColumnIdentifier::ColumnIdentifier()
{
	this->name = "";
	this->id = -1;
}
string ColumnIdentifier::toString() const
{
	return name + "_" + std::to_string(id);
}




ColumnInfo::ColumnInfo(std::string name, std::string type)
{
	this->column = ColumnIdentifier(name);
	this->type = type;
	numberOfUniqueValues = 1;
}
ColumnInfo::ColumnInfo()
{
	this->column = ColumnIdentifier("");
	type = "";
	numberOfUniqueValues = 1;
}
ColumnInfo::ColumnInfo(std::string name, double numberOfUniqueValues, std::string type)
{
	this->column = ColumnIdentifier(name);
	this->type = type;
	this->numberOfUniqueValues = numberOfUniqueValues;
}
ColumnInfo::ColumnInfo(const ColumnIdentifier & column, double numberOfUniqueValues, std::string type)
{
	this->column = column;
	this->type = type;
	this->numberOfUniqueValues = numberOfUniqueValues;
}

ColumnInfo::ColumnInfo(const JoinColumnInfo & info)
{
	this->column = info.column;
	this->type = info.type;
	numberOfUniqueValues = info.numberOfUniqueValues;
}



JoinColumnInfo::JoinColumnInfo(const ColumnInfo & col)
{
	column = col.column;
	type = col.type;
	numberOfUniqueValues = col.numberOfUniqueValues;
}
JoinColumnInfo::JoinColumnInfo()
{

}
