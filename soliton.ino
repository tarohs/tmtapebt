//#include "tmtapebt.h"

#define SOLCYCLE (20) // 125ms step for soliton demo
#define SOLFADECYCLE (16) // 100ms step for soliton cross fade
#define SOLAUTOQUIT (200)
int solautocount;

typedef struct {
  float  pos;
  int  dir;
  int  width;
  float  speed;
  float  col[3];
} sol_t;
static sol_t sol[5];
//= {
//  {0., 1, 2, 1., {1., 0, 0}},
//  {0., 1, 2, 1.2, {0, 1., 0}},
//  {0., 1, 2, 1.5, {0, 0, 1.}},
//  {0., 1, 3, .8, {.5, 0, .5}},
//  {0., 1, 4, .5, {.5, .5, 0}}
//};
int NSOL = sizeof(sol) / sizeof(sol_t);

void
init_soliton(void)
{
  for (int i = 0; i < NSOL; i++) {
    sol[i].pos = (float)(random(0, NUM_LEDS - 1));
    sol[i].dir = random(0, 1) * 2 - 1;
    sol[i].width = random(2, 5);
    sol[i].speed = (float)random(40, 250) / 100.;
    sol[i].col[0] = (float)random(0, 128) / 255.;
    sol[i].col[1] = (float)random(0, 128) / 255.;
    sol[i].col[2] = (float)random(0, 128) / 255.;
    Serial.printf("sol %d: width %d, speed %f, (%f, %f, %f)\n",
                  i, sol[i].width, sol[i].speed,
                  sol[i].col[0], sol[i].col[1], sol[i].col[2]);
  }
  solautocount = 0;
  return;
}


void
update_soliton(void)
{
  float lc[NUM_LEDS][3];

  static int solcount = 0;

  if (++solcount < SOLCYCLE) {
    return;
  }
  solcount = 0;
  // init LEDS
  for (int i = 0; i < NUM_LEDS; i++) {
    lc[i][0] = 0.;
    lc[i][1] = 0.;
    lc[i][2] = 0.;
  }

  // draw soliton & move, reflect
  for (int i = 0; i < NSOL; i++) {
    sol_t *sp = sol + i;
    int spp = iround(sp -> pos);
    for (int p = 0; p <= sp -> width; p++) {
      float v = 1. - (float)p / (float)(sp -> width);
      if (0 <= spp - p) {
        lc[spp - p][0] += v * (float)(sp -> col[0]);
        lc[spp - p][1] += v * (float)(sp -> col[1]);
        lc[spp - p][2] += v * (float)(sp -> col[2]);
      }
      if (spp + p < NUM_LEDS) {
        lc[spp + p][0] += v * (float)(sp -> col[0]);
        lc[spp + p][1] += v * (float)(sp -> col[1]);
        lc[spp + p][2] += v * (float)(sp -> col[2]);
      }
    }
    sol[i].pos += sol[i].speed * sol[i].dir;
    if (sol[i].pos < 0) {
      sol[i].pos = -sol[i].pos;
      sol[i].dir = 1;
    } else if ((float)NUM_LEDS - .5 < sol[i].pos) {
      sol[i].pos = 2 * ((float)NUM_LEDS - .5) - sol[i].pos;
      sol[i].dir = -1;
    }
  }
  // display
  for (int i = 0; i < NUM_LEDS; i++) {
    if (1. < lc[i][0]) {
      lc[i][0] = 1.;
    }
    if (1. < lc[i][1]) {
      lc[i][1] = 1.;
    }
    if (1. < lc[i][2]) {
      lc[i][2] = 1.;
    }
    write_led(i, (rgb_t){
      (byte)(lc[i][0] * 255.),
      (byte)(lc[i][1] * 255.),
      (byte)(lc[i][2] * 255.)},
      SOLFADECYCLE);
  }
  solautocount++;
  if (isautodemo && SOLAUTOQUIT <= solautocount) {
    init_autodemo();
  }

  return;
}
