#ifndef __fleye_ShaderPass_H_
#define __fleye_ShaderPass_H_

#include "fleye/config.h"
#include "fleye/compiledshader.h"
#include "fleye/textureinput.h"
#include "fleye/plugin.h"

#include <vector>

struct GLTexture;
struct FrameBufferObject;

struct ShaderPass
{
	std::vector<TextureInput> inputs;
	std::string vertexSource;
	std::string fragmentSourceWithoutTextures;
	std::vector<CompiledShader> shaderCahe;
	std::vector<FrameBufferObject*> fboPool;

	GLTexture* finalTexture;	
	FleyePlugin* drawPlugin;
	int numberOfPasses;
	int passStartIndex; // offset to pass index, to enable multipass algorithms using different shaders
	
	inline ShaderPass() : drawPlugin(0), numberOfPasses(0), passStartIndex(0), finalTexture(0) {}
};

#endif
