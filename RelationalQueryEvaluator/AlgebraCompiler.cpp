#include "AlgebraVisitors.h"
#include "ExpressionVisitors.h"
#include <math.h>
#include <algorithm>


using namespace std;

const ulong AlgebraCompiler::NUMBER_OF_PLANS = 5;

const ulong AlgebraCompiler::LIMIT_FOR_GREEDY_JOIN_ORDER_ALGORITHM = 8;

const ulong AlgebraCompiler::MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM = 20;


shared_ptr<PhysicalPlan> AlgebraCompiler::generateSortParameters(const PossibleSortParameters & parameters, const shared_ptr<PhysicalPlan> & plan)
{
	if (parameters.parameters.size() == 0)
	{
		return plan;
	}
	
	if (plan->sortedBy.parameters.size() == 0)
	{
		SortOperator * op = new SortOperator(PossibleSortParameters(), parameters);
		shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, TimeComplexity::sort(size), columns, plan));
		newPlan->sortedBy = parameters;
		return newPlan;
	}
	
	ulong sortedIndex=0;
	ulong sortByIndex=0;
	SortParameters currentSorted = plan->sortedBy.parameters[0];
	SortParameters currentSortBy = parameters.parameters[0];
	PossibleSortParameters sortedBy;
	

	while (true)
	{
		SortParameters newSortedBy;
		for (auto sortByIt = currentSortBy.values.begin(); sortByIt != currentSortBy.values.begin(); ++sortByIt)
		{
			for (auto sortedByIt = currentSorted.values.begin(); sortedByIt != currentSorted.values.begin(); ++sortedByIt)
			{
				if (sortByIt->column.id == sortedByIt->column.id)
				{
					SortOrder sortedByOrder =sortByIt->order;
					SortOrder sortByOrder = sortedByIt->order;
					if ((sortedByOrder == SortOrder::UNKNOWN) || (sortedByOrder == sortByOrder) || (sortByOrder == SortOrder::UNKNOWN))
					{
						newSortedBy.values.push_back(*sortByIt);
						if (sortByOrder == SortOrder::UNKNOWN)
						{
							newSortedBy.values.back().order = sortedByOrder;
						}
						currentSortBy.values.erase(sortByIt);
						currentSorted.values.erase(sortedByIt);
					}
					break;
				}
				else
				{
					for (auto othersIt = sortedByIt->others.begin(); othersIt != sortedByIt->others.end(); ++othersIt)
					{
						if (sortByIt->column.id == othersIt->id)
						{
							SortOrder sortedByOrder = sortByIt->order;
							SortOrder sortByOrder = sortedByIt->order;
							if ((sortedByOrder == SortOrder::UNKNOWN) || (sortedByOrder == sortByOrder) || (sortByOrder == SortOrder::UNKNOWN))
							{
								newSortedBy.values.push_back(*sortByIt);
								if (sortByOrder == SortOrder::UNKNOWN)
								{
									newSortedBy.values.back().order = sortedByOrder;
								}
								currentSortBy.values.erase(sortByIt);
								currentSorted.values.erase(sortedByIt);
								break;
							}
						}
					}
				}
			}
		}

		if (newSortedBy.values.size() == 0)
		{
			break;
		}

		bool end = false;
		if (currentSorted.values.size()==0)
		{
			sortedIndex++;
			if (plan->sortedBy.parameters.size() < sortedIndex)
			{
				currentSorted = plan->sortedBy.parameters[sortedIndex];
			}
			else
			{
				end = true;
			}
		}
		if (currentSortBy.values.size() == 0)
		{
			sortByIndex++;
			if (parameters.parameters.size() < sortedIndex)
			{
				currentSortBy = parameters.parameters[sortedIndex];
			}
			else
			{
				end = true;
			}
		}
		if (end)
		{
			break;
		}
	}
	
	PossibleSortParameters sortBy;
	if (currentSortBy.values.size() > 0)
	{
		sortBy.parameters.push_back(currentSortBy);
	}
	
	for (ulong i = sortedIndex; i < parameters.parameters.size(); ++i)
	{
		sortBy.parameters.push_back(parameters.parameters[i]);
	}
	SortOperator * op = new SortOperator(sortedBy, sortBy);
	double time;
	if (sortedBy.parameters.size() == 0)
	{
		time = TimeComplexity::sort(size);
	}
	else
	{
		double numberOfUniqueSortedValues = 1;
		for (auto it = sortBy.parameters.begin(); it != sortBy.parameters.end();++it)
		{
			for (auto it2 =it->values.begin(); it2 != it->values.end(); ++it2)
			{
				numberOfUniqueSortedValues *= columns[it2->column.id].numberOfUniqueValues;
			}
		}
		numberOfUniqueSortedValues = min(numberOfUniqueSortedValues, size / 2);
		time= numberOfUniqueSortedValues*TimeComplexity::sort(size / numberOfUniqueSortedValues);
	}
	shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, time, columns, plan));
	newPlan->sortedBy = sortedBy;
	for (ulong i = sortedIndex; i < parameters.parameters.size(); ++i)
	{
		newPlan->sortedBy.parameters.push_back(parameters.parameters[i]);
	}
	return newPlan;

	/*
	ulong matchedColumns = 0;
	for (ulong i = 0; i < parameters.size() && i < plan->sortedBy.size(); ++i)
	{
		SortParameter sortParameter = parameters[i];
		SortParameter sortedBy = plan->sortedBy[i];
		if (sortedBy.column.name == sortParameter.column.name)
		{
			if (sortedBy.order == sortParameter.order)
			{
				++matchedColumns;
			}
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
			SortOperator * op = new SortOperator(vector<SortParameter>(), parameters);
			shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, TimeComplexity::sort(size), columns, plan));
			newPlan->sortedBy = parameters;
			return newPlan;
		}
		else
		{
			//only partial sort is needed
			double numberOfUniqueSortedValues = 1;
			vector<SortParameter> sortedBy;
			for (ulong i = 0; i < matchedColumns; ++i)
			{
				sortedBy.push_back(parameters[i]);
				numberOfUniqueSortedValues *= columns[parameters[i].column.id].numberOfUniqueValues;
			}
			numberOfUniqueSortedValues = min(numberOfUniqueSortedValues, size / 2);
			SortOperator * op = new SortOperator(sortedBy, vector<SortParameter>(parameters.begin() + matchedColumns, parameters.end()));
			shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(op, size, numberOfUniqueSortedValues*TimeComplexity::sort(size / numberOfUniqueSortedValues), columns, plan));
			newPlan->sortedBy = parameters;
			return newPlan;
		}
	}
	*/
}

