#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "Texture.hpp"

#include <RenderResource.hpp>

namespace Resources {

struct Framebuffer : RenderResource {
	static const unsigned MAXIMUM_COLOR_ATTACHMENTS_COUNT = 4;

	unsigned GL_id;

	Texture* colorAttachments[MAXIMUM_COLOR_ATTACHMENTS_COUNT];
};


extern std::string defaultFramebufferName;

}
#endif
