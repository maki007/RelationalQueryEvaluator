#ifndef AlgebraHPP
#define AlgebraHPP

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

#include <iostream>
#include <string>
#include <sstream>
#include "XmlUtils.h"
#include "Expressions.h"
#include <memory>
#include <map>
XERCES_CPP_NAMESPACE_USE

typedef unsigned long long int ulong;
typedef unsigned int uint;


namespace rafe {

	class AlgebraVisitor;

	/**
	* Abstract class for algebraic operation.
	*/
	class AlgebraNodeBase
	{
	public:
		/**
		* Stores ouput columns of this node. Map key is unique column identifier and map stores the information about column.
		*/
		std::map<int, ColumnInfo> outputColumns;

		ulong lineNumber = 0; /**< Stores the line number of the input element for this node. */

		AlgebraNodeBase * parent; /**< Stores the pointer on parent in the algebra tree. */

		/**
		* Creates the instance of AlgebraNodeBase.
		*/
		AlgebraNodeBase::AlgebraNodeBase();

		/**
		* Creates the instance of AlgebraNodeBase.
		* @param element representing input node.
		*/
		AlgebraNodeBase(DOMElement * element);

		/**
		* Helper method for creating algebra tree from DOM tree.
		* @param node representing input node.
		* @return newely created Algebra node.
		*/
		AlgebraNodeBase * constructChildren(DOMElement * node);

		/**
		* Method for calling visit[node] on given AlgebraVisitor.
		* @param v AlgebraVisitor on which to call function.
		*/
		virtual void accept(AlgebraVisitor &v) = 0;

