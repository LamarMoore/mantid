#ifndef MANTID_MDEVENTS_MDHISTOWORKSPACE_H_
#define MANTID_MDEVENTS_MDHISTOWORKSPACE_H_

#include "MantidKernel/System.h"
#include "MantidKernel/Exception.h"
#include "MantidGeometry/MDGeometry/MDPoint.h"
#include "MantidGeometry/MDGeometry/IMDDimension.h"
#include "MantidGeometry/MDGeometry/MDHistoDimension.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidGeometry/MDGeometry/MDImplicitFunction.h"


namespace Mantid
{
namespace MDEvents
{

  /** MDHistoWorkspace:
  *
  * An implementation of IMDWorkspace that contains a (normally dense) histogram
  * representation in up to 4 dimensions.
  *
  * This will be the result of a slice or rebin of another workspace, e.g. a
  * MDEventWorkspace.
  *
  * This will be used by ParaView e.g. for visualization.
  *
  * @author Janik Zikovsky
  * @date 2011-03-24 11:21:06.280523
  */
  class DLLExport MDHistoWorkspace : public API::IMDWorkspace
  {
  public:
    MDHistoWorkspace(Mantid::Geometry::MDHistoDimension_sptr dimX,
        Mantid::Geometry::MDHistoDimension_sptr dimY=Mantid::Geometry::MDHistoDimension_sptr(),
        Mantid::Geometry::MDHistoDimension_sptr dimZ=Mantid::Geometry::MDHistoDimension_sptr(),
        Mantid::Geometry::MDHistoDimension_sptr dimT=Mantid::Geometry::MDHistoDimension_sptr());

    MDHistoWorkspace(std::vector<Mantid::Geometry::MDHistoDimension_sptr> & dimensions);

    virtual ~MDHistoWorkspace();

    void init(std::vector<Mantid::Geometry::MDHistoDimension_sptr> & dimensions);

    virtual const std::string id() const
    { return "MDHistoWorkspace"; }

    virtual size_t getMemorySize() const;

    /// Get the number of points (bins in this case) associated with the workspace;
    uint64_t getNPoints() const
    {
      return m_length;
    }

    /// Get the number of dimensions
    size_t getNumDims() const
    {
      return numDimensions;
    }

    // --------------------------------------------------------------------------------------------
    /** @return a const reference to the indexMultiplier array.
     * To find the index into the linear array, dim0 + indexMultiplier[0]*dim1 + ...
     */
    const size_t * getIndexMultiplier() const
    {
      return indexMultiplier;
    }

    /** @return the direct pointer to the signal array. For speed */
    signal_t * getSignalArray()
    {
      return m_signals;
    }

    /** @return the inverse of volume of EACH cell in the workspace. For normalizing. */
    coord_t getInverseVolumeVolume() const
    {
      return m_inverseVolume;
    }

    /** @return the direct pointer to the error squared array. For speed */
    signal_t * getErrorSquaredArray()
    {
      return m_errors;
    }

    void setTo(signal_t signal, signal_t error);

    void applyImplicitFunction(Mantid::Geometry::MDImplicitFunction * function, signal_t signal, signal_t error);


    // --------------------------------------------------------------------------------------------
    /// Get the x-dimension mapping.
    boost::shared_ptr<const Mantid::Geometry::IMDDimension> getXDimension() const
    {
      return m_dimensions[0];
    }

    /// Get the y-dimension mapping.
    boost::shared_ptr<const Mantid::Geometry::IMDDimension> getYDimension() const
    {
      if (m_dimensions.size() < 2) throw std::runtime_error("MDHistoWorkspace does not have a Y dimension.");
      return m_dimensions[1];
    }

    /// Get the z-dimension mapping.
    boost::shared_ptr<const Mantid::Geometry::IMDDimension> getZDimension() const
    {
      if (m_dimensions.size() < 3) throw std::runtime_error("MDHistoWorkspace does not have a X dimension.");
      return m_dimensions[2];
    }

    /// Get the t-dimension mapping.
    boost::shared_ptr<const Mantid::Geometry::IMDDimension> getTDimension() const
    {
      if (m_dimensions.size() < 4) throw std::runtime_error("MDHistoWorkspace does not have a T dimension.");
      return m_dimensions[3];
    }

