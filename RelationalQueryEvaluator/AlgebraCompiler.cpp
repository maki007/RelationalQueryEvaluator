#include "AlgebraVisitors.h"
#include "ExpressionVisitors.h"
#include <math.h>
#include <algorithm>




const ulong AlgebraCompiler::NUMBER_OF_PLANS = 5;

const ulong AlgebraCompiler::LIMIT_FOR_GREEDY_JOIN_ORDER_ALGORITHM = 8;

const ulong AlgebraCompiler::MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM = 20;


std::shared_ptr<PhysicalPlan> AlgebraCompiler::generateSortParameters(const std::vector<SortParameter> & parameters, const std::shared_ptr<PhysicalPlan> & plan)
{
	ulong matchedColumns = 0;
	for (ulong i = 0; i < parameters.size() && i < plan->sortedBy.size(); ++i)
	{
		SortParameter sortParameter = parameters[i];
		SortParameter sortedBy = plan->sortedBy[i];
		if (sortedBy.column.name == sortParameter.column.name && sortedBy.order == sortParameter.order)
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
			SortOperator * op = new SortOperator(std::vector<ColumnIdentifier>(), parameters);
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, TimeComplexity::sort(size), plan->columns, plan));
			newPlan->sortedBy = parameters;
			return newPlan;
		}
		else
		{
			//only partial sort is needed
			double size = plan->size;
			double numberOfUniqueSortedValues = 1;
			std::vector<ColumnIdentifier> sortedBy;
			for (ulong i = 0; i < matchedColumns; ++i)
			{
				sortedBy.push_back(parameters[i].column);
				numberOfUniqueSortedValues *= plan->columns[parameters[i].column.id].numberOfUniqueValues;
			}
			numberOfUniqueSortedValues = std::min(numberOfUniqueSortedValues, size / 2);
			SortOperator * op = new SortOperator(sortedBy, std::vector<SortParameter>(parameters.begin() + matchedColumns, parameters.end()));
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, numberOfUniqueSortedValues*TimeComplexity::sort(size / numberOfUniqueSortedValues), plan->columns, plan));
			newPlan->sortedBy = parameters;
			return plan;
		}
	}

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

void AlgebraCompiler::insertPlan(std::vector<std::shared_ptr<PhysicalPlan> > & plans, std::shared_ptr<PhysicalPlan> & plan)
{
	plans.push_back(plan);
	std::push_heap(plans.begin(), plans.end(), PhysicalPlan::Comparator);
	while (plans.size() > NUMBER_OF_PLANS)
	{
		std::pop_heap(plans.begin(), plans.end(), PhysicalPlan::Comparator);
		plans.pop_back();
	}

}

