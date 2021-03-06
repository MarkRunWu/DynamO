/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.dynamomd.org
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

#include <dynamo/interactions/squarebond.hpp>
#include <dynamo/BC/BC.hpp>

#include <dynamo/units/units.hpp>
#include <dynamo/globals/global.hpp>
#include <dynamo/particle.hpp>
#include <dynamo/interactions/intEvent.hpp>
#include <dynamo/species/species.hpp>
#include <dynamo/2particleEventData.hpp>
#include <dynamo/dynamics/dynamics.hpp>
#include <dynamo/simulation.hpp>
#include <dynamo/schedulers/scheduler.hpp>
#include <dynamo/NparticleEventData.hpp>
#include <dynamo/outputplugins/outputplugin.hpp>
#include <magnet/xmlwriter.hpp>
#include <magnet/xmlreader.hpp>
#include <cmath>
#include <iomanip>

namespace dynamo {
  ISquareBond::ISquareBond(const magnet::xml::Node& XML, dynamo::Simulation* tmp):
    Interaction(tmp,NULL) //A temporary value!
  { operator<<(XML); }
	    
  void 
  ISquareBond::operator<<(const magnet::xml::Node& XML)
  {
    Interaction::operator<<(XML);
  
    try {
      _diameter = Sim->_properties.getProperty(XML.getAttribute("Diameter"),
					       Property::Units::Length());
      _lambda = Sim->_properties.getProperty(XML.getAttribute("Lambda"),
					     Property::Units::Dimensionless());

      if (XML.hasAttribute("Elasticity"))
	_e = Sim->_properties.getProperty(XML.getAttribute("Elasticity"),
					  Property::Units::Dimensionless());
      else
	_e = Sim->_properties.getProperty(1.0, Property::Units::Dimensionless());

      intName = XML.getAttribute("Name");
    }
    catch (boost::bad_lexical_cast &)
      {
	M_throw() << "Failed a lexical cast in CISquareWell";
      }
  }

  double 
  ISquareBond::getCaptureEnergy() const 
  { return 0.0; }

  double 
  ISquareBond::maxIntDist() const 
  { return _diameter->getMaxValue()
      * _lambda->getMaxValue(); }

  void 
  ISquareBond::initialise(size_t nID)
  { ID = nID; }

  bool 
  ISquareBond::captureTest(const Particle& p1, const Particle& p2) const
  {
    double d = (_diameter->getProperty(p1.getID())
		+ _diameter->getProperty(p2.getID())) * 0.5;

    double l = (_lambda->getProperty(p1.getID())
		+ _lambda->getProperty(p2.getID())) * 0.5;
  
#ifdef DYNAMO_DEBUG
    if (Sim->dynamics->sphereOverlap(p1, p2, d))
      derr << "Warning! Two particles might be overlapping"
	   << "Overlap is " << Sim->dynamics->sphereOverlap(p1, p2, d) 
	/ Sim->units.unitLength()
	   << "\nd = " << d / Sim->units.unitLength() << std::endl;
#endif
 
    return Sim->dynamics->sphereOverlap(p1, p2, l * d);
  }

  bool
  ISquareBond::validateState(const Particle& p1, const Particle& p2, bool textoutput) const
  {
    double d = (_diameter->getProperty(p1.getID())
		+ _diameter->getProperty(p2.getID())) * 0.5;
    double l = (_lambda->getProperty(p1.getID())
		+ _lambda->getProperty(p2.getID())) * 0.5;

    if (!Sim->dynamics->sphereOverlap(p1, p2, d * l))
      {
	if (textoutput)
	  derr << "Particle " << p1.getID() << " and Particle " << p2.getID() 
	       << " are bonded and cannot exceed a distance of " << l * d / Sim->units.unitLength()
	       << " but they are at a distance of " 
	       << Sim->BCs->getDistance(p1, p2) / Sim->units.unitLength()
	       << std::endl;
	
	return true;
      }

    if (Sim->dynamics->sphereOverlap(p1, p2, d))
      {
	if (textoutput)
	  derr << "Particle " << p1.getID() << " and Particle " << p2.getID() 
	       << " are bonded with an inner hard core at " << d / Sim->units.unitLength()
	       << " but they are at a distance of " 
	       << Sim->BCs->getDistance(p1, p2) / Sim->units.unitLength()
	       << std::endl;

	return true;
      }

    return false;
  }

