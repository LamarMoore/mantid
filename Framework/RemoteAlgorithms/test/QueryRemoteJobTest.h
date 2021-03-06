// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include <cxxtest/TestSuite.h>

#include "MantidAPI/AlgorithmManager.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/FacilityInfo.h"
#include "MantidRemoteAlgorithms/QueryRemoteJob.h"

using namespace Mantid::RemoteAlgorithms;

class QueryRemoteJobTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static QueryRemoteJobTest *createSuite() { return new QueryRemoteJobTest(); }
  static void destroySuite(QueryRemoteJobTest *suite) { delete suite; }

  void test_algorithm() {
    testAlg =
        Mantid::API::AlgorithmManager::Instance().create("QueryRemoteJob", 1);
    TS_ASSERT(testAlg);
    TS_ASSERT_EQUALS(testAlg->name(), "QueryRemoteJob");
    TS_ASSERT_EQUALS(testAlg->version(), 1);
  }

  void test_castAlgorithm() {
    // can create
    std::shared_ptr<QueryRemoteJob> a;
    TS_ASSERT(a = std::make_shared<QueryRemoteJob>());

    // can cast to inherited interfaces and base classes
    TS_ASSERT(
        dynamic_cast<Mantid::RemoteAlgorithms::QueryRemoteJob *>(a.get()));
    TS_ASSERT(dynamic_cast<Mantid::API::Algorithm *>(a.get()));
    TS_ASSERT(dynamic_cast<Mantid::Kernel::PropertyManagerOwner *>(a.get()));
    TS_ASSERT(dynamic_cast<Mantid::API::IAlgorithm *>(a.get()));
    TS_ASSERT(dynamic_cast<Mantid::Kernel::IPropertyManager *>(a.get()));
  }

  void test_init() {
    if (!testAlg->isInitialized())
      TS_ASSERT_THROWS_NOTHING(testAlg->initialize());

    TS_ASSERT(testAlg->isInitialized());

    QueryRemoteJob qr;
    TS_ASSERT_THROWS_NOTHING(qr.initialize());
  }

  // TODO: when we have a RemoteJobManager capable of creating
  // algorithms for different types of compute resources (example:
  // Fermi@SNS), create different algorithms for them
  void test_propertiesMissing() {
    QueryRemoteJob alg1;
    TS_ASSERT_THROWS_NOTHING(alg1.initialize());
    // id missing
    TS_ASSERT_THROWS(alg1.setPropertyValue("ComputeResource", "missing!"),
                     const std::invalid_argument &);

    TS_ASSERT_THROWS(alg1.execute(), const std::runtime_error &);
    TS_ASSERT(!alg1.isExecuted());

    QueryRemoteJob alg2;
    TS_ASSERT_THROWS_NOTHING(alg2.initialize());
    // compute resource missing
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("JobID", "missing001"));

    TS_ASSERT_THROWS(alg2.execute(), const std::runtime_error &);
    TS_ASSERT(!alg2.isExecuted());
  }

  void test_wrongProperty() {
    QueryRemoteJob qr;
    TS_ASSERT_THROWS_NOTHING(qr.initialize();)
    TS_ASSERT_THROWS(qr.setPropertyValue("job", "whatever"),
                     const std::runtime_error &);
    TS_ASSERT_THROWS(qr.setPropertyValue("id", "whichever"),
                     const std::runtime_error &);
    TS_ASSERT_THROWS(qr.setPropertyValue("ComputeRes", "anything"),
                     const std::runtime_error &);
  }

  void test_propertiesOK() {
    testFacilities.emplace_back("SNS", "Fermi");

    const Mantid::Kernel::FacilityInfo &prevFac =
        Mantid::Kernel::ConfigService::Instance().getFacility();
    for (auto &testFacility : testFacilities) {
      const std::string facName = testFacility.first;
      const std::string compName = testFacility.second;

      Mantid::Kernel::ConfigService::Instance().setFacility(facName);
      QueryRemoteJob qr;
      TS_ASSERT_THROWS_NOTHING(qr.initialize());
      TS_ASSERT_THROWS_NOTHING(
          qr.setPropertyValue("ComputeResource", compName));
      TS_ASSERT_THROWS_NOTHING(qr.setPropertyValue("JobID", "000001"));
      // TODO: this would run the algorithm and do a remote
      // connection. uncomment only when/if we have a mock up for this
      // TS_ASSERT_THROWS(qr.execute(), std::exception);
      TS_ASSERT(!qr.isExecuted());
    }
    Mantid::Kernel::ConfigService::Instance().setFacility(prevFac.name());
  }

  // TODO: void test_runOK() - with a mock when we can add it.
  // ideally, with different compute resources to check the remote job
  // manager factory, etc.

private:
  Mantid::API::IAlgorithm_sptr testAlg;
  std::vector<std::pair<std::string, std::string>> testFacilities;
};
