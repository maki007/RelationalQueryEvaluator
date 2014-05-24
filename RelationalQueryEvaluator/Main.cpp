#include <stdio.h>
#include <cstdlib>
#include<iostream>
#include<fstream>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>

#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "XmlHandler.h"
#include "PhysicalOperatorVisitor.h"

using namespace std;

void writeOutput(string & fileName, string & content)
{
	ofstream myfile;
	string s("");
	s.append(fileName);
	myfile.open(s.c_str());
	myfile << content;
	myfile.close();
}

void drawAlgebra(shared_ptr<AlgebraNodeBase> algebraRoot, string & fileName)
{
	GraphDrawingVisitor visitor;
	algebraRoot->accept(visitor);

	writeOutput(fileName, visitor.result);
}



void drawPlan(std::vector<std::shared_ptr<PhysicalPlan> > & result, string & fileName)
{
	PhysicalOperatorDrawingVisitor planDrawer;
	sort(result.begin(), result.end(), PhysicalPlan::Comparator);
	for (auto it = result.begin(); it != result.end(); ++it)
	{
		(*it)->plan->accept(planDrawer);
		planDrawer.nodeCounter++;
	}
	planDrawer.result += "\n}";

	writeOutput(fileName, planDrawer.result);
}


void drawPlan(std::vector<std::shared_ptr<PhysicalOperator> > & result, string & fileName)
{
	PhysicalOperatorDrawingVisitorWithouSorts planDrawer;
	for (auto it = result.begin(); it != result.end(); ++it)
	{
		(*it)->accept(planDrawer);
		planDrawer.nodeCounter++;
	}
	planDrawer.result += "\n}";

	writeOutput(fileName, planDrawer.result);
}

void boboxPlan(std::vector<std::shared_ptr<PhysicalOperator> > & result, string & fileName)
{
	BoboxPlanWritingPhysicalOperatorVisitor planDrawer;
	if (result.size() > 0)
	{
		writeOutput(fileName, planDrawer.writePlan(*(result.begin())));
	}
	else
	{
		writeOutput(fileName, string(""));
	}
}

int main(int argc, const char *argv[])
{
	
	if (argc != 2)
	{
		printf("SchemaValidator <file containg xml file>\n");
		return 0;
	}
	
	ifstream reader;
	reader.open(argv[1]);
	if (reader.is_open())
	{
		while (!reader.eof())
		{

			clock_t begin = clock();

			string line;
			getline(reader, line);
			if (line.size() == 0)
				continue;

			line = "data/" + line;
			shared_ptr<AlgebraNodeBase> algebraRoot = XmlHandler::GenerateRelationalAlgebra(line.c_str());
			if (algebraRoot == 0)
			{
				return 1;
			}
			drawAlgebra(algebraRoot, line + string("._1.txt"));

			SemanticChecker semanticChecker;
			algebraRoot->accept(semanticChecker);
			if (semanticChecker.containsErrors == true)
			{
				cout << "semantic error in " << line << endl;
				return 1;
			}

			GroupingVisitor groupVisitor;
			algebraRoot->accept(groupVisitor);
			drawAlgebra(algebraRoot, line + string("._2.txt"));


			SelectionSpitingVisitor selectionSpliter;
			algebraRoot->accept(selectionSpliter);

			/*SelectionColectingVisitor selectionColecter;
			algebraRoot->accept(selectionColecter);*/

			//move selection up

			/*for (auto it = selectionColecter.selections.begin(); it != selectionColecter.selections.end(); ++it)
			{
				PushSelectionDownVisitor pushDownVisitor(it->get());
				pushDownVisitor.pushDown();
			}*/

			SelectionFusingVisitor selectionFuser;
			algebraRoot->accept(selectionFuser);



			drawAlgebra(algebraRoot, line + string("._3.txt"));

			AlgebraCompiler algebraCompiler;
			algebraRoot->accept(algebraCompiler);

			drawPlan(algebraCompiler.result, line + string("._4.txt"));

			vector<shared_ptr<PhysicalOperator> > clonedPlans;

			for (auto it = algebraCompiler.result.begin(); it != algebraCompiler.result.end(); ++it)
			{

				CloningPhysicalOperatorVisitor cloner;
				(*it)->plan->accept(cloner);
				clonedPlans.push_back(shared_ptr<PhysicalOperator>(cloner.result));
			}

			for (auto it = clonedPlans.begin(); it != clonedPlans.end(); ++it)
			{
				SortResolvingPhysicalOperatorVisitor sortResolver;
				(*it)->accept(sortResolver);
			}

			drawPlan(clonedPlans, line + string("._5.txt"));

			boboxPlan(clonedPlans, line + string("._6.bbx"));


			clock_t end = clock();
			double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			cout << "Elapsed time: " << elapsed_secs << endl;
		}
	}
	system("_drawAlgebra.bat");
	return 0;
}