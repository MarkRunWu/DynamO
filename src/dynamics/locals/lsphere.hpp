/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2010  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

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

#ifndef CLSphere_HPP
#define CLSphere_HPP

#include "local.hpp"

class CLSphere: public Local
{
public:
  CLSphere(const XMLNode&, DYNAMO::SimData*);
  CLSphere(DYNAMO::SimData*, Iflt, Vector , Iflt, 
	 std::string, CRange*, bool nrender = true);

  virtual ~CLSphere() {}

  virtual Local* Clone() const { return new CLSphere(*this); };

  virtual LocalEvent getEvent(const Particle&) const;

  virtual void runEvent(const Particle&, const LocalEvent&) const;
  
  virtual bool isInCell(const Vector &, const Vector &) const;

  virtual void initialise(size_t);

  virtual void operator<<(const XMLNode&);

  virtual void write_povray_info(std::ostream&) const;

protected:
  virtual void outputXML(xmlw::XmlStream&) const;

  Vector  vPosition;
  Iflt e;
  Iflt radius;
  Iflt r2;
  bool render;
};

#endif
