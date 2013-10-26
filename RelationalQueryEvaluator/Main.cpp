#include <stdio.h>

#include "XmlHandler.h"


int main(int argc, const char *argv[])
{
	if (argc != 2)
	{
		printf("SchemaValidator <xml file>\n");
		return 0;
	}

    AlgebraNodeBase * algebraRoot = XmlHandler::GenerateRelationalAlgebra(argv[1]);
	delete algebraRoot;
	return 0;
}