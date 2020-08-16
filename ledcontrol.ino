
rgb_t
xfade(rgb_t x, rgb_t y, int fadecount, int fadecycle)
{
// x: (0)<--fadecount-->(fadecycle) :y
  rgb_t z;
  if (fadecount <= 0 || fadecycle <= 0) {
    return x;
  }
  z.r = (x.r * (fadecycle - fadecount) + y.r * fadecount) / fadecycle;
  z.g = (x.g * (fadecycle - fadecount) + y.g * fadecount) / fadecycle;
  z.b = (x.b * (fadecycle - fadecount) + y.b * fadecount) / fadecycle;
  return z;
}


int
write_led(int pos, rgb_t col, int fadecycle)
{
  if (pos < 0 || NUM_LEDS <= pos ||
      col.r < 0 || 255 < col.r ||
      col.g < 0 || 255 < col.g ||
      col.b < 0 || 255 < col.b ||
      fadecycle <= 0) {
    return 0;
  }
  if (FADECYCLE_MAX < fadecycle) {
    fadecycle = FADECYCLE_MAX;
  }
  if (0 <= ledfadecount[pos]) {
    leddbuf[pos] = xfade(ledwbuf[pos], leddbuf[pos], ledfadecount[pos], ledfadecycle[pos]);
  }
  ledwbuf[pos] = col;
  ledfadecount[pos] = fadecycle - 1;
  ledfadecycle[pos] = fadecycle;

  return 1;
}


int
vcorr(int v)
{
  float fv = pow(((float)v / 255.), 3.);
  return (int)(fv * (BRTMAX - BRTMIN) + BRTMIN);
}


void
update_led(void)
{
  rgb_t col;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (ledfadecount[i] < 0) {
       continue;
    }
    col = xfade(ledwbuf[i], leddbuf[i], ledfadecount[i], ledfadecycle[i]);
    leds[i] = CRGB(vcorr(col.r), vcorr(col.g), vcorr(col.b));
    ledfadecount[i] = ledfadecount[i] - 1;
    if (ledfadecount[i] == -1) {
      leddbuf[i] = ledwbuf[i];
    }
  }
  FastLED.show();
//  Serial.printf("%d (%d) %d->%d\n",
//    ledwbuf[0][0], fadecount, leddbuf[0][0], 
//    xfade(ledwbuf[0], leddbuf[0]));
  return;
}


void
setall(rgb_t col, int fadecycle)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    write_led(i, col, fadecycle);
  }
  return;
}
