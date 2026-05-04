#include <iostream>
#include <memory>

#include <lvk/LVK.h>
#include <lvk/HelpersImGui.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "sphere.h"
#include "model.h"
#include "camera.h"
#include "terrain.h"

// TODO: New terrain shader for greyscale
// TODO: Research procedural terrain generation and implement one algorithm
// TODO: Texturing Terrain


static bool resized = false;
static bool showWireframe = true;
static bool drawMesh = false;
static float chromaticAberrationStrength = 0.003f;
static float exposure = 1.5f;

static void renderGui(lvk::ImGuiRenderer& imgui, lvk::Framebuffer& framebuff, lvk::ICommandBuffer& cmdBuff)
{
	imgui.beginFrame(framebuff);
	{
		ImGui::Begin("Test");
		ImGui::Checkbox("Draw Mesh", &drawMesh);
		ImGui::Checkbox("Show Wireframe", &showWireframe);
		ImGui::SliderFloat("Chromatic Aberration", &chromaticAberrationStrength, 0.0f, 0.1f);
		ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f);
		ImGui::End();
	}
	imgui.endFrame(cmdBuff);
}

static void onFrameBufferResize(GLFWwindow* window, int width, int height)
{
	resized = true;
}

static void setMouseCallbacks(GLFWwindow* window)
{
	glfwSetCursorPosCallback(window, [](auto* window, double x, double y) { ImGui::GetIO().MousePos = ImVec2((float)x, (float)y); });
	glfwSetMouseButtonCallback(window, [](auto* window, int button, int action, int mods) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
			? ImGuiMouseButton_Left
			: (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2((float)xpos, (float)ypos);
		io.MouseDown[imguiButton] = action == GLFW_PRESS;
		});
}