vector<shared_ptr<Expression> > AlgebraCompiler::serializeExpression(shared_ptr<Expression> condition)
{
	vector<shared_ptr<Expression> > result;
	if ((typeid(*(condition)) == typeid(GroupedExpression)))
	{
		shared_ptr<GroupedExpression> groupedCondition = dynamic_pointer_cast<GroupedExpression>(condition);
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

shared_ptr<Expression> AlgebraCompiler::deserializeExpression(const vector<shared_ptr<Expression> > & condition)
{
	if (condition.size() == 0)
	{
		return shared_ptr<Expression>(0);
	}
	if (condition.size() == 1)
	{
		return condition[0];
	}
	else
	{
		return shared_ptr<Expression>(new GroupedExpression(GroupedOperator::AND, condition));
	}
}

void AlgebraCompiler::insertPlan(vector<shared_ptr<PhysicalPlan> > & plans, shared_ptr<PhysicalPlan> & plan)
{
	plans.push_back(plan);
	push_heap(plans.begin(), plans.end(), PhysicalPlan::Comparator);
	while (plans.size() > NUMBER_OF_PLANS)
	{
		pop_heap(plans.begin(), plans.end(), PhysicalPlan::Comparator);
		plans.pop_back();
	}

}

void AlgebraCompiler::visitTable(Table * node)
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
				parameter.order = index->order;
				parameter.column = index->column;
				physicalPlan->sortedBy.parameters.push_back(parameter);
			}
		}
	}
	result.push_back(shared_ptr<PhysicalPlan>(physicalPlan));

	for (auto it = node->indices.begin(); it != node->indices.end(); ++it)
	{
		if (it->type == UNCLUSTERED)
		{
			PhysicalPlan * physicalPlan = new PhysicalPlan(new ScanAndSortByIndex(), (double)node->numberOfRows,
				TimeComplexity::unClusteredScan(double(node->numberOfRows)), node->columns);

			for (auto index = it->columns.begin(); index != it->columns.end(); ++index)
			{
				SortParameter parameter;
				parameter.order = index->order;
				parameter.column = index->column;
				physicalPlan->sortedBy.parameters.push_back(parameter);
			}
			result.push_back(shared_ptr<PhysicalPlan>(physicalPlan));
		}
	}
	size = node->numberOfRows;
	columns.clear();
	for (auto it = node->columns.begin(); it != node->columns.end(); ++it)
	{
		columns[it->column.id] = *it;
	}
	
}

