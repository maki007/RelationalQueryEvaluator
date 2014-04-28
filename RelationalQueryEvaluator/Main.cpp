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
	s.append(".txt");
	myfile.open(s.c_str());
	myfile << content;
	myfile.close();
}

void drawAlgebra(shared_ptr<AlgebraNodeBase> algebraRoot, string & fileName)
{
	unique_ptr<GraphDrawingVisitor> visitor(new GraphDrawingVisitor());
	algebraRoot->accept(*visitor);

	writeOutput(fileName, visitor->result);
}



void drawPlan(std::vector<std::shared_ptr<PhysicalPlan> > & result, string & fileName)
{
	unique_ptr<PhysicalOperatorDrawingVisitor> planDrawer(new PhysicalOperatorDrawingVisitor());
	sort(result.begin(), result.end(), PhysicalPlan::Comparator);
	for(auto it=result.begin();it!=result.end();++it)
	{
		(*it)->plan->accept(*planDrawer);
		planDrawer->nodeCounter++;
	}
	planDrawer->result+="\n}";

	writeOutput(fileName, planDrawer->result);
}


void drawPlan(std::vector<std::shared_ptr<PhysicalOperator> > & result, string & fileName)
{
	unique_ptr<PhysicalOperatorDrawingVisitorWithouSorts> planDrawer(new PhysicalOperatorDrawingVisitorWithouSorts());
	for (auto it = result.begin(); it != result.end(); ++it)
	{
		(*it)->accept(*planDrawer);
		planDrawer->nodeCounter++;
	}
	planDrawer->result += "\n}";

	writeOutput(fileName, planDrawer->result);
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
			getline(reader,line);
			if(line.size()==0)
				continue;

			line="data/"+line;
			shared_ptr<AlgebraNodeBase> algebraRoot = XmlHandler::GenerateRelationalAlgebra(line.c_str());
			if(algebraRoot==0)
			{
				return 1;
			}
			drawAlgebra(algebraRoot,line+string("._1"));

			unique_ptr<SemanticChecker> semanticChecker(new SemanticChecker());
			algebraRoot->accept(*semanticChecker);
			if(semanticChecker->containsErrors==true)
			{
				cout << "semantic error in " << line << endl;
				return 1;
			}
			
			unique_ptr<GroupingVisitor> groupVisitor(new GroupingVisitor());
			algebraRoot->accept(*groupVisitor);
			drawAlgebra(algebraRoot,line+string("._2"));
			
			
			shared_ptr<AlgebraCompiler> algebraCompiler(new AlgebraCompiler());
			algebraRoot->accept(*algebraCompiler);

			drawPlan(algebraCompiler->result, line + string("._3"));
			
			vector<shared_ptr<PhysicalOperator> > clonedPlans;

			for (auto it = algebraCompiler->result.begin(); it != algebraCompiler->result.end(); ++it)
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

			drawPlan(clonedPlans, line + string("._4"));



			clock_t end = clock();
			double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			cout << "Elapsed time: " << elapsed_secs << endl;
		}
	}
	system("_drawAlgebra.bat");
	return 0;
}