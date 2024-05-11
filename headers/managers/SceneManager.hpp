#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <unordered_map>

#include "SceneNode.hpp"
#include "Cube.hpp"
#include "Light.hpp"

#include <freetype/ft2build.h>
#include <freetype/freetype.h>

namespace SceneResources {

inline constexpr const char* FULLSCREEN_QUAD_SHADER_NAME	= "Fullscreen_Quad";
inline constexpr const char* ENVIRONMENT_SHADER_NAME		= "Default_Environment";
inline constexpr const char* GAUSSIAN_BLUR_SHADER_NAME		= "Gaussian_Blur";
inline constexpr const char* BLOOM_SHADER_NAME				= "Bloom";
inline constexpr const char* BLOOM_FINAL_SHADER_NAME		= "Bloom_Final";
inline constexpr const char* TEXT_RENDERING_SHADER_NAME		= "Render_Text";
inline constexpr const char* IRRADIANCE_MAP_SHADER_NAME		= "Irradiance_Map";
inline constexpr const char* PREFILTER_HDR_SHADER_NAME		= "Prefilter_HDR";
inline constexpr const char* BRDF_LUT_SHADER_NAME			= "BRDF_LUT";
inline constexpr const char* PREVIEW_SCREEN_SHADER_NAME		= "Preview_Screen";
inline constexpr const char* SDF_SCENE_SHADER_NAME			= "SDF_Scene_Shader";
inline constexpr const char* BACKGROUND_2D_TEXTURE_NAME		= "BACKGROUND_2D_TEXTURE";
inline constexpr const char* SKYBOX_TEXTURE_NAME			= "SKYBOX_TEXTURE";
inline constexpr const char* EQUIRECTANGULAR_TEXTURE_NAME	= "EQUIRECTANGULAR_TEXTURE";


struct LightDesc {
	std::string name = "";

	glm::vec4 color = glm::vec4(0.0);
	glm::vec3 position = glm::vec3(0.0);
	glm::vec3 direction = glm::vec3(0.0);
	float cutoff_angle = 0.0;

	SceneLight::LightType type;
};

class SceneManager final {
public:
	enum EnvironmentType : uint32_t {
		BACKGROUND_IMAGE_2D = 0,
		SKYBOX,
		EQUIRECTANGULAR,

		COUNT
	};

	struct PostProcessInfo {
		bool enableBlur;
		bool enableBloom;
		unsigned windowWidth;
		unsigned windowHeight;
	};

	SceneManager(const SceneManager& obj) = delete;

	static SceneManager* getInstance() {
		if (!instancePtr)
			instancePtr = new SceneManager();

		return instancePtr;
	}

	static constexpr const char* ROOT_NODE_NAME = "RootNode";

	SceneNode& createRootNode();
	SceneNode& getRootNode();

	SceneNode& createSceneNode(const std::string& name);
	SceneLight& createSceneLight(const LightDesc& lightDesc);

	void deleteSceneNode(const SceneHandle handle);
	void deleteSceneLight(const SceneHandle handle);

	inline SceneNode& getSceneNode(const SceneHandle handle) { return *sceneNodes_[handle]; }
	inline SceneLight& getSceneLight(const SceneHandle handle) { return *sceneLights_[handle]; }

	bool hasSceneNode(const std::string& name);
	bool hasSceneLight(const std::string& name);

	void updateLights();
	inline const LightData& getLightData() const { return lightData_; }

	void createEnvironment(const EnvironmentType envType, const std::vector<std::string>& textureNames, bool isHdr = false);
	void createEnvironment(const EnvironmentType envType, const std::string& textureName, bool isHdr = false);
	void createImageBasedLightingTextures(const EnvironmentType envType);
	void drawEnvironment();

	void createBackground2D(const std::string& textureName, bool isHdr = false);
	void drawBackground2D();

	void createSkybox(const std::vector<std::string>& textureNames, bool isHdr = false);
	void drawSkybox();

	void createEquirectangular(const std::string& textureName, bool isHdr = false);
	void drawEquirectangular();

	inline const Resources::ResourceHandle getBackground2DHandle() const { return Background2DHandle_; }
	inline const Resources::ResourceHandle getSkyboxHandle() const { return SkyboxHandle_; }
	inline const Resources::ResourceHandle getEquirectangularHandle() const { return EquirectHandle_; }

	inline EnvironmentType getEnvironmentType() { return envType_; }
	inline void setEnvironmentType(EnvironmentType envType) { envType_ = envType; }

	inline void setEnableBlur(bool enabled = true) { postProcessInfo_.enableBlur = enabled; }
	inline void setEnableBloom(bool enabled = true) { postProcessInfo_.enableBloom = enabled; }

	inline bool getEnableBlur() const { return postProcessInfo_.enableBlur; };
	inline bool getEnableBloom() const { return postProcessInfo_.enableBloom; };

	void createPostProcess(const PostProcessInfo& ppi);
	void performPostProcess(const Resources::ResourceHandle inputTextureHandle);
	void createFullscreenQuad();
	void drawFullscreenQuad(const Resources::ResourceHandle inputTextureHandle, Resources::Shader* shader = nullptr);
	void drawToDefaultFramebuffer(const Resources::ResourceHandle inputTextureHandle);