void AlgebraCompiler::visitSort(Sort * node)
{
	node->child->accept(*this);
	vector<shared_ptr<PhysicalPlan> > newResult;

	for (auto it = result.begin(); it != result.end(); ++it)
	{
		insertPlan(newResult, generateSortParameters(PossibleSortParameters(node->parameters), *it));
	}
	result = newResult;
}

void AlgebraCompiler::visitGroup(Group * node)
{
	node->child->accept(*this);
	vector<shared_ptr<PhysicalPlan>> newResult;
	vector<SortParameter> parameters;
	for (auto it = node->groupColumns.begin(); it != node->groupColumns.end(); ++it)
	{
		SortParameter parameter;
		parameter.order = SortOrder::UNKNOWN;
		parameter.column = *it;
		parameters.push_back(parameter);
	}

	double newSize = 1;
	for (auto it2 = node->groupColumns.begin(); it2 != node->groupColumns.end(); ++it2)
	{
		newSize *= columns[(*it2).id].numberOfUniqueValues;
	}
	newSize = min(newSize, size / 2);

	std::map<int, ColumnInfo> newColumns;

	for (auto column = node->groupColumns.begin(); column != node->groupColumns.end(); ++column)
	{
		ColumnInfo newColumn(*column, columns[column->id].numberOfUniqueValues*(newSize / size));
		newColumns[column->id] = newColumn;
	}
	for (auto function = node->agregateFunctions.begin(); function != node->agregateFunctions.end(); ++function)
	{
		ColumnInfo newColumn(function->output, newSize);
		newColumns[function->output.id] = newColumn;
	}

	for (auto it = result.begin(); it != result.end(); ++it)
	{
		shared_ptr<PhysicalPlan> sortedPlan = generateSortParameters(parameters, *it);
		shared_ptr<PhysicalPlan> sortedGroup(new PhysicalPlan(new SortedGroup(), newSize, TimeComplexity::sortedGroup(size) + TimeComplexity::aggregate(size, node->agregateFunctions.size()),
			newColumns, sortedPlan));
		sortedGroup->sortedBy = sortedPlan->sortedBy;

		shared_ptr<PhysicalPlan> hashedGroup(new PhysicalPlan(new HashGroup(), newSize, TimeComplexity::hash(size) + TimeComplexity::aggregate(size, node->agregateFunctions.size()),
			newColumns, *it));
		insertPlan(newResult, hashedGroup);
		insertPlan(newResult, sortedGroup);
	}
	result = newResult;
	size = newSize;
	columns = newColumns;
}

void AlgebraCompiler::visitColumnOperations(ColumnOperations * node)
{
	node->child->accept(*this);
	vector<shared_ptr<PhysicalPlan>> newResult;

	map<int, ColumnInfo> newColumns;
	for (auto operation = node->operations.begin(); operation != node->operations.end(); ++operation)
	{
		ColumnInfo newColumn(operation->result, size);
		if (operation->expression != 0 && typeid(*(operation->expression)) == typeid(Column))
		{
			shared_ptr<Column> column = dynamic_pointer_cast<Column>(operation->expression);
			newColumn.numberOfUniqueValues = columns[column->column.id].numberOfUniqueValues;
		}
		else if (operation->expression != 0)
		{
			MaxOfUniqueValuesExpressionVisitor visitor(&columns);
			operation->expression->accept(visitor);
			newColumn.numberOfUniqueValues = visitor.result;
		}
		else
		{
			newColumn.numberOfUniqueValues = columns[operation->result.id].numberOfUniqueValues;
		}
		newColumns[operation->result.id] = newColumn;
	}

	for (auto it = result.begin(); it != result.end(); ++it)
	{
		shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new ColumnsOperationsOperator(node->operations), size, 0, newColumns, *it));

		for (uint i = 0; i < (*it)->sortedBy.parameters.size(); ++i)
		{
			SortParameters parameters = (*it)->sortedBy.parameters[i];
			//if (newColumns.find((*it)->sortedBy[i].column.id) == newColumns.end())
			newPlan->sortedBy.parameters.push_back(parameters);
			
		}
		insertPlan(newResult, newPlan);
	}
	result = newResult;
	columns = newColumns;
}


