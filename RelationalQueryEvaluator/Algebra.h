
#ifndef AlgebraHPP
#define AlgebraHPP

class AlgebraNodeBase
{
public:
	AlgebraNodeBase()
	{
	
	}
};

class UnaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	AlgebraNodeBase * child;

};

class BinaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	AlgebraNodeBase * leftChild;
	AlgebraNodeBase * rightChild;
};

class Table : public AlgebraNodeBase
{
	
};

class Sort : public UnaryAlgebraNodeBase
{
	
};

class Group : public UnaryAlgebraNodeBase
{
	
};

class ColumnOperations: public UnaryAlgebraNodeBase
{

};

class Join : public BinaryAlgebraNodeBase
{
	
};

class Selection : public UnaryAlgebraNodeBase
{
	
};


#endif