#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "Expressions.h"
#include "ExpressionVisitors.h"
#include <exception>

namespace rafe {

	using namespace std;

	void  AlgebraVisitor::visitTable(Table * node)
	{

	}

	void  AlgebraVisitor::visitSort(Sort * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visitGroup(Group * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visitColumnOperations(ColumnOperations * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visitSelection(Selection * node)
	{
		node->child->accept(*this);
	}

	void  AlgebraVisitor::visitJoin(Join * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	void  AlgebraVisitor::visitAntiJoin(AntiJoin * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	void  AlgebraVisitor::visitUnion(Union * node)
	{
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
	}

	void AlgebraVisitor::visitGroupedJoin(GroupedJoin * node)
	{
		for (auto it = node->children.begin(); it != node->children.end(); ++it)
		{
			(*it)->accept(*this);
		}
	}

	void AlgebraVisitor::serializeExpression(shared_ptr<Expression> & condition, vector<shared_ptr<Expression> > & result)
	{
		if (condition == 0)
			return;
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
	}

	shared_ptr<Expression> AlgebraVisitor::deserializeExpression(const vector<shared_ptr<Expression> > & condition)
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

	void AlgebraVisitor::removeSelection(Selection * node)
	{
		node->child->parent = node->parent;
		node->parent->replaceChild(node, node->child);

	}

	void AlgebraVisitor::insertSelection(AlgebraNodeBase * node, shared_ptr<Selection> & selection)
	{
		selection->child = node->parent->replaceChild(node, std::static_pointer_cast<AlgebraNodeBase>(selection));
		selection->parent = node->parent;
		node->parent = selection.get();
		selection->outputColumns = selection->child->outputColumns;
	}


	GraphDrawingVisitor::GraphDrawingVisitor()
	{
		result = "";
		nodeCounter = 0;
	}

	void GraphDrawingVisitor::generateText(std::string & label, UnaryAlgebraNodeBase * node)
	{
		int identifier = nodeCounter;

		result.append("node");
		result.append(to_string(nodeCounter));
		result.append("[label=\"" + label + "\"]\n");
		int childIdentifier = ++nodeCounter;
		node->child->accept(*this);

		result.append("node");
		result.append(to_string(childIdentifier));
		result.append(" -> node");
		result.append(to_string(identifier));
		result.append("[headport=s, tailport=n,label=\"   \"]\n");

	}

	void GraphDrawingVisitor::generateText(std::string & label, BinaryAlgebraNodeBase * node)
	{
		int identifier = nodeCounter;

		result.append("node");
		result.append(to_string(nodeCounter));
		result.append("[label=\"" + label + "\"]\n");

		int childIdentifier = ++nodeCounter;
		node->leftChild->accept(*this);
		result.append("node");
		result.append(to_string(childIdentifier));
		result.append(" -> node");
		result.append(to_string(identifier));
		result.append("[headport=s, tailport=n,label=\"   \"]\n");

		childIdentifier = ++nodeCounter;
		node->rightChild->accept(*this);
		result.append("node");
		result.append(to_string(childIdentifier));
		result.append(" -> node");
		result.append(to_string(identifier));
		result.append("[headport=s, tailport=n,label=\"   \"]\n");

	}

	void GraphDrawingVisitor::visitSort(Sort * node)
	{
		result = "digraph g {node [shape=box]\n graph[rankdir=\"BT\", concentrate=true];\n";
		string label = "Sort";
		if (node->parameters.size() != 0)
		{
			label += "\n";
		}
		for (auto it = node->parameters.begin(); it != node->parameters.end(); ++it)
		{
			label += it->column.toString();
			if (it->order == ASCENDING)
			{
				label += " asc";
			}
			else
			{
				label += " desc";
			}
			if (it != node->parameters.end() - 1)
				label += ", ";
		}
		generateText(label, node);

		result += "\n}";
	}

	void GraphDrawingVisitor::visitGroup(Group * node)
	{
		string label = "Group\n";
		label += "groupBy ";
		for (auto it = node->groupColumns.begin(); it != node->groupColumns.end(); ++it)
		{
			label += it->input.toString();
			label += ", ";
		}
		if (node->groupColumns.size() == 0)
		{
			label += "nothing,";
		}

		for (auto it = node->agregateFunctions.begin(); it != node->agregateFunctions.end(); ++it)
		{
			label += it->output.toString();
			label += "=";
			label += it->functionName;
			if (it->parameter.name == "")
			{
				label += "()";
			}
			else
			{
				label += "(" + it->parameter.toString() + ")";
			}
			label += ";";
		}

		generateText(label, node);
	}

	void GraphDrawingVisitor::visitTable(Table * node)
	{
		result.append("node");
		result.append(to_string(nodeCounter));
		string label = "[label=\"Table ";
		label += node->name;
		label += "\n number of rows: ";
		label += to_string(node->numberOfRows);
		label += "\n columns: ";
		for (auto it = node->columns.begin(); it != node->columns.end(); ++it)
		{
			label += it->column.toString();
			label += "(";
			label += it->type;
			label += ",";
			label += to_string((ulong)it->numberOfUniqueValues);
			label += ")";

			if (it != node->columns.end() - 1)
			{
				label += ", ";
			}
		}
		label += "\n indices: ";

		for (auto it = node->indices.begin(); it != node->indices.end(); ++it)
		{
			if (it->type == IndexType::CLUSTERED)
			{
				label += "CLUSTERED";
			}
			else
			{
				label += "UNCLUSTERED";
			}
			label += "(";
			for (auto it2 = it->columns.begin(); it2 != it->columns.end(); ++it2)
			{
				label += it2->column.name;
				if (it2 != it->columns.end() - 1)
				{
					label += ", ";
				}
			}
			label += ")";
			if (it != node->indices.end() - 1)
			{
				label += ", ";
			}
		}

		label += "\"]\n";
		result.append(label);
	}

	void GraphDrawingVisitor::visitColumnOperations(ColumnOperations * node)
	{
		string label = "ColumnOperations\n";
		for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
		{
			label += it->result.toString();
			if (it->expression != 0)
			{
				shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
				it->expression->accept(*visitor);
				label += " = ";
				label += visitor->result;
			}
			if (it != node->operations.end() - 1)
			{
				label += ", ";
			}
		}
		generateText(label, node);

	}

	void GraphDrawingVisitor::visitSelection(Selection * node)
	{
		string label = "Selection\n";
		shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
		node->condition->accept(*visitor);
		label += visitor->result;
		generateText(label, node);
	}

	void GraphDrawingVisitor::visitJoin(Join * node)
	{
		string label = "Join\n";
		//crossjoin
		if (node->condition != 0)
		{
			shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
			node->condition->accept(*visitor);
			label += visitor->result;
		}
		generateText(label, node);
	}

	void GraphDrawingVisitor::visitAntiJoin(AntiJoin * node)
	{
		string label = "AntiJoin\n";
		shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
		node->condition->accept(*visitor);
		label += visitor->result;
		generateText(label, node);
	}

	void GraphDrawingVisitor::visitUnion(Union * node)
	{
		generateText(string("Union"), node);
	}

	void GraphDrawingVisitor::generateText(std::string & label, GroupedAlgebraNode * node)
	{
		int identifier = nodeCounter;

		result.append("node");
		result.append(to_string(nodeCounter));
		result.append("[label=\"" + label + "\"]\n");


		int childIdentifier;
		for (auto it = node->children.begin(); it != node->children.end(); ++it)
		{
			childIdentifier = ++nodeCounter;
			(*it)->accept(*this);
			result.append("node");
			result.append(to_string(childIdentifier));
			result.append(" -> node");
			result.append(to_string(identifier));
			result.append("[headport=s, tailport=n,label=\"   \"]\n");
		}

	}

	void GraphDrawingVisitor::visitGroupedJoin(GroupedJoin * node)
	{
		string label = "GroupedJoin\n";
		shared_ptr<WritingExpressionVisitor> visitor(new WritingExpressionVisitor());
		if (node->condition != 0)
		{
			node->condition->accept(*visitor);
			label += visitor->result;
		}
		generateText(label, node);
	}




	GroupingVisitor::GroupingVisitor()
	{

	}

	void GroupingVisitor::visitJoin(Join * node)
	{
		if (node->condition.get() != 0)
		{
			node->condition->accept(GroupingExpressionVisitor(&(node->condition)));
		}
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);
		GroupedJoin * groupedOperator = new GroupedJoin();
		groupedOperator->outputJoinColumns = node->outputJoinColumns;

		groupedOperator->outputColumns.clear();
		for (auto it = groupedOperator->outputJoinColumns.begin(); it != groupedOperator->outputJoinColumns.end(); ++it)
		{
			groupedOperator->outputColumns.insert(make_pair(it->column.id, *it));
			groupedOperator->outputColumns[it->column.id].column.name = it->newColumn;
		}


		groupedOperator->condition = node->condition;

		resolveJoins(node, groupedOperator);
		for (auto it = groupedOperator->children.begin(); it != groupedOperator->children.end(); ++it)
		{
			(*it)->parent = groupedOperator;
		}

	}


	void GroupingVisitor::resolveJoins(Join * node, GroupedJoin * groupedOperator)
	{
		vector<shared_ptr<AlgebraNodeBase> > oldChildren;
		oldChildren.resize(2);
		oldChildren[0] = node->leftChild;
		oldChildren[1] = node->rightChild;
		shared_ptr<Expression> newCondition = 0;
		ulong numberOfChildreninFirstChild = 0;
		for (ulong i = 0; i < 2; ++i)
		{
			if (typeid(*(oldChildren[i])) == typeid(GroupedJoin))
			{
				shared_ptr<GroupedJoin> newNode = dynamic_pointer_cast<GroupedJoin>(oldChildren[i]);
				if (groupedOperator->condition != 0)
				{
					groupedOperator->condition->accept(RenamingJoinConditionExpressionVisitor(i, &newNode->outputJoinColumns));
				}

				if (i == 1)
				{
					if (newNode->condition != 0)
					{
						GetColumnsNodesVisitor visitor;
						newNode->condition->accept(visitor);
						for (auto it = visitor.nodes.begin(); it != visitor.nodes.end(); ++it)
						{
							(*it)->input += groupedOperator->children.size();
						}
					}
				}

				vector<shared_ptr<AlgebraNodeBase>> children = newNode->children;
				if (newCondition == 0)
				{
					newCondition = newNode->condition;
				}
				else
				{
					if (newNode->condition != 0)
					{
						newCondition = shared_ptr<Expression>(new BinaryExpression(newNode->condition, newCondition, BinaryOperator::AND));
					}
				}

				for (auto it = children.begin(); it != children.end(); ++it)
				{
					groupedOperator->children.push_back(*it);
				}
			}
			else
			{
				groupedOperator->children.push_back(oldChildren[i]);
			}
			if (i == 0)
			{
				numberOfChildreninFirstChild = groupedOperator->children.size();
			}
		}


		if (groupedOperator->condition != 0)
		{
			GetColumnsNodesVisitor visitor;
			groupedOperator->condition->accept(visitor);
			for (long long int i = 1; i >= 0; --i)
			{
				if (typeid(*(oldChildren[i])) == typeid(GroupedJoin))
				{
					shared_ptr<GroupedJoin> join = dynamic_pointer_cast<GroupedJoin>(oldChildren[i]);
					for (auto it = join->outputJoinColumns.begin(); it != join->outputJoinColumns.end(); ++it)
					{
						int columnId = it->column.id;
						for (auto it2 = visitor.nodes.begin(); it2 != visitor.nodes.end(); ++it2)
						{
							if ((*it2)->column.id == columnId)
							{
								(*it2)->input = it->input + numberOfChildreninFirstChild;
							}
						}
						for (auto it2 = groupedOperator->outputJoinColumns.begin(); it2 != groupedOperator->outputJoinColumns.end(); ++it2)
						{
							if ((it2)->column.id == columnId)
							{
								(it2)->input = it->input + numberOfChildreninFirstChild;;
							}
						}
					}
				}
				else
				{
					for (auto it2 = visitor.nodes.begin(); it2 != visitor.nodes.end(); ++it2)
					{
						if ((*it2)->input == i)
						{
							(*it2)->input = numberOfChildreninFirstChild;
						}
					}
					for (auto it2 = groupedOperator->outputJoinColumns.begin(); it2 != groupedOperator->outputJoinColumns.end(); ++it2)
					{
						if ((it2)->input == i)
						{
							(it2)->input = numberOfChildreninFirstChild;
						}
					}

				}
				numberOfChildreninFirstChild = 0;

			}

		}

		if (newCondition.get() == 0)
		{
			newCondition = groupedOperator->condition;
		}
		else
		{
			if (groupedOperator->condition.get() != 0)
			{
				newCondition = shared_ptr<Expression>(new BinaryExpression(groupedOperator->condition, newCondition, BinaryOperator::AND));
			}
		}
		groupedOperator->condition = newCondition;
		groupedOperator->parent = node->parent;
		node->parent->replaceChild(node, shared_ptr<AlgebraNodeBase>(groupedOperator));
		if (groupedOperator->condition.get() != 0)
		{
			groupedOperator->condition->accept(GroupingExpressionVisitor(&(groupedOperator->condition)));
		}
	}

	void GroupingVisitor::visitColumnOperations(ColumnOperations * node)
	{
		for (auto it = node->operations.begin(); it != node->operations.end(); ++it)
		{
			if (it->expression.get() != 0)
			{
				it->expression->accept(GroupingExpressionVisitor(&it->expression));
			}
		}
		node->child->accept(*this);
	}

	void GroupingVisitor::visitSelection(Selection * node)
	{
		node->condition->accept(GroupingExpressionVisitor(&(node->condition)));
		node->child->accept(*this);
	}



	void GroupingVisitor::visitAntiJoin(AntiJoin * node)
	{
		node->condition->accept(GroupingExpressionVisitor(&(node->condition)));
		node->leftChild->accept(*this);
		node->rightChild->accept(*this);

	}
};