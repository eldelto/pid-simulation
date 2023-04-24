#include <stdlib.h>
#include <stdbool.h>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include <physac.h>

#define PADDING 20

static Rectangle build_slider_rect(const Rectangle group, const int index) {
  return (Rectangle) {
    .x = group.x + PADDING,
      .y = group.y + PADDING + (index * (15 + PADDING)),
      .width = group.width - (2 * PADDING + 10),
      .height = 15,
  };
}

struct sim_rectangle {
  Rectangle rectangle;
  PhysicsBody physics_body;
  Color color; 
};

struct sim_rectangle sim_new_rectangle(const Rectangle rectangle, const Color color) {
  const Vector2 position = {
    .x = rectangle.x,
    .y = rectangle.y,
  };
  PhysicsBody physics_body = CreatePhysicsBodyRectangle(position, 
    rectangle.width, 
    rectangle.height, 
    10);

  return (struct sim_rectangle) {
    .rectangle = rectangle,
    .physics_body = physics_body,
    .color = color
  };
}

void sim_free_rectangle(struct sim_rectangle* rect) {
  DestroyPhysicsBody(rect->physics_body);
  rect->physics_body = NULL;
}

void sim_draw_rectangle(struct sim_rectangle* rect) {
  rect->rectangle.x = rect->physics_body->position.x;
  rect->rectangle.y = rect->physics_body->position.y;

  const Vector2 origin = {
    .x = rect->rectangle.width/2,
    .y = rect->rectangle.height/2,
  };

  DrawRectanglePro(rect->rectangle, 
    origin,
    rect->physics_body->orient * RAD2DEG,
    rect->color);
}

struct data_set {
  double* data;
  unsigned int data_len;
  unsigned int cursor;
  const char* name;
};

struct data_set new_data_set(unsigned int length, char* name) {
  double* data = calloc(length, sizeof(double));

  return (struct data_set) {
    .data = data,
    .data_len = length,
    .cursor = 0,
    .name = name,
  };
}

void free_data_set(struct data_set* ds) {
  free(ds->data);
  ds->data = NULL;
}

void data_set_push(struct data_set* ds, double value) {
  ds->cursor = ++ds->cursor % ds->data_len;
  ds->data[ds->cursor] = value;
}

double data_set_get(const struct data_set* const ds, unsigned int i) {
  const unsigned int index = (ds->cursor + 1 + i) % ds->data_len;
  return ds->data[index];
}

#define DATA_SETS_MAX 5
struct line_graph {
  const Rectangle rect;
  struct data_set data_sets[DATA_SETS_MAX];
  unsigned int data_sets_len;
};

struct line_graph new_line_graph(const Rectangle rect) {
  return (struct line_graph) {
    .rect = rect,
    .data_sets = {},
    .data_sets_len = 0,
  };
}

void free_line_graph(struct line_graph* lg) {
  for (unsigned int i = 0; i < lg->data_sets_len; ++i)
    free_data_set(&lg->data_sets[i]);
}

struct data_set* add_data_set(struct line_graph* lg, char* name) {
  struct data_set data_set = new_data_set(lg->rect.width, name);
  lg->data_sets_len++;
  if (lg->data_sets_len > DATA_SETS_MAX)
    lg->data_sets_len = DATA_SETS_MAX;

  const unsigned int i = lg->data_sets_len - 1;
  lg->data_sets[i] = data_set;

  return &lg->data_sets[i];
}

const Color colors[DATA_SETS_MAX] = {RED, BLUE, GREEN, PURPLE, ORANGE};

void draw_line_graph(struct line_graph* lg) {
  const int x = lg->rect.x;
  const int y = lg->rect.y;

  const int y_mid = y + (lg->rect.height / 2);

  DrawLine(x, y_mid, x + lg->rect.width, y_mid, BLACK);
  DrawLine(x, y, x, y + lg->rect.height, BLACK);

  for (unsigned int j = 0; j < lg->data_sets_len; ++j) {
    const Color color = colors[j];
    const struct data_set data_set = lg->data_sets[j];
    DrawLine(x + PADDING, y + 5 + (PADDING * j), x + PADDING + 10, y + 5 + (PADDING * j), color);
    DrawText(data_set.name, x + PADDING + 20, y + (PADDING * j), 10, BLACK);

    for(unsigned int i = 1; i < data_set.data_len; ++i) {
      const int x_line = x + i;
      const int y_previous = y_mid + data_set_get(&data_set, i - 1);
      const int y = y_mid + data_set_get(&data_set, i);
      DrawLine(x_line-1, y_previous, x_line, y, color);
    }
  }
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

  const Rectangle p_gain_slider = build_slider_rect(control_group, 0);
  const Rectangle i_gain_slider = build_slider_rect(control_group, 1);
  const Rectangle d_gain_slider = build_slider_rect(control_group, 2);

  const Rectangle graph_rect = {
    .x = PADDING,
    .y = PADDING,
    .width = 300,
    .height = screen_height - (3 * PADDING),
  };
  struct line_graph graph = new_line_graph(graph_rect);
  struct data_set* position_data = add_data_set(&graph, "Position");
  struct data_set* neg_position_data = add_data_set(&graph, "Neg Position");

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

  while (!WindowShouldClose()) {
    RunPhysicsStep();

    const Vector2 force = {
      .x = 0,
      .y = -p_gain * 100,
    };
    PhysicsAddForce(seesaw.physics_body, force);
    data_set_push(position_data, seesaw.physics_body->position.y / 2);
    data_set_push(neg_position_data, -seesaw.physics_body->position.y / 2);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    sim_draw_rectangle(&seesaw);
    sim_draw_rectangle(&floor);

    draw_line_graph(&graph);

    GuiGroupBox(control_group, "Controller Gains");
    p_gain = GuiSliderBar(p_gain_slider, "P", TextFormat("%.1f", p_gain), p_gain, 0, 100);
    i_gain = GuiSliderBar(i_gain_slider, "I", TextFormat("%.1f", i_gain), i_gain, 0, 100);
    d_gain = GuiSliderBar(d_gain_slider, "D", TextFormat("%.1f", d_gain), d_gain, 0, 100);

    DrawFPS(screen_width - 90, screen_height - 30);

    EndDrawing();
  }

  sim_free_rectangle(&seesaw);
  sim_free_rectangle(&floor);
  free_line_graph(&graph);
  ClosePhysics();
  CloseWindow();

  return EXIT_SUCCESS;
}
