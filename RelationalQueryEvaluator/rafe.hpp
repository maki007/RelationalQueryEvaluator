#ifndef rafe_hpp_
#define rafe_hpp_

#include <ostream>
#include <vector>
#include <string>

#include "PhysicalOperator.h"

namespace rafe {

	class rafe_query {
	public:
		rafe_query();

		bool process(const std::string & fname, std::ostream &err);
		bool dump_dot(std::ostream &out);
		bool dump_bobolang(std::ostream &out);
	private:
		std::vector<std::shared_ptr<PhysicalOperator> > clonedPlans_;
	};
};


#endif