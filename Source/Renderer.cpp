#include "Renderer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void APIENTRY glDebugOutput(GLenum source,GLenum type,unsigned int id,GLenum severity,GLsizei length,const char* message,const void* userParam);

Render::Render()
{
}

void Render::initWindow()
{
	stbi_set_flip_vertically_on_load(true);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); // Debug stuff ISJUNGTI KAI ZIURIM FPS KIEK!!!

	window = glfwCreateWindow(Settings::width, Settings::height, "AmazingOpengl", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Couldn't make a window!" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Couldn't load GLAD" << std::endl;
		glfwTerminate();
	}

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
	{
		std::cout << "Failed to intialise debugger" << std::endl;
	}

	// glEnable(GL_CULL_FACE); // Face culling turn on, on real game.
	// glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glClearDepth(0.0);
	glViewport(0, 0, Settings::width, Settings::height);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	camera.init(glm::vec3(-Settings::BLOCK_COUNT_X - 2, -Settings::BLOCK_COUNT_Y, -Settings::BLOCK_COUNT_Z), glm::vec3(1.0f, 0.0f, 0.0f), Settings::walkspeed, Settings::fov);

	///////////////////
	// We register the callback functions after we've created the window and before the render loop is initiated.
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

}

void Render::initBuffers(World& world)
{
	// VAOs ir VBOs
	VAO.generate();

	VBOoutline.generate();
	prevVBOoutline.generate();

	VAO.bindVBO(VBOinfo(VBOoutline, sizeof(glm::mat4), 0,1,std::vector<AttributeInfo>{
		AttributeInfo(0,4,GL_FLOAT,GL_FALSE,0),
		AttributeInfo(1,4,GL_FLOAT,GL_FALSE,sizeof(glm::vec4)),
		AttributeInfo(2,4,GL_FLOAT,GL_FALSE,2 * sizeof(glm::vec4)),
		AttributeInfo(3,4,GL_FLOAT,GL_FALSE,3 * sizeof(glm::vec4))
	}));
	VAO.bindVBO(VBOinfo(prevVBOoutline, sizeof(glm::mat4), 0, 1, std::vector<AttributeInfo>{
		AttributeInfo(4,4, GL_FLOAT, GL_FALSE, 0),
		AttributeInfo(5,4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4)),
		AttributeInfo(6,4, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec4)),
		AttributeInfo(7,4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4))
	}));

	// Compute Shader Buffers, visi IMMUTABLE! Isskyrus 2


	cullInputBuffer.generate();
	cullInputBuffer.data(sizeof(chunkInput) * world.chunkComputeData.size(), world.chunkComputeData.data(), GL_DYNAMIC_STORAGE_BIT, 0, true);

	cullOutputBuffer.generate();
	cullOutputBuffer.data(sizeof(DrawArraysIndirectCommand) * world.chunks.size(), (DrawArraysIndirectCommand*)nullptr, GL_DYNAMIC_STORAGE_BIT, 1, true);

	cullCountBuffer.generate();
	cullCountBuffer.data(sizeof(uint32_t), (uint32_t*)nullptr, GL_STREAM_COPY, 2, false);

	VBOfaces.generate();
	VBOfaces.data(sizeof(uint32_t) * world.faceCoordsandData.size(), world.faceCoordsandData.data(), GL_DYNAMIC_STORAGE_BIT, 3, true);

	mappingBuffer.generate();
	mappingBuffer.data(sizeof(uint32_t) * world.chunks.size(), (uint32_t*)nullptr, GL_DYNAMIC_STORAGE_BIT, 4, true);

	textBuffer.generate();
	textBuffer.data(100000 * sizeof(glyphVertex), (glyphVertex*)nullptr, GL_DYNAMIC_STORAGE_BIT, 5, true);

	// Framebuffers

	previousVelocityTex.generate();
	previousVelocityTex.settings(GL_RG32F, GL_RG, Settings::width, Settings::height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, false, false);

	mainFBO.generate();
	mainFBO.initTexture("mainFBO", Settings::width, Settings::height, GL_NEAREST, GL_NEAREST, GL_NEAREST, GL_NEAREST, GL_RGB16F, GL_RGB);
	mainFBO.addColorAttachment(GL_RG32F, GL_RG, Settings::width, Settings::height, GL_NEAREST, GL_NEAREST);

	history1FBO.generate();
	history1FBO.initRenderBuffer("History 1", Settings::width, Settings::height, GL_LINEAR, GL_LINEAR, GL_RGB16F, GL_RGB);

	history2FBO.generate();
	history2FBO.initRenderBuffer("History 2", Settings::width, Settings::height, GL_LINEAR, GL_LINEAR, GL_RGB16F, GL_RGB);
}

