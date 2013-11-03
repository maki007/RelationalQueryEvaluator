#ifndef AlgebraExpressionHPP
#define AlgebraExpressionHPP


class Expression
{

};

class BinaryAritmeticExpression : public Expression
{

};

class AritmeticFunction : public Expression
{

};


class BinaryBooleanExpression : public Expression
{

};

class UnaryBooleanExpression : public Expression
{

};

class BooleanOperator : public Expression
{

};

class BooleanPredicate : public Expression
{

};

class Constant : public Expression
{

};

class Column : public Expression
{

};

#endif