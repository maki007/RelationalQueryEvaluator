#include <stdio.h>

#include "XmlHandler.h"
#include <fstream>

int main(int argc, const char *argv[])
{
	if (argc != 2)
	{
		printf("SchemaValidator <xml file>\n");
		return 0;
	}

    AlgebraNodeBase * algebraRoot = XmlHandler::GenerateRelationalAlgebra(argv[1]);
	GraphDrawingVisitor * visitor=new GraphDrawingVisitor();
	algebraRoot->accept(*visitor);

	std::ofstream myfile;
	myfile.open ("data/example.txt");
	myfile << visitor->result ;
	myfile.close();

	delete algebraRoot;
	delete visitor;
	return 0;
}