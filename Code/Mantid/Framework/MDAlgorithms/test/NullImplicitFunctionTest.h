#ifndef NULL_IMPLICIT_FUNCTION_TEST_H_
#define NULL_IMPLICIT_FUNCTION_TEST_H_

#include <cxxtest/TestSuite.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "MantidMDAlgorithms/NullImplicitFunction.h"

class NullImplicitFunctionTest: public CxxTest::TestSuite
{

public:

void testGetName()
{
  using namespace Mantid::MDAlgorithms;
  NullImplicitFunction function;

  TSM_ASSERT_EQUALS("The static and dynamic names do not align", NullImplicitFunction::functionName(), function.getName());
}

void testEvaluateReturnsTrue()
{
  using namespace Mantid::MDAlgorithms;
  NullImplicitFunction function;
  Mantid::coord_t coord[3] = {0, 0, 0};
  TS_ASSERT(function.isPointContained(coord));
}

void testToXMLEmpty()
{
  using namespace Mantid::MDAlgorithms;
  NullImplicitFunction function;

  TSM_ASSERT_EQUALS("The xml string should be empty for any instance of this type", std::string() , function.toXMLString());
}

};
#endif
