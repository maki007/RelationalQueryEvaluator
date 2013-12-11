#include "AlgebraVisitors.h"
#include <math.h>

void AlgebraCompiler::visit(Table * node)
{
	result.clear();
	
	PhysicalPlan * physicalPlan=new PhysicalPlan(new TableScan(),(double)node->numberOfRows,
				TimeComplexityConstants::TABLE_SCAN*node->numberOfRows, node->columns);
	
	physicalPlan->indices=node->indices;
	for(auto it=node->indices.begin();it!=node->indices.end();++it)
	{
		if(it->type==CLUSTERED)
		{
			for(auto index=it->columns.begin();index!=it->columns.end();++index)
			{
				SortParameter parameter;
				parameter.order=ASCENDING;
				parameter.column=*index;
				physicalPlan->sortedBy.push_back(parameter);
			}
		}
	}
	result.push_back(std::shared_ptr<PhysicalPlan>(physicalPlan));

	for(auto it=node->indices.begin();it!=node->indices.end();++it)
	{
		if(it->type==UNCLUSTERED)
		{

			PhysicalPlan * physicalPlan=new PhysicalPlan(new ScanAndSortByIndex(),(double)node->numberOfRows,
				TimeComplexityConstants::SORT_SCAN*node->numberOfRows, node->columns);
		
			for(auto index=it->columns.begin();index!=it->columns.end();++index)
			{
				SortParameter parameter;
				parameter.order=ASCENDING;
				parameter.column=*index;
				physicalPlan->sortedBy.push_back(parameter);
			}
			result.push_back(std::shared_ptr<PhysicalPlan>(physicalPlan));
		}	
	}
}

std::shared_ptr<PhysicalPlan> AlgebraCompiler::generateSortParameters(const std::vector<SortParameter> & parameters,const std::shared_ptr<PhysicalPlan> & plan)
{
	std::size_t matchedColumns=0;
	for(std::size_t i=0;i<parameters.size() && i<plan->sortedBy.size();++i)
	{
		SortParameter sortParameter=parameters[i];
		SortParameter sortedBy=plan->sortedBy[i];
		if(sortedBy.column==sortParameter.column && sortedBy.order==sortParameter.order)
		{
			++matchedColumns;
		}
		else
		{
			break;
		}
	}
	if(matchedColumns==parameters.size())
	{
		return plan;
	}
	else
	{
		if(matchedColumns==0)
		{
			//todo make into function

			double size=plan->size;
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new SortOperator(),size,TimeComplexityConstants::SORT*size * std::log(size)/std::log(2),plan));
			newPlan->sortedBy=parameters;
			return newPlan;
		}
		else
		{
			//add partial sort
			return plan;
		}
	}
	
}

void AlgebraCompiler::visit(Sort * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan> > newResult;

	for(auto it=result.begin();it!=result.end();++it)
	{
		newResult.push_back(generateSortParameters(node->parameters,*it));
	}
	result=newResult;

}

void AlgebraCompiler::visit(Group * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;
	std::vector<SortParameter> parameters;
	for(auto it=node->groupColumns.begin();it!=node->groupColumns.end();++it)
	{
		SortParameter parameter;
		parameter.order=ASCENDING;
		parameter.column=*it;
		parameters.push_back(parameter);
	}
	
	for(auto it=result.begin();it!=result.end();++it)
	{

		double newSize=1;
		for(auto it2=node->groupColumns.begin();it2!=node->groupColumns.end();++it2)
		{
			newSize*=(*it)->columns[(*it2)].numberOfUniqueValues;
		}

		std::shared_ptr<PhysicalPlan> sortedPlan=generateSortParameters(parameters,*it);
		std::shared_ptr<PhysicalPlan> sortedGroup(new PhysicalPlan(new SortedGroup(),newSize,TimeComplexityConstants::SORTED_GROUP*newSize,sortedPlan));
		sortedGroup->sortedBy=sortedPlan->sortedBy;	
		newResult.push_back(sortedGroup);

		std::shared_ptr<PhysicalPlan> hashedGroup(new PhysicalPlan(new HashGroup(),newSize,TimeComplexityConstants::HASHED_GROUP*newSize,*it));
		newResult.push_back(hashedGroup);
	}
	result=newResult;
}

void AlgebraCompiler::visit(ColumnOperations * node)
{
	node->child->accept(*this);
	result.clear();
}

void AlgebraCompiler::visit(Selection * node)
{
	node->child->accept(*this);
	result.clear();
}

void AlgebraCompiler::visit(Join * node)
{
	throw new std::exception("Not Suported: join should be replaced with groupedJoin");
}

void AlgebraCompiler::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	result.clear();
}
	
void AlgebraCompiler::visit(Union * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	result.clear();
}

void AlgebraCompiler::visit(GroupedJoin * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);	
	}
	result.clear();
}