void AlgebraCompiler::visit(Table * node)
{
	result.clear();

	PhysicalPlan * physicalPlan = new PhysicalPlan(new TableScan(), (double)node->numberOfRows,
		TimeComplexity::clusteredScan(double(node->numberOfRows)), node->columns);

	physicalPlan->indices = node->indices;
	for (auto it = node->indices.begin(); it != node->indices.end(); ++it)
	{
		if (it->type == CLUSTERED)
		{
			for (auto index = it->columns.begin(); index != it->columns.end(); ++index)
			{
				SortParameter parameter;
				parameter.order = ASCENDING;
				parameter.column = *index;
				physicalPlan->sortedBy.push_back(parameter);
			}
		}
	}
	result.push_back(std::shared_ptr<PhysicalPlan>(physicalPlan));

	for (auto it = node->indices.begin(); it != node->indices.end(); ++it)
	{
		if (it->type == UNCLUSTERED)
		{

			PhysicalPlan * physicalPlan = new PhysicalPlan(new ScanAndSortByIndex(), (double)node->numberOfRows,
				TimeComplexity::unClusteredScan(double(node->numberOfRows)), node->columns);

			for (auto index = it->columns.begin(); index != it->columns.end(); ++index)
			{
				SortParameter parameter;
				parameter.order = ASCENDING;
				parameter.column = *index;
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

	for (auto it = result.begin(); it != result.end(); ++it)
	{
		insertPlan(newResult, generateSortParameters(node->parameters, *it));
	}
	result = newResult;
}

void AlgebraCompiler::visit(Group * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;
	std::vector<SortParameter> parameters;
	for (auto it = node->groupColumns.begin(); it != node->groupColumns.end(); ++it)
	{
		SortParameter parameter;
		parameter.order = ASCENDING;
		parameter.column = *it;
		parameters.push_back(parameter);
	}

	for (auto it = result.begin(); it != result.end(); ++it)
	{

		double newSize = 1;
		for (auto it2 = node->groupColumns.begin(); it2 != node->groupColumns.end(); ++it2)
		{
			newSize *= (*it)->columns[(*it2).id].numberOfUniqueValues;
		}
		newSize = std::min(newSize, (*it)->size / 2);
		std::map<int, ColumnInfo> hashedGroupColumns, sortedGroupColumns;
		for (auto column = node->groupColumns.begin(); column != node->groupColumns.end(); ++column)
		{
			ColumnInfo newColumn(*column, (*it)->columns[column->id].numberOfUniqueValues*(newSize / (*it)->size));
			hashedGroupColumns[column->id] = newColumn;
			sortedGroupColumns[column->id] = newColumn;
		}
		for (auto function = node->agregateFunctions.begin(); function != node->agregateFunctions.end(); ++function)
		{
			ColumnInfo newColumn(function->output, newSize);
			hashedGroupColumns[function->output.id] = newColumn;
			sortedGroupColumns[function->output.id] = newColumn;
		}

		std::shared_ptr<PhysicalPlan> sortedPlan = generateSortParameters(parameters, *it);
		std::shared_ptr<PhysicalPlan> sortedGroup(new PhysicalPlan(new SortedGroup(), newSize, TimeComplexity::sortedGroup((*it)->size) + TimeComplexity::aggregate((*it)->size, node->agregateFunctions.size()),
			sortedGroupColumns, sortedPlan));
		sortedGroup->sortedBy = sortedPlan->sortedBy;

		std::shared_ptr<PhysicalPlan> hashedGroup(new PhysicalPlan(new HashGroup(), newSize, TimeComplexity::hash((*it)->size) + TimeComplexity::aggregate((*it)->size, node->agregateFunctions.size()),
			hashedGroupColumns, *it));
		insertPlan(newResult, hashedGroup);
		insertPlan(newResult, sortedGroup);
	}
	result = newResult;
}

void AlgebraCompiler::visit(ColumnOperations * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;

	for (auto it = result.begin(); it != result.end(); ++it)
	{
		std::map<int, ColumnInfo> columns;
		for (auto operation = node->operations.begin(); operation != node->operations.end(); ++operation)
		{
			ColumnInfo newColumn(operation->result, (*it)->size);
			if (operation->expression != 0 && typeid(*(operation->expression)) == typeid(Column))
			{
				std::shared_ptr<Column> column = std::dynamic_pointer_cast<Column>(operation->expression);
				newColumn.numberOfUniqueValues = (*it)->columns[column->column.id].numberOfUniqueValues;
			}
			columns[operation->result.id] = newColumn;
		}
		std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new ColumnsOperationsOperator(node->operations), (*it)->size, 0, columns, *it));
		insertPlan(newResult, newPlan);
	}
	result = newResult;
}

void AlgebraCompiler::visit(Selection * node)
{
	node->child->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;


	std::vector<std::shared_ptr<Expression> > condition = serializeExpression(node->condition);

	for (auto it = result.begin(); it != result.end(); ++it)
	{
		if ((*it)->indices.size() != 0)
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
										if (condColumn->column.id == column->id)
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
										if (condColumn->column.id == column->id)
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
				ulong i = 0;
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
						for (ulong j = 0; j < indexConditions.size(); ++j)
						{
							for (ulong k = 0; k < possibleConditions[i].size(); ++k)
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
					std::map<int, ColumnInfo> newColumns = (*it)->columns;
					for (auto col = newColumns.begin(); col != newColumns.end(); ++col)
					{
						col->second.numberOfUniqueValues *= sizeVisitor.size;
					}
					std::shared_ptr<PhysicalPlan> indexPlan(new PhysicalPlan(new IndexScan(*expression), size,
						TimeComplexity::indexSearch((*it)->size) + TimeComplexity::unClusteredScan(size), (*it)->columns));
					//TODO: to the filer keeping order
					//indexPlan->sortedBy = (*it)->sortedBy;

					double newSize = size;
					std::vector<std::shared_ptr<Expression> > newCondition;
					std::vector<std::shared_ptr<Expression> >  serialExpresion = serializeExpression(*expression);

					for (auto conditionPart = condition.begin(); conditionPart != condition.end(); ++conditionPart)
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
						for (auto col = newColumns.begin(); col != newColumns.end(); ++col)
						{
							col->second.numberOfUniqueValues *= sizeVisitor.size;
						}
						insertPlan(newResult, std::shared_ptr<PhysicalPlan>(new PhysicalPlan(new Filter(filterCondition), newSize, TimeComplexity::filter(size), newColumns, indexPlan)));
						insertPlan(newResult, std::shared_ptr<PhysicalPlan>(new PhysicalPlan(new FilterKeepingOrder(filterCondition), newSize, TimeComplexity::filterKeppeingOrder(size), newColumns, indexPlan)));
					}
				}
			}
		}

		SizeEstimatingExpressionVisitor sizeVisitor(&((*it)->columns));
		node->condition->accept(sizeVisitor);
		if ((*it)->sortedBy.size() != 0)
		{
			std::shared_ptr<PhysicalPlan> sortedPlan(new PhysicalPlan(new FilterKeepingOrder(node->condition), (*it)->size*sizeVisitor.size,
				TimeComplexity::filterKeppeingOrder((*it)->size), (*it)->columns, *it));
			sortedPlan->sortedBy = (*it)->sortedBy;
			insertPlan(newResult, sortedPlan);
		}
		std::shared_ptr<PhysicalPlan> unsortedPlan(new PhysicalPlan(new Filter(node->condition), (*it)->size*sizeVisitor.size,
			TimeComplexity::filter((*it)->size), (*it)->columns, *it));
		insertPlan(newResult, unsortedPlan);
	}
	result = newResult;
}

