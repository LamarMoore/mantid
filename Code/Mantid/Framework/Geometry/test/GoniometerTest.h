#ifndef MANTID_TESTGONIOMETER__
#define MANTID_TESTGONIOMETER__

#include <cxxtest/TestSuite.h>
#include "MantidGeometry/Instrument/Goniometer.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include "MantidGeometry/Quat.h"
using namespace Mantid::Geometry;

class GoniometerTest : public CxxTest::TestSuite
{
public:
  void test_AxisConstructor()
  {
    GoniometerAxis a("axis1",V3D(1.,0,0),3.,CW,angRadians);
    TS_ASSERT_EQUALS(a.name,"axis1");
    TS_ASSERT_EQUALS(a.rotationaxis[0],1.);
    TS_ASSERT_EQUALS(a.angle,3.);
    TS_ASSERT_EQUALS(a.sense,-1);
    TS_ASSERT_DIFFERS(a.sense,angDegrees);
  }

  void test_Goniometer()
  {
    Goniometer G;
    MantidMat M(3,3);
    // Check simple constructor    
    M.identityMatrix();
    TS_ASSERT_EQUALS(G.getR(),M);
    TS_ASSERT_THROWS(G.setRotationAngle("Axis4",3),std::invalid_argument);
    TS_ASSERT_THROWS_ANYTHING(G.setRotationAngle(1,2));
    TS_ASSERT_EQUALS((G.axesInfo()).compare("No axis is found\n"),0); 
    TS_ASSERT_THROWS_NOTHING(G.pushAxis("Axis1", 1., 0., 0.,30));
    TS_ASSERT_THROWS_NOTHING(G.pushAxis("Axis2", 0., 0., 1.,30));
    TS_ASSERT_THROWS(G.pushAxis("Axis2", 0., 0., 1.,30),std::invalid_argument);
    TS_ASSERT_THROWS_NOTHING(G.setRotationAngle("Axis2", 25));
    TS_ASSERT_THROWS_NOTHING(G.setRotationAngle(0, -17));
    TS_ASSERT_EQUALS(G.getAxis(1).angle,25.);
    TS_ASSERT_EQUALS(G.getAxis("Axis1").angle,-17);
    TS_ASSERT_DELTA(G.axesInfo().find("-17"),52,20);
    M=G.getR();
    //test some matrix elements
    TS_ASSERT_DELTA(M[0][0],9.063078e-01,1e-6);
    TS_ASSERT_DELTA(M[0][1],-4.226183e-01,1e-6);
    TS_ASSERT_DELTA(M[0][2],0,1e-6);
    TS_ASSERT_DELTA(M[1][1],8.667064e-01,1e-6);
    TS_ASSERT_DELTA(M[1][2],2.923717e-01,1e-6);
    //goniometer from a rotation matrix or copied
    Goniometer G1(M),G2(G);
    TS_ASSERT_EQUALS(M,G1.getR());
    TS_ASSERT_EQUALS(G1.axesInfo(),std::string("Goniometer was initialized from a rotation matrix. No information about axis is available.\n"));
    TS_ASSERT_THROWS_ANYTHING(G.pushAxis("Axis2", 0., 0., 1.,30));
    TS_ASSERT_EQUALS(M,G2.getR());
  }

  void test_makeUniversalGoniometer()
  {
    Goniometer G;
    G.makeUniversalGoniometer();
    TS_ASSERT_EQUALS(G.getNumberAxes(), 3);
    TS_ASSERT_EQUALS(G.getAxis(2).name, "phi");
    TS_ASSERT_EQUALS(G.getAxis(1).name, "chi");
    TS_ASSERT_EQUALS(G.getAxis(0).name, "omega");
  }

  void test_copy()
  {
    Goniometer G, G1;
    G1.makeUniversalGoniometer();
    G = G1;
    TS_ASSERT_EQUALS(G.getNumberAxes(), 3);
    TS_ASSERT_EQUALS(G.getAxis(2).name, "phi");
    TS_ASSERT_EQUALS(G.getAxis(1).name, "chi");
    TS_ASSERT_EQUALS(G.getAxis(0).name, "omega");
  }


  /** Test to make sure the goniometer rotation works as advertised
   * for a simple universal goniometer.
   */
  void test_UniversalGoniometer_getR()
  {
    Goniometer G;
    V3D init, rot;

    G.makeUniversalGoniometer();
    TS_ASSERT_EQUALS(G.getNumberAxes(), 3);

    init = V3D(0,0,1.0);
    G.setRotationAngle("phi", 45.0);
    G.setRotationAngle("chi",  0.0);
    G.setRotationAngle("omega",  0.0);
    
    rot = G.getR() * init;

    TS_ASSERT_DELTA( rot.X(), 0.707, 0.001);
    TS_ASSERT_DELTA( rot.Y(), 0.000, 0.001);
    TS_ASSERT_DELTA( rot.Z(), 0.707, 0.001);

    init = V3D(0,0,1.0);
    G.setRotationAngle("phi", 45.0);
    G.setRotationAngle("chi", 90.0);
    G.setRotationAngle("omega",  0.0);
    rot = G.getR() * init;

    TS_ASSERT_DELTA( rot.X(),  0.000, 0.001);
    TS_ASSERT_DELTA( rot.Y(),  0.707, 0.001);
    TS_ASSERT_DELTA( rot.Z(),  0.707, 0.001);

    init = V3D(-1, 0, 0);
    G.setRotationAngle("phi", 90.0);
    G.setRotationAngle("chi", 90.0);
    G.setRotationAngle("omega",  0.0);
    rot = G.getR() * init;
    TS_ASSERT_DELTA( rot.X(),  0.000, 0.001);
    TS_ASSERT_DELTA( rot.Y(),  0.000, 0.001);
    TS_ASSERT_DELTA( rot.Z(),  1.000, 0.001);

  }

  void test_getEulerAngles()
  {
    Goniometer G;
    MantidMat rotA;
    G.makeUniversalGoniometer();
    G.setRotationAngle("phi", 45.0);
    G.setRotationAngle("chi", 23.0);
    G.setRotationAngle("omega", 7.0);
    rotA = G.getR();

    std::vector<double> angles = G.getEulerAngles("yzy"); 

    G.setRotationAngle("phi", angles[2]);
    G.setRotationAngle("chi", angles[1]);
    G.setRotationAngle("omega", angles[0]);

    // Those goniometer angles re-create the initial rotation matrix.
    TS_ASSERT( rotA.equals(G.getR(), 0.0001) );
  }

  void test_getEulerAngles2()
  {
    Goniometer G;
    MantidMat rotA;
    std::vector<double> angles; 
    G.makeUniversalGoniometer();
    for (double phi=-172.;phi<=180.;phi+=30.)
      for (double chi=-171.;chi<=180.;chi+=30.)
        for (double omega=-175.3;omega<=180.;omega+=30.)
          {
            G.setRotationAngle("phi", phi);
            G.setRotationAngle("chi", chi);
            G.setRotationAngle("omega", omega);
            rotA = G.getR();
            angles = G.getEulerAngles("yzy"); 
            G.setRotationAngle("phi", angles[2]);
            G.setRotationAngle("chi", angles[1]);
            G.setRotationAngle("omega", angles[0]);
            TS_ASSERT( rotA.equals(G.getR(), 0.0001) );
          }
  }
};

#endif