void AlgebraCompiler::generateIndexScan(std::vector<std::shared_ptr<PhysicalPlan> >::iterator plan, vector<shared_ptr<Expression> > & condition, vector<shared_ptr<PhysicalPlan>> & newResult)
{
	for (auto index = (*plan)->indices.begin(); index != (*plan)->indices.end(); ++index)
	{
		//posible conditions possibleConditions[i] -> i-th column conditions
		vector<vector<shared_ptr<Expression> > > possibleConditions;
		for (auto column = index->columns.begin(); column != index->columns.end(); ++column)
		{
			possibleConditions.push_back(vector<shared_ptr<Expression> >());
			for (auto conditionPart = condition.begin(); conditionPart != condition.end(); ++conditionPart)
			{
				bool breakLoop = false;
				if (typeid(**conditionPart) == typeid(BinaryExpression))
				{
					shared_ptr<BinaryExpression> expression = dynamic_pointer_cast<BinaryExpression>(*conditionPart);
					switch (expression->operation)
					{
					case BinaryOperator::LOWER:
					case BinaryOperator::LOWER_OR_EQUAL:
					case BinaryOperator::EQUALS:
						bool insertCondition = false;
						if (typeid(*(expression->leftChild)) == typeid(Column))
						{
							if (typeid(*(expression->rightChild)) == typeid(Constant))
							{
								shared_ptr<Column> condColumn = dynamic_pointer_cast<Column>(expression->leftChild);
								if (condColumn->column.id == column->column.id)
								{
									insertCondition = true;
								}
							}
						}

						if (typeid(*(expression->leftChild)) == typeid(Constant))
						{
							if (typeid(*(expression->rightChild)) == typeid(Column))
							{
								shared_ptr<Column> condColumn = dynamic_pointer_cast<Column>(expression->rightChild);
								if (condColumn->column.id == column->column.id)
								{
									
									insertCondition = true;
								}
							}
						}
						
						if (insertCondition)
						{
							if (expression->operation == BinaryOperator::EQUALS)
							{
								possibleConditions.back().clear();
								possibleConditions.back().push_back(expression);
								breakLoop = true;
							}
							else
							{
								possibleConditions.back().push_back(expression);
							}
						}
						break;
					}
				}
				if (breakLoop)
				{
					break;
				}
			}
		}

		//possibleConditions[i] contains one == condition or more <= < > >= conditions.
		for (ulong i = 0; i < possibleConditions.size(); i++)
		{
			if (possibleConditions[i].size()>1)
			{
				vector<shared_ptr<Expression> > newExpressions;
				for (ulong j = 0; j < possibleConditions[i].size(); j++)
				{
					for (ulong k = j+1; k < possibleConditions[i].size(); k++)
					{
						newExpressions.push_back(shared_ptr<Expression>(new BinaryExpression(possibleConditions[i][j], possibleConditions[i][k], BinaryOperator::AND)));
					}
				}
				possibleConditions[i] = newExpressions;
			}
		}

		//generating all possible conditions
		ulong i = 0;
		vector<shared_ptr<Expression>> indexConditions;
		while (i<possibleConditions.size() && possibleConditions[i].size()>0)
		{
			vector<shared_ptr<Expression>> newIndexConditions;
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
						newIndexConditions.push_back(shared_ptr<Expression>(new BinaryExpression(indexConditions[j], possibleConditions[i][k], BinaryOperator::AND)));
					}
				}
			}
			indexConditions = newIndexConditions;
			++i;
		}

		PossibleSortParameters sortedBy(index->columns);

		for (auto expression = indexConditions.begin(); expression != indexConditions.end(); ++expression)
		{
			(*expression)->accept(GroupingExpressionVisitor(&(*expression)));
			double oldSize = size;
			double newSize = size;
			SizeEstimatingExpressionVisitor sizeVisitor(&(columns));
			(*expression)->accept(sizeVisitor);
			newSize *= sizeVisitor.size;
			map<int, ColumnInfo> newColumns = columns;
			for (auto col = newColumns.begin(); col != newColumns.end(); ++col)
			{
				col->second.numberOfUniqueValues *= sizeVisitor.size;
			}
			shared_ptr<PhysicalPlan> indexPlan(new PhysicalPlan(new IndexScan(*expression, *index), newSize,
				TimeComplexity::indexSearch(oldSize) + TimeComplexity::unClusteredScan(newSize), newColumns));
			indexPlan->sortedBy = sortedBy;
			vector<shared_ptr<Expression> > newCondition;
			vector<shared_ptr<Expression> >  serialExpresion = serializeExpression(*expression);

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
			shared_ptr<Expression> filterCondition = deserializeExpression(newCondition);
			if (filterCondition != 0)
			{
				sizeVisitor = SizeEstimatingExpressionVisitor(&(columns));
				filterCondition->accept(sizeVisitor);
				oldSize = newSize;
				newSize *= sizeVisitor.size;
				
				for (auto col = newColumns.begin(); col != newColumns.end(); ++col)
				{
					col->second.numberOfUniqueValues *= sizeVisitor.size;
				}

				shared_ptr<PhysicalPlan> filterWithOrder(new PhysicalPlan(new FilterKeepingOrder(filterCondition), newSize, TimeComplexity::filterKeppeingOrder(oldSize), newColumns, indexPlan));
				filterWithOrder->sortedBy = sortedBy;
				insertPlan(newResult, shared_ptr<PhysicalPlan>(new PhysicalPlan(new Filter(filterCondition), newSize, TimeComplexity::filter(oldSize), newColumns, indexPlan)));
				insertPlan(newResult, filterWithOrder);

			}
			else
			{
				insertPlan(newResult, indexPlan);
			}

		}
	}
}

