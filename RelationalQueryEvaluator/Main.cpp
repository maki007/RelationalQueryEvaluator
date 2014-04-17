#include <stdio.h>
#include <cstdlib>
#include<iostream>
#include<fstream>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "XmlHandler.h"
#include "PhysicalOperatorVisitor.h"

using namespace std;

void drawAlgebra(shared_ptr<AlgebraNodeBase> algebraRoot, string & line)
{
	unique_ptr<GraphDrawingVisitor> visitor(new GraphDrawingVisitor());
	algebraRoot->accept(*visitor);

	ofstream myfile;
	string s("");
	s.append(line);
	s.append(".txt");
	myfile.open (s.c_str());
	myfile << visitor->result ;
	myfile.close();
}



void drawPlan(shared_ptr<AlgebraNodeBase> algebraRoot, string & line)
{
	unique_ptr<AlgebraCompiler> algebraCompiler(new AlgebraCompiler());
	algebraRoot->accept(*algebraCompiler);
	
	unique_ptr<PhysicalOperatorDrawingVisitor> planDrawer(new PhysicalOperatorDrawingVisitor());
	sort(algebraCompiler->result.begin(), algebraCompiler->result.end(), PhysicalPlan::Comparator);
	for(auto it=algebraCompiler->result.begin();it!=algebraCompiler->result.end();++it)
	{
		(*it)->plan->accept(*planDrawer);
		planDrawer->nodeCounter++;
	}
	planDrawer->result+="\n}";

	ofstream myfile;
	string s("");
	s.append(line);
	s.append(".txt");
	myfile.open (s.c_str());
	myfile << planDrawer->result ;
	myfile.close();

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
			
			drawPlan(algebraRoot,line+string("._3"));;
		}
	}
	system("_drawAlgebra.bat");
	return 0;
}