void AlgebraCompiler::visit(Join * node)
{
	throw new std::exception("Not Suported: join should be replaced with groupedJoin");
}

void AlgebraCompiler::visit(Union * node)
{
	node->leftChild->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> leftInput = result;
	node->rightChild->accept(*this);
	std::vector<std::shared_ptr<PhysicalPlan>> rightInput = result;
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;

	for (auto leftIt = leftInput.begin(); leftIt != leftInput.end(); ++leftIt)
	{
		for (auto rightIt = rightInput.begin(); rightIt != rightInput.end(); ++rightIt)
		{
			std::shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new UnionOperator(), (*leftIt)->size + (*rightIt)->size,
				0, (*leftIt)->columns, *leftIt, *rightIt));
			insertPlan(newResult, newPlan);
		}
	}
	result = newResult;
}




std::vector<ulong> AlgebraCompiler::getAllSubsets(std::vector<ulong> & arr, ulong n, ulong k) const
{
	std::vector<ulong> result;
	// current subset is given by array of indexes
	ulong combinationNumber = 1;

	for (ulong a = n; a >= n - k + 1; --a)
	{
		combinationNumber *= a;
	}

	for (ulong a = 1; a <= k; ++a)
	{
		combinationNumber /= a;
	}

	std::vector<ulong> idx;
	idx.resize(k);
	for (ulong i = 0; i < k; i++) idx[i] = i;
	std::vector<ulong> res;
	res.resize(k);
	// loop through all subsets 
	ulong count = 0;
	while (true)
	{

		++count;
		ulong i = 0;
		for (auto it = idx.begin(); it != idx.end(); ++it)
		{
			res[i++] = arr[*it];
		}
		result.push_back(setIndex(res));

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
	std::vector<std::shared_ptr<Expression>> cond;
	if (node->condition != 0)
	{
		cond = serializeExpression(node->condition);
	}

	std::vector<std::shared_ptr<ConditionInfo>> conditions;
	for (auto it = cond.begin(); it != cond.end(); ++it)
	{
		std::shared_ptr<ConditionInfo> info = std::shared_ptr<ConditionInfo>(new ConditionInfo());
		info->condition = *it;
		info->condition->accept(JoinInfoReadingExpressionVisitor(&info->inputs, &info->type));
		conditions.push_back(info);
	}

	//calling subtrees
	std::vector<std::vector<std::shared_ptr<PhysicalPlan>>> results;
	std::vector<std::shared_ptr<PhysicalPlan>> newResult;
	for (auto it = node->children.begin(); it != node->children.end(); ++it)
	{
		(*it)->accept(*this);
		results.push_back(result);
	}

	//insert subtrees into structure of join info
	ulong n = results.size();
	std::vector<JoinInfo> plans;
	ulong input = 0;
	for (auto it = results.begin(); it != results.end(); ++it)
	{
		JoinInfo newPlans;
		newPlans.plans = *it;
		newPlans.processedPlans.insert(input);
		for (auto col = it->at(0)->columns.begin(); col != it->at(0)->columns.end(); ++col)
		{
			JoinColumnInfo joinColumnInfo;
			joinColumnInfo.input = input;
			joinColumnInfo.column = col->second.column;
			newPlans.columns.push_back(joinColumnInfo);
		}

		//newPlans.columns

		for (ulong j = 0; j < n; ++j)
		{
			if (j != input)
			{
				newPlans.unProcessedPlans.insert(j);
			}
		}
		newPlans.condition = conditions;
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
			ulong newIndex = setIndex(it->processedPlans);
			allSubsets[newIndex] = *it;
			lastInsertedPlans.push_back(&allSubsets[newIndex]);
		}


		for (ulong i = 1; i < n; ++i)
		{
			std::vector<JoinInfo *> currentPlans;
			for (auto it = lastInsertedPlans.begin(); it != lastInsertedPlans.end(); ++it)
			{
				JoinInfo * current = *it;
				std::set<ulong>::iterator max = (current->processedPlans.end());
				--max;
				for (auto it2 = current->unProcessedPlans.find((*max) + 1); it2 != current->unProcessedPlans.end(); ++it2)
				{
					JoinInfo newPlans;
					newPlans.processedPlans = current->processedPlans;
					newPlans.unProcessedPlans = current->unProcessedPlans;
					newPlans.processedPlans.insert(*it2);
					newPlans.unProcessedPlans.erase(*it2);
					ulong newIndex = setIndex(newPlans.processedPlans);
					allSubsets[newIndex] = newPlans;
					currentPlans.push_back(&allSubsets[newIndex]);
				}
			}
			lastInsertedPlans = currentPlans;
			for (auto it = currentPlans.begin(); it != currentPlans.end(); ++it)
			{
				JoinInfo * current = *it;
				for (ulong j = 1; j <= current->processedPlans.size() / 2; ++j)
				{
					std::vector<ulong> input;
					ulong wholeSet = 0;
					for (auto it2 = current->processedPlans.begin(); it2 != current->processedPlans.end(); ++it2)
					{
						input.push_back(*it2);
						wholeSet |= ulong(1) << (*it2);
					}
					std::vector<ulong> subsets = getAllSubsets(input, i + 1, j);
					for (auto it2 = subsets.begin(); it2 != subsets.end(); ++it2)
					{
						ulong leftIndex = *it2;
						ulong rightIndex = wholeSet&(~leftIndex);

						//std::cout << i + 1 << " " << wholeSet << "==" << (leftIndex | rightIndex) << " " << leftIndex << " " << rightIndex << std::endl;
						join(allSubsets[leftIndex], allSubsets[rightIndex], **it);

					}
				}
			}
		}
		for (auto it = allSubsets.back().plans.begin(); it != allSubsets.back().plans.end(); ++it)
		{
			insertPlan(newResult, *it);
		}
	}
	else
	{
		//greedy algorithm	
		std::vector<JoinInfo> heap;

		for (auto it = plans.begin(); it != plans.end(); ++it)
		{
			auto end = it->processedPlans.end();
			ulong max = *(--end);
			for (auto it2 = it->unProcessedPlans.find(max + 1); it2 != it->unProcessedPlans.end(); ++it2)
			{
				greedyJoin(it, it2, plans, heap);
			}
		}

		std::vector<JoinInfo> lastPlans;
		lastPlans.clear();
		lastPlans.swap(heap);

		for (ulong i = 2; i < n; ++i)
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
			insertPlan(newResult, it->plans[0]);
		}
	}

	result = newResult;
}


