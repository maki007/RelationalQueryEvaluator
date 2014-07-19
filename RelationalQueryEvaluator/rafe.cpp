#include "rafe.hpp"

#include "Algebra.h"
#include "AlgebraVisitors.h"
#include "XmlHandler.h"
#include "PhysicalOperatorVisitor.h"

namespace rafe {

	rafe_query::rafe_query()
	{
	}

	bool rafe_query::process(const std::string & fname, std::ostream &err)
	{
		clonedPlans_.clear();

		std::shared_ptr<AlgebraNodeBase> algebraRoot = XmlHandler::GenerateRelationalAlgebra(fname.c_str());
		if (algebraRoot == 0)
		{
			err << "parse error in " << fname << std::endl;
			return false;
		}

		//drawAlgebra(algebraRoot,line+string("._1.txt"));

		std::unique_ptr<SemanticChecker> semanticChecker(new SemanticChecker());
		algebraRoot->accept(*semanticChecker);
		if (semanticChecker->containsErrors == true)
		{
			err << "semantic error in " << fname << std::endl;
			return false;
		}

		std::unique_ptr<GroupingVisitor> groupVisitor(new GroupingVisitor());
		algebraRoot->accept(*groupVisitor);

		//drawAlgebra(algebraRoot,line+string("._2.txt"));

		std::shared_ptr<AlgebraCompiler> algebraCompiler(new AlgebraCompiler());
		algebraRoot->accept(*algebraCompiler);

		//drawPlan(algebraCompiler->result, line + string("._3.txt"));

		for (auto it = algebraCompiler->result.begin(); it != algebraCompiler->result.end(); ++it)
		{

			CloningPhysicalOperatorVisitor cloner;
			(*it)->plan->accept(cloner);
			clonedPlans_.push_back(std::shared_ptr<PhysicalOperator>(cloner.result));
		}

		for (auto it = clonedPlans_.begin(); it != clonedPlans_.end(); ++it)
		{
			SortResolvingPhysicalOperatorVisitor sortResolver;
			(*it)->accept(sortResolver);
		}

		return true;
	}

	bool rafe_query::dump_dot(std::ostream &out)
	{
		std::unique_ptr<PhysicalOperatorDrawingVisitorWithouSorts> planDrawer(new PhysicalOperatorDrawingVisitorWithouSorts());
		for (auto it = clonedPlans_.begin(); it != clonedPlans_.end(); ++it)
		{
			(*it)->accept(*planDrawer);
			planDrawer->nodeCounter++;
		}
		planDrawer->result += "\n}";

		out << planDrawer->result;

		return true;
	}

	bool rafe_query::dump_bobolang(std::ostream &out)
	{
		std::unique_ptr<BoboxPlanWritingPhysicalOperatorVisitor> planDrawer(new BoboxPlanWritingPhysicalOperatorVisitor());
		if (clonedPlans_.size() > 0)
		{
			out << planDrawer->writePlan(*clonedPlans_.begin());
			return true;
		}
		else
		{
			return false;
		}
	}
};
