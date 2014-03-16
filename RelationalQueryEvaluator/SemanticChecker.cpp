#include "AlgebraVisitors.h"
#include <set>
#include "ExpressionVisitors.h"
#include <exception>


SemanticChecker::SemanticChecker()
{
	containsErrors=false;
	lastId = 1;
}

int SemanticChecker::nextId()
{
	return lastId++;
}

void SemanticChecker::visit(Table * node)
{
	outputColumns.clear();

	std::set<std::string> hashSet;
	for(auto it=node->columns.begin();it!=node->columns.end();++it)
	{
		if (hashSet.find(it->column.name) == hashSet.end())
		{
			hashSet.insert(it->column.name);
			outputColumns[it->column.name] = ColumnInfo(it->column.name, it->type);
			outputColumns[it->column.name].column.id = nextId();
			it->column = outputColumns[it->column.name].column;
		}
		else
		{
			ReportError("Column names should be unique");
		}
	}

	bool containsClusteredIndex=false;
	for(auto it=node->indices.begin();it!=node->indices.end();++it)
	{
		if(it->type==CLUSTERED)
		{
			if(containsClusteredIndex==false)
			{
				containsClusteredIndex=true;
			}
			else
			{
				ReportError("Table can contain only one clustered index");
			}
		}
		std::set<std::string>  indexSet;
		for(auto it2=it->columns.begin();it2!=it->columns.end();++it2)
		{
			if(hashSet.find(it2->name)==hashSet.end())
			{
				ReportError("Index cannot be on non-existing column");
			}
			else
			{
				it2->id = outputColumns[it2->name].column.id;
			}
			if (indexSet.find(it2->name) == indexSet.end())
			{
				indexSet.insert(it2->name);
			}
			else
			{
				ReportError("Index contains same column multiple times");
			}
		}
	}

}

void SemanticChecker::visit(Sort * node)
{
	node->child->accept(*this);
	for(auto it=node->parameters.begin();it!=node->parameters.end();++it)
	{
		if(outputColumns.find(it->column.name)==outputColumns.end())
		{
			ReportError("Column doesn't exist");
		}
		else
		{
			it->column.id = outputColumns[it->column.name].column.id;
		}
	}
}

void SemanticChecker::visit(Group * node)
{
	node->child->accept(*this);
	for(auto it=node->agregateFunctions.begin();it!=node->agregateFunctions.end();++it)
	{
		if(it->function!=COUNT)
		{
			if(outputColumns.find(it->parameter.name)==outputColumns.end())
			{
				ReportError("Column doesn't exist");
			}
			else
			{
				it->parameter.id = outputColumns[it->parameter.name].column.id;
			}
		}
	}
	std::map<std::string, ColumnInfo> groupColumns;
	for(auto it=node->groupColumns.begin();it!=node->groupColumns.end();++it)
	{
		if(outputColumns.find(it->name)==outputColumns.end())
		{
			ReportError("Column doesn't exist");
		}
		else
		{
			if (groupColumns.find(it->name) == groupColumns.end())
			{
				groupColumns[it->name] = ColumnInfo(it->name, "");
				groupColumns[it->name].column = outputColumns[it->name].column;
				it->id = outputColumns[it->name].column.id;
			}
			else
			{
				ReportError("Cannot group by same column twice");
			}
		}
		
	}
	outputColumns = groupColumns;
	for(auto it=node->agregateFunctions.begin();it!=node->agregateFunctions.end();++it)
	{
		if(outputColumns.find(it->output.name)==outputColumns.end())
		{
			outputColumns[it->output.name] = ColumnInfo(it->output.name, "");
			outputColumns[it->output.name].column.id = nextId();
			it->output.id = outputColumns[it->output.name].column.id;
		}
		else
		{
			ReportError("Columns must have different name");
		}
	}
	if(outputColumns.size()==0)
	{
		ReportError("Group must group by one column or must contain at least one agegate function");
	}
}