	inline const Resources::ResourceHandle getPostProcessTextureHandle() const { return postProcessTextureHandle_; }
	inline const Resources::ResourceHandle getIrradianceMapSkyboxTextureHandle() const { return irradianceMapSkyboxTextureHandle_; }
	inline const Resources::ResourceHandle getIrradianceMapEquirectTextureHandle() const { return irradianceMapEquirectTextureHandle_; }
	inline const Resources::ResourceHandle getPrefilterHDRMapSkyboxTextureHandle() const { return prefilterHDRSkyboxTextureHandle_; }
	inline const Resources::ResourceHandle getPrefilterHDRMapEquirectTextureHandle() const { return prefilterHDREquirectTextureHandle_; }
	inline const Resources::ResourceHandle getBRDFLUTSkyboxTextureHandle() const { return brdfLUTSkyboxTextureHandle_; }
	inline const Resources::ResourceHandle getBRDFLUTEquirectTextureHandle() const { return brdfLUTEquirectTextureHandle_; }

	bool initializeFreeType(const std::string& fontFilename, const unsigned fontHeight = 48);
	void setTextProjectionMatrix(const glm::mat4 proj);
	void drawText(const std::string& text, float x, float y, float scale, glm::vec3 color);

	void createPreviewScreen();
	void drawPreviewScreen(const Resources::ResourceHandle textureHandle, const float alpha = 1.0f);

	void initializeSDFScene();
	void drawSDFScene(const glm::mat4& invCameraMatrix, const float winWidth, const float winHeight, const float time);

	void cleanUp();

	~SceneManager() { cleanUp(); }

private:
	std::unordered_map<SceneHandle, SceneNode*> sceneNodes_;
	std::unordered_map<SceneHandle, SceneLight*> sceneLights_;

	LightData lightData_;

	std::unordered_map<SceneHandle, ISceneObject*> sceneObjects_;

	SceneNode* rootNode_ = nullptr;

	unsigned VAOFullscreenQuad_ = 0;
	unsigned VBOFullscreenQuad_ = 0;

	unsigned VAODefaultQuad_ = 0;
	unsigned VBODefaultQuad_ = 0;
	unsigned VAODefaultCube_ = 0;
	unsigned VBODefaultCube_ = 0;

	Geometry::Cube defaultCube_{glm::vec3(0.0f)};

	Resources::ResourceHandle Background2DHandle_;
	Resources::ResourceHandle SkyboxHandle_;
	Resources::ResourceHandle EquirectHandle_;

	EnvironmentType envType_;
	std::array<uint32_t, EnvironmentType::COUNT> envHdr_ = {};
	int irradianceMapSize_ = 32;
	int prefilteredHDRMapSize_ = 256;
	unsigned maxMipLevelsPrefilterHDR_ = 5;
	int brdfLUTSize_ = 512;

	struct FreeTypeCharacter {
		Resources::ResourceHandle textureHandle;
		glm::ivec2 size;
		glm::ivec2 bearing;
		unsigned advance;
	};

	std::unordered_map<char, FreeTypeCharacter> freeTypeChars_;
	unsigned VAOTextQuad_;
	unsigned VBOTextQuad_;
	glm::mat4 textProjMat_;

	PostProcessInfo postProcessInfo_ = {};
	Resources::ResourceHandle blurXFramebufferHandle_;
	Resources::ResourceHandle blurYFramebufferHandle_;
	Resources::ResourceHandle bloomFramebufferHandle_;
	Resources::ResourceHandle bloomFinalFramebufferHandle_;

	Resources::ResourceHandle postProcessTextureHandle_;
	Resources::ResourceHandle irradianceMapSkyboxTextureHandle_;
	Resources::ResourceHandle irradianceMapEquirectTextureHandle_;
	Resources::ResourceHandle prefilterHDRSkyboxTextureHandle_;
	Resources::ResourceHandle prefilterHDREquirectTextureHandle_;
	Resources::ResourceHandle brdfLUTSkyboxTextureHandle_;
	Resources::ResourceHandle brdfLUTEquirectTextureHandle_;
	Resources::ResourceHandle rgbaNoiseTextureHandle_;
	Resources::ResourceHandle rockTextureHandle_;

	Resources::ResourceHandle fullscreenShaderHandle_;
	Resources::ResourceHandle environmentShaderHandle_;
	Resources::ResourceHandle gaussianBlurShaderHandle_;
	Resources::ResourceHandle bloomShaderHandle_;
	Resources::ResourceHandle bloomFinalShaderHandle_;
	Resources::ResourceHandle textRenderingShaderHandle_;
	Resources::ResourceHandle previewScreenShaderHandle_;
	Resources::ResourceHandle SDFSceneShaderHandle_;

	void initializeDefaultCube();
	void drawDefaultCube();

	void initializeDefaultQuad();
	void drawDefaultQuad();

	static SceneManager* instancePtr;
	SceneManager() {};
};

}

#endif // SCENE_MANAGER_HPP