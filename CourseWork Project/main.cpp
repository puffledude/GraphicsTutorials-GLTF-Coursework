#include "../NCLGL/window.h"
#include "Renderer.h"

int main() {
	Window w("Make your own project!", 1280, 720, false);
	float cooldown = 0.2f;
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_C) &&cooldown<=0.0f) {
			cooldown = 0.2f;
			renderer.startTransition();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F) &&cooldown<=0.0f) {
			cooldown = 0.2f;
			renderer.setFXAA();
		}

		cooldown -= w.GetTimer()->GetTimeDeltaSeconds();
	}
	return 0;
}