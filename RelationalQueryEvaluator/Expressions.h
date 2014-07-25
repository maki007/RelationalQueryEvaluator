#ifndef ExpressionHPP
#define ExpressionHPP
#include <memory>
#include <vector>
#include <string>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/validators/common/Grammar.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include "XmlUtils.h"
#include "AlgebraStructures.h"

XERCES_CPP_NAMESPACE_USE

namespace rafe {

	typedef unsigned long long int ulong;



	class ExpressionVisitorBase;
	/**
	* Base class for expression in expression tree.
	* Expression trees are used in algebra nodes like selection, join, antijoin or column operations.
	*/
	class Expression
	{
	public:
		Expression * parent; /**< Stores pointer on the parent in expression tree. */

		/**
		* Method for calling visit[node] on given ExpressionVisitor.
		* @param v ExpressionVisitor, on which to call visit function.
		*/
		virtual void accept(ExpressionVisitorBase &v) = 0;

		/**
		* Helper function for choosing which node is going to conctructed from XML element.
		* @param node - element containing information expression.
		*/
		static Expression * constructChildren(DOMElement * node);

		/**
		* Replaces child from this class with new expression tree.
		* @param oldChild - child to replace.
		* @param newChild - child to be replaced.
		*/
		virtual void replaceChild(Expression * oldChild, std::shared_ptr<Expression> newChild) = 0;

	protected:
		Expression()
			: parent(0)
		{}
	};

	/**
	* Class representing unary expression (one child).
	* Currently only unary expresion is negation.
	*/
	class UnaryExpression : public Expression
	{
	public:

		UnaryOperator operation; /**< Stores infromation about type of unary operation. */
		std::shared_ptr<Expression> child; /**< Stores pointer to child tree node.  */

		/**
		* Creates new instance of UnaryExpression.
		* @param node - domelement hoding information about node and its child nodes.
		* @param op - type of unary operation.
		*/
		UnaryExpression(DOMElement * node, UnaryOperator op);

		/**
		* Creates new instance of UnaryExpression.
		* @param node - new child node.
		* @param op - type of unary operation.
		*/
		UnaryExpression(std::shared_ptr<Expression> node, UnaryOperator op);

		void accept(ExpressionVisitorBase &v);
		void replaceChild(Expression * oldChild, std::shared_ptr<Expression> newChild);
	};

	/**
	* Class representing binary expression (two children).
	* It is used to store artimeric(+,-,*,/) and logic operations (and,or).
	*/
	class BinaryExpression : public Expression
	{
	public:
		BinaryOperator operation; /**< Stores infromation about type of binary operation. */
		std::shared_ptr<Expression> leftChild;  /**< Stores pointer to left child tree node.  */
		std::shared_ptr<Expression> rightChild; /**< Stores pointer to right child tree node.  */

		/**
		* Creates new instance of BinaryExpression
		* @param node - domelement hoding information about node and its child nodes.
		* @param op - type of binary operation
		*/
		BinaryExpression(DOMElement * node, BinaryOperator op);

		/**
		* Creates new instance of BinaryExpression.
		* @param leftChild - new left child node
		* @param rightChild - new right child node
		* @param op - type of binary operation
		*/
		BinaryExpression(std::shared_ptr<Expression> & leftChild, std::shared_ptr<Expression> & rightChild, BinaryOperator op);
		void accept(ExpressionVisitorBase &v);
		void replaceChild(Expression * oldChild, std::shared_ptr<Expression> newChild);
	};

	/**
	* Represents co called n-nary expresion, it is a function call with variable argument number.
	* N-nary expression can be for expample sql function like or date or arithmetic function sqrt
	*/
	class NnaryExpression : public Expression
	{
	public:
		std::string name;  /**< Stores function name. */
		std::string returnType; /**< Stores function return type. */
		std::vector<std::shared_ptr<Expression>> arguments; /**< Stores call arguments. Arguments are also expression trees. */

		/**
		* Creates new instace of NnaryExpression.
		* @param node - element containing infromation about node and it's children
		* @param name - function name
		* @param returnType - return type of function
		*/
		NnaryExpression(DOMElement * node, const std::string & name, const std::string & returnType);
		void accept(ExpressionVisitorBase &v);
		void replaceChild(Expression * oldChild, std::shared_ptr<Expression> newChild);
	};

	/**
	* Represents constant expression. It doesnt have any child expressions.
	*/
	class Constant : public Expression
	{
	public:
		std::string value; /**< Stores constant value. */
		std::string type; /**< Stores constant type. */

		/**
		* Creates new instance of Constant.
		* @param node - element containing infromation about constant.
		*/
		Constant(DOMElement * node);
		void accept(ExpressionVisitorBase &v);
		void replaceChild(Expression * oldChild, std::shared_ptr<Expression> newChild);
	};

	/**
	* Represents table column in expression tree.
	*/
	class Column : public Expression
	{
	public:
		ColumnIdentifier column; /**< Identifies column with name and unique id. */
		std::string type; /**< Type of the values stored in the column. */
		ulong input; /**< This field is used for identifing relation of the columns. */

		/**
		* Creates new instance of Column.
		* @param node - element storing information about column.
		*/
		Column(DOMElement * node);
		void accept(ExpressionVisitorBase &v);
		void replaceChild(Expression * oldChild, std::shared_ptr<Expression> newChild);
	};

	/**
	* Represents grouped logical or aritmetic comutative expression.
	* It has variable number of children. Currently it is used only for AND and OR operations.
	*/
	class GroupedExpression : public Expression
	{
	public:
		GroupedOperator operation;  /**< Identifies column with name and unique id. */
		std::vector<std::shared_ptr<Expression>> children; /**< Stores call arguments. Arguments are also expression trees. */

		/**
		* Creates new instance of GrouppedExpression.
		*/
		GroupedExpression();

		/**
		* Creates new instance of GrouppedExpression.
		* @param operation - type of operation.
		* @param children - child nodes.
		*/
		GroupedExpression(GroupedOperator operation, const std::vector<std::shared_ptr<Expression>> & children);
		void accept(ExpressionVisitorBase &v);
		void replaceChild(Expression * oldChild, std::shared_ptr<Expression> newChild);
	};
};
#endif