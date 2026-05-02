#include "shader.h"

lvk::Holder<lvk::ShaderModuleHandle> ShaderProcessor::loadShaderModule(lvk::IContext& ctx, const char* path)
{
	const std::string code = readShaderFile(path);
	const lvk::ShaderStage stage = getShaderStage(path);

	if (code.empty())
	{
		LLOGW("Shader file is empty: %s\n", path);
		return {};
	}

	lvk::Result result;

	lvk::ShaderModuleDesc desc{ code.c_str(), stage, path };
	lvk::Holder<lvk::ShaderModuleHandle> handle = ctx.createShaderModule(desc, &result);

	if (!result.isOk())
	{
		LLOGW("Shader result is not OK: %s\n", path);
		return {};
	}

	LLOGL("Loaded shader module from file: %s\n", path);

	return handle;
}

std::string ShaderProcessor::readShaderFile(const char* path)
{
	std::unordered_set<std::string> guard;
	return readTextFile(path, guard);
}

lvk::ShaderStage ShaderProcessor::getShaderStage(const char* path)
{
	const auto file = std::filesystem::path(path);
	const auto extension = file.extension().string();

	if (extension == ".vert") return lvk::Stage_Vert;
	if (extension == ".frag") return lvk::Stage_Frag;
	if (extension == ".geom") return lvk::Stage_Geom;
	if (extension == ".comp") return lvk::Stage_Comp;
	if (extension == ".tesc") return lvk::Stage_Tesc;
	if (extension == ".tese") return lvk::Stage_Tese;

	LLOGW("Unknown shader extension: %s\n", extension.c_str());
	return lvk::Stage_Vert;
}

std::string ShaderProcessor::readTextFile(const char* path, std::unordered_set<std::string>& includeGuard)
{
	if (!includeGuard.insert(path).second)
	{
		LLOGW("Circular include detected: %s\n", path);
		return {};
	}

	std::ifstream in(path, std::ios::binary);
	if (!in)
	{
		LLOGW("Failed to open text file %s\n", path);
		return {};
	}

	std::string code;
	in.seekg(0, std::ios::end);
	code.reserve(in.tellg());
	in.seekg(0, std::ios::beg);
	code.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

	// Remove UTF-8 BOM
	if (code.size() >= 3 &&
		static_cast<unsigned char>(code[0]) == 0xEF &&
		static_cast<unsigned char>(code[1]) == 0xBB &&
		static_cast<unsigned char>(code[2]) == 0xBF)
	{
		code.erase(0, 3);
	}

	// Handle #include <file>
	size_t pos = 0;
	while ((pos = code.find("#include", pos)) != std::string::npos)
	{
		const auto start = code.find('<', pos);
		const auto end = code.find('>', start);

		if (start == std::string::npos || end == std::string::npos)
			break;

		std::string includeFile = code.substr(start + 1, end - start - 1);

		// Build include path relative to current file’s directory
		std::string basePath(path);
		auto lastSlash = basePath.find_last_of("/\\");
		std::string dir = (lastSlash != std::string::npos) ? basePath.substr(0, lastSlash) : "";
		std::string includePath = dir.empty() ? includeFile : (dir + "/" + includeFile);

		std::string includeCode = readTextFile(includePath.c_str(), includeGuard);

		code.replace(pos, end - pos + 1, includeCode);
		pos += includeCode.size(); // advance past inserted code
	}

	return code;
}