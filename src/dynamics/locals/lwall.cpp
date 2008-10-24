/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2008  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#include "lwall.hpp"
#include "../liouvillean/liouvillean.hpp"
#include "localEvent.hpp"
#include "../NparticleEventData.hpp"
#include "../overlapFunc/CubePlane.hpp"
#include "../units/units.hpp"
#include "../../datatypes/vector.xml.hpp"

CLWall::CLWall(const XMLNode& XML, const DYNAMO::SimData* tmp):
  CLocal(tmp, "LocalWall")
{
  operator<<(XML);
}

CLocalEvent 
CLWall::getEvent(const CParticle& part) const
{
  Sim->Dynamics.Liouvillean().updateParticle(part);

  return CLocalEvent(part, Sim->Dynamics.Liouvillean().getWallCollision
		     (part, vPosition, vNorm), WALL, *this);
}

CNParticleData
CLWall::runEvent(const CLocalEvent& event) const
{
  return CNParticleData(Sim->Dynamics.Liouvillean().runWallCollision
			(event.getParticle(),vNorm,e));
}

bool 
CLWall::isInCell(const CVector<>& Origin, const CVector<>& CellDim) const
{
  return DYNAMO::OverlapFunctions::CubePlane
    (Origin, CellDim, vPosition, vNorm);
}

void 
CLWall::initialise(size_t nID)
{
  ID = nID;
}

void 
CLWall::operator<<(const XMLNode& XML)
{
  range.set_ptr(CRange::loadClass(XML,Sim));
  
  try {
    e = boost::lexical_cast<Iflt>(XML.getAttribute("Elasticity"));
    XMLNode xBrowseNode = XML.getChildNode("Norm");
    localName = XML.getAttribute("Name");
    vNorm << xBrowseNode;
    vNorm = vNorm.unitVector();
    xBrowseNode = XML.getChildNode("Origin");
    vPosition << xBrowseNode;
    vPosition *= Sim->Dynamics.units().unitLength();
  } 
  catch (boost::bad_lexical_cast &)
    {
      D_throw() << "Failed a lexical cast in CLWall";
    }
}

void 
CLWall::outputXML(xmlw::XmlStream& XML) const
{
  XML << xmlw::attr("Type") << "Wall" 
      << xmlw::attr("Name") << localName
      << xmlw::attr("Elasticity") << e
      << range
      << xmlw::tag("Norm")
      << vNorm
      << xmlw::endtag("Norm")
      << xmlw::tag("Origin")
      << vPosition / Sim->Dynamics.units().unitLength()
      << xmlw::endtag("Origin");
}
