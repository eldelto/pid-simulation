#include <stdlib.h>
#include <stdbool.h>

#include "../simutil.h"

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include <physac.h>

struct pid_controller {
  double p_gain;
  double i_gain;
  double d_gain;
  double i_error;
  double last_error;
};

double pid_calculate_output(struct pid_controller *pid, double set_point, double value) {
  const double error_value = value - set_point;
  pid->i_error += (error_value * pid->i_gain);
  const double error_change = error_value - pid->last_error;
  pid->last_error = error_value;

  const double p_out = error_value * pid->p_gain;
  const double i_out = pid->i_error;
  const double d_out = error_change * pid->d_gain;

  return p_out + i_out + d_out;
}

static double clamp_thrust(double thrust) {
  if (thrust < 0) return 0;
  else if (thrust > 50000) return 50000;
  else return thrust;
}

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
  struct sim_data_set* position_data = sim_add_data_set(&graph, "Position");
  struct sim_data_set* neg_position_data = sim_add_data_set(&graph, "Set Point");

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
  SetPhysicsGravity(0, 0);//9.8);

  float p_gain = 0;
  float i_gain = 0;
  float d_gain = 0;
  bool second_set_point = false;

  struct pid_controller pid = {};

  const double set_point_1 = screen_height/2;
  const double set_point_2 = screen_height/4;

  while (!WindowShouldClose()) {
    RunPhysicsStep();

    pid.p_gain = p_gain;
    pid.i_gain = i_gain;
    pid.d_gain = d_gain;

    const double set_point = second_set_point ? set_point_2 : set_point_1;

    double thrust = pid_calculate_output(&pid, set_point, seesaw.physics_body->position.y);
    // thrust = clamp_thrust(thrust);

    const Vector2 force = {
      .x = 0,
      .y = -thrust/10,
    };
    PhysicsAddForce(seesaw.physics_body, force);
    sim_push_data_point(position_data, seesaw.physics_body->position.y - screen_height/2.0);
    sim_push_data_point(neg_position_data, set_point - screen_height/2.0);

    BeginDrawing();
    ClearBackground(WHITE);

    sim_draw_rectangle(&seesaw);
    sim_draw_rectangle(&floor);

    sim_draw_graph(&graph);

    GuiGroupBox(control_group, "Controller Gains");
    p_gain = GuiSliderBar(p_gain_slider, "P", TextFormat("%.1f", p_gain), p_gain, 0, 100);
    i_gain = GuiSliderBar(i_gain_slider, "I", TextFormat("%.1f", i_gain), i_gain, 0, 10);
    d_gain = GuiSliderBar(d_gain_slider, "D", TextFormat("%.1f", d_gain), d_gain, 0, 5000);
    if (GuiButton(set_point_button, "Change setpoint")) {
      second_set_point = !second_set_point;
    }

    DrawFPS(screen_width - 90, screen_height - 30);

    EndDrawing();
  }

  sim_free_rectangle(&seesaw);
  sim_free_rectangle(&floor);
  sim_free_graph(&graph);
  ClosePhysics();
  CloseWindow();

  return EXIT_SUCCESS;
}
