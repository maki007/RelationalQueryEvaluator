#include <stdio.h>
#include <cstdlib>
#include<iostream>
#include<fstream>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>

#include "Algebra.h"
#include "AlgebraVisitor.h"
#include "XmlHandler.h"

using namespace std;

void drawAlgebra(shared_ptr<AlgebraNodeBase> algebraRoot, string & line)
{
	std::unique_ptr<GraphDrawingVisitor> visitor(new GraphDrawingVisitor());
	algebraRoot->accept(*visitor);

	ofstream myfile;
	string s("");
	s.append(line);
	s.append(".txt");
	myfile.open (s.c_str());
	myfile << visitor->result ;
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
			drawAlgebra(algebraRoot,line+std::string("._1"));
			std::unique_ptr<GroupingVisitor> groupVisitor(new GroupingVisitor());
			algebraRoot->accept(*groupVisitor);

			drawAlgebra(algebraRoot,line+std::string("._2"));
		}
	}
	//system("drawAlgebra.bat");
	return 0;
}