/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifdef DYNAMO_visualizer
# include <coil/coilMaster.hpp>
#endif

#include "species.hpp"
#include <magnet/xmlwriter.hpp>
#include <memory>
#include "../coilRenderObj.hpp"

namespace Gtk {
  class VBox;
  class RadioButton;
}

class SpPoint: public Species, public CoilRenderObj
{
public:
  template<class T1>
  SpPoint(dynamo::SimData* sim, CRange* r, T1 nmass, std::string nName,
	  unsigned int ID, std::string nIName="Bulk"):
    Species(sim, "SpPoint", r, nmass, nName, ID, nIName)
  {}
  
  SpPoint(const magnet::xml::Node& XML, dynamo::SimData* nSim, unsigned int nID):
    Species(nSim, "", NULL, 0, "", nID,"")
  { operator<<(XML); }

  virtual void initialise();

  virtual void operator<<(const magnet::xml::Node& XML);

  virtual Species* Clone() const { return new SpPoint(*this); }

  virtual double getScalarMomentOfInertia(size_t ID) const 
  { M_throw() << "Species has no intertia"; }

#ifdef DYNAMO_visualizer
  virtual magnet::thread::RefPtr<coil::RenderObj>& getCoilRenderObj() const;
  virtual void updateRenderData(magnet::GL::Context&) const;
#endif

protected:

#ifdef DYNAMO_visualizer
  mutable magnet::thread::RefPtr<coil::RenderObj> _renderObj;
  mutable magnet::thread::RefPtr<coil::CoilRegister> _coil;
#endif

protected:
  virtual void outputXML(magnet::xml::XmlStream& XML) const;
};
