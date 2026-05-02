#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_set>
#include <filesystem>

#include <lvk/LVK.h>

class ShaderProcessor
{
public:
	static lvk::Holder<lvk::ShaderModuleHandle> loadShaderModule(lvk::IContext& ctx, const char* path);

private:
	static std::string readShaderFile(const char* path);
	static lvk::ShaderStage getShaderStage(const char* path);
	static std::string readTextFile(const char* path, std::unordered_set<std::string>& includeGuard);
};