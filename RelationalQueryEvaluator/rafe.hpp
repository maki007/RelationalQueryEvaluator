#ifndef rafe_hpp_
#define rafe_hpp_

#include <ostream>
#include <vector>
#include <string>

#include "PhysicalOperator.h"

namespace rafe {

	/**
	* Represents relational query.
	*/
	class rafe_query {
	public:

		/**
		* Creates new instance of rafe_query.
		*/
		rafe_query();

		/**
		* Creates new instance of ScanAndSortByIndex.
		* @param fname - name of XML file with query. 
		* @param err - ostream used for writing error messages.
		* @returns true if the function completed operation successfully, false otherwise.
		*/
		bool process(const std::string & fname, std::ostream &err);
		
		/**
		* Writes query to ostream in dot language.
		* @param out - ostream for result.
		* @returns true if the function completed operation successfully, false otherwise.
		*/
		bool dump_dot(std::ostream &out);
		
		/**
		* Writes query to ostream in Bobolang language.
		* @param out - ostream for result.
		* @returns true if the function completed operation successfully, false otherwise.
		*/
		bool dump_bobolang(std::ostream &out);
	private:
		std::vector<std::shared_ptr<PhysicalOperator> > clonedPlans_;
	};
};


#endif