void Render::initTextures()
{
	textures.generate();
	std::string path = "./Resource/Textures/block_textures/";
	textures.data(16, 16, true, std::vector<std::string>
	{
		    path + "air.png",
			path + "grassSide.png",
			path + "grassTop.png",
			path + "grassBottom.png",
			path + "stone_bricks.png",
			path + "diamond_block.png",
			path + "blue_wool.png",
			path + "emerald_block.png",
			path + "stripped_crimson_stem_side.png",
			path + "stripped_crimson_stem_top.png",
			path + "pink_concrete_powder.png",
			path + "cherry_planks.png",
	});

	characters.generate();
	characters.data("./Resource/Textures/fonts/minecraft.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, false, false, true);
	UserInterface::loadGlyphs(minecraftCharDatas, "./Font_Metadatas/metadata.json");
}

void Render::initShaders()
{
	program.init("./Resource/Shaders/Rendering/Everything/vertexShader.vert", "./Resource/Shaders/Rendering/Everything/fragmentShader.frag", "Basic");
	hudProgram.init("./Resource/Shaders/UI/HUD/hudVertex.vert", "./Resource/Shaders/UI/HUD/hudfragment.frag", "HUD");
	textProgram.init("./Resource/Shaders/UI/Text/textVertex.vert", "./Resource/Shaders/UI/Text/textFragment.frag", "Text");
	screenProgram.init("./Resource/Shaders/Rendering/Screen/screenVertex.vert", "./Resource/Shaders/Rendering/Screen/screenFragment.frag", "Screen");
	TAAProgram.init("./Resource/Shaders/TAA/taaVertex.vert", "./Resource/Shaders/TAA/taaFragment.frag", "TAA");
	outlineProgram.init("./Resource/Shaders/Rendering/Outline/outlineVertex.vert", "./Resource/Shaders/Rendering/Outline/outlineFragment.frag", "Outline");
	cullProgram.init("./Resource/Shaders/frustumCulling.comp");
	context.set(&cullProgram, &program, &screenProgram, &TAAProgram, &camera, Settings::width / 2, Settings::height / 2, true);
	glfwSetWindowUserPointer(window, &context);

	program.use();
	program.uniformMatrix4fv("model", glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
	program.uniformMatrix4fv("prevModel", glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
	program.uniformMatrix4fv("proj", glm::perspectiveZO(glm::radians(camera.getFov()), (float)Settings::width / (float)Settings::height, Settings::far_plane, Settings::near_plane));
	program.uniformMatrix4fv("prevView", glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp));
	program.uniform1i("TAA", Settings::TAA);
	program.setTextureBindless("ourTexture", textures.handle);

	textProgram.use();
	textProgram.setTextureBindless("u_MSDFtexture", characters.handle);
	textProgram.uniform1f("u_pxRange", 4.0f);
	textProgram.uniform2f("u_textureSize", (float)characters.width, (float)characters.height);
	textProgram.uniformMatrix4fv("u_ortho", glm::ortho(0.0f, (float)Settings::width, 0.0f, (float)Settings::height));
	textProgram.uniform1f("size", 100.0f);
	//textProgram.uniform3f("u_color", glm::vec3(1.0f, 1.0f, 1.0f));

	screenProgram.use();
	screenProgram.uniform1i("currentFrame", 0);
	screenProgram.uniform1f("sharpness", Settings::sharpness);
	screenProgram.uniform1i("cas", Settings::CAS);

	TAAProgram.use();
	TAAProgram.uniform1i("currentFrame", 0);
	TAAProgram.uniform1i("previousFrame", 1);
	TAAProgram.uniform1i("currentVelocityFrame", 2);
	TAAProgram.uniform1i("previousVelocityFrame", 3);
	TAAProgram.uniform1i("depthBuffer", 4);
	TAAProgram.uniform1f("stddevss", Settings::stddev);
	TAAProgram.uniform2f("resolution", Settings::width, Settings::height);

	cullProgram.use();
	cullProgram.uniform1f("far_plane", Settings::far_plane);
	cullProgram.uniform1f("near_plane", Settings::near_plane);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cullOutputBuffer.id);
	glBindBuffer(GL_PARAMETER_BUFFER, cullCountBuffer.id);
	Utils::change_frustrum(cullProgram, program, camera);
	
}

void Render::initRenderVariables(World &world,Player &player)
{
	firstMouse = true;
	xLast = Settings::width / 2;
	yLast = Settings::height / 2;
	taaFrames = 0;
	cullCountNumber = 0;
	currentFrame = glfwGetTime();
	lastFrame = glfwGetTime();
	deltaTime = 0;
	oddframe = true;
	nbFrames = 0;
	lastTime = 0;
	lastFps = 0;

	model = glm::mat4(1.0f);
	proj = glm::perspectiveZO(glm::radians(camera.getFov()), (float)Settings::width / (float)Settings::height, Settings::far_plane, Settings::near_plane);
	noJitterProj = glm::perspectiveZO(glm::radians(camera.getFov()), (float)Settings::width / (float)Settings::height, Settings::far_plane, Settings::near_plane);
	prevProj = noJitterProj;
	view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
	prevView = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);

	jitter = glm::vec2(0.0f);
	offset = glm::vec2(0.0f);

	player.pos = camera.cameraPos;
	player.getBlockLookAtCoords(camera, world);
	currentOutlineModel = glm::translate(glm::mat4(1.0f), player.lookAtBlockCoords);
	Utils::generateOutlineMatrices(currentOutlineModel, prevOutlineMatrices);

	drawMainBuffers[0] = GL_COLOR_ATTACHMENT0;
	drawMainBuffers[1] = GL_COLOR_ATTACHMENT1;
	drawNormalBuffers[0] = GL_COLOR_ATTACHMENT0;

	glBindVertexArray(VAO.id);
}

