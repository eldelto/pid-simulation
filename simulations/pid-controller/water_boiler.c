#include <stdlib.h>
#include <stdbool.h>

#include "pid.h"
#include "../simutil.h"

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

int main(void) {
  const int screen_width = 1000;
  const int screen_height = 564;

  const int control_group_width = 200;
  const Rectangle control_group = {
    .x = screen_width - (control_group_width + PADDING),
    .y = PADDING,
    .width = control_group_width,
    .height = screen_height - (3 * PADDING),
  };

  const Rectangle p_gain_slider = sim_build_slider(control_group, 0);
  const Rectangle i_gain_slider = sim_build_slider(control_group, 1);
  const Rectangle d_gain_slider = sim_build_slider(control_group, 2);
  const Rectangle set_point_button = sim_build_button(control_group, 3);

  const Rectangle graph_rect = {
    .x = PADDING,
    .y = PADDING,
    .width = 300,
    .height = screen_height - (3 * PADDING),
  };
  struct sim_graph graph = sim_new_graph(graph_rect);
  struct sim_data_set* set_point_data = sim_add_data_set(&graph, "Set Point", GREEN);
  struct sim_data_set* water_temp_data = sim_add_data_set(&graph, "Water Temp", BLUE);
  struct sim_data_set* heater_temp_data = sim_add_data_set(&graph, "Heater Temp", RED);

  InitWindow(screen_width, screen_height, "Proportional System");
  SetTargetFPS(60);

  float p_gain = 0;
  float i_gain = 0;
  float d_gain = 0;
  bool second_set_point = false;

  struct pid_controller pid = {};

  const double set_point_1 = 30;
  const double set_point_2 = 80;

  double heater_temp = set_point_1;
  double water_temp = heater_temp;

  const Vector2 origin = {};
  const Rectangle tank_rect = {
    .x = 420,
    .y = 130,
    .width = 200,
    .height = 300,
  };

  const int wall_thickness = 5;
  const Rectangle inner_tank_rect = {
    .x = tank_rect.x + wall_thickness,
    .y = tank_rect.y + wall_thickness,
    .width = tank_rect.width - (wall_thickness * 2),
    .height = tank_rect.height - (wall_thickness * 2),
  };

  const Rectangle water_rect = {
    .x = inner_tank_rect.x,
    .y = inner_tank_rect.y + 80,
    .width = inner_tank_rect.width,
    .height = inner_tank_rect.height - 80,
  };

  const Rectangle heater_rect = {
    .x = inner_tank_rect.x + (inner_tank_rect.width/2) - 5,
    .y = inner_tank_rect.y + inner_tank_rect.height - 90,
    .width = 10,
    .height = 90,
  };

  while (!WindowShouldClose()) {
    water_temp += (heater_temp - water_temp) / 40;

    pid.p_gain = p_gain;
    pid.i_gain = i_gain / 1000;
    pid.d_gain = d_gain * 10;

    const double set_point = second_set_point ? set_point_2 : set_point_1;

    double heater_temp_change = pid_calculate_output(&pid, set_point, water_temp);
    heater_temp_change = sim_clamp_value(heater_temp_change, -5, 5);
    heater_temp = sim_clamp_value(heater_temp - heater_temp_change, 22, 200);
    
    // PhysicsAddForce(seesaw.physics_body, force);
    sim_push_data_point(water_temp_data, water_temp * 1.5);
    sim_push_data_point(heater_temp_data, heater_temp * 1.5);
    sim_push_data_point(set_point_data, set_point * 1.5);

    BeginDrawing();
    ClearBackground(WHITE);

    DrawRectanglePro(tank_rect, origin, 0, LIGHTGRAY);
    DrawRectanglePro(inner_tank_rect, origin, 0, DARKGRAY);
    DrawRectanglePro(water_rect, origin, 0, BLUE);
    DrawRectanglePro(heater_rect, origin, 0, (Color){ sim_clamp_value(55 + (heater_temp * 1.5), 55, 255), 55, 55, 255 });

    sim_draw_graph(&graph);

    GuiGroupBox(control_group, "Controller Gains");
    p_gain = GuiSliderBar(p_gain_slider, "P", TextFormat("%.2f", p_gain), p_gain, 0, 1);
    i_gain = GuiSliderBar(i_gain_slider, "I", TextFormat("%.2f", i_gain), i_gain, 0, 1);
    d_gain = GuiSliderBar(d_gain_slider, "D", TextFormat("%.2f", d_gain), d_gain, 0, 1);
    if (GuiButton(set_point_button, "Change setpoint")) {
      second_set_point = !second_set_point;
    }

    EndDrawing();
  }

  sim_free_graph(&graph);
  CloseWindow();

  return EXIT_SUCCESS;
}
