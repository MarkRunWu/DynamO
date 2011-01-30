/*  DYNAMO:- Event driven molecular dynamics simulator 
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

#ifdef DYNAMO_visualizer
# include <gtkmm.h>
#endif

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <cstring>
#include "include.hpp"
#include "../../extcode/xmlwriter.hpp"
#include "../../extcode/xmlParser.h"
#include "../ranges/1range.hpp"
#include "../ranges/1RAll.hpp"
#include "../../simulation/particle.hpp"
#include "../../base/is_simdata.hpp"
#include <boost/tokenizer.hpp>

#ifdef DYNAMO_visualizer
# include <magnet/thread/mutex.hpp>
# include "../liouvillean/CompressionL.hpp"
# include <magnet/HSV.hpp>
# include <coil/RenderObj/Spheres.hpp>
#endif

void 
SpPoint::operator<<(const XMLNode& XML)
{
  range.set_ptr(CRange::loadClass(XML,Sim));
    
    try {
      mass = boost::lexical_cast<double>(XML.getAttribute("Mass"))
	* Sim->dynamics.units().unitMass();
      spName = XML.getAttribute("Name");
      intName = XML.getAttribute("IntName");
    } 
    catch (boost::bad_lexical_cast &)
      {
	M_throw() << "Failed a lexical cast in SpPoint";
      }

}

void 
SpPoint::outputXML(xml::XmlStream& XML) const
{
  XML << xml::attr("Mass") 
      << mass / Sim->dynamics.units().unitMass()
      << xml::attr("Name") << spName
      << xml::attr("IntName") << intName
      << xml::attr("Type") << "Point"
      << range;
}

void
SpPoint::initialise()
{ 
  if (IntPtr == NULL)
    M_throw() << "SpPoint missing a matching interaction";
}


#ifdef DYNAMO_visualizer
# include <magnet/gtk/numericEntry.hpp>

namespace { 
  class DynamoSphereRenderer: public RTSpheres
  {
  public:
    DynamoSphereRenderer(size_t N, std::string name):
      RTSpheres(N, name)
    {
      for (size_t i(0); i < 4; ++i)
	_color.s[i] = 255;
    }
    
    virtual void initGTK() 
    {
      _optList.reset(new Gtk::VBox);//The Vbox of options
      _colorMode.reset(new Gtk::RadioButton("Single Color"));
      _R.reset(new Gtk::SpinButton);
      _G.reset(new Gtk::SpinButton);
      _B.reset(new Gtk::SpinButton);
      _A.reset(new Gtk::SpinButton);
      _characteristicV.reset(new Gtk::Entry);

      {//Single color and RGBA boxes
	Gtk::HBox* box = manage(new Gtk::HBox);
	
	box->pack_start(*_colorMode, true, true);_colorMode->show();
	_colorMode->signal_group_changed()
	  .connect(sigc::mem_fun(*this, &DynamoSphereRenderer::guiUpdate));

	Gtk::Label* label = manage(new Gtk::Label("RGBA"));
	box->pack_start(*label, false, false); label->show();

	
	_R->set_increments(1.0, 1.0);
	_R->set_range(0.0, 255.0);
	_R->set_value(_color.s[0]);
	
	_G->set_increments(1.0, 1.0);
	_G->set_range(0.0, 255.0);
	_G->set_value(_color.s[0]);
	
	_B->set_increments(1.0, 1.0);
	_B->set_range(0.0, 255.0);
	_B->set_value(_color.s[0]);
	
	_A->set_increments(1.0, 1.0);
	_A->set_range(0.0, 255.0);
	_A->set_value(_color.s[0]);      

	box->pack_start(*_R, false, false); _R->show();
	box->pack_start(*_G, false, false); _G->show();
	box->pack_start(*_B, false, false); _B->show();
	box->pack_start(*_A, false, false); _A->show();
	
	_R->signal_changed()
	  .connect(sigc::mem_fun(*this, &DynamoSphereRenderer::guiUpdate));
	_G->signal_changed()
	  .connect(sigc::mem_fun(*this, &DynamoSphereRenderer::guiUpdate));
	_B->signal_changed()
	  .connect(sigc::mem_fun(*this, &DynamoSphereRenderer::guiUpdate));
	_A->signal_changed()
	  .connect(sigc::mem_fun(*this, &DynamoSphereRenderer::guiUpdate));
	
	_optList->add(*box);
	box->show();

	//Horizontal seperator
	Gtk::HSeparator* line = manage(new Gtk::HSeparator);
	line->show();
	_optList->add(*line);
      }

      Gtk::RadioButton::Group group = _colorMode->get_group();

      {//Color by ID
	Gtk::RadioButton* btn = manage(new Gtk::RadioButton("Color by ID"));
	btn->set_group(group);
	btn->show();
	_optList->add(*btn);

	//Horizontal seperator
	Gtk::HSeparator* line = manage(new Gtk::HSeparator);
	line->show();
	_optList->add(*line);
      }

      {//Color by Speed
	Gtk::HBox* box = manage(new Gtk::HBox);	
	
	Gtk::RadioButton* btn = manage(new Gtk::RadioButton("Color by Speed"));
	btn->set_group(group);
	btn->show();
	box->pack_start(*btn, true, true);
	
	box->pack_start(*_characteristicV, false, false);
	_characteristicV->show();
	_characteristicV->set_text("1.00");
	
	_characteristicV->signal_changed()
	  .connect(sigc::bind<Gtk::Entry&>(&magnet::Gtk::forceNumericEntry, *_characteristicV));
	box->show();
	_optList->add(*box);
      }
      
      _optList->show();
    }

    virtual void showControls(Gtk::ScrolledWindow* win) 
    {
      win->remove();
      _optList->unparent();
      win->add(*_optList);
      win->show();
    }
    
  protected:

    void guiUpdate()
    {

    }


    std::auto_ptr<Gtk::VBox> _optList;
    std::auto_ptr<Gtk::RadioButton> _colorMode;
    std::auto_ptr<Gtk::SpinButton> _R;
    std::auto_ptr<Gtk::SpinButton> _G;
    std::auto_ptr<Gtk::SpinButton> _B;
    std::auto_ptr<Gtk::SpinButton> _A;
    std::auto_ptr<Gtk::Entry> _characteristicV;

    cl_uchar4 _color;
  };
}


magnet::thread::RefPtr<RenderObj>& 
SpPoint::getCoilRenderObj() const
{
  if (!_renderObj.isValid())
    {
      _renderObj = new DynamoSphereRenderer(range->size(), "Species: " + spName);
      particleData.resize(range->size());
      particleColorData.resize(range->size());
    }

  return _renderObj;
}

void
SpPoint::updateRenderObj(magnet::CL::CLGLState& CLState) const
{
  if (!_renderObj.isValid())
    M_throw() << "Updating before the render object has been fetched";
  
  ///////////////////////POSITION DATA UPDATE
  //Check if the system is compressing and adjust the radius scaling factor
  float factor = 1;
  if (Sim->dynamics.liouvilleanTypeTest<LCompression>())
    factor = (1 + static_cast<const LCompression&>(Sim->dynamics.getLiouvillean()).getGrowthRate() * Sim->dSysTime);
 
  double diam = getIntPtr()->hardCoreDiam() * factor;
  
  size_t sphID(0);
  BOOST_FOREACH(unsigned long ID, *range)
    {
      Vector pos = Sim->particleList[ID].getPosition();
      
      Sim->dynamics.BCs().applyBC(pos);
      
      for (size_t i(0); i < NDIM; ++i)
	particleData[sphID].s[i] = pos[i];
      
      particleData[sphID++].w = diam * 0.5;
    }

  {
    CLState.getCommandQueue().enqueueWriteBuffer
      (static_cast<RTSpheres&>(*_renderObj).getSphereDataBuffer(),
       false, 0, range->size() * sizeof(cl_float4), &particleData[0]);
  }

  ///////////////////////COLOR DATA UPDATE
  if (_initialColorData)
    {
      _initialColorData = false;
      updateColorObj(CLState);
    }
}

void 
SpPoint::updateColorObj(magnet::CL::CLGLState& CLState) const
{

//  switch (_colorMode)
//    {
//    case IDHSV:
//      {
	size_t np = range->size();
	for (size_t sphID(0); sphID < np; ++ sphID)
	  magnet::color::HSVtoRGB(particleColorData[sphID], ((float)(sphID)) / np);
//      }
//      break;
//    case CONSTANT:
//      for (size_t id(0); id < range->size(); ++id)
//	for (size_t cc(0); cc < 4; ++cc)
//	  particleColorData[id].s[cc] = _constColor[cc];
//      break;
//    }

  {
    CLState.getCommandQueue().enqueueWriteBuffer
      (static_cast<RTSpheres&>(*_renderObj).getColorDataBuffer(),
       false, 0, range->size() * sizeof(cl_uchar4), &particleColorData[0]);
  }
}
#endif