    boost::shared_ptr<Mantid::Geometry::IMDDimension> getDimensionNum(size_t index)
    {
      if (index >= m_dimensions.size()) throw std::runtime_error("MDHistoWorkspace does not have a dimension at that index.");
      return m_dimensions[index];
    }

    /// Get the dimension with the specified id.
    boost::shared_ptr<const Mantid::Geometry::IMDDimension> getDimension(std::string id) const
    {
      for (size_t i=0; i < m_dimensions.size(); ++i)
        if (m_dimensions[i]->getDimensionId() == id)
          return m_dimensions[i];
      throw std::invalid_argument("Dimension tagged " + id + " was not found in the MDHistoWorkspace");
    }

    /// Get the dimension ids in their order
    const std::vector<std::string> getDimensionIDs() const
    {
      throw Mantid::Kernel::Exception::NotImplementedError("Not yet!");
    }

    /// All MD type workspaces have an effective geometry. MD type workspaces must provide this geometry in a serialized format.
    std::string getGeometryXML() const;



    /// Sets the signal at the specified index.
    void setSignalAt(size_t index, signal_t value)
    {
      m_signals[index] = value;
    }

    /// Sets the error at the specified index.
    void setErrorAt(size_t index, signal_t value)
    {
      m_errors[index] = value;
    }




    /// Get the error of the signal at the specified index.
    signal_t getErrorAt(size_t index) const
    {
      return m_errors[index];
    }

    /// Get the error at the specified index given in 4 dimensions (typically X,Y,Z,t)
    signal_t getErrorAt(size_t index1, size_t index2) const
    {
      return m_errors[index1 + indexMultiplier[0]*index2];
    }

    /// Get the error at the specified index given in 4 dimensions (typically X,Y,Z,t)
    signal_t getErrorAt(size_t index1, size_t index2, size_t index3) const
    {
      return m_errors[index1 + indexMultiplier[0]*index2 + indexMultiplier[1]*index3];
    }

    /// Get the error at the specified index given in 4 dimensions (typically X,Y,Z,t)
    signal_t getErrorAt(size_t index1, size_t index2, size_t index3, size_t index4) const
    {
      return m_errors[index1 + indexMultiplier[0]*index2 + indexMultiplier[1]*index3 + indexMultiplier[2]*index4];
    }



    /// Get the signal at the specified index.
    signal_t getSignalAt(size_t index) const
    {
      return m_signals[index];
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t)
    signal_t getSignalAt(size_t index1, size_t index2) const
    {
      return m_signals[index1 + indexMultiplier[0]*index2];
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t)
    signal_t getSignalAt(size_t index1, size_t index2, size_t index3) const
    {
      return m_signals[index1 + indexMultiplier[0]*index2 + indexMultiplier[1]*index3];
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t)
    signal_t getSignalAt(size_t index1, size_t index2, size_t index3, size_t index4) const
    {
      return m_signals[index1 + indexMultiplier[0]*index2 + indexMultiplier[1]*index3 + indexMultiplier[2]*index4];
    }




    /// Get the signal at the specified index, normalized by cell volume
    signal_t getSignalNormalizedAt(size_t index) const
    {
      return m_signals[index] * m_inverseVolume;
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t), normalized by cell volume
    signal_t getSignalNormalizedAt(size_t index1, size_t index2) const
    {
      return m_signals[index1 + indexMultiplier[0]*index2] * m_inverseVolume;
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t), normalized by cell volume
    signal_t getSignalNormalizedAt(size_t index1, size_t index2, size_t index3) const
    {
      return m_signals[index1 + indexMultiplier[0]*index2 + indexMultiplier[1]*index3] * m_inverseVolume;
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t), normalized by cell volume
    signal_t getSignalNormalizedAt(size_t index1, size_t index2, size_t index3, size_t index4) const
    {
      return m_signals[index1 + indexMultiplier[0]*index2 + indexMultiplier[1]*index3 + indexMultiplier[2]*index4] * m_inverseVolume;
    }



    /// Get the error of the signal at the specified index, normalized by cell volume
    signal_t getErrorNormalizedAt(size_t index) const
    {
      return m_errors[index] * m_inverseVolume;
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t), normalized by cell volume
    signal_t getErrorNormalizedAt(size_t index1, size_t index2) const
    {
      return getErrorAt(index1,index2) * m_inverseVolume;
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t), normalized by cell volume
    signal_t getErrorNormalizedAt(size_t index1, size_t index2, size_t index3) const
    {
      return getErrorAt(index1,index2,index3) * m_inverseVolume;
    }

