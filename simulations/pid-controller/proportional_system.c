#include <stdlib.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define PADDING 20

static Rectangle build_slider_rect(const Rectangle group, const int index) {
  return (Rectangle) {
    .x = group.x + PADDING,
      .y = group.y + PADDING + (index * (15 + PADDING)),
      .width = group.width - (2 * PADDING + 10),
      .height = 15,
  };
}

int main(void) {
  const int screen_width = 800;
  const int screen_height = 450;

  const int control_group_width = 200;
  const Rectangle control_group = {
    .x = screen_width - (control_group_width + PADDING),
    .y = PADDING,
    .width = control_group_width,
    .height = screen_height - (2 * PADDING),
  };

  const Rectangle p_gain_slider = build_slider_rect(control_group, 0);
  const Rectangle i_gain_slider = build_slider_rect(control_group, 1);
  const Rectangle d_gain_slider = build_slider_rect(control_group, 2);

  InitWindow(screen_width, screen_height, "Proportional System");
  SetTargetFPS(60);

  float p_gain = 0;
  float i_gain = 0;
  float d_gain = 0;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    GuiGroupBox(control_group, "Controller Gains");
    p_gain = GuiSliderBar(p_gain_slider, "P", TextFormat("%.1f", p_gain), p_gain, 0, 100);
    i_gain = GuiSliderBar(i_gain_slider, "I", TextFormat("%.1f", i_gain), i_gain, 0, 100);
    d_gain = GuiSliderBar(d_gain_slider, "D", TextFormat("%.1f", d_gain), d_gain, 0, 100);

    EndDrawing();
  }

  CloseWindow();

  return EXIT_SUCCESS;
}