void Render::init(World& world, Player& player)
{
	Render::initWindow();
	Render::initBuffers(world);
	Render::initTextures();
	Render::initShaders();
	Render::initRenderVariables(world, player);
}

void Render::process_input(GLFWwindow* window,Player &player)
{
	player.pos = camera.cameraPos;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Utils::strafeMoveCheck(window, GLFW_KEY_A, GLFW_KEY_D, camera.cameraFront, deltaTime, camera);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Utils::strafeMoveCheck(window, GLFW_KEY_A, GLFW_KEY_D, -camera.cameraFront, deltaTime, camera);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Utils::strafeMoveCheck(window, GLFW_KEY_W, GLFW_KEY_S, camera.cameraRight, deltaTime, camera);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Utils::strafeMoveCheck(window, GLFW_KEY_W, GLFW_KEY_S, -camera.cameraRight, deltaTime, camera);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera.cameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * camera.cameraSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		camera.cameraPos -= glm::vec3(0.0f, 1.0f, 0.0f) * camera.cameraSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Settings::sprintspeed += deltaTime * 5.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Settings::sprintspeed -= deltaTime * 5.0f;
		if (Settings::sprintspeed < 5.0f)
		{
			Settings::sprintspeed = 5.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera.cameraSpeed = Settings::sprintspeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		camera.cameraSpeed = Settings::walkspeed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		Settings::outlineScale += deltaTime * 0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		Settings::outlineScale -= deltaTime * 0.01f;
	}
}

void Render::renderSetSettingsAndVariables()
{
	mainFBO.use();
	glDrawBuffers(2, drawMainBuffers);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_GREATER);
	glEnable(GL_DEPTH_TEST);

	currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	model = glm::mat4(1.0f);
	view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
	proj = glm::perspectiveZO(glm::radians(camera.getFov()), (float)Settings::width / (float)Settings::height, Settings::far_plane, Settings::near_plane);
}

void Render::cullingAndProgramUse(World &world)
{
	// Culling and Rendering
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cullCountBuffer.id);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &cullCountNumber);

	cullProgram.use();
	cullProgram.uniformMatrix4fv("view", view);
	glDispatchCompute((world.chunks.size() + 63) / 64, 1, 1);
	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	program.use();
}

