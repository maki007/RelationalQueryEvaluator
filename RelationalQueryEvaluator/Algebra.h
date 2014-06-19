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

class AlgebraVisitor;


/**
* Abstract class for algebraic operation.
*/
class AlgebraNodeBase
{
public:
	/**
	* Stores ouput columns of this node. Key is unique column identifier and value stores information about column.
	*/
	std::map<int, ColumnInfo> outputColumns;  
	
	ulong lineNumber = 0; /**< Stores the line number of input element of this node. */
	
	AlgebraNodeBase * parent; /**< Stores the parent in algebra tree. */
	
	/**
	* Create the instance of AlgebraNodeBase.
	*/
	AlgebraNodeBase::AlgebraNodeBase();
	
	/**
	* Create the instance of AlgebraNodeBase.
	* @param element representing input node.
	*/
	AlgebraNodeBase(DOMElement * element);

	/**
	* Helper method for creating algebra tree from dom.
	* @param node representing input node.
	* @return newely created Algebra node
	*/
	AlgebraNodeBase * constructChildren(DOMElement * node);

	/**
	* Method for calling visit[node] on given AlgebraVisitor
	* @param v AlgebraVisitor, on which to call function 
	*/
	virtual void accept(AlgebraVisitor &v) = 0;

	/**
	* Replaces one child of this node with other one
	* @param oldChild node to be replaced
	* @param newChild new node to replace old one
	* @return replaced child
	*/
	virtual std::shared_ptr<AlgebraNodeBase> replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild) = 0;
};

/**
* Abstract class for unary algebraic operation.
*/
class UnaryAlgebraNodeBase : public AlgebraNodeBase
{
public:
	std::shared_ptr <AlgebraNodeBase> child;  /**< stores pointer to child tree node */  
	
	/**
	* Create the instance of UnaryAlgebraNodeBase.
	*/
	UnaryAlgebraNodeBase();

	/**
	* Create the instance of UnaryAlgebraNodeBase.
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
	std::shared_ptr <AlgebraNodeBase> leftChild;  /**< Stores pointer to left child node. */  
	std::shared_ptr <AlgebraNodeBase> rightChild;  /**< Stores pointer to right child node. */ 

	/**
	* Create the instance of BinaryAlgebraNodeBase.
	*/
	BinaryAlgebraNodeBase();

	/**
	* Create the instance of UnaryAlgebraNodeBase.
	* @param element representing input node.
	*/
	BinaryAlgebraNodeBase(DOMElement * element);

	/**
	* Helper method for join and antijoin. Reeds all needed parameters from dom tree.
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
	std::vector<std::shared_ptr<AlgebraNodeBase>> children; /**< Stores list of this node children. */  
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
	* Create the instance of NullaryAlgebraNodeBase.
	* @param element representing input node.
	*/
	NullaryAlgebraNodeBase(DOMElement * element);
	virtual void accept(AlgebraVisitor &v) = 0;
	std::shared_ptr<AlgebraNodeBase> replaceChild(AlgebraNodeBase * oldChild, std::shared_ptr<AlgebraNodeBase> & newChild);
};

/**
* Represents algebraic operation reading table.
*/
class Table : public NullaryAlgebraNodeBase
{
public:
	std::string name;  /**< Name of the table. */  
	std::vector<ColumnInfo> columns; /**< List holding column information. */
	double numberOfRows;  /**< Stores number of rows in current table. */
	std::vector<Index> indices; /**< Stores list of indices  on this table. */
	
	/**
	* Create the instance of Table.
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
	std::vector<SortParameter> parameters;  /**< Determines by which columns and what direction should Sort sort current relation. */  

	/**
	* Create the instance of Sort.
	* @param element representing input node.
	*/
	Sort(DOMElement * element);
	
	void accept(AlgebraVisitor &v);
};

/**
* Represents algebraic operation group. It groups by given columns and also computes agregate functions.
*/
class Group : public UnaryAlgebraNodeBase
{
public:
	std::vector<GroupColumn> groupColumns;  /**< Determines by which columns should current relation by grouped by.*/  
	std::vector<AgregateFunction> agregateFunctions; /**< Stores agregate function information associated with this group node. */

	/**
	* Create the instance of Group.
	* @param element representing input node.
	*/
	Group(DOMElement * element);

	void accept(AlgebraVisitor &v);
};
/**
* Represents algebraic operation extended projection.
* It eliminates some columns and computes new ones using simple operation like +,-,*,/ or other functions.
*/
class ColumnOperations : public UnaryAlgebraNodeBase
{
public:
	std::vector<ColumnOperation> operations;  /**< Stores list of to be performed operations on columns.*/ 

	/**
	* Create the instance of ColumnOperations.
	* @param element representing input node.
	*/
	ColumnOperations(DOMElement * element);

	void accept(AlgebraVisitor &v);
};

/**
*  Represents algebraic operation selection.
*  It filters input rows and copies to output only rows satisfying given condition.
*/
class Selection : public UnaryAlgebraNodeBase
{
public:
	std::shared_ptr<Expression> condition; /**< Condition for filtering given input.*/ 
	
	/**
	* Create the instance of Selection.
	*/
	Selection();
	
	/**
	* Create the instance of Selection.
	* @param element representing input node.
	*/
	Selection(DOMElement * element);
	
	/**
	* Create the instance of Selection.
	* @param cond Expression - filter condition
	*/
	Selection(std::shared_ptr<Expression> & cond);

	void accept(AlgebraVisitor &v);
};

/**
* Represents algebraic operation join on two tables
*/
class Join : public BinaryAlgebraNodeBase
{
public:
	
	std::shared_ptr<Expression> condition;  /**< Condition for joining.*/ 
	
	std::vector<JoinColumnInfo> outputJoinColumns; /**< List of output columns from join*/

	/**
	* Create the instance of Join.
	* @param element representing input node.
	*/
	Join(DOMElement * element);

	void accept(AlgebraVisitor &v);
};

/**
* Represents algebraic operation antijoin.
* Antijoin is basicly generalized difference but the columns doesn't have to be same.
*/
class AntiJoin : public BinaryAlgebraNodeBase
{
public:
	
	std::shared_ptr<Expression> condition; /**< Condition for antijoin joining.*/ 

	std::vector<JoinColumnInfo> outputJoinColumns;  /**< list of output columns from join*/
	
	/**
	* Create the instance of AntiJoin.
	* @param element representing input node.
	*/
	AntiJoin(DOMElement * element);
	
	void accept(AlgebraVisitor &v);
};

/**
* Represents algebraic operation set union.
* Rows from second input will be appended to rows from first input.
*/
class Union : public BinaryAlgebraNodeBase
{
public:
	/**
	* Create the instance of Union.
	* @param element representing input node.
	*/
	Union(DOMElement * element);
	void accept(AlgebraVisitor &v);
};

/**
* Represents algebraic operation join on multiple tables
*/
class GroupedJoin : public GroupedAlgebraNode
{
public:
	std::shared_ptr<Expression> condition;  /**< Condition for joining.*/ 
	std::vector<JoinColumnInfo> outputJoinColumns; /**< List of output columns from join*/
	void accept(AlgebraVisitor &v);
};

#endif


