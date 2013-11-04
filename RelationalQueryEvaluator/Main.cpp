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
			unique_ptr<AlgebraNodeBase> algebraRoot = XmlHandler::GenerateRelationalAlgebra(line.c_str());
			if(algebraRoot==0)
			{
				return 1;
			}

			GraphDrawingVisitor * visitor=new GraphDrawingVisitor();
			algebraRoot->accept(*visitor);

			ofstream myfile;
			string s("");
			s.append(line);
			s.append(".txt");

			myfile.open (s.c_str());
			myfile << visitor->result ;
			myfile.close();

			delete visitor;
			
		}
	}
	system("drawAlgebra.bat");
	getchar();
	return 0;
}