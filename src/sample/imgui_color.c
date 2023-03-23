#include <ctoy.h>
#include "../cimgui/cimgui.h"
#include "../util/imgui_ctoy.c"
#include "../util/imgui_gles2.c"

static float R = 0.0f;
static float G = 0.0f;
static float B = 0.0f;

ImGuiContext *_ctx = NULL;

void ctoy_begin(void)
{
   ctoy_window_title("Color ImGui !");

   _ctx = igCreateContext(NULL);
   ImGui_ImplCToy_Init();
}

void ctoy_end(void)
{
   ImGui_ImplOpenGLES2_DestroyDeviceObjects();
   igDestroyContext(_ctx);
}

void ctoy_main_loop(void)
{
   glClearColor(R, G, B, 1);
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

   ImGui_ImplOpenGLES2_NewFrame();
   ImGui_ImplCToy_NewFrame();
   igNewFrame();
   
   {
      igLabelText("", "Choose a color ...");
      igNewLine();
      igSliderFloat("red", &R, 0.0f, 1.0f, "%.3f", 1.0f);
      igSliderFloat("green", &G, 0.0f, 1.0f, "%.3f", 1.0f);
      igSliderFloat("blue", &B, 0.0f, 1.0f, "%.3f", 1.0f);
   }

   igRender();
   ImGui_ImplOpenGLES2_RenderDrawData(igGetDrawData());
   
   ctoy_swap_buffer(NULL);
}
