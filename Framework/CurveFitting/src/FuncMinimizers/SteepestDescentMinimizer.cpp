//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidCurveFitting/FuncMinimizers/SteepestDescentMinimizer.h"

#include "MantidAPI/CostFunctionFactory.h"
#include "MantidAPI/FuncMinimizerFactory.h"

#include "MantidKernel/Logger.h"

namespace Mantid {
namespace CurveFitting {
namespace FuncMinimisers {

namespace {
// Get a reference to the logger
Kernel::Logger g_log("SteepestDescentMinimizer");
} // namespace

DECLARE_FUNCMINIMIZER(SteepestDescentMinimizer, SteepestDescent)

/// Return a concrete type to initialize m_gslSolver
/// gsl_multimin_fdfminimizer_vector_bfgs2
const gsl_multimin_fdfminimizer_type *
SteepestDescentMinimizer::getGSLMinimizerType() {
  return gsl_multimin_fdfminimizer_steepest_descent;
}

} // namespace FuncMinimisers
} // namespace CurveFitting
} // namespace Mantid
