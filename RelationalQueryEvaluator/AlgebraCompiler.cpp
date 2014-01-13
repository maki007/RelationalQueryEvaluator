#include "AlgebraVisitors.h"
#include "ExpressionVisitors.h"
#include <math.h>
#include <algorithm>


const ulong AlgebraCompiler::NUMBER_OF_PLANS = 5;

const ulong AlgebraCompiler::LIMIT_FOR_GREEDY_JOIN_ORDER_ALGORITHM = 8;

const ulong AlgebraCompiler::MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM = 20;


std::shared_ptr<PhysicalPlan> AlgebraCompiler::generateSortParameters(const std::vector<SortParameter> & parameters, const std::shared_ptr<PhysicalPlan> & plan)
{
	std::size_t matchedColumns = 0;
	for (std::size_t i = 0; i<parameters.size() && i<plan->sortedBy.size(); ++i)
	{
		SortParameter sortParameter = parameters[i];
		SortParameter sortedBy = plan->sortedBy[i];
		if (sortedBy.column == sortParameter.column && sortedBy.order == sortParameter.order)
		{
			++matchedColumns;
		}
		else
		{
			break;
		}
	}
	if (matchedColumns == parameters.size())
	{
		//no sort needed
		return plan;
	}
	else
	{
		if (matchedColumns == 0)
		{
			double size = plan->size;
			SortOperator * op = new SortOperator(std::vector<std::string>(), parameters);
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, TimeComplexity::sort(size), plan->columns, plan));
			newPlan->sortedBy = parameters;
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
			SortOperator * op = new SortOperator(sortedBy, std::vector<SortParameter>(parameters.begin() + matchedColumns, parameters.end()));
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, numberOfUniqueSortedValues*TimeComplexity::sort(size / numberOfUniqueSortedValues), plan->columns, plan));
			newPlan->sortedBy = parameters;
			return plan;
		}
	}

}

std::vector<std::shared_ptr<PhysicalPlan> > AlgebraCompiler::getBestPlans(std::vector<std::shared_ptr<PhysicalPlan> > & plans)
{
	std::sort(plans.begin(), plans.end(), PhysicalPlan::Comparator);
	while (plans.size() > NUMBER_OF_PLANS)
	{
		plans.pop_back();
	}
	return plans;
}

std::vector<std::shared_ptr<Expression> > AlgebraCompiler::serializeExpression(std::shared_ptr<Expression> condition)
{
	std::vector<std::shared_ptr<Expression> > result;
	if ((typeid(*(condition)) == typeid(GroupedExpression)))
	{
		std::shared_ptr<GroupedExpression> groupedCondition = std::dynamic_pointer_cast<GroupedExpression>(condition);
		if (groupedCondition->operation == GroupedOperator::AND)
		{
			result = groupedCondition->children;
		}
		else
		{
			result.push_back(condition);
		}
	}
	else
	{
		result.push_back(condition);
	}
	return result;
}

std::shared_ptr<Expression> AlgebraCompiler::deserializeExpression(const std::vector<std::shared_ptr<Expression> > & condition)
{
	if (condition.size() == 0)
	{
		return std::shared_ptr<Expression>(0);
	}
	if (condition.size() == 1)
	{
		return condition[0];
	}
	else
	{
		return std::shared_ptr<Expression>(new GroupedExpression(GroupedOperator::AND, condition));
	}
}

void AlgebraCompiler::visit(Table * node)
{
	result.clear();
	
	PhysicalPlan * physicalPlan=new PhysicalPlan(new TableScan(),(double)node->numberOfRows,
				TimeComplexity::clusteredScan(double(node->numberOfRows)), node->columns);
	
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
				TimeComplexity::unClusteredScan(double(node->numberOfRows)), node->columns);
		
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

void AlgebraCompiler::visit(Sort * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan> > newResult;

	for(auto it=result.begin();it!=result.end();++it)
	{
		newResult.push_back(generateSortParameters(node->parameters,*it));
	}
	result = getBestPlans(newResult);

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
		newSize = std::min(newSize, (*it)->size / 2);
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
		std::shared_ptr<PhysicalPlan> sortedGroup(new PhysicalPlan(new SortedGroup(), newSize, TimeComplexity::sortedGroup((*it)->size) + TimeComplexity::aggregate((*it)->size,node->agregateFunctions.size()),
			sortedGroupColumns,sortedPlan));
		sortedGroup->sortedBy=sortedPlan->sortedBy;	
		
		std::shared_ptr<PhysicalPlan> hashedGroup(new PhysicalPlan(new HashGroup(), newSize, TimeComplexity::hash((*it)->size) + TimeComplexity::aggregate((*it)->size, node->agregateFunctions.size()),
			hashedGroupColumns,*it));
				
		newResult.push_back(hashedGroup);
		newResult.push_back(sortedGroup);
	}
	result = getBestPlans(newResult);
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
		std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new ColumnsOperationsOperator(node->operations),(*it)->size,0,columns,*it));
		newResult.push_back(newPlan);
	}
	
	result = getBestPlans(newResult);
}

