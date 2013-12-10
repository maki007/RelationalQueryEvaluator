#include "AlgebraVisitors.h"
#include <math.h>

void AlgebraCompiler::visit(Table * node)
{
	result.clear();
	PhysicalPlan * physicalPlan=new PhysicalPlan();
	physicalPlan->plan=std::shared_ptr<PhysicalOperator>(new TableScan());
	physicalPlan->size=(double)node->numberOfRows;
	physicalPlan->timeComplexity= TimeComplexityConstants::TABLE_SCAN*node->numberOfRows;
	physicalPlan->columns=node->columns;
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
			PhysicalPlan * physicalPlan=new PhysicalPlan();
			physicalPlan->plan=std::shared_ptr<PhysicalOperator>(new ScanAndSortByIndex());
			physicalPlan->size=(double)node->numberOfRows;
			physicalPlan->timeComplexity= TimeComplexityConstants::SORT_SCAN*node->numberOfRows;
			physicalPlan->columns=node->columns;
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
			
			SortOperator * sort= new SortOperator();
			sort->child=plan->plan;
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan());
			newPlan->columns=plan->columns;
			newPlan->sortedBy=parameters;
			newPlan->plan=std::shared_ptr<SortOperator>(sort);
			double size=plan->size;
			newPlan->size=size;
			newPlan->timeComplexity=plan->timeComplexity+ TimeComplexityConstants::SORT*size * std::log(size)/std::log(2);
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
		std::shared_ptr<PhysicalPlan> sortedPlan=generateSortParameters(parameters,*it);
		std::shared_ptr<PhysicalPlan> sortedGroup(new PhysicalPlan());
		sortedGroup->columns=sortedPlan->columns;
		sortedGroup->sortedBy=sortedPlan->sortedBy;
		sortedGroup->size=sortedPlan->size;//todo
		sortedGroup->timeComplexity=sortedPlan->timeComplexity+TimeComplexityConstants::SORTED_GROUP*sortedGroup->size;
		SortedGroup *s=new SortedGroup();
		s->child=sortedPlan->plan;
		sortedGroup->plan=std::shared_ptr<PhysicalOperator>(s);
		newResult.push_back(sortedGroup);

		std::shared_ptr<PhysicalPlan> hashedGroup(new PhysicalPlan());
		hashedGroup->columns=sortedPlan->columns;
		hashedGroup->size=sortedPlan->size;//todo
		hashedGroup->timeComplexity=sortedPlan->timeComplexity+TimeComplexityConstants::HASHED_GROUP*hashedGroup->size;
		HashGroup *h=new HashGroup();
		h->child=sortedPlan->plan;
		hashedGroup->plan=std::shared_ptr<PhysicalOperator>(h);
		newResult.push_back(hashedGroup);
	}
	newResult=result;
}

void AlgebraCompiler::visit(ColumnOperations * node)
{
	node->child->accept(*this);
}

void AlgebraCompiler::visit(Selection * node)
{
	node->child->accept(*this);
}

void AlgebraCompiler::visit(Join * node)
{
	throw new std::exception("Not Suported: join should be replaced with groupedJoin");
}

void AlgebraCompiler::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}
	
void AlgebraCompiler::visit(Union * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
}

void AlgebraCompiler::visit(GroupedJoin * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);	
	}
}
