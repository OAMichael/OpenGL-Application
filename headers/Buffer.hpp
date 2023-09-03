#ifndef BUFFER_HPP
#define BUFFER_HPP


#include <string>
#include <vector>

namespace Resources {

struct Buffer {
	unsigned int GL_id;
	std::string name;

	unsigned int target;

	std::vector<unsigned char> data;

	Buffer() {};
};

}

#endif