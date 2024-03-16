#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include <string>

#include <RenderResource.hpp>

namespace Resources {

struct Sampler : RenderResource {
	enum Filter : uint32_t {
		NEAREST = GL_NEAREST,
		LINEAR = GL_LINEAR,
		NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
	};

	enum WrapMode : uint32_t {
		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
		CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
		MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
		REPEAT = GL_REPEAT,
#ifndef __ANDROID__
		MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE
#endif
	};

	unsigned int GL_id;

	uint32_t minFilter = Filter::NEAREST_MIPMAP_LINEAR;
	uint32_t magFilter = Filter::LINEAR;

	uint32_t wrapS = WrapMode::REPEAT;
	uint32_t wrapT = WrapMode::REPEAT;
	uint32_t wrapR = WrapMode::REPEAT;

	enum DefaultSamplers : uint32_t {
		DEFAULT_SAMPLER_NEAREST_CLAMP  = 0,
		DEFAULT_SAMPLER_NEAREST_REPEAT = 1,
		DEFAULT_SAMPLER_LINEAR_REPEAT  = 2,
		DEFAULT_SAMPLER_LINEAR_CLAMP   = 3,
		DEFAULT_SAMPLER_LINEAR_MIPMAP_LINEAR_CLAMP = 4,

		COUNT
	};
};

extern std::string defaultSamplersNames[Sampler::DefaultSamplers::COUNT];

}

#endif
