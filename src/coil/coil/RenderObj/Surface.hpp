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
#pragma once

#include "Triangles.hpp"
#include <time.h>
#include <magnet/math/vector.hpp>

namespace coil {
  class RSurface : public RTriangles
  {
  public:
    RSurface(std::string name, size_t N = 10, Vector origin = Vector(-10,-1.0,-10), Vector axis1 = Vector(20,0,0),
	     Vector axis2 = Vector(0,0,20), Vector axis3 = Vector(0,1,0));

    virtual void init(const std::tr1::shared_ptr<magnet::thread::TaskQueue>& systemQueue);

    virtual void glRender(const magnet::GL::Camera& cam, RenderMode mode = DEFAULT);

    virtual Glib::RefPtr<Gdk::Pixbuf> getIcon();

    virtual bool deletable() { return true; }

    virtual magnet::math::Vector getDimensions() const;
    virtual magnet::math::Vector getCentre() const;

  protected:
    void clTick() {}

    size_t _N;

    Vector _origin;
    Vector _axis1;
    Vector _axis2;     
    Vector _axis3;
  };
}
