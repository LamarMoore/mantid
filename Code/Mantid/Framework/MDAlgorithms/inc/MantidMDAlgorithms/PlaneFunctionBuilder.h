#ifndef PLANEFUNCTONBUILDER_H_
#define PLANEFUNCTONBUILDER_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include <vector>
#include <memory>
#include "MantidKernel/System.h"
#include "MantidMDAlgorithms/Plane3DImplicitFunction.h"
#include "MantidAPI/ImplicitFunctionBuilder.h"
#include "MantidAPI/ImplicitFunctionParameter.h"
#include "MantidMDAlgorithms/NormalParameter.h"
#include "MantidMDAlgorithms/OriginParameter.h"

namespace Mantid
{

    namespace MDAlgorithms
    {
        /**

        This class is for building PlaneImplicitFunction

        @author Owen Arnold, Tessella plc
        @date 01/10/2010

        Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

        This file is part of Mantid.

        Mantid is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 3 of the License, or
        (at your option) any later version.

        Mantid is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.

        File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
        Code Documentation is available at: <http://doxygen.mantidproject.org>
        */

        class DLLExport PlaneFunctionBuilder : public Mantid::API::ImplicitFunctionBuilder
        {
        private:
            mutable OriginParameter m_origin;
            mutable NormalParameter m_normal;
            mutable WidthParameter m_width;
        public:
            PlaneFunctionBuilder();
            void addNormalParameter(const NormalParameter& parameter);
            void addOriginParameter(const OriginParameter& parameter);
            void addWidthParameter(const WidthParameter& width);
            Mantid::Geometry::MDImplicitFunction* create() const;
            ~PlaneFunctionBuilder();
        };

    }
}

#endif
