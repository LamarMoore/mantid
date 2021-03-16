# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
#pylint: disable=no-init

"""
System test for MDNorm
"""

from mantid.simpleapi import *
import systemtesting


class MDNormHYSPECTest(systemtesting.MantidSystemTest):

    tolerance=1e-8

    def requiredMemoryMB(self):
        return 5000

    def requiredFiles(self):
        return ['HYS_13656_event.nxs','HYS_13657_event.nxs','HYS_13658_event.nxs']

    def runTest(self):
        runTestFirst()

        runTestFinal()

    def runTestFirst(self):
        config.setFacility('SNS')
        Load(Filename='HYS_13656-13658',OutputWorkspace='sum')
        SetGoniometer(Workspace='sum', Axis0='s1,0,1,0,1')
        GenerateEventsFilter(InputWorkspace='sum',
                             OutputWorkspace='splboth',
                             InformationWorkspace='info',
                             UnitOfTime='Nanoseconds',
                             LogName='s1',
                             MaximumLogValue=90,
                             LogValueInterval=1)
        FilterEvents(InputWorkspace='sum',
                     SplitterWorkspace='splboth',
                     InformationWorkspace='info',
                     FilterByPulseTime=True,
                     GroupWorkspaces=True,
                     OutputWorkspaceIndexedFrom1=True,
                     OutputWorkspaceBaseName='split')
        DeleteWorkspace('sum')
        DeleteWorkspace('splboth')
        DeleteWorkspace('info')
        DgsReduction(SampleInputWorkspace='split',
                     SampleInputMonitorWorkspace='split_1',
                     IncidentEnergyGuess=50,
                     SofPhiEIsDistribution=False,
                     TimeIndepBackgroundSub=True,
                     TibTofRangeStart=10400,
                     TibTofRangeEnd=12400,
                     OutputWorkspace='reduced')
        SetUB(Workspace='reduced',
              a=5.823,
              b=6.475,
              c=3.186,
              u='0,1,0',
              v='0,0,1')
        CropWorkspaceForMDNorm(InputWorkspace='reduced',
                               XMin=-25,
                               XMax=49,
                               OutputWorkspace='reduced')
        ConvertToMD(InputWorkspace='reduced',
                    QDimensions='Q3D',
                    Q3DFrames='Q_sample',
                    OutputWorkspace='md',
                    MinValues='-11,-11,-11,-25',
                    MaxValues='11,11,11,49')
        DeleteWorkspace("split")
        DeleteWorkspace("reduced")
        DeleteWorkspace("PreprocessedDetectorsWS")
        DeleteWorkspace("TOFCorrectWS")
        MergeMD(InputWorkspaces='md', OutputWorkspace='merged')
        DeleteWorkspace("md")
        MDNorm(InputWorkspace='merged',
               Dimension0Name='QDimension1',
               Dimension0Binning='-5,0.05,5',
               Dimension1Name='QDimension2',
               Dimension1Binning='-5,0.05,5',
               Dimension2Name='DeltaE',
               Dimension2Binning='-2,2',
               Dimension3Name='QDimension0',
               Dimension3Binning='-0.5,0.5',
               SymmetryOperations='x,y,z;x,-y,z;x,y,-z;x,-y,-z',
               OutputWorkspace='result',
               OutputDataWorkspace='dataMD',
               OutputNormalizationWorkspace='normMD')
        DeleteWorkspace("dataMD")
        DeleteWorkspace("normMD")
        DeleteWorkspace("merged")


    def runTestBackgroundClassic(self):
        config.setFacility('SNS')
        Load(Filename='HYS_13656-13658',OutputWorkspace='sum')
        SetGoniometer(Workspace='sum', Axis0='s1,0,1,0,1')
        GenerateEventsFilter(InputWorkspace='sum',
                             OutputWorkspace='splboth',
                             InformationWorkspace='info',
                             UnitOfTime='Nanoseconds',
                             LogName='s1',
                             MaximumLogValue=90,
                             LogValueInterval=1)
        FilterEvents(InputWorkspace='sum',
                     SplitterWorkspace='splboth',
                     InformationWorkspace='info',
                     FilterByPulseTime=True,
                     GroupWorkspaces=True,
                     OutputWorkspaceIndexedFrom1=True,
                     OutputWorkspaceBaseName='split')
        DeleteWorkspace('sum')
        DeleteWorkspace('splboth')
        DeleteWorkspace('info')
        DgsReduction(SampleInputWorkspace='split',
                     SampleInputMonitorWorkspace='split_1',
                     IncidentEnergyGuess=50,
                     SofPhiEIsDistribution=False,
                     TimeIndepBackgroundSub=True,
                     TibTofRangeStart=10400,
                     TibTofRangeEnd=12400,
                     OutputWorkspace='reduced')
        SetUB(Workspace='reduced',
              a=5.823,
              b=6.475,
              c=3.186,
              u='0,1,0',
              v='0,0,1')
        CropWorkspaceForMDNorm(InputWorkspace='reduced',
                               XMin=-25,
                               XMax=49,
                               OutputWorkspace='reduced')
        ConvertToMD(InputWorkspace='reduced',
                    QDimensions='Q3D',
                    Q3DFrames='Q_sample',
                    OutputWorkspace='md',
                    MinValues='-11,-11,-11,-25',
                    MaxValues='11,11,11,49')
        DeleteWorkspace("split")
        DeleteWorkspace("reduced")
        DeleteWorkspace("PreprocessedDetectorsWS")
        DeleteWorkspace("TOFCorrectWS")
        MergeMD(InputWorkspaces='md', OutputWorkspace='merged')
        DeleteWorkspace("md")
        MDNorm(InputWorkspace='merged',
               Dimension0Name='QDimension1',
               Dimension0Binning='-5,0.05,5',
               Dimension1Name='QDimension2',
               Dimension1Binning='-5,0.05,5',
               Dimension2Name='DeltaE',
               Dimension2Binning='-2,2',
               Dimension3Name='QDimension0',
               Dimension3Binning='-0.5,0.5',
               SymmetryOperations='x,y,z;x,-y,z;x,y,-z;x,-y,-z',
               OutputWorkspace='result',
               OutputDataWorkspace='dataMD',
               OutputNormalizationWorkspace='normMD')

        # old way - use reduced_1 as the background
       dgs_data=CloneWorkspace('reduced_1')
       data_MDE=mtd['merged']
       
       if mtd.doesExist('background_MDE'):
           DeleteWorkspace('background_MDE')
       
       for i in range(data_MDE.getNumExperimentInfo()):
           phi, chi, omega = data_MDE.getExperimentInfo(i).run().getGoniometer().getEulerAngles('YZY')
           AddSampleLogMultiple(Workspace=dgs_data,
                                LogNames='phi, chi, omega',
                                LogValues='{},{},{}'.format(phi,chi,omega))
           SetGoniometer(Workspace=dgs_data, Goniometers='Universal')
           ConvertToMD(InputWorkspace=dgs_data,
                       QDimensions='Q3D',
                       dEAnalysisMode='Direct',
                       Q3DFrames="Q_sample",
                       MinValues='-11,-11,-11,-25',
                       MaxValues='11,11,11,49',
                       PreprocDetectorsWS='-',
                       OverwriteExisting=False,
                       OutputWorkspace='background_MDE')
       
       MDNorm(InputWorkspace='background_MDE',
              Dimension0Name='QDimension1',
              Dimension0Binning='-5,0.05,5',
              Dimension1Name='QDimension2',
              Dimension1Binning='-5,0.05,5',
              Dimension2Name='DeltaE',
              Dimension2Binning='-2,2',
              Dimension3Name='QDimension0',
              Dimension3Binning='-0.5,0.5',
              SymmetryOperations='x,y,z;x,-y,z;x,y,-z;x,-y,-z',
              OutputWorkspace='backgroundMDH',
              OutputDataWorkspace='background_dataMD',
              OutputNormalizationWorkspace='background_normMD')

    # Generate expected files
    clean_data=MinusMD('result','backgroundMDH')


    def runTestFinal(self):
        Load(Filename='HYS_13656',OutputWorkspace='sum')
        SetGoniometer(Workspace='sum', Axis0='s1,0,1,0,1')
        GenerateEventsFilter(InputWorkspace='sum',
                             OutputWorkspace='splboth',
                             InformationWorkspace='info',
                             UnitOfTime='Nanoseconds',
                             LogName='s1',
                             MaximumLogValue=90,
                             LogValueInterval=1)
        FilterEvents(InputWorkspace='sum',
                     SplitterWorkspace='splboth',
                     InformationWorkspace='info',
                     FilterByPulseTime=True,
                     GroupWorkspaces=True,
                     OutputWorkspaceIndexedFrom1=True,
                     OutputWorkspaceBaseName='split')
        DgsReduction(SampleInputWorkspace='split',
                     SampleInputMonitorWorkspace='split_1',
                     IncidentEnergyGuess=50,
                     SofPhiEIsDistribution=False,
                     TimeIndepBackgroundSub=True,
                     TibTofRangeStart=10400,
                     TibTofRangeEnd=12400,
                     OutputWorkspace='reduced')
        SetUB(Workspace='reduced',
              a=5.823,
              b=6.475,
              c=3.186,
              u='0,1,0',
              v='0,0,1')
        CropWorkspaceForMDNorm(InputWorkspace='reduced',
                               XMin=-25,
                               XMax=49,
                               OutputWorkspace='reduced')
        ConvertToMD(InputWorkspace='reduced',
                    QDimensions='Q3D',
                    Q3DFrames='Q_sample',
                    OutputWorkspace='md',
                    MinValues='-11,-11,-11,-25',
                    MaxValues='11,11,11,49')
        MergeMD(InputWorkspaces='md', OutputWorkspace='merged')

        # Prepare background
        # use reduced_1 as the background and convert to MD
        dgs_data=CloneWorkspace('reduced_1')
        data_MDE=mtd['merged']
        ConvertToMD(InputWorkspace=dgs_data,
                    QDimensions='Q3D',
                    dEAnalysisMode='Direct',
                    Q3DFrames="Q_sample",
                    MinValues='-11,-11,-11,-25',
                    MaxValues='11,11,11,49',
                    PreprocDetectorsWS='-',
                    OverwriteExisting=False,
                    OutputWorkspace='background_MDE')

        # MD Normalize data and backgroun in 1 call
        MDNorm(InputWorkspace='merged',
               BackgroundWorkspace='background_MDE',
               Dimension0Name='QDimension1',
               Dimension0Binning='-5,0.05,5',
               Dimension1Name='QDimension2',
               Dimension1Binning='-5,0.05,5',
               Dimension2Name='DeltaE',
               Dimension2Binning='-2,2',
               Dimension3Name='QDimension0',
               Dimension3Binning='-0.5,0.5',
               SymmetryOperations='x,y,z;x,-y,z;x,y,-z;x,-y,-z',
               OutputWorkspace='result',
               OutputDataWorkspace='dataMD',
               OutputNormalizationWorkspace='normMD',
               BackgroundOutputWorkspace='backgroundMDH',
               BackgroundOutputDataWorkspace='bkgdMD',
               BackgroundOutputNormalizationWorkspace='bkgdNormMD')

        # Clean
        # TODO - shall this input to MDNorm too????
        clean_data=MinusMD('result','backgroundMDH')

    def validate(self):
        self.tolerance = 1e-8
        return 'result','MDNormHYSPEC.nxs'