void AlgebraCompiler::greedyJoin(std::vector<JoinInfo>::iterator &it, std::set<ulong>::iterator &it2, std::vector<JoinInfo> & plans, std::vector<JoinInfo> & heap)
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
		std::push_heap(heap.begin(), heap.end(), JoinInfo::Comparator);
		while (heap.size() > MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM)
		{
			std::pop_heap(heap.begin(), heap.end(), JoinInfo::Comparator);
			heap.pop_back();
		}
	}

}


void AlgebraCompiler::join(const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan)
{

	std::vector<std::shared_ptr<ConditionInfo>> equalConditions;
	std::vector<std::shared_ptr<ConditionInfo>> lowerConditions;
	std::vector<std::shared_ptr<ConditionInfo>> otherConditions;
	for (auto it = left.condition.begin(); it != left.condition.end(); ++it)
	{
		bool containsLeft = false, constainsRight = false;
		for (auto it2 = left.processedPlans.begin(); it2 != left.processedPlans.end(); ++it2)
		{
			if ((*it)->inputs.find(*it2) != (*it)->inputs.end())
			{
				containsLeft = true;
			}
		}

		for (auto it3 = right.processedPlans.begin(); it3 != right.processedPlans.end(); ++it3)
		{
			if ((*it)->inputs.find(*it3) != (*it)->inputs.end())
			{
				constainsRight = true;
			}
		}
		if (containsLeft && constainsRight)
		{
			switch ((*it)->type)
			{
			case ConditionType::EQUALS:
				equalConditions.push_back(*it);
				break;
			case ConditionType::LOWER:
				lowerConditions.push_back(*it);
				break;
			default:
				otherConditions.push_back(*it);
				break;
			}
		}
		else
		{
			newPlan.condition.push_back(*it);
		}

	}

	if (equalConditions.size() > 0)
	{
		for (auto first = left.plans.begin(); first != left.plans.end(); ++first)
		{
			for (auto second = right.plans.begin(); second != right.plans.end(); ++second)
			{

				double newSize = (*first)->size * (*second)->size;
				std::vector<std::shared_ptr<Expression>> expressions;
				for (auto cond = equalConditions.begin(); cond != equalConditions.end(); ++cond)
				{
					expressions.push_back((*cond)->condition);
				}
				HashJoin * hashJoin = new HashJoin(deserializeExpression(expressions));
				std::shared_ptr<PhysicalPlan> hashedInput;
				std::shared_ptr<PhysicalPlan> notHashedInput;

				if ((*first)->size < (*second)->size)
				{
					hashedInput = *first;
					notHashedInput = *second;
				}
				else
				{
					hashedInput = *second;
					notHashedInput = *first;
				}
				double time = TimeComplexity::hashjoin(std::min((*first)->size, (*second)->size), std::max((*first)->size, (*second)->size));
				std::shared_ptr<PhysicalPlan> hashPlan(new PhysicalPlan(hashJoin, newSize, time, std::map<int, ColumnInfo>(), hashedInput, notHashedInput));
				insertPlan(newPlan.plans, hashPlan);
				//std::shared_ptr<MergeJoin> mergePlan(0);




			}
		}
	}

}
void AlgebraCompiler::visit(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	result.clear();
}