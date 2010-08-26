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
#include "trajectory.hpp"
#include "../../dynamics/dynamics.hpp"
#include "../../dynamics/units/units.hpp"
#include "../../dynamics/globals/globEvent.hpp"
#include "../../dynamics/interactions/intEvent.hpp"
#include "../../dynamics/locals/localEvent.hpp"
#include "../../dynamics/NparticleEventData.hpp"
#include "../../dynamics/systems/system.hpp"
#include "../../dynamics/BC/BC.hpp"

OPTrajectory::OPTrajectory(const DYNAMO::SimData* t1, const XMLNode&):
  OutputPlugin(t1,"Trajectory")
{}

OPTrajectory::OPTrajectory(const OPTrajectory& trj):
  OutputPlugin(trj)
{
  if (trj.logfile.is_open())
    trj.logfile.close();
}

void
OPTrajectory::initialise()
{
  if (logfile.is_open())
    logfile.close();
  
  logfile.open("trajectory.out", std::ios::out|std::ios::trunc);
}

void
OPTrajectory::printData(const size_t& p1,
			 const size_t& p2) const
{
  size_t id1 = ((p1 < p2) 
		? p1 : p2);
  
  size_t id2 = ((p1 > p2) 
		? p1 : p2);

  Vector  rij = Sim->vParticleList[id1].getPosition()
    - Sim->vParticleList[id2].getPosition();

  Sim->dynamics.BCs().applyBC(rij);
  
  rij /= Sim->dynamics.units().unitLength();

  logfile << " p1 " << id1
	  << " p2 " << id2
	  << " |r12| " << rij.nrm()
	  << " r12 < ";
  
  for (size_t iDim(0); iDim < NDIM; ++iDim)
    logfile << rij[iDim] << " ";

  logfile << ">";
}

void 
OPTrajectory::eventUpdate(const IntEvent& eevent, 
				   const PairEventData&)
{
  logfile << "INTERACTION " << eevent.getInteractionID()
	  << " TYPE " << eevent.getType()
	  << " t " << Sim->dSysTime / Sim->dynamics.units().unitTime() 
	  << " dt " << eevent.getdt() / Sim->dynamics.units().unitTime();
  
  printData(eevent.getParticle1ID(),
	    eevent.getParticle2ID());
  
  logfile << "\n";
}

void 
OPTrajectory::eventUpdate(const GlobalEvent& eevent, 
			   const NEventData& SDat)
{
  logfile << "GLOBAL " << eevent.getGlobalID()
	  << " TYPE " << eevent.getType()
	  << " t " << Sim->dSysTime / Sim->dynamics.units().unitTime() 
	  << " dt " << eevent.getdt() / Sim->dynamics.units().unitTime()
	  << "\n";

  BOOST_FOREACH(const ParticleEventData& pData, SDat.L1partChanges)
    logfile << "    1PEvent p1 " << pData.getParticle().getID()
	    << "\n";
  
  BOOST_FOREACH(const PairEventData& pData, SDat.L2partChanges)
    {
      logfile << "    2PEvent";

      printData(pData.particle1_.getParticle().getID(),
		pData.particle2_.getParticle().getID());

      logfile << "\n";
    }
}

void 
OPTrajectory::eventUpdate(const LocalEvent& eevent, 
			   const NEventData& SDat)
{
  logfile << "LOCAL " << eevent.getLocalID()
    	  << " TYPE " << eevent.getType()
	  << " t " << Sim->dSysTime / Sim->dynamics.units().unitTime() 
	  << " dt " << eevent.getdt() / Sim->dynamics.units().unitTime()
	  << "\n";

  BOOST_FOREACH(const ParticleEventData& pData, SDat.L1partChanges)
    {
      logfile << "    1PEvent p1 " << pData.getParticle().getID()
	      << " delP1 < ";

      Vector delP = pData.getDeltaP();
      
      for (size_t iDim(0); iDim < NDIM; ++iDim)
	logfile << delP[iDim] << " ";
      
      logfile << ">"
	      << "\n";
    }
  BOOST_FOREACH(const PairEventData& pData, SDat.L2partChanges)
    {
      logfile << "    2PEvent";
      
      printData(pData.particle1_.getParticle().getID(),
		pData.particle2_.getParticle().getID());
      
      logfile << "\n";
    }
}

void 
OPTrajectory::eventUpdate(const CSystem& sys, const NEventData& SDat, 
			   const Iflt& dt)
{
  logfile << "SYSTEM " << sys.getID()
	  << " TYPE " << sys.getType()
	  << " t " << Sim->dSysTime / Sim->dynamics.units().unitTime() 
	  << " dt " << dt / Sim->dynamics.units().unitTime()
	  << "\n";

  BOOST_FOREACH(const ParticleEventData& pData, SDat.L1partChanges)
    logfile << "    1PEvent p1 " << pData.getParticle().getID()
	    << "\n";
  
  BOOST_FOREACH(const PairEventData& pData, SDat.L2partChanges)
    {
      logfile << "    2PEvent";
      
      printData(pData.particle1_.getParticle().getID(),
		pData.particle2_.getParticle().getID());
      
      logfile << "\n";
    }
}

void 
OPTrajectory::output(xmlw::XmlStream& XML)
{}
