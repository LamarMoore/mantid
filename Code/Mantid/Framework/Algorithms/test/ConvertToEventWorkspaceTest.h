#ifndef MANTID_ALGORITHMS_CONVERTTOEVENTWORKSPACETEST_H_
#define MANTID_ALGORITHMS_CONVERTTOEVENTWORKSPACETEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidKernel/Timer.h"
#include "MantidKernel/System.h"
#include <iostream>
#include <iomanip>

#include "MantidAlgorithms/ConvertToEventWorkspace.h"
#include "MantidTestHelpers/WorkspaceCreationHelper.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidAlgorithms/CheckWorkspacesMatch.h"

using namespace Mantid;
using namespace Mantid::Algorithms;
using namespace Mantid::API;
using namespace Mantid::DataObjects;

class ConvertToEventWorkspaceTest : public CxxTest::TestSuite
{
public:

    
  void test_Init()
  {
    ConvertToEventWorkspace alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
  }
  
  void test_exec()
  {
    // Create the input
    Workspace2D_sptr inWS = WorkspaceCreationHelper::create2DWorkspaceWithFullInstrument(50, 10, true);

    inWS->dataY(0)[0] = 1.0;
    inWS->dataY(0)[1] = 3.0;
    inWS->dataE(0)[1] = 4.0;
    inWS->dataY(0)[2] = 0.0;
    inWS->dataE(0)[2] = 0.0;

    // Name of the output workspace.
    std::string outWSName("ConvertToEventWorkspaceTest_OutputWS");
  
    ConvertToEventWorkspace alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("InputWorkspace", inWS) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("OutputWorkspace", outWSName) );
    TS_ASSERT_THROWS_NOTHING( alg.execute(); );
    TS_ASSERT( alg.isExecuted() );
    
    // Retrieve the workspace from data service.
    EventWorkspace_sptr outWS;
    TS_ASSERT_THROWS_NOTHING( outWS = boost::dynamic_pointer_cast<EventWorkspace>(AnalysisDataService::Instance().retrieve(outWSName)) );
    TS_ASSERT(outWS);
    if (!outWS) return;
    
    // This performs a full comparison (histogram
    CheckWorkspacesMatch matcher;
    matcher.initialize();
    matcher.setProperty("Workspace1", boost::dynamic_pointer_cast<MatrixWorkspace>(inWS));
    matcher.setProperty("Workspace2", boost::dynamic_pointer_cast<MatrixWorkspace>(outWS));
    matcher.setProperty("CheckType", false);
    matcher.setProperty("Tolerance", 1e-6);
    matcher.execute();
    TS_ASSERT( matcher.isExecuted() );
    TS_ASSERT_EQUALS( matcher.getPropertyValue("Result"), "Success!" );

    // Event-specific checks
    TS_ASSERT_EQUALS( outWS->getNumberEvents(), 499 );
    TS_ASSERT_EQUALS( outWS->getEventList(1).getNumberEvents(), 10);

    // Check a couple of events
    EventList & el = outWS->getEventList(0);
    TS_ASSERT_EQUALS( el.getWeightedEventsNoTime().size(), 9);
    WeightedEventNoTime ev;
    ev = el.getWeightedEventsNoTime()[0];
    TS_ASSERT_DELTA( ev.tof(), 0.5, 1e-6);
    TS_ASSERT_DELTA( ev.weight(), 1.0, 1e-6);
    TS_ASSERT_DELTA( ev.errorSquared(), 2.0, 1e-6);
    ev = el.getWeightedEventsNoTime()[1];
    TS_ASSERT_DELTA( ev.tof(), 1.5, 1e-6);
    TS_ASSERT_DELTA( ev.weight(), 3.0, 1e-6);
    TS_ASSERT_DELTA( ev.errorSquared(), 16.0, 1e-6);
    // Skipped an event because the bin was 0.0 weight
    ev = el.getWeightedEventsNoTime()[2];
    TS_ASSERT_DELTA( ev.tof(), 3.5, 1e-6);
    TS_ASSERT_DELTA( ev.weight(), 2.0, 1e-6);
    TS_ASSERT_DELTA( ev.errorSquared(), 2.0, 1e-6);

    // Remove workspace from the data service.
    AnalysisDataService::Instance().remove(outWSName);
  }
  

};


#endif /* MANTID_ALGORITHMS_CONVERTTOEVENTWORKSPACETEST_H_ */

