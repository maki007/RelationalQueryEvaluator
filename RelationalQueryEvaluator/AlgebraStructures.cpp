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

bool ColumnIdentifier::operator<(const ColumnIdentifier& other) const
{
	return this->id < other.id;
}
bool ColumnIdentifier::operator ==(const ColumnIdentifier& other) const
{
	return this->id == other.id;
}


SortParameter::SortParameter(const ColumnIdentifier & column, SortOrder order)
{
	this->column = column;
	this->order = order;
}

SortParameter::SortParameter(const ColumnIdentifier & column, const ColumnIdentifier & other, SortOrder order)
{
	this->column = column;
	this->order = order;
	this->others.insert(other);
}

SortParameter::SortParameter()
{
	this->order = SortOrder::UNKNOWN;
}

SortParameters::SortParameters(const SortParameter & value)
{
	values.push_back(value);
}

SortParameters::SortParameters(const std::vector<SortParameter> & values)
{
	this->values = std::list<SortParameter>(values.begin(), values.end());
}

SortParameters::SortParameters()
{

}

bool SortParameters::isKnown() const
{
	return values.size() <= 1;
}

PossibleSortParameters::PossibleSortParameters(const std::vector<SortParameters> & parameters)
{
	this->parameters = parameters;
}

PossibleSortParameters::PossibleSortParameters(const std::vector<SortParameter> & parameters)
{
	for (auto it = parameters.begin(); it != parameters.end(); ++it)
	{
		this->parameters.push_back(*it);
	}
}

PossibleSortParameters::PossibleSortParameters()
{
}

string Index::toString()
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

GroupColumn::GroupColumn(const ColumnIdentifier & input)
{
	this->input = input;
}