#include "Source/Renderer.h"


int main()
{
	Render render;
	World world;
	Player player;
	world.init();
	render.init(world,player);
	while (!glfwWindowShouldClose(render.window))
	{
		render.render(world,player);
	}
	glfwTerminate();
	return 0;
}