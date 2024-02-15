#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <string>
#include <vector>

#include <RenderResource.hpp>

namespace Resources {

struct Buffer : RenderResource {

	unsigned int GL_id;
	unsigned int target;

	std::vector<unsigned char> data;

	Buffer() {};
};

}

#endif