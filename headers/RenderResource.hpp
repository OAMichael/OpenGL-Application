#ifndef RENDER_RESOURCE_HPP
#define RENDER_RESOURCE_HPP

#define INVALID_HANDLE 0xFFFFFFFFFFFFFFFFu

#include <string>
#include <unordered_set>

namespace Resources {

struct ResourceHandle {
	uint64_t nativeHandle = INVALID_HANDLE;

	bool operator==(const ResourceHandle& other) const { return nativeHandle == other.nativeHandle; }
};

struct RenderResource {
	
	enum ResourceType : int {
		INVALID_RESOURCE = -1,
		BUFFER = 0,
		IMAGE = 1,
		SAMPLER = 2,
		TEXTURE = 3,
		MATERIAL = 4,
		SHADER = 5,
		FRAMEBUFFER = 6
	};

	ResourceType type = INVALID_RESOURCE;

	std::string name;
	std::string uri;

	ResourceHandle handle;
};

struct RenderResourceDesc {

	std::string name = "";
	std::string uri = "";
};

}

template<>
struct std::hash<Resources::ResourceHandle>
{
	std::size_t operator()(const Resources::ResourceHandle& handle) const noexcept
	{
		return std::hash<uint64_t>{}(handle.nativeHandle);
	}
};


#endif // RENDER_RESOURCE_HPP