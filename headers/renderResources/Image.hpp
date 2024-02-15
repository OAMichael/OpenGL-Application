#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>
#include <vector>

#include <RenderResource.hpp>

namespace Resources {

struct Image : RenderResource {
	
	int width = -1;
	int height = -1;
	int components = -1;
	int bits = -1;
	int format = -1;

	std::vector<unsigned char> image;

	Image() {};

	enum DefaultImages : uint32_t {
		DEFAULT_IMAGE_WHITE = 0,
		DEFAULT_IMAGE_BLACK = 1,

		COUNT
	};
};


extern std::string defaultImagesNames[Image::DefaultImages::COUNT];

}
#endif
