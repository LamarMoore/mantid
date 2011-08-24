#ifndef BOX_FUNCTION_BUILDER_TEST_H_
#define BOX_FUNCTION_BUILDER_TEST_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include "MantidMDAlgorithms/BoxFunctionBuilder.h"

using namespace Mantid::MDAlgorithms;

class BoxBuilderTest : public CxxTest::TestSuite
{


public:

    void testCreateInvalidOriginThrows()
    {
      BoxFunctionBuilder builder;
      builder.addWidthParameter(WidthParameter(1));
      builder.addHeightParameter(HeightParameter(1));
      builder.addDepthParameter(DepthParameter(1));
      TSM_ASSERT_THROWS("Building without a valid origin is not possible.", builder.create(), std::invalid_argument);
    }

    void testCreateInvalidWidthThrows()
    {
      BoxFunctionBuilder builder;
      builder.addHeightParameter(HeightParameter(1));
      builder.addDepthParameter(DepthParameter(1));
      builder.addOriginParameter(OriginParameter(1, 1, 1));
      TSM_ASSERT_THROWS("Building without a valid width is not possible.", builder.create(), std::invalid_argument);
    }

    void testCreateInvalidDepthThrows()
    {
      BoxFunctionBuilder builder;
      builder.addWidthParameter(WidthParameter(1));
      builder.addHeightParameter(HeightParameter(1));
      builder.addOriginParameter(OriginParameter(1, 1, 1));
      TSM_ASSERT_THROWS("Building without a valid depth is not possible.", builder.create(), std::invalid_argument);
    }

    void testCreateInvalidHeightThrows()
    {
      BoxFunctionBuilder builder;
      builder.addWidthParameter(WidthParameter(1));
      builder.addDepthParameter(DepthParameter(1));
      builder.addOriginParameter(OriginParameter(1, 1, 1));
      TSM_ASSERT_THROWS("Building without a valid height is not possible.", builder.create(), std::invalid_argument);
    }

    void testCreate()
    {
      BoxFunctionBuilder builder;
      builder.addWidthParameter(WidthParameter(1));
      builder.addHeightParameter(HeightParameter(2));
      builder.addDepthParameter(DepthParameter(3));
      builder.addOriginParameter(OriginParameter(4, 5, 6));
      Mantid::Geometry::MDImplicitFunction_sptr impFunction(builder.create());

      BoxImplicitFunction* boxFunction = dynamic_cast<BoxImplicitFunction*>(impFunction.get());
      TSM_ASSERT("The function generated is not a boxFunction", boxFunction != NULL);
      TSM_ASSERT_EQUALS("Box function has not been generated by builder correctly", 4.5 ,boxFunction->getUpperX());
      TSM_ASSERT_EQUALS("Box function has not been generated by builder correctly", 6 ,boxFunction->getUpperY());
      TSM_ASSERT_EQUALS("Box function has not been generated by builder correctly", 7.5 ,boxFunction->getUpperZ());
    }

};



#endif
