#include "AlgebraVisitors.h"
#include "ExpressionVisitors.h"
#include <math.h>
#include <algorithm>
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
		//no sort needed
		return plan;
	}
	else
	{
		if(matchedColumns==0)
		{
			double size=plan->size;
			SortOperator * op = new SortOperator(std::vector<std::string>(), parameters);
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, TimeComplexityConstants::SORT*size * std::log(size) / std::log(2), plan->columns, plan));
			newPlan->sortedBy=parameters;
			return newPlan;
		}
		else
		{
			//only partial sort is needed
			double size = plan->size;
			double numberOfUniqueSortedValues = 1;
			std::vector<std::string> sortedBy;
			for (std::size_t i = 0; i < matchedColumns; ++i)
			{
				sortedBy.push_back(parameters[i].column);
				numberOfUniqueSortedValues *= plan->columns[parameters[i].column].numberOfUniqueValues;
			}
			numberOfUniqueSortedValues = std::min(numberOfUniqueSortedValues, size / 2);
			SortOperator * op = new SortOperator(sortedBy, std::vector<SortParameter>(parameters.begin() + matchedColumns , parameters.end()));
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, numberOfUniqueSortedValues*(TimeComplexityConstants::SORT*size / numberOfUniqueSortedValues * std::log(size*numberOfUniqueSortedValues) / std::log(2)), plan->columns, plan));
			newPlan->sortedBy = parameters;
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
		newSize = std::max(newSize, (*it)->size / 2);
		std::map<std::string,ColumnInfo> hashedGroupColumns,sortedGroupColumns;
		for(auto column=node->groupColumns.begin();column!=node->groupColumns.end();++column)
		{
			ColumnInfo newColumn(*column,(*it)->columns[*column].numberOfUniqueValues);
			hashedGroupColumns[*column]=newColumn;
			sortedGroupColumns[*column]=newColumn;
		}
		for(auto function=node->agregateFunctions.begin();function!=node->agregateFunctions.end();++function)
		{
			ColumnInfo newColumn(function->output,newSize);
			hashedGroupColumns[function->output]=newColumn;
			sortedGroupColumns[function->output]=newColumn;
		}

		std::shared_ptr<PhysicalPlan> sortedPlan=generateSortParameters(parameters,*it);
		std::shared_ptr<PhysicalPlan> sortedGroup(new PhysicalPlan(new SortedGroup(),newSize,TimeComplexityConstants::SORTED_GROUP*newSize,
			sortedGroupColumns,sortedPlan));
		sortedGroup->sortedBy=sortedPlan->sortedBy;	
		
		std::shared_ptr<PhysicalPlan> hashedGroup(new PhysicalPlan(new HashGroup(),newSize,TimeComplexityConstants::HASHED_GROUP*newSize,
			hashedGroupColumns,*it));
				
		newResult.push_back(hashedGroup);
		newResult.push_back(sortedGroup);
	}
	result=newResult;
}

void AlgebraCompiler::visit(ColumnOperations * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;
	
	for(auto it=result.begin();it!=result.end();++it)
	{
		std::map<std::string,ColumnInfo> columns;
		for(auto operation=node->operations.begin();operation!=node->operations.end();++operation)
		{
			ColumnInfo newColumn(operation->result,(*it)->size);
			columns[operation->result]=newColumn;
		}
		std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new ColumnsOperationsOperator(),(*it)->size,0,columns,*it));
		newResult.push_back(newPlan);
	}
	
	result=newResult;
}

void AlgebraCompiler::visit(Selection * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;
	for(auto it=result.begin();it!=result.end();++it)
	{
		if((*it)->indices.size()!=0)
		{
			std::shared_ptr<PhysicalPlan> indexPlan(new PhysicalPlan(new IndexScan(), (*it)->size,
				TimeComplexityConstants::INDEX_SCAN*std::log2((*it)->size), (*it)->columns));
			indexPlan->sortedBy = (*it)->sortedBy;
			newResult.push_back(indexPlan);
		}
			
		SizeEstimatingExpressionVisitor sizeVisitor((*it)->size, &((*it)->columns));
		node->condition->accept(sizeVisitor);
		if((*it)->sortedBy.size()!=0)
		{
			std::shared_ptr<PhysicalPlan> sortedPlan(new PhysicalPlan(new FilterKeepingOrder(node->condition), (*it)->size*sizeVisitor.size,
				TimeComplexityConstants::FILTER_KEEPING_ORDER*(*it)->size,(*it)->columns,*it));
			sortedPlan->sortedBy=(*it)->sortedBy;
			newResult.push_back(sortedPlan);
		}
		std::shared_ptr<PhysicalPlan> unsortedPlan(new PhysicalPlan(new Filter(node->condition), (*it)->size*sizeVisitor.size,
			TimeComplexityConstants::FILTER*(*it)->size,(*it)->columns,*it));
		newResult.push_back(unsortedPlan);
	}
	
	result=newResult;
}

void AlgebraCompiler::visit(Join * node)
{
	throw new std::exception("Not Suported: join should be replaced with groupedJoin");
}

void AlgebraCompiler::visit(Union * node)
{
	node->leftChild->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> leftInput=result;
	node->rightChild->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> rightInput=result;
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;

	for(auto leftIt=leftInput.begin();leftIt!=leftInput.end();++leftIt)
	{
		for(auto rightIt=rightInput.begin();rightIt!=rightInput.end();++rightIt)
		{
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new UnionOperator(),(*leftIt)->size+(*rightIt)->size,
				0,(*leftIt)->columns,*leftIt,*rightIt));
			newResult.push_back(newPlan);
		}
	}

	result=newResult;
}

void AlgebraCompiler::visit(GroupedJoin * node)
{
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);	
	}
	result.clear();
}

void AlgebraCompiler::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	result.clear();
}