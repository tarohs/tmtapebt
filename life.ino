#define LIFECHGCYCLE (80)
#define LIFEFADECYCLE (80)
#define LIFEAUTOQUIT (40)

byte rulebits[8];
byte lifecurr[NUM_LEDS];
byte lifenext[NUM_LEDS];
int lifecount;
int islifeloop;
int demolifecount;

int
init_life(int ruleno, int isloop)
{
  if (ruleno < 0 || 255 < ruleno) {
    return 0;
  }
  islifeloop = isloop;
  for (int i = 0; i < 8; i++) {
    rulebits[i] = (byte)(ruleno % 2);
    ruleno >>= 1;
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == NUM_LEDS / 2) {
      lifecurr[i] = 1;
    } else {
      lifecurr[i] = 0;
    }
  }
  lifecount = 0;
  demolifecount = 0;
  return 1;
}


void
update_life(void)
{
  if (lifecount == 0) {
    for (int i = 0; i < NUM_LEDS; i++) {
      int st = 0;;
      if (1 <= i) {
        st += (int)lifecurr[i - 1];
      } else if (islifeloop) {
        st += (int)lifecurr[NUM_LEDS - 1];
      }
      st <<= 1;
      st += (int)lifecurr[i];
      st <<= 1;
      if (i < NUM_LEDS - 1) {
        st += (int)lifecurr[i + 1];
      } else if (islifeloop) {
        st += (int)lifecurr[0];
      }
      lifenext[i] = rulebits[st];
      if (lifecurr[i] == 0 && lifenext[i] == 1) {
        write_led(i, COL_BLUE, LIFECHGCYCLE);
      } else if (lifecurr[i] == 1 && lifenext[i] == 0) {
        write_led(i, COL_RED, LIFECHGCYCLE);
      }
    }
  } else if (lifecount == LIFECHGCYCLE) {
    for (int i = 0, st = 0; i < NUM_LEDS; i++) {
      lifecurr[i] = lifenext[i];
      if (lifecurr[i] == 0) {
        write_led(i, COL_BLACK, LIFEFADECYCLE);
      } else {
        write_led(i, COL_WHITE, LIFEFADECYCLE);
      }
    }
  } else if (lifecount == LIFECHGCYCLE + LIFEFADECYCLE) {
    lifecount = 0;
    demolifecount++;
    if (isautodemo && LIFEAUTOQUIT <= demolifecount) {
      init_autodemo();
    }
    return;
  }
  lifecount++;
  return;
}