void Render::TAA1()
{
	if (Settings::TAA)
	{
		taaFrames = (taaFrames) % 16 + 1;
		noJitterProj = proj;
		program.uniformMatrix4fv("prevView", prevView);
		program.uniformMatrix4fv("prevProj", prevProj);
		program.uniformMatrix4fv("noJitterProj", noJitterProj);
		jitter = glm::vec2((Utils::Halton(taaFrames, 2) - 0.5f) / Settings::width, (Utils::Halton(taaFrames, 3) - 0.5f) / Settings::height);
		offset = jitter * 2.0f;
		proj[2][0] += offset.x;
		proj[2][1] += offset.y;
		TAAProgram.use();
		TAAProgram.uniform1f("stddevss", Settings::stddev);
		program.use();
	}
}

void Render::setMVPAndRender(World &world)
{
	program.uniformMatrix4fv("model", model);
	program.uniformMatrix4fv("view", view);
	program.uniformMatrix4fv("proj", proj);

	glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, 0, world.chunks.size(), sizeof(DrawArraysIndirectCommand));

	program.uniformMatrix4fv("prevModel", model);
}

void Render::playerLookAtBlockRender(World &world,Player& player)
{
	player.getBlockLookAtCoords(camera, world);
	if (player.isLookingAtBlock)
	{
		currentOutlineModel = glm::translate(glm::mat4(1.0f), player.lookAtBlockCoords);
		Utils::generateOutlineMatrices(currentOutlineModel, outlineMatrices);
		glNamedBufferData(VBOoutline.id, outlineMatrices.size() * sizeof(glm::mat4), outlineMatrices.data(), GL_DYNAMIC_DRAW);

		glNamedBufferData(prevVBOoutline.id, prevOutlineMatrices.size() * sizeof(glm::mat4), prevOutlineMatrices.data(), GL_DYNAMIC_DRAW);

		outlineProgram.use();
		outlineProgram.uniformMatrix4fv("view", view);
		outlineProgram.uniformMatrix4fv("prevView", prevView);       // already stored
		outlineProgram.uniformMatrix4fv("proj", proj);           // jittered
		outlineProgram.uniformMatrix4fv("noJitterProj", noJitterProj);
		outlineProgram.uniformMatrix4fv("prevProj", prevProj);       // last frame's noJitterProj
		outlineProgram.uniform1f("scale", Settings::outlineScale);
		outlineProgram.uniform1f("thickness", Settings::outlineThickness);
		outlineProgram.uniform1f("time", currentFrame);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, outlineMatrices.size());
		prevOutlineMatrices = outlineMatrices;
	}
}

void Render::TAA2()
{
	prevView = view;
	prevProj = noJitterProj;

	// TAA
	if (Settings::TAA)
	{
		TAAProgram.use();
		glDisable(GL_DEPTH_TEST);
		glBindTextureUnit(0, mainFBO.colorAttachment[0].id);
		glBindTextureUnit(2, mainFBO.colorAttachment[1].id);
		glBindTextureUnit(3, previousVelocityTex.id);
		glBindTextureUnit(4, mainFBO.depthAttachment);
		oddframe = !oddframe;
		glDrawBuffers(1, drawNormalBuffers);

		if (oddframe)
		{
			glBindTextureUnit(1, history2FBO.colorAttachment[0].id);
			history1FBO.use();
		}
		else
		{
			glBindTextureUnit(1, history1FBO.colorAttachment[0].id);
			history2FBO.use();
		}

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glCopyImageSubData(mainFBO.colorAttachment[1].id, GL_TEXTURE_2D, 0, 0, 0, 0,
			previousVelocityTex.id, GL_TEXTURE_2D, 0, 0, 0, 0, Settings::width, Settings::height, 1);

		screenProgram.use();
		glDisable(GL_DEPTH_TEST);
		if (oddframe) glBindTextureUnit(0, history1FBO.colorAttachment[0].id);
		else glBindTextureUnit(0, history2FBO.colorAttachment[0].id);

	}
}