void AlgebraCompiler::visitSelection(Selection * node)
{
	node->child->accept(*this);
	vector<shared_ptr<PhysicalPlan>> newResult;
	vector<shared_ptr<Expression> > condition = serializeExpression(node->condition);
	std::map<int, ColumnInfo> newColumns=columns;
	SizeEstimatingExpressionVisitor sizeVisitor(&newColumns);
	node->condition->accept(sizeVisitor);
	double newSize = size*sizeVisitor.size;
	for (auto col = newColumns.begin(); col != newColumns.end(); ++col)
	{
		col->second.numberOfUniqueValues *= sizeVisitor.size;
	}

	for (auto it = result.begin(); it != result.end(); ++it)
	{
		if ((*it)->indices.size() != 0)
		{
			generateIndexScan(it, condition, newResult);
		}
		
		if ((*it)->sortedBy.parameters.size() != 0)
		{
			shared_ptr<PhysicalPlan> sortedPlan(new PhysicalPlan(new FilterKeepingOrder(node->condition), newSize,
				TimeComplexity::filterKeppeingOrder(size), columns, *it));
			sortedPlan->sortedBy = (*it)->sortedBy;
			insertPlan(newResult, sortedPlan);
		}
		shared_ptr<PhysicalPlan> unsortedPlan(new PhysicalPlan(new Filter(node->condition), newSize,
			TimeComplexity::filter(size), columns, *it));
		insertPlan(newResult, unsortedPlan);
	}
	result = newResult;
	columns = newColumns;
	size = newSize;
}

void AlgebraCompiler::visitJoin(Join * node)
{
	throw new exception("Not Suported: join should be replaced with groupedJoin");
}

void AlgebraCompiler::visitUnion(Union * node)
{
	node->leftChild->accept(*this);
	double leftSize = size;
	vector<shared_ptr<PhysicalPlan>> leftInput = result;
	node->rightChild->accept(*this);
	double rightSize = size;
	vector<shared_ptr<PhysicalPlan>> rightInput = result;
	vector<shared_ptr<PhysicalPlan>> newResult;

	for (auto leftIt = leftInput.begin(); leftIt != leftInput.end(); ++leftIt)
	{
		for (auto rightIt = rightInput.begin(); rightIt != rightInput.end(); ++rightIt)
		{
			shared_ptr<PhysicalPlan> newPlan(new PhysicalPlan(new UnionOperator(), leftSize + rightSize,
				TimeComplexity::Union(leftSize, rightSize), columns, *leftIt, *rightIt));
			insertPlan(newResult, newPlan);
		}
	}
	result = newResult;
	size = leftSize + rightSize;
}