int main()
{
	int width = 800;
	int height = 600;

	GLFWwindow* window = lvk::initWindow("Terrain", width, height, true);
	if (!window)
	{
		std::cerr << "Window is invalid.\n";
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwSetFramebufferSizeCallback(window, &onFrameBufferResize);

	{
		// context
		lvk::ContextConfig config{};
		config.enableValidation = true;
		config.presentModes[0] = lvk::PresentMode_FIFO;

		std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, config);

		std::unique_ptr<lvk::ImGuiRenderer> imGuiCtx = std::make_unique<lvk::ImGuiRenderer>(*ctx, window, nullptr, 13.0f);

		setMouseCallbacks(window);

		// shaders
		lvk::Holder<lvk::ShaderModuleHandle> vertShader = ShaderProcessor::loadShaderModule(*ctx, (std::string(SHADERS_DIR) + "/default.vert").c_str());
		lvk::Holder<lvk::ShaderModuleHandle> fragShader = ShaderProcessor::loadShaderModule(*ctx, (std::string(SHADERS_DIR) + "/default.frag").c_str());
		lvk::Holder<lvk::ShaderModuleHandle> postVertShader = ShaderProcessor::loadShaderModule(*ctx, (std::string(SHADERS_DIR) + "/post.vert").c_str());
		lvk::Holder<lvk::ShaderModuleHandle> postFragShader = ShaderProcessor::loadShaderModule(*ctx, (std::string(SHADERS_DIR) + "/post.frag").c_str());

		// depth texture
		lvk::TextureDesc depthTexDesc{};
		depthTexDesc.type = lvk::TextureType_2D;
		depthTexDesc.format = lvk::Format_Z_F32;
		depthTexDesc.dimensions = { (uint32_t)width, (uint32_t)height };
		depthTexDesc.usage = lvk::TextureUsageBits_Attachment;
		depthTexDesc.debugName = "Depth Buffer";
		lvk::Holder<lvk::TextureHandle> depthTexture = ctx->createTexture(depthTexDesc);

		// off-screen texture
		lvk::TextureDesc offScreenDesc{};
		offScreenDesc.type = lvk::TextureType_2D;
		offScreenDesc.format = ctx->getSwapchainFormat();
		offScreenDesc.dimensions = { (uint32_t)width, (uint32_t)height };
		offScreenDesc.usage = lvk::TextureUsageBits_Attachment | lvk::TextureUsageBits_Sampled;
		offScreenDesc.debugName = "Offscreen Buffer";
		lvk::Holder<lvk::TextureHandle> offScreenTexture = ctx->createTexture(offScreenDesc);

		// terrain buffer data
		Terrain terrain{ *ctx };
		terrain.heightScale = 300.0f;
		terrain.terrainScale = 1000.0f;
		terrain.loadHeightmap((std::string(RESOURCE_DIR) + "/heightmaps/default.png").c_str());
		terrain.generateTerrain(511, 511);

		// Attribute layout
		const lvk::VertexInput vdesc = {
			.attributes = {
				{
					.location = 0,
					.format = lvk::VertexFormat::VertexFormat_Float3,
					.offset = offsetof(Vertex, position)
				},
				{
					.location = 1,
					.format = lvk::VertexFormat::VertexFormat_Float3,
					.offset = offsetof(Vertex, normal)
				},
				{
					.location = 2,
					.format = lvk::VertexFormat::VertexFormat_Float2,
					.offset = offsetof(Vertex, uv)
				},

			},
			.inputBindings = { {.stride = sizeof(Vertex) } }
		};

		// Pipelines
		lvk::RenderPipelineDesc pipelineDesc{};
		pipelineDesc.vertexInput = vdesc;
		pipelineDesc.smVert = vertShader;
		pipelineDesc.smFrag = fragShader;
		pipelineDesc.color[0].format = ctx->getSwapchainFormat();
		pipelineDesc.depthFormat = ctx->getFormat(depthTexture);
		// solid pipeline
		lvk::Holder<lvk::RenderPipelineHandle> solidPipeline = ctx->createRenderPipeline(pipelineDesc);
		// wireframe pipeline
		uint32_t isWireframe = 1;
		lvk::SpecializationConstantEntry wireframeSpecInfoEntry{};
		wireframeSpecInfoEntry.constantId = 0;
		wireframeSpecInfoEntry.size = sizeof(uint32_t);

		lvk::RenderPipelineDesc wireframePipelineDesc{};
		wireframePipelineDesc.vertexInput = vdesc;
		wireframePipelineDesc.smVert = vertShader;
		wireframePipelineDesc.smFrag = fragShader;
		wireframePipelineDesc.color[0].format = ctx->getSwapchainFormat();
		wireframePipelineDesc.depthFormat = ctx->getFormat(depthTexture);
		wireframePipelineDesc.polygonMode = lvk::PolygonMode_Line;
		wireframePipelineDesc.specInfo.entries[0] = wireframeSpecInfoEntry;
		wireframePipelineDesc.specInfo.data = &isWireframe;
		wireframePipelineDesc.specInfo.dataSize = sizeof(isWireframe);
		lvk::Holder<lvk::RenderPipelineHandle> wireframePipeline = ctx->createRenderPipeline(wireframePipelineDesc);

		// post-process pipeline
		lvk::RenderPipelineDesc postPipelineDesc{};
		postPipelineDesc.smVert = postVertShader;
		postPipelineDesc.smFrag = postFragShader;
		postPipelineDesc.color[0].format = ctx->getSwapchainFormat();
		lvk::Holder<lvk::RenderPipelineHandle> postPipeline = ctx->createRenderPipeline(postPipelineDesc);

		LVK_ASSERT(solidPipeline.valid());
		LVK_ASSERT(wireframePipeline.valid());
		LVK_ASSERT(postPipeline.valid());

		// Uniform buffer
		lvk::Holder<lvk::BufferHandle> uniformBuf = ctx->createBuffer(
			{ .usage = lvk::BufferUsageBits_Uniform,
			  .storage = lvk::StorageType_Device,
			  .size = sizeof(UniformData),
			  .debugName = "Buffer: per-frame" },
			nullptr);

		// Camera
		Camera cam{ glm::vec3(0.0f, 1.0f, 0.75f), glm::vec3(0.0f, 0.0f, 0.0f) };
		cam.assignWindowObject(window);

		double timeStamp = glfwGetTime();
		float deltaSeconds = 0.0f;

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			const double currTimeStamp = glfwGetTime();
			deltaSeconds = static_cast<float>(currTimeStamp - timeStamp);
			timeStamp = currTimeStamp;

			glfwGetFramebufferSize(window, &width, &height);
			if (!width || !height)
			{
				continue;
			}

			if (resized)
			{
				std::cout << "Window resize detected recreating swapchain.\n";
				ctx->recreateSwapchain(width, height);

				lvk::TextureDesc resizedDepthTexDesc{};
				resizedDepthTexDesc.type = lvk::TextureType_2D;
				resizedDepthTexDesc.format = lvk::Format_Z_F32;
				resizedDepthTexDesc.dimensions = { (uint32_t)width, (uint32_t)height };
				resizedDepthTexDesc.usage = lvk::TextureUsageBits_Attachment;
				resizedDepthTexDesc.debugName = "Depth Buffer";
				depthTexture = ctx->createTexture(resizedDepthTexDesc);

				resized = false;
			}

			const float ratio = width / static_cast<float>(height);
			cam.setAspectRatio(ratio);
			cam.handleInput(window, deltaSeconds);

			// transform
			glm::vec3 meshPosition{ 0.0f, 0.0f, 0.0f };
			glm::vec3 meshScale{ 1.0f, 1.0f, 1.0f };
			glm::mat4 model = glm::mat4(1.0f); // identity
			model = glm::translate(model, meshPosition);
			const float rotationSpeed = 15.0f;
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * rotationSpeed * 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, meshScale);

			// uniform
			UniformData uniformData{};
			uniformData.cameraPosition = glm::vec4(cam.getCameraPosition(), 1.0f);
			uniformData.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			uniformData.model = model;
			uniformData.view = cam.getViewMatrix();
			uniformData.proj = cam.getProjMatrix();
			uniformData.chromaticAberrationStrength = chromaticAberrationStrength;
			uniformData.exposure = exposure;
			uniformData.outputTexId = offScreenTexture.index();

			// 1st Pass
			lvk::RenderPass scenePass{};
			scenePass.color[0].loadOp = lvk::LoadOp_Clear;
			scenePass.color[0].storeOp = lvk::StoreOp_Store;
			scenePass.color[0].clearColor.float32[0] = 0.8f;
			scenePass.color[0].clearColor.float32[1] = 0.8f;
			scenePass.color[0].clearColor.float32[2] = 0.8f;
			scenePass.color[0].clearColor.float32[3] = 1.0f;
			scenePass.depth.loadOp = lvk::LoadOp_Clear;
			scenePass.depth.clearDepth = 1.0f;

			lvk::Framebuffer frameBuf{};
			frameBuf.color[0].texture = offScreenTexture;
			frameBuf.depthStencil.texture = depthTexture;

			lvk::ICommandBuffer& cmd = ctx->acquireCommandBuffer();
			cmd.cmdUpdateBuffer(uniformBuf, uniformData);
			cmd.cmdBeginRendering(scenePass, frameBuf);
			cmd.cmdPushDebugGroupLabel("Render Mesh", 0xff0000ff);
			{
				// Bindings
				cmd.cmdBindVertexBuffer(0, terrain.getTerrainData().vertexBuffer);
				cmd.cmdBindIndexBuffer(terrain.getTerrainData().indexBuffer, lvk::IndexFormat_UI32);
				// soild pipeline
				cmd.cmdBindRenderPipeline(solidPipeline);
				cmd.cmdBindDepthState({ .compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true });
				{
					cmd.cmdPushConstants(ctx->gpuAddress(uniformBuf));
					if (drawMesh)
						cmd.cmdDrawIndexed((uint32_t)terrain.getTerrainData().indices.size());
				}

				if (showWireframe)
				{
					cmd.cmdBindRenderPipeline(wireframePipeline);
					cmd.cmdSetDepthBiasEnable(true);
					cmd.cmdSetDepthBias(0.0f, -1.0f, 0.0f);
					cmd.cmdDrawIndexed((uint32_t)terrain.getTerrainData().indices.size());
				}
			}
			cmd.cmdPopDebugGroupLabel();
			cmd.cmdEndRendering();

			// 2nd Pass
			lvk::RenderPass postPass;
			postPass.color[0].loadOp = lvk::LoadOp_DontCare;

			lvk::Framebuffer finalFrameBuf;
			finalFrameBuf.color[0].texture = ctx->getCurrentSwapchainTexture();

			cmd.cmdBeginRendering(postPass, finalFrameBuf);
			cmd.cmdPushDebugGroupLabel("Post Process Pass", 0x00ff00ff);
			{
				cmd.cmdBindRenderPipeline(postPipeline);
				cmd.cmdBindDepthState({ .compareOp = lvk::CompareOp_AlwaysPass, .isDepthWriteEnabled = false });
				cmd.cmdDraw(3);
			}

			renderGui(*imGuiCtx, finalFrameBuf, cmd);

			cmd.cmdPopDebugGroupLabel();
			cmd.cmdEndRendering();

			// present
			ctx->submit(cmd, ctx->getCurrentSwapchainTexture());
		}
		// End render loop
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}