void AlgebraCompiler::visit(Selection * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;


	std::vector<std::shared_ptr<Expression> > condition = serializeExpression(node->condition);

	for(auto it=result.begin();it!=result.end();++it)
	{
		if((*it)->indices.size()!=0)
		{
			for (auto index = (*it)->indices.begin(); index != (*it)->indices.end(); ++index)
			{
				std::vector<std::vector<std::shared_ptr<Expression> > > possibleConditions;
				for (auto column = index->columns.begin(); column != index->columns.end(); ++column)
				{
					possibleConditions.push_back(std::vector<std::shared_ptr<Expression> >());
					for (auto conditionPart = condition.begin(); conditionPart != condition.end(); ++conditionPart)
					{
						if (typeid(**conditionPart) == typeid(BinaryExpression))
						{
							std::shared_ptr<BinaryExpression> expression = std::dynamic_pointer_cast<BinaryExpression>(*conditionPart);
							switch (expression->operation)
							{
							case BinaryOperator::LOWER:
							case BinaryOperator::LOWER_OR_EQUAL:
							case BinaryOperator::EQUALS:

								if (typeid(*(expression->leftChild)) == typeid(Column))
								{
									if (typeid(*(expression->rightChild)) == typeid(Constant))
									{
										std::shared_ptr<Column> condColumn = std::dynamic_pointer_cast<Column>(expression->leftChild);
										if (condColumn->name == (*column))
										{
											possibleConditions.back().push_back(expression);
										}
									}
								}

								if (typeid(*(expression->leftChild)) == typeid(Constant))
								{
									if (typeid(*(expression->rightChild)) == typeid(Column))
									{
										std::shared_ptr<Column> condColumn = std::dynamic_pointer_cast<Column>(expression->rightChild);
										if (condColumn->name == (*column))
										{
											possibleConditions.back().push_back(expression);
										}
									}
								}

								break;
							default:
								break;
							}
						}
					}
				}
				std::size_t i= 0;
				std::vector<std::shared_ptr<Expression>> indexConditions;
				while (i<possibleConditions.size() && possibleConditions[i].size()>0)
				{
					std::vector<std::shared_ptr<Expression>> newIndexConditions;
					if (indexConditions.size() == 0)
					{
						for (auto conditionPart = possibleConditions[i].begin(); conditionPart != possibleConditions[i].end(); ++conditionPart)
						{
							newIndexConditions.push_back(*conditionPart);
						}
					}
					else
					{
						for (std::size_t j=0; j<indexConditions.size(); ++j)
						{
							for (std::size_t k = 0; k<possibleConditions[i].size(); ++k)
							{
								newIndexConditions.push_back(std::shared_ptr<Expression>(new BinaryExpression(indexConditions[j], possibleConditions[i][k], BinaryOperator::AND)));
							}
						}
					}
					indexConditions = newIndexConditions;
					++i;
				}

				for (auto expression = indexConditions.begin(); expression != indexConditions.end(); ++expression)
				{
					(*expression)->accept(GroupingExpressionVisitor(&(*expression)));
					double size = (*it)->size;
					SizeEstimatingExpressionVisitor sizeVisitor(&((*it)->columns));
					(*expression)->accept(sizeVisitor);
					size *= sizeVisitor.size;
					std::shared_ptr<PhysicalPlan> indexPlan(new PhysicalPlan(new IndexScan(*expression), size,
						TimeComplexity::indexSearch((*it)->size) + TimeComplexity::unClusteredScan(size),(*it)->columns));
					//TODO: to the filer keeping order
					//indexPlan->sortedBy = (*it)->sortedBy;

					double newSize = size;
					std::vector<std::shared_ptr<Expression> > newCondition;
					std::vector<std::shared_ptr<Expression> >  serialExpresion=serializeExpression(*expression);

					for (auto conditionPart = condition.begin(); conditionPart != condition.end();++conditionPart)
					{
						bool isMatch = false;
						for (auto expressionPart = serialExpresion.begin(); expressionPart != serialExpresion.end(); ++expressionPart)
						{
							if (*expressionPart == *conditionPart)
							{
								isMatch = true;
							}
						}
						if (isMatch == false)
						{
							newCondition.push_back(*conditionPart);
						}
					}
					std::shared_ptr<Expression> filterCondition = deserializeExpression(newCondition);
					if (filterCondition != 0)
					{
						sizeVisitor = SizeEstimatingExpressionVisitor(&((*it)->columns));
						filterCondition->accept(sizeVisitor);
						newSize *= sizeVisitor.size;
						newResult.push_back(std::shared_ptr<PhysicalPlan>(new PhysicalPlan(new Filter(filterCondition), newSize, TimeComplexity::filter(size), (*it)->columns, indexPlan)));
						newResult.push_back(std::shared_ptr<PhysicalPlan>(new PhysicalPlan(new FilterKeepingOrder(filterCondition), newSize, TimeComplexity::filterKeppeingOrder(size), (*it)->columns, indexPlan)));
					}
				}
			}
		}
			
		SizeEstimatingExpressionVisitor sizeVisitor(&((*it)->columns));
		node->condition->accept(sizeVisitor);
		if((*it)->sortedBy.size()!=0)
		{
			std::shared_ptr<PhysicalPlan> sortedPlan(new PhysicalPlan(new FilterKeepingOrder(node->condition), (*it)->size*sizeVisitor.size,
				TimeComplexity::filterKeppeingOrder((*it)->size), (*it)->columns, *it));
			sortedPlan->sortedBy=(*it)->sortedBy;
			newResult.push_back(sortedPlan);
		}
		std::shared_ptr<PhysicalPlan> unsortedPlan(new PhysicalPlan(new Filter(node->condition), (*it)->size*sizeVisitor.size,
			TimeComplexity::filter((*it)->size), (*it)->columns, *it));
		newResult.push_back(unsortedPlan);
	}
	
	result = getBestPlans(newResult);
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

	result = getBestPlans(newResult);
}




