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
#include <vector>
#include <cmath>

#include <string.h>
#include <stdlib.h>
#include <magnet/math/vector.hpp>
#include <magnet/exception.hpp>

namespace magnet {
  namespace GL {
    namespace objects {
      namespace primitives {
	/*! \brief This class contains functions which generate the
            vertex data for an OpenGL cylinder.
	 */
	class Arrow
	{
	public:

	  inline static std::vector<GLfloat> getVertices(size_t LOD, GLfloat head_length_ratio = 0.5,
							 GLfloat body_radius_ratio = 0.5)
	  {
	    //Point vertex, plus 3 body vertices per LOD
	    std::vector<GLfloat> vertices;
	    for (size_t slice = 0; slice < LOD; ++slice)
	      {		
		GLfloat angle = slice * 2.0f * M_PI / LOD;
		GLfloat x = 0.5f * std::sin(angle);
		GLfloat y = 0.5f * std::cos(angle);
		
		//Head cone vertex
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(1 - head_length_ratio);

		//Head-cylinder vertex
		vertices.push_back(x * body_radius_ratio);
		vertices.push_back(y * body_radius_ratio);
		vertices.push_back(1 - head_length_ratio);

		//tail-cylinder vertex
		vertices.push_back(x * body_radius_ratio);
		vertices.push_back(y * body_radius_ratio);
		vertices.push_back(0);
	      }

	    //Add the point vertex for the arrow
	    vertices.push_back(0); vertices.push_back(0); vertices.push_back(1);

	    return vertices;
	  }

	  inline static std::vector<GLfloat> getNormals(size_t LOD)
	  {
	    std::vector<GLfloat> normals;

	    for (size_t slice = 0; slice < LOD; ++slice)
	      {		
		GLfloat angle = slice * 2.0f * M_PI / LOD;
		GLfloat x = std::sin(angle);
		GLfloat y = std::cos(angle);
		
		//Head cone vertex
		normals.push_back(x);
		normals.push_back(y);
		normals.push_back(0);

		//Head-cylinder vertex
		normals.push_back(x);
		normals.push_back(y);
		normals.push_back(0);

		//tail-cylinder vertex
		normals.push_back(x);
		normals.push_back(y);
		normals.push_back(0);
	      }

	    normals.push_back(0); normals.push_back(0); normals.push_back(1);

	    return normals;
	  }

	  inline static std::vector<GLuint> getIndices(size_t LOD)
	  {
	    //This is the index of the point vertex, and also the
	    //number of body vertices
	    size_t point_vertex = LOD * 3;

	    std::vector<GLuint> indices;

	    for (size_t slice = 0; slice < LOD; ++slice)
	      {
		//The cone triangle is first
		indices.push_back(point_vertex);
		indices.push_back((3 * (slice+1) + 0) % point_vertex);
		indices.push_back((3 * slice + 0) % point_vertex);

		//The body triangles
		indices.push_back((3 * (slice + 1) + 1) % point_vertex);
		indices.push_back((3 * slice + 2) % point_vertex);
		indices.push_back((3 * slice + 1) % point_vertex);

		indices.push_back((3 * (slice + 1) + 1) % point_vertex);
		indices.push_back((3 * (slice + 1) + 2) % point_vertex);
		indices.push_back((3 * slice + 2) % point_vertex);
	      }

	    return indices;
	  }

	protected:
	};
      }
    }
  }
}