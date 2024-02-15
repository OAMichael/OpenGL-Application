#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "Texture.hpp"

#include <RenderResource.hpp>

namespace Resources {

struct Framebuffer : RenderResource {
	static const unsigned MAXIMUM_COLOR_ATTACHMENTS_COUNT = 4;

	unsigned GL_id;
	size_t colorAttachmentsCount;

	Texture* colorAttachments[MAXIMUM_COLOR_ATTACHMENTS_COUNT];
	Texture* depthAttachment;
};


extern std::string defaultFramebufferName;

}
#endif