  void
  ISquareBond::checkOverlaps(const Particle& part1, const Particle& part2) const
  {
    Vector  rij = part1.getPosition() - part2.getPosition();
    Sim->BCs->applyBC(rij);
    double r2 = rij.nrm2();

    double d = (_diameter->getProperty(part1.getID())
		+ _diameter->getProperty(part2.getID())) * 0.5;
    double d2 = d * d;
    double l = (_lambda->getProperty(part1.getID())
		+ _lambda->getProperty(part2.getID())) * 0.5;
  
    double ld2 = d * l * d * l;


    if (r2 < d2)
      derr << "Possible bonded overlap occured in diagnostics\n ID1=" << part1.getID() 
	   << ", ID2=" << part2.getID() << "\nR_ij^2=" 
	   << r2 / pow(Sim->units.unitLength(),2)
	   << "\nd^2=" 
	   << d2 / pow(Sim->units.unitLength(),2) << std::endl;
  
    if (r2 > ld2)
      derr << "Possible escaped bonded pair in diagnostics\n ID1=" << part1.getID() 
	   << ", ID2=" << part2.getID() << "\nR_ij^2=" 
	   << r2 / pow(Sim->units.unitLength(),2)
	   << "\n(lambda * d)^2=" 
	   << ld2 / pow(Sim->units.unitLength(),2) << std::endl;
  }

  IntEvent 
  ISquareBond::getEvent(const Particle &p1, 
			const Particle &p2) const 
  {    
#ifdef DYNAMO_DEBUG
    if (!Sim->dynamics->isUpToDate(p1))
      M_throw() << "Particle 1 is not up to date";
  
    if (!Sim->dynamics->isUpToDate(p2))
      M_throw() << "Particle 2 is not up to date";

    if (p1 == p2)
      M_throw() << "You shouldn't pass p1==p2 events to the interactions!";
#endif 

    double d = (_diameter->getProperty(p1.getID())
		+ _diameter->getProperty(p2.getID())) * 0.5;
    double l = (_lambda->getProperty(p1.getID())
		+ _lambda->getProperty(p2.getID())) * 0.5;

    IntEvent retval(p1, p2, HUGE_VAL, NONE, *this);

    double dt = Sim->dynamics->SphereSphereInRoot(p1, p2, d);
    if (dt != HUGE_VAL)
      retval = IntEvent(p1, p2, dt, CORE, *this);

    dt = Sim->dynamics->SphereSphereOutRoot(p1, p2, l * d);
    if (retval.getdt() > dt)
      retval = IntEvent(p1, p2, dt, BOUNCE, *this);
  
    return retval;
  }

  void
  ISquareBond::runEvent(Particle& p1, Particle& p2, const IntEvent& iEvent) const
  {
    ++Sim->eventCount;

#ifdef DYNAMO_DEBUG
    if ((iEvent.getType() != BOUNCE) && (iEvent.getType() != CORE))
      M_throw() << "Unknown type found";
#endif

    double d = (_diameter->getProperty(p1.getID())
		+ _diameter->getProperty(p2.getID())) * 0.5;
    double d2 = d * d;

    double e = (_e->getProperty(p1.getID())
		+ _e->getProperty(p2.getID())) * 0.5;

    PairEventData EDat(Sim->dynamics->SmoothSpheresColl
		       (iEvent, e, d2, iEvent.getType()));

    Sim->signalParticleUpdate(EDat);
    
    //Now we're past the event, update the scheduler and plugins
    Sim->ptrScheduler->fullUpdate(p1, p2);
  
    BOOST_FOREACH(shared_ptr<OutputPlugin> & Ptr, Sim->outputPlugins)
      Ptr->eventUpdate(iEvent,EDat);
  }
    
  void 
  ISquareBond::outputXML(magnet::xml::XmlStream& XML) const
  {
    XML << magnet::xml::attr("Type") << "SquareBond"
	<< magnet::xml::attr("Diameter") << _diameter->getName()
	<< magnet::xml::attr("Lambda") << _lambda->getName()
	<< magnet::xml::attr("Name") << intName
	<< magnet::xml::attr("Elasticity") << _e->getName()
	<< *range;
  }
}