    /// Get the signal at the specified index given in 4 dimensions (typically X,Y,Z,t), normalized by cell volume
    signal_t getErrorNormalizedAt(size_t index1, size_t index2, size_t index3, size_t index4) const
    {
      return getErrorAt(index1,index2,index3,index4) * m_inverseVolume;
    }


    /// Return a vector containing a copy of the signal data in the workspace. TODO: Make this more efficient if needed.
    virtual std::vector<signal_t> getSignalDataVector() const;
    virtual std::vector<signal_t> getErrorDataVector() const;

    /// Getter for collapsed dimensions.
    Mantid::Geometry::VecIMDDimension_const_sptr getNonIntegratedDimensions() const;


    //======================================================================================
    //================= METHODS THAT WON'T GET IMPLEMENTED PROBABLY =====================
    //======================================================================================

    /// Get the point at the specified index.
    const Mantid::Geometry::SignalAggregate& getPoint(size_t index) const
    {
      (void) index; // Avoid compiler warning
      throw Mantid::Kernel::Exception::NotImplementedError("Not yet!");
    }

    /// Get the cell at the specified index/increment.
    const Mantid::Geometry::SignalAggregate& getCell(size_t dim1Increment) const
    {
      (void) dim1Increment; // Avoid compiler warning
      throw Mantid::Kernel::Exception::NotImplementedError("Not yet!");
    }

    /// Get the cell at the specified index/increment.
    const Mantid::Geometry::SignalAggregate& getCell(size_t dim1Increment, size_t dim2Increment) const
    {
      (void) dim1Increment; (void) dim2Increment; // Avoid compiler warning
      throw Mantid::Kernel::Exception::NotImplementedError("Not yet!");
    }

    /// Get the cell at the specified index/increment.
    const Mantid::Geometry::SignalAggregate& getCell(size_t dim1Increment, size_t dim2Increment, size_t dim3Increment) const
    {
      (void) dim1Increment; (void) dim2Increment; (void) dim3Increment; // Avoid compiler warning
      throw Mantid::Kernel::Exception::NotImplementedError("Not yet!");
    }

    /// Get the cell at the specified index/increment.
    const Mantid::Geometry::SignalAggregate& getCell(size_t dim1Increment, size_t dim2Increment, size_t dim3Increment, size_t dim4Increment) const
    {
      (void) dim1Increment; (void)    // --------------------------------------------------------------------------------------------
 dim2Increment; (void) dim3Increment; (void) dim4Increment; // Avoid compiler warning
      throw Mantid::Kernel::Exception::NotImplementedError("Not yet!");
    }

    /// Get the cell at the specified index/increment.
    const Mantid::Geometry::SignalAggregate& getCell(...) const
    {
      throw Mantid::Kernel::Exception::NotImplementedError("MDHistoWorkspace does not support more than 4 dimensions!");
    }

    /// Horace sytle implementations need to have access to the underlying file.
    std::string getWSLocation() const
    {
      return "";
    }

    //======================================================================================
    //================= END METHODS THAT WON'T GET IMPLEMENTED PROBABLY =====================
    //======================================================================================




  private:
    /// Number of dimensions in this workspace
    size_t numDimensions;

    /// Linear array of signals for each bin
    signal_t * m_signals;

    /// Linear array of errors for each bin
    signal_t * m_errors;

    /// Length of the m_signals / m_errors arrays.
    size_t m_length;

    /// Vector of the dimensions used, in the order X Y Z t
    std::vector<Mantid::Geometry::MDHistoDimension_sptr> m_dimensions;

    /// To find the index into the linear array, dim0 + indexMultiplier[0]*dim1 + ...
    size_t * indexMultiplier;

    /// Inverse of the volume of EACH cell
    coord_t m_inverseVolume;

  };

  /// A shared pointer to a MDHistoWorkspace
  typedef boost::shared_ptr<MDHistoWorkspace> MDHistoWorkspace_sptr;


} // namespace Mantid
} // namespace MDEvents

#endif  /* MANTID_MDEVENTS_MDHISTOWORKSPACE_H_ */