vector<ulong> AlgebraCompiler::getAllSubsets(vector<ulong> & arr, ulong n, ulong k) const
{
	vector<ulong> result;
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

	vector<ulong> idx;
	idx.resize(k);
	for (ulong i = 0; i < k; i++) idx[i] = i;
	vector<ulong> res;
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

void AlgebraCompiler::visitGroupedJoin(GroupedJoin * node)
{
	vector<shared_ptr<Expression>> cond;
	if (node->condition != 0)
	{
		cond = serializeExpression(node->condition);
	}

	vector<shared_ptr<ConditionInfo>> conditions;
	for (auto it = cond.begin(); it != cond.end(); ++it)
	{
		shared_ptr<ConditionInfo> info = shared_ptr<ConditionInfo>(new ConditionInfo());
		info->condition = *it;
		info->condition->accept(JoinInfoReadingExpressionVisitor(&info->inputs, &info->type));
		conditions.push_back(info);
	}

	//calling subtrees
	vector<vector<shared_ptr<PhysicalPlan>>> results;
	vector<shared_ptr<PhysicalPlan>> newResult;
	std::vector<double> sizes;
	std::vector<std::map<int,ColumnInfo>> allColumns;

	for (auto it = node->children.begin(); it != node->children.end(); ++it)
	{
		(*it)->accept(*this);
		results.push_back(result);
		sizes.push_back(size);
		allColumns.push_back(columns);
	}

	//insert subtrees into structure of join info
	ulong n = results.size();
	vector<JoinInfo> plans;
	ulong input = 0;
	for (uint i = 0; i < allColumns.size();++i)
	{
		JoinInfo newPlans;
		newPlans.size = sizes[i];
		newPlans.plans = results[i];
		newPlans.processedPlans.insert(input);
		for (auto col = allColumns[i].begin(); col != allColumns[i].end(); ++col)
		{
			JoinColumnInfo joinColumnInfo(col->second);
			joinColumnInfo.input = input;
			joinColumnInfo.column = col->second.column;
			newPlans.columns[joinColumnInfo.column.id] = joinColumnInfo;
		}

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
		vector<JoinInfo> allSubsets;
		allSubsets.resize(1 << n);

		vector<JoinInfo *> lastInsertedPlans;
		//insert plans with one join
		for (auto it = plans.begin(); it != plans.end(); ++it)
		{
			ulong newIndex = setIndex(it->processedPlans);
			allSubsets[newIndex] = *it;
			lastInsertedPlans.push_back(&allSubsets[newIndex]);
		}


		for (ulong i = 1; i < n; ++i)
		{
			vector<JoinInfo *> currentPlans;
			for (auto it = lastInsertedPlans.begin(); it != lastInsertedPlans.end(); ++it)
			{
				JoinInfo * current = *it;
				set<ulong>::iterator max = (current->processedPlans.end());
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
					vector<ulong> input;
					ulong wholeSet = 0;
					for (auto it2 = current->processedPlans.begin(); it2 != current->processedPlans.end(); ++it2)
					{
						input.push_back(*it2);
						wholeSet |= ulong(1) << (*it2);
					}
					vector<ulong> subsets = getAllSubsets(input, i + 1, j);
					for (auto it2 = subsets.begin(); it2 != subsets.end(); ++it2)
					{
						ulong leftIndex = *it2;
						ulong rightIndex = wholeSet&(~leftIndex);

						//cout << i + 1 << " " << wholeSet << "==" << (leftIndex | rightIndex) << " " << leftIndex << " " << rightIndex << endl;
						join(allSubsets[leftIndex], allSubsets[rightIndex], **it);

					}
				}
			}
		}
		for (auto it = allSubsets.back().plans.begin(); it != allSubsets.back().plans.end(); ++it)
		{
			insertPlan(newResult, *it);
		}
		columns.clear();
		for (auto it = allSubsets.back().columns.begin(); it != allSubsets.back().columns.end();++it)
		{
			columns[it->first] = it->second;
		}
		size = allSubsets.back().size;
	}
	else
	{
		//greedy algorithm	
		vector<JoinInfo> heap;

		for (auto it = plans.begin(); it != plans.end(); ++it)
		{
			auto end = it->processedPlans.end();
			ulong max = *(--end);
			for (auto it2 = it->unProcessedPlans.find(max + 1); it2 != it->unProcessedPlans.end(); ++it2)
			{
				greedyJoin(it, it2, plans, heap);
			}
		}

		vector<JoinInfo> lastPlans;
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


void AlgebraCompiler::greedyJoin(vector<JoinInfo>::iterator &it, set<ulong>::iterator &it2, vector<JoinInfo> & plans, vector<JoinInfo> & heap)
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
		push_heap(heap.begin(), heap.end(), JoinInfo::Comparator);
		while (heap.size() > MAX_HEAP_SIZE_IN_GREEDY_ALGORITHM)
		{
			pop_heap(heap.begin(), heap.end(), JoinInfo::Comparator);
			heap.pop_back();
		}
	}

}


	