void Render::drawToQuad()
{
	if(Settings::TAA == false)
	{
		screenProgram.use();
		glDisable(GL_DEPTH_TEST);
		glBindTextureUnit(0, mainFBO.colorAttachment[0].id);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Render::renderHUD(Player& player)
{
	nbFrames++;
	if (currentFrame - lastTime >= 1) {
		lastFps = nbFrames;
		nbFrames = 0;
		lastTime += 1;
	}

	hudProgram.use();
	hudProgram.uniform1f("time", currentFrame);
	glDrawArrays(GL_TRIANGLES, 0, 12);


	textProgram.use();
	textProgram.uniform1f("time", currentFrame);
	UserInterface::drawText(std::to_string(lastFps) + " FPS", 0, Settings::height - 40.0f, 50.0f, glm::vec3(1.0f, 1.0f, 1.0f), textProgram, minecraftCharDatas, textBuffer.id, characters.id);
	UserInterface::drawText("X: " + std::to_string(int(player.lookAtBlockCoords.x)) + " Y: " + std::to_string(int(player.lookAtBlockCoords.y)) + " Z: " + std::to_string(int(player.lookAtBlockCoords.z)), 
		Settings::width - 450.0f, Settings::height - 60.0f, 50.0f, glm::vec3(1.0f, 1.0f, 1.0f), textProgram, minecraftCharDatas, textBuffer.id, characters.id);
	UserInterface::drawText("X: " + std::to_string(int(player.pos.x)) + " Y: " + std::to_string(int(player.pos.y)) + " Z: " + std::to_string(int(player.pos.z)),
		Settings::width - 450.0f, Settings::height - 500.0f, 50.0f, glm::vec3(1.0f, 1.0f, 1.0f), textProgram, minecraftCharDatas, textBuffer.id, characters.id);
}

void Render::renderMiscEnd()
{
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void Render::render(World &world,Player &player)
{
	Render::process_input(window,player);
	Render::renderSetSettingsAndVariables();
	Render::cullingAndProgramUse(world);
	Render::TAA1();
	Render::setMVPAndRender(world);
	Render::playerLookAtBlockRender(world, player);
	Render::TAA2();
	Render::drawToQuad();
	Render::renderHUD(player);
	Render::renderMiscEnd();
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xpos, double ypos)
{
	CallbackStruct* ctx = (CallbackStruct*)glfwGetWindowUserPointer(window);
	float nowFov = ctx->camera->getFov();
	nowFov -= ypos;
	if (nowFov < 1.0f)
	{
		nowFov = 1.0f;
	}
	else if (nowFov > 105.0f)
	{
		nowFov = 105.0f;
	}
	ctx->camera->setFov(nowFov);
	Utils::change_frustrum(*ctx->cullShader, *ctx->projShader, *ctx->camera);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	CallbackStruct* ctx = (CallbackStruct*)glfwGetWindowUserPointer(window);
	if (ctx->firstMouse)
	{
		ctx->xLast = xpos;
		ctx->yLast = ypos;
		ctx->firstMouse = false;
	}
	float xoffset = xpos - ctx->xLast;
	float yoffset = ctx->yLast - ypos;
	ctx->xLast = xpos;
	ctx->yLast = ypos;

	float sensitivity = 0.1f;

	ctx->camera->yaw += xoffset * sensitivity;
	ctx->camera->pitch += yoffset * sensitivity;

	if (ctx->camera->pitch > 89.0f)
	{
		ctx->camera->pitch = 89.0f;
	}
	else if (ctx->camera->pitch < -89.0f)
	{
		ctx->camera->pitch = -89.0f;
	}

	glm::vec3 dir;
	const float radPitch = glm::radians(ctx->camera->pitch);
	const float radYaw = glm::radians(ctx->camera->yaw);
	dir.x = cos(radYaw) * cos(radPitch);
	dir.y = sin(radPitch);
	dir.z = sin(radYaw) * cos(radPitch);
	ctx->camera->cameraFront = glm::normalize(dir);
	ctx->camera->updateCameraRight();
	ctx->camera->updateCameraUp();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	CallbackStruct* ctx = (CallbackStruct*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_T:
			Settings::TAA = !Settings::TAA;
			ctx->projShader->use();
			ctx->projShader->uniform1i("TAA", Settings::TAA);
			break;

		case GLFW_KEY_C:
			Settings::CAS = !Settings::CAS;
			ctx->screenShader->use();
			ctx->screenShader->uniform1f("cas", Settings::CAS);
			break;
		}
	}
}

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