		/**
		* Replaces one child of this node with other one.
		* @param oldChild node to be replaced.
		* @param newChild new node to replace the old one.
		* @return replaced child.
		*/
		virtual std::shared_ptr<AlgebraNodeBase> replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild) = 0;
	};

	/**
	* Abstract class for unary algebraic operation.
	*/
	class UnaryAlgebraNodeBase : public AlgebraNodeBase
	{
	public:
		std::shared_ptr <AlgebraNodeBase> child;  /**< Stores pointer to child node. */

		/**
		* Creates the instance of UnaryAlgebraNodeBase.
		*/
		UnaryAlgebraNodeBase();

		/**
		* Creates the instance of UnaryAlgebraNodeBase.
		* @param element representing input node.
		*/
		UnaryAlgebraNodeBase(DOMElement * element);

		virtual void accept(AlgebraVisitor &v) = 0;
		std::shared_ptr<AlgebraNodeBase> replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild);
	};

	/**
	* Abstract class for binary algebraic operation.
	*/
	class BinaryAlgebraNodeBase : public AlgebraNodeBase
	{
	public:
		std::shared_ptr <AlgebraNodeBase> leftChild;  /**< Stores pointer to the left child node. */
		std::shared_ptr <AlgebraNodeBase> rightChild;  /**< Stores pointer to the right child node. */

		/**
		* Creates the instance of BinaryAlgebraNodeBase.
		*/
		BinaryAlgebraNodeBase();

		/**
		* Creates the instance of UnaryAlgebraNodeBase.
		* @param element representing input node.
		*/
		BinaryAlgebraNodeBase(DOMElement * element);

		/**
		* Helper method for join and antijoin. Reads all needed parameters from DOM tree.
		* @param node - input DOM element.
		* @param condition - output join condition.
		* @param outputColumns - join output columns.
		*/
		void constructJoinParameters(DOMElement * node, std::shared_ptr<Expression> & condition, std::vector<JoinColumnInfo> & outputColumns);

		virtual void accept(AlgebraVisitor &v) = 0;
		std::shared_ptr<AlgebraNodeBase> replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild);
	};

	/**
	* Abstract class for n-nary algebraic operation.
	*/
	class GroupedAlgebraNode : public AlgebraNodeBase
	{
	public:
		std::vector<std::shared_ptr<AlgebraNodeBase>> children; /**< Stores list containg the children of this node. */
		virtual void accept(AlgebraVisitor &v) = 0;
		std::shared_ptr<AlgebraNodeBase> replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild);
	};

	/**
	* Abstract class for nullary algebraic operation.
	*/
	class NullaryAlgebraNodeBase : public AlgebraNodeBase
	{
	public:
		/**
		* Creates the instance of NullaryAlgebraNodeBase.
		* @param element representing input node.
		*/
		NullaryAlgebraNodeBase(DOMElement * element);
		virtual void accept(AlgebraVisitor &v) = 0;
		std::shared_ptr<AlgebraNodeBase> replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild);
	};

	/**
	* Represents the algebraic operation for reading table.
	*/
	class Table : public NullaryAlgebraNodeBase
	{
	public:
		std::string name;  /**< Name of the table. */
		std::vector<ColumnInfo> columns; /**< List holding information about read columns. */
		double numberOfRows;  /**< Stores number of rows in the current table. */
		std::vector<Index> indices; /**< Stores list of indices on this table. */

		/**
		* Creates the instance of Table.
		* @param element representing input node.
		*/
		Table(DOMElement * element);
		void accept(AlgebraVisitor &v);
	};

	/**
	* Represents algebraic operation sort table, this node can be only on the top of algebra tree.
	*/
	class Sort : public UnaryAlgebraNodeBase
	{
	public:
		std::vector<SortParameter> parameters;  /**< Determines how should the sort operator sort the current relation. */

		/**
		* Creates the instance of Sort.
		* @param element representing input node.
		*/
		Sort(DOMElement * element);

		void accept(AlgebraVisitor &v);
	};

	/**
	* Represents the algebraic operation group. It groups by given columns and also computes agregate functions.
	*/
	class Group : public UnaryAlgebraNodeBase
	{
	public:
		std::vector<GroupColumn> groupColumns;  /**< Determines by which columns should current relation by grouped by.*/
		std::vector<AgregateFunction> agregateFunctions; /**< Stores aggregate function information associated with this group node. */

		/**
		* Creates the instance of Group.
		* @param element representing input node.
		*/
		Group(DOMElement * element);

		void accept(AlgebraVisitor &v);
	};
	/**
	* Represents the extended projection.
	* It eliminates some columns and computes new ones using simple operation like +,-,*,/ or other functions.
	*/
	class ColumnOperations : public UnaryAlgebraNodeBase
	{
	public:
		std::vector<ColumnOperation> operations;  /**< Stores list of to be performed operations on columns.*/

		/**
		* Creates the instance of ColumnOperations.
		* @param element representing input node.
		*/
		ColumnOperations(DOMElement * element);

		void accept(AlgebraVisitor &v);
	};

	/**
	*  Represents the selection operation.
	*  It filters input rows and copies only rows satisfying given condition to output.
	*/
	class Selection : public UnaryAlgebraNodeBase
	{
	public:
		std::shared_ptr<Expression> condition; /**< Condition for filtering given input.*/

		/**
		* Creates the instance of Selection.
		*/
		Selection();

		/**
		* Creates the instance of Selection.
		* @param element representing input node.
		*/
		Selection(DOMElement * element);

		/**
		* Creates the instance of Selection.
		* @param cond Expression - filter condition.
		*/
		Selection(std::shared_ptr<Expression> & cond);

		void accept(AlgebraVisitor &v);
	};

	/**
	* Represents binary join operation.
	*/
	class Join : public BinaryAlgebraNodeBase
	{
	public:

		std::shared_ptr<Expression> condition;  /**< Join condition.*/

		std::vector<JoinColumnInfo> outputJoinColumns; /**< List of output columns from join operator.*/

		/**
		* Creates the instance of Join.
		* @param element representing input node.
		*/
		Join(DOMElement * element);

		void accept(AlgebraVisitor &v);
	};

	/**
	* Represents algebraic operation antijoin.
	* Antijoin is generalized difference.
	*/
	class AntiJoin : public BinaryAlgebraNodeBase
	{
	public:

		std::shared_ptr<Expression> condition; /**< Condition for antijoin joining.*/

		std::vector<JoinColumnInfo> outputJoinColumns;  /**< List of join's output columns. */

		/**
		* Creates the instance of AntiJoin.
		* @param element representing input node.
		*/
		AntiJoin(DOMElement * element);

		void accept(AlgebraVisitor &v);
	};

	/**
	* Represents algebraic operation bag union.
	* Rows from second input will be appended to rows from first input.
	*/
	class Union : public BinaryAlgebraNodeBase
	{
	public:
		/**
		* Creates the instance of Union.
		* @param element representing input node.
		*/
		Union(DOMElement * element);
		void accept(AlgebraVisitor &v);
	};

	/**
	* Represents the algebraic operation join on multiple tables.
	*/
	class GroupedJoin : public GroupedAlgebraNode
	{
	public:
		std::shared_ptr<Expression> condition;  /**< Condition for joining.*/
		std::vector<JoinColumnInfo> outputJoinColumns; /**< List of output columns from join operator. */
		void accept(AlgebraVisitor &v);
	};
};
#endif