void AlgebraCompiler::join(const JoinInfo & left, const JoinInfo & right, JoinInfo & newPlan)
{
	vector<shared_ptr<ConditionInfo>> equalConditions;
	vector<shared_ptr<ConditionInfo>> lowerConditions;
	vector<shared_ptr<ConditionInfo>> otherConditions;
	for (auto it = left.condition.begin(); it != left.condition.end(); ++it)
	{
		bool containsLeft = false, constainsRight = false;
		for (auto it2 = left.columns.begin(); it2 != left.columns.end(); ++it2)
		{
			if ((*it)->inputs.find((*it2).first) != (*it)->inputs.end())
			{
				containsLeft = true;
			}
		}

		for (auto it3 = right.columns.begin(); it3 != right.columns.end(); ++it3)
		{
			if ((*it)->inputs.find((*it3).first) != (*it)->inputs.end())
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

	newPlan.columns = left.columns;
	for (auto it = right.columns.begin(); it != right.columns.end(); ++it)
	{
		newPlan.columns[it->first] = it->second;
	}

	

	if (equalConditions.size() > 0)
	{
		double newSize = left.size * right.size;
		for (auto it = equalConditions.begin(); it != equalConditions.end(); ++it)
		{
			uint leftColumnIndex = *((*it)->inputs.begin());
			uint rightColumnIndex = *(--((*it)->inputs.end()));
			newSize /= max(newPlan.columns[leftColumnIndex].numberOfUniqueValues, newPlan.columns[rightColumnIndex].numberOfUniqueValues);
		}
		newSize = max(double(1), newSize);
		newPlan.size = newSize;
		for (std::map <ulong, JoinColumnInfo>::iterator it = newPlan.columns.begin(); it != newPlan.columns.end(); ++it)
		{
			it->second.numberOfUniqueValues = min(it->second.numberOfUniqueValues,newSize);
		}
		newPlan.size = newSize;
		vector<shared_ptr<Expression>> expressions;
		for (auto cond = equalConditions.begin(); cond != equalConditions.end(); ++cond)
		{
			expressions.push_back((*cond)->condition);
		}
		map<int, ColumnInfo> newColumns;
		for (auto it = newPlan.columns.begin(); it != newPlan.columns.end(); ++it)
		{
			newColumns[it->first] = JoinColumnInfo(it->second);
			if (left.columns.find(it->first) != left.columns.end())
			{
				newColumns[it->first].numberOfUniqueValues = max(newColumns[it->first].numberOfUniqueValues, newSize);
			}
			else
			{
				newColumns[it->first].numberOfUniqueValues = max(newColumns[it->first].numberOfUniqueValues, newSize);
			}
		}
		shared_ptr<Expression> condition=deserializeExpression(expressions);

		for (auto first = left.plans.begin(); first != left.plans.end(); ++first)
		{
			for (auto second = right.plans.begin(); second != right.plans.end(); ++second)
			{
				HashJoin * hashJoin = new HashJoin(condition);
				shared_ptr<PhysicalPlan> hashedInput;
				shared_ptr<PhysicalPlan> notHashedInput;

				if (left.size < right.size)
				{
					hashedInput = *first;
					notHashedInput = *second;
				}
				else
				{
					hashedInput = *second;
					notHashedInput = *first;
				}
				double time = TimeComplexity::hashJoin(min(left.size, right.size), max(left.size, right.size));
				shared_ptr<PhysicalPlan> hashPlan(new PhysicalPlan(hashJoin, newSize, time, newColumns, hashedInput, notHashedInput));
				if (lowerConditions.size() + otherConditions.size() > 0)
				{
					time += TimeComplexity::filter(newSize);
					shared_ptr<PhysicalPlan> filterPlan(new PhysicalPlan(new Filter(deserializeConditionInfo(lowerConditions, otherConditions)), newSize, time, newColumns, hashPlan));
					insertPlan(newPlan.plans, filterPlan);
				}
				else
				{
					insertPlan(newPlan.plans, hashPlan);
				}

				PossibleSortParameters leftSortParameters;
				leftSortParameters.parameters.push_back(SortParameters());
				std::map<int, int> equalPairs;
				SortParameter leftParameter;
				for (auto cond = equalConditions.begin(); cond != equalConditions.end(); ++cond)
				{
					ulong firstColumn = *((*cond)->inputs.begin());
					ulong secondColumn = *(--((*cond)->inputs.end()));

					
					if (left.columns.find(firstColumn) != left.columns.end())
					{
						leftParameter.column = left.columns.at(firstColumn).column;
						leftParameter.order = SortOrder::UNKNOWN;
						equalPairs[leftParameter.column.id] = right.columns.at(secondColumn).column.id;
					}
					else
					{
						leftParameter.column = left.columns.at(secondColumn).column;
						leftParameter.order = SortOrder::UNKNOWN;
						equalPairs[leftParameter.column.id] = right.columns.at(firstColumn).column.id;
					}
				}
				leftSortParameters.parameters[0].values.push_back(leftParameter);

				shared_ptr<PhysicalPlan> leftSortedPlan;
				leftSortedPlan = generateSortParameters(leftSortParameters, *first);
				PossibleSortParameters rightSortParameters;
				int s = leftSortedPlan->sortedBy.parameters.size();
				for (auto it = leftSortedPlan->sortedBy.parameters.begin(); it != leftSortedPlan->sortedBy.parameters.end(); ++it)
				{
					rightSortParameters.parameters.push_back(SortParameters());
					for (auto it2 = it->values.begin(); it2 != it->values.end(); ++it2)
					{
						if (equalPairs.find(it2->column.id) == equalPairs.end())
						{
							bool found = false;
							for (auto it3 = it2->others.begin(); it3 != it2->others.end(); ++it3)
							{
								if (equalPairs.find(it3->id) != equalPairs.end())
								{
									rightSortParameters.parameters[rightSortParameters.parameters.size() - 1].values.push_back(SortParameter(right.columns.at(equalPairs[it3->id]).column, it2->order));
									found = true;
									break;
								}
							}
							if (!found)
							{
								goto endLoop;
							}
						}
						else
						{
							rightSortParameters.parameters[rightSortParameters.parameters.size() - 1].values.push_back(SortParameter(right.columns.at(equalPairs[it2->column.id]).column, it2->order));
						}
					}
				}
			endLoop:

				shared_ptr<PhysicalPlan> rightSortedPlan;
				rightSortedPlan = generateSortParameters(rightSortParameters, *second);

				MergeEquiJoin * mergeJoin = new MergeEquiJoin(condition);
				time = TimeComplexity::mergeEquiJoin(left.size, right.size);
				shared_ptr<PhysicalPlan> mergePlan(new PhysicalPlan(mergeJoin, newSize, time, newColumns, leftSortedPlan,rightSortedPlan));
				//add sorted by
				insertPlan(newPlan.plans, mergePlan);
				
			}
		}
	}
	if (lowerConditions.size() > 0)
	{
	}

	if (otherConditions.size() > 0)
	{
		throw new exception("other condition should be empty");
	}

	
	if (lowerConditions.size() + equalConditions.size() == 0)
	{
		for (auto first = left.plans.begin(); first != left.plans.end(); ++first)
		{
			for (auto second = right.plans.begin(); second != right.plans.end(); ++second)
			{
				map<int, ColumnInfo> newColumns;
				for (auto it = newPlan.columns.begin(); it != newPlan.columns.end(); ++it)
				{
					newColumns[it->first] = ColumnInfo(it->second);
				}
				double newSize = left.size * right.size;
				newPlan.size = newSize;
				shared_ptr<PhysicalPlan> crossJoinPlan(new PhysicalPlan(new CrossJoin(), newSize, TimeComplexity::crossJoin(left.size, right.size), newColumns, *first, *second));
				insertPlan(newPlan.plans, crossJoinPlan);
			}
		}
	}
}



void AlgebraCompiler::visitAntiJoin(AntiJoin * node)
{
	node->leftChild->accept(*this);
	node->rightChild->accept(*this);
	result.clear();
}