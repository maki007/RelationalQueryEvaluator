#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>


#include "XmlHandler.h"

int main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		printf("SchemaValidator <xml file> <xml file> <xml file> ... \n");
		return 0;
	}

	for(int i=1;i<argc;++i)
	{
		AlgebraNodeBase * algebraRoot = XmlHandler::GenerateRelationalAlgebra(argv[i]);
		if(algebraRoot==0)
		{
			return 1;
		}

		GraphDrawingVisitor * visitor=new GraphDrawingVisitor();
		algebraRoot->accept(*visitor);

		std::ofstream myfile;
		std::string s("");
		s.append(argv[i]);
		s.append(".txt");

		myfile.open (s.c_str());
		myfile << visitor->result ;
		myfile.close();

		delete algebraRoot;
		delete visitor;
	}

	system("drawAlgebra.bat");
	return 0;
}