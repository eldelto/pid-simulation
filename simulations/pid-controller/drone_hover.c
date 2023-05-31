#include <stdlib.h>
#include <stdbool.h>

#include "pid.h"
#include "../simutil.h"

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include <physac.h>

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
  struct sim_data_set* position_data = sim_add_data_set(&graph, "Position", BLUE);
  struct sim_data_set* thrust_data = sim_add_data_set(&graph, "Thrust", RED);

  const int seesaw_width = 250;
  const Rectangle seesaw_rectangle = {
    .x = screen_width / 2 ,
    .y = screen_height / 2,
    .width = seesaw_width,
    .height = 15,
  };

  struct sim_rectangle seesaw = sim_new_rectangle(seesaw_rectangle, BLACK);
  seesaw.physics_body->freezeOrient = true;

  const Rectangle floor_rect = {
    .x = screen_width / 2 ,
    .y = screen_height - (PADDING / 2 ),
    .width = screen_width,
    .height = PADDING,
  };
  struct sim_rectangle floor = sim_new_rectangle(floor_rect, LIGHTGRAY);
  floor.physics_body->enabled = false;

  InitWindow(screen_width, screen_height, "Proportional System");
  SetTargetFPS(60);

  InitPhysics();
  SetPhysicsGravity(0, 9.8);

  float p_gain = 0;
  float i_gain = 0;
  float d_gain = 0;
  bool second_set_point = false;

  struct pid_controller pid = {};

  const double set_point_1 = screen_height/2;
  const double set_point_2 = screen_height/4;

  double thrust = 0;

  while (!WindowShouldClose()) {
    RunPhysicsStep();

    pid.p_gain = p_gain * 100;
    pid.i_gain = i_gain;
    pid.d_gain = d_gain * 300;

    const double set_point = second_set_point ? set_point_2 : set_point_1;

    double new_thrust = -pid_calculate_output(&pid, set_point, seesaw.physics_body->position.y);
    double thrust_change = new_thrust - thrust;
    thrust_change = sim_clamp_value(thrust_change, -1000, 1000);
    thrust += thrust_change;
    thrust = sim_clamp_value(thrust, 0, 50000);

    const Vector2 force = {
      .x = 0,
      .y = -thrust,
    };
    PhysicsAddForce(seesaw.physics_body, force);

    sim_push_data_point(set_point_data, -(set_point - screen_height / 2.0));
    sim_push_data_point(position_data, -(seesaw.physics_body->position.y - screen_height / 2.0));
    sim_push_data_point(thrust_data, thrust / 100.0);

    BeginDrawing();
    ClearBackground(WHITE);

    sim_draw_rectangle(&seesaw);
    sim_draw_rectangle(&floor);

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

  sim_free_rectangle(&seesaw);
  sim_free_rectangle(&floor);
  sim_free_graph(&graph);
  ClosePhysics();
  CloseWindow();

  return EXIT_SUCCESS;
}