void SemanticChecker::visit(ColumnOperations * node)
{
	node->child->accept(*this);

	for(auto it=node->operations.begin();it!=node->operations.end();++it)
	{
		if(it->expression!=0)
		{
			std::shared_ptr<SemanticExpressionVisitor> expresionVisitor;
			expresionVisitor=std::shared_ptr<SemanticExpressionVisitor>(new SemanticExpressionVisitor());
			expresionVisitor->outputColumns0=outputColumns;
			it->expression->accept(*expresionVisitor);
			containsErrors|=expresionVisitor->containsErrors;
		}
		else
		{
			if (outputColumns.find(it->result.name) == outputColumns.end())
			{
				ReportError("Column not found");
			}
			else
			{
				it->result.id = outputColumns[it->result.name].column.id;
			}
		}
	}

	outputColumns.clear();
	for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
	{
		if (outputColumns.find(it->result.name) == outputColumns.end())
		{
			outputColumns[it->result.name] = ColumnInfo(it->result.name, "");
			if (it->expression != 0)
			{
				outputColumns[it->result.name].column.id = nextId();
				it->result.id = outputColumns[it->result.name].column.id;
			}
			else
			{
				outputColumns[it->result.name].column.id = it->result.id;
			}
		}
		else
		{
			ReportError("Columns must have different name");
		}
	}
}

void SemanticChecker::visit(Selection * node)
{
	node->child->accept(*this);
	std::shared_ptr<SemanticExpressionVisitor> expresionVisitor;
	expresionVisitor=std::shared_ptr<SemanticExpressionVisitor>(new SemanticExpressionVisitor());
	expresionVisitor->outputColumns0=outputColumns;
	node->condition->accept(*expresionVisitor);
	containsErrors|=expresionVisitor->containsErrors;
}

void SemanticChecker::visit(Join * node)
{
	std::map<std::string,ColumnInfo> outputColumns0,outputColumns1;
	node->leftChild->accept(*this);
	outputColumns0=outputColumns;
	node->rightChild->accept(*this);
	outputColumns1=outputColumns;

	if(node->condition!=0)
	{
		std::shared_ptr<SemanticExpressionVisitor> expresionVisitor;
		expresionVisitor=std::shared_ptr<SemanticExpressionVisitor>(new SemanticExpressionVisitor());
		expresionVisitor->outputColumns0=outputColumns0;
		expresionVisitor->outputColumns1=outputColumns1;
		node->condition->accept(*expresionVisitor);
		containsErrors|=expresionVisitor->containsErrors;
	}

	checkJoinOutPutParameters(outputColumns0, outputColumns1, node);

}

void SemanticChecker::visit(AntiJoin * node)
{
	std::map<std::string,ColumnInfo> outputColumns0,outputColumns1;
	node->leftChild->accept(*this);
	outputColumns0=outputColumns;
	node->rightChild->accept(*this);
	outputColumns1=outputColumns;

	std::shared_ptr<SemanticExpressionVisitor> expresionVisitor;
	expresionVisitor=std::shared_ptr<SemanticExpressionVisitor>(new SemanticExpressionVisitor());
	expresionVisitor->outputColumns0=outputColumns0;
	expresionVisitor->outputColumns1=outputColumns1;
	node->condition->accept(*expresionVisitor);
	containsErrors|=expresionVisitor->containsErrors;

	checkJoinOutPutParameters(outputColumns0, outputColumns1,node);
}



void SemanticChecker::visit(Union * node)
{
	std::map<std::string,ColumnInfo> outputColumns0,outputColumns1;
	node->leftChild->accept(*this);
	outputColumns0=outputColumns;
	node->rightChild->accept(*this);
	outputColumns1=outputColumns;
	
	if(outputColumns0.size()!=outputColumns1.size())
	{
		ReportError("Union requires inputs to have same size"); 
	}

	for(auto it=outputColumns0.begin();it!=outputColumns0.end();++it)
	{
		if(outputColumns1.find(it->first)==outputColumns1.end())
		{
			ReportError("Union requires inputs to have same columns"); 
		}	
	}
}

void SemanticChecker::visit(GroupedJoin * node)
{
	throw new std::exception("Not Suported: GroupedJoin cannot be in input algebra");
}

void SemanticChecker::ReportError(const char * error)
{
	containsErrors=true;
	std::cout << error << std::endl;
}