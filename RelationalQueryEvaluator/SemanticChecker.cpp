#include "AlgebraVisitors.h"
#include <set>
#include "ExpressionVisitors.h"
#include <exception>


SemanticChecker::SemanticChecker()
{
	containsErrors=false;
}

void SemanticChecker::visit(Table * node)
{
	outputColumns.clear();

	std::set<std::string> hashSet;
	for(auto it=node->columns.begin();it!=node->columns.end();++it)
	{
		if(hashSet.find(it->name)==hashSet.end())
		{
			hashSet.insert(it->name);
			outputColumns[it->name]=ColumnInfo(it->name,it->type);
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
			if(hashSet.find(*it2)==hashSet.end())
			{
				ReportError("Index cannot be on non-existing column");
			}
			if(indexSet.find(*it2)==indexSet.end())
			{
				indexSet.insert(*it2);
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
		if(outputColumns.find(it->column)==outputColumns.end())
		{
			ReportError("Column doesn't exist");
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
			if(outputColumns.find(it->parameter)==outputColumns.end())
			{
				ReportError("Column doesn't exist");
			}
		}
	}

	outputColumns.clear();
	for(auto it=node->groupColumns.begin();it!=node->groupColumns.end();++it)
	{
		if(outputColumns.find(*it)==outputColumns.end())
		{
			outputColumns[*it]=ColumnInfo(*it,"");
		}
		else
		{
			ReportError("Cannot group by same column twice");
		}
	}
	for(auto it=node->agregateFunctions.begin();it!=node->agregateFunctions.end();++it)
	{
		if(outputColumns.find(it->output)==outputColumns.end())
		{
			outputColumns[it->output]=ColumnInfo(it->output,"");
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
			if(outputColumns.find(it->result)==outputColumns.end())
			{
				ReportError("Column not found");
			}
		}
	}

	outputColumns.clear();
	for(auto it=node->operations.begin();it!=node->operations.end();++it)
	{
		if(outputColumns.find(it->result)==outputColumns.end())
		{
			outputColumns[it->result]=ColumnInfo(it->result,"");
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
	outputColumns.clear();
	for(auto it=node->outputColumns.begin();it!=node->outputColumns.end();++it)
	{
		if(outputColumns.find(it->newName)==outputColumns.end())
		{
			outputColumns[it->newName]=ColumnInfo(it->newName,"");
		}
		else
		{
			ReportError("Columns must have different name"); 
		}
	}
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

	outputColumns.clear();
	for(auto it=node->outputColumns.begin();it!=node->outputColumns.end();++it)
	{
		if(outputColumns.find(it->newName)==outputColumns.end())
		{
			if(it->input==0)
			{
				if(outputColumns0.find(it->newName)==outputColumns0.end())
				{
					ReportError("Column not found in input 0"); 
				}
				else
				{
					outputColumns[it->newName]=ColumnInfo(it->newName,"");
				}
			}
			else if(it->input==1)
			{
				if(outputColumns1.find(it->newName)==outputColumns1.end())
				{
					ReportError("Column not found in input 1"); 
				}
				else
				{
					outputColumns[it->newName]=ColumnInfo(it->newName,"");
				}
			}
			else
			{
				throw new std::exception("Error");
			}

			
		}
		else
		{
			ReportError("Columns must have different name"); 
		}
	}
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