std::vector<std::size_t> AlgebraCompiler::getAllSubsets(std::vector<std::size_t> & arr, std::size_t n, std::size_t k) const 
{
	std::vector<std::size_t> result;
	// current subset is given by array of indexes
	std::size_t combinationNumber = 1;

	for (std::size_t a = n; a >= n - k + 1; --a)
	{
		combinationNumber *= a;
	}

	for (std::size_t a = 1; a <= k ; ++a)
	{
		combinationNumber /= a;
	}

	std::vector<std::size_t> idx;
	idx.resize(k);
	for (std::size_t i = 0; i < k; i++) idx[i] = i;

	// loop through all subsets 
	std::size_t count = 0;
	while (true)
	{
		++count;
		result.push_back(setIndex(idx));

		if (k * 2 == n && count == combinationNumber / 2)
			break;


		// check for termination 
		if (idx[0] == n - k) break;

		// advance to next subset 
		for (int i = 0; i < k; i++) {
			// if idx[i] has room to increment 
			if (i + 1 == k || idx[i] + 1 < idx[i + 1]) {
				// increment index and stop here 
				idx[i]++;
				break;
			}
			else {
				// otherwise reset index 
				idx[i] = i;
			}
		}
	}

	return result;
}

void AlgebraCompiler::visit(GroupedJoin * node)
{
	std::vector<ConditionInfo> conditions;
	std::vector<std::shared_ptr<Expression>> cond;
	if (node->condition != 0)
	{
		cond = serializeExpression(node->condition);
	}

	for (auto it = cond.begin(); it != cond.end();++it)
	{
		ConditionInfo info;
		info.condition=*it;
		info.inputs.resize(node->children.size());
		info.condition->accept(JoinInfoReadingExpressionVisitor(&info.inputs,&info.type));
		conditions.push_back(info);
	}

	std::vector < std::vector<std::shared_ptr<PhysicalPlan>>> results;
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;
	for(auto it=node->children.begin();it!=node->children.end();++it)
	{
		(*it)->accept(*this);	
		results.push_back(result);
	}

	std::size_t n = results.size();

	std::vector<JoinInfo> plans;
	std::size_t input = 0;
	for (auto it = results.begin(); it != results.end(); ++it)
	{
		JoinInfo newPlans;
		newPlans.plans = *it;
		newPlans.processedPlans.insert(input);

		for (std::size_t j = 0; j < n; ++j)
		{
			if (j != input)
			{
				newPlans.unProcessedPlans.insert(j);
			}
		}
		plans.push_back(newPlans);
		++input;
	}

	if (n <= LIMIT_FOR_GREEDY_JOIN_ORDER_ALGORITHM)
	{
		std::vector<JoinInfo> allSubsets;
		allSubsets.resize(1 << n);
		
		std::vector<JoinInfo *> lastInsertedPlans;
		//insert plans with one join
		for (auto it = plans.begin(); it != plans.end(); ++it)
		{
			std::size_t newIndex = setIndex(it->processedPlans);
			allSubsets[newIndex] = *it;
			lastInsertedPlans.push_back(&allSubsets[newIndex]);
		}

		
		for (std::size_t i = 1; i < n; ++i)
		{
			std::vector<JoinInfo *> currentPlans;
			for (auto it = lastInsertedPlans.begin(); it != lastInsertedPlans.end(); ++it)
			{
				JoinInfo * current = *it;
				std::set<std::size_t>::iterator max = (current->processedPlans.end());
				--max;
				for (auto it2 = current->unProcessedPlans.find((*max) + 1); it2 != current->unProcessedPlans.end(); ++it2)
				{
					JoinInfo newPlans;
					newPlans.processedPlans = current->processedPlans;
					newPlans.unProcessedPlans = current->unProcessedPlans;
					newPlans.processedPlans.insert(*it2);
					newPlans.unProcessedPlans.erase(*it2);
					std::size_t newIndex = setIndex(newPlans.processedPlans);
					allSubsets[newIndex] = newPlans;
					currentPlans.push_back(&allSubsets[newIndex]);
				}
			}
			lastInsertedPlans = currentPlans;
			for (auto it = currentPlans.begin(); it != currentPlans.end(); ++it)
			{
				JoinInfo * current = *it;
				for (std::size_t j = 1; j <= current->processedPlans.size() / 2; ++j)
				{
					std::vector<std::size_t> input;
					for (auto it2 = current->processedPlans.begin(); it2 != current->processedPlans.end(); ++it2)
					{
						input.push_back(*it2);
					}
					std::vector<std::size_t> subsets = getAllSubsets(input, i+1, j);
					for (auto it2 = subsets.begin(); it2 != subsets.end(); ++it2)
					{
						std::size_t  leftIndex = *it2;
						std::size_t rightIndex = setIndex(allSubsets[*it2].unProcessedPlans);

						join(allSubsets[leftIndex], allSubsets[rightIndex], **it);
					}
				}
			}
		}
		for (auto it = allSubsets.back().plans.begin(); it != allSubsets.back().plans.end(); ++it)
		{
			newResult.push_back(*it);
		}
	}
	else
	{
		//greedy algorithm	
		std::vector<JoinInfo> heap;

		for (auto it = plans.begin(); it != plans.end(); ++it)
		{
			auto end=it->processedPlans.end();
			--end;
			std::size_t max = *end;
			for (auto it2 = it->unProcessedPlans.find(max+1); it2 != it->unProcessedPlans.end(); ++it2)
			{
				greedyJoin(it, it2, plans, heap);
			}

		}

		std::vector<JoinInfo> lastPlans;
		lastPlans.clear();
		lastPlans.swap(heap);
		
		for (std::size_t i = 2; i < n; ++i)
		{
			for (auto it = lastPlans.begin(); it != lastPlans.end(); ++it)
			{
				for (auto it2 = it->unProcessedPlans.begin(); it2 != it->unProcessedPlans.end(); ++it2)
				{
					greedyJoin(it, it2, plans, heap);
				}
			}
			lastPlans.clear();
			lastPlans.swap(heap);
		}
		for (auto it = lastPlans.begin(); it != lastPlans.end(); ++it)
		{
			newResult.push_back(it->plans[0]);
		}
	}


	result = getBestPlans(newResult);
}


void AlgebraCompiler::greedyJoin(std::vector<JoinInfo>::iterator &it, std::set<std::size_t>::iterator &it2, std::vector<JoinInfo> & plans, std::vector<JoinInfo> & heap)
{
	JoinInfo newPlans;
	newPlans.processedPlans = it->processedPlans;
	newPlans.unProcessedPlans = it->unProcessedPlans;
	newPlans.processedPlans.insert(*it2);
	newPlans.unProcessedPlans.erase(*it2);
	join(*it, plans[*it2], newPlans);
	for (auto it3 = newPlans.plans.begin(); it3 != newPlans.plans.end(); ++it3)
	{
		JoinInfo insertedPlan;
		insertedPlan.processedPlans = newPlans.processedPlans;
		insertedPlan.unProcessedPlans = newPlans.unProcessedPlans;
		insertedPlan.plans.push_back(*it3);
		heap.push_back(insertedPlan);
		std::push_heap(heap.begin(), heap.end(), JoinInfo::compare);
		while (heap.size() > MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM)
		{
			std::pop_heap(heap.begin(), heap.end(), JoinInfo::compare);
			heap.pop_back();
		}
	}

}


void AlgebraCompiler::join(const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan)
{
	
}
void AlgebraCompiler::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	result.clear();
}