#include "tmtapebt.h"
#include "turing.h"

#include <math.h>

#include <FastLED.h>
#define DATA_PIN (21)
#define LED_TYPE    WS2811B
CRGB leds[NUM_LEDS];
#define BRTMAX (255.)
#define BRTMIN (0.)

#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
#define DEVICENAME "ESPtape"

hw_timer_t *timer = NULL;
volatile int isupdate = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
//#define TIMERDIV (80 * 250) // timer = 250us (0.25ms)
#define TIMERDIV (40 * 250) // timer = 250us (0.25ms)
#define ANIMDIV (6250 / 250)  // 6.25ms step for anim

rgb_t ledwbuf[NUM_LEDS] = {0};
rgb_t leddbuf[NUM_LEDS] = {0};
int ledfadecount[NUM_LEDS];
        // -1 for not xfading, fadecycle..0 for (0->1/1->0)
byte ledfadecycle[NUM_LEDS];
#define FADECYCLE_MAX (120)
  // 2 * F_MAX * F_MAX shouln't over INT_MAX (32767)

char   demomode;
static int  isautodemo;

#define fabs(x) ((x) > 0 ? (x) : -(x))
#define iround(x) ((int)((x) + .5))

void IRAM_ATTR
timer_intr(void)
{
  portENTER_CRITICAL_ISR(&timerMux);
  isupdate = 1;
  portEXIT_CRITICAL_ISR(&timerMux);
  return;
}


//................................................
int
ch2hex(char *buf)
{
  int r = 0;

  for (int i = 0; i < 2; i++) {
    if ('0' <= buf[i] && buf[i] <= '9') {
      r = (r << 4) + (buf[i] - '0'); 
    } else if ('A' <= buf[i] && buf[i] <= 'F') {
      r = (r << 4) + (buf[i] - 'A' + 10); 
    } else if ('a' <= buf[i] && buf[i] <= 'f') {
      r = (r << 4) + (buf[i] - 'a' + 10); 
    } else {
      return -1;
    }
  }
  return r;
}


int
ch6hex(char *buf, rgb_t *colp)
{
  int r = ch2hex(buf);
  int g = ch2hex(buf + 2);
  int b = ch2hex(buf + 4);
  if (0 <= r && r < 256 &&
      0 <= g && g < 256 &&
      0 <= b && b < 256) {
    colp -> r = (byte)r;
    colp -> g = (byte)g;
    colp -> b = (byte)b;
    return 1;
  } else {
    return 0;      
  }
}


int
getarg2(char *chp, int *arg1, int *arg2)
{
  *arg1 = atoi(chp);
  chp = strchr(chp, ',');
  if (chp == NULL) {
    return 0;
  }
  *arg2 = atoi(chp + 1);
  return 1;
}


void
init_autodemo(void)
{
  isautodemo = 1;
  switch(random(0, 4)) {
  case 0:
    docmd("ds");
    break;
  case 1:
    switch (random(0, 4)) {
    case 0:
      docmd("dl30");
      break;
    case 1:
      docmd("dl110");
      break;
    case 2:
      docmd("dL30");
      break;
    case 3:
      docmd("dL110");
      break;
    }
    break;
  case 2:
    switch(random(0, 4)) {
      case 0:
        docmd("dt22");
        break;
      case 1:
        docmd("dt32");
        break;
      case 2:
        docmd("dt42");
        break;
      case 3:
        docmd("dt23");
        break;
    }
    break;
  case 3:
    switch(random(0, 5)) {
    case 0:
      docmd("dtadd3,4");
      break;
    case 1:
      docmd("dtadb13,6");
      break;
    case 2:
      docmd("dtmul3,4");
      break;
    case 3:
      docmd("dtdiv15,6");
      break;
    case 4:
      docmd("dteuc21,15");
      break;
    }
    break;
  }
  return;
}


#define ANIMFADECYCLE (80) // 500ms step for set (all) LED cross fade

int
docmd(char *buf) {
  int arg1, arg2;
  Serial.printf("cmd: >>%s<<\n", buf);
  if (buf[0] == 'd' || buf[0] == 'D') { // demo:
    if (buf[1] == 'a' || buf[1] == 'A') {
      init_autodemo();
      return 2;
    } else if (buf[1] == '0') {
      demomode = '0';
      Serial.printf("demo off\n");
      return 1;
    } else if (buf[1] == 's' || buf[1] == 'S') {
      demomode = 's';
      init_soliton();
      Serial.printf("demo soliton\n");
      return 1;
    } else if (buf[1] == 't' || buf[1] == 'T') {
      demomode = 't';
      if (strcmp(buf + 2, "22") == 0) {
        init_tapezero(tmbb22);
        init_turing(tmbb22);
      } else if (strcmp(buf + 2, "32") == 0) {
        init_tapezero(tmbb32);
        init_turing(tmbb32);
      } else if (strcmp(buf + 2, "42") == 0) {
        init_tapezero(tmbb42);
        init_turing(tmbb42);
      } else if (strcmp(buf + 2, "23") == 0) {
        init_tapezero(tmbb23);
        init_turing(tmbb23);
      } else if (strncmp(buf + 2, "add", 3) == 0) {
        if (getarg2(buf + 5, &arg1, &arg2) == 0 || arg1 <= 0 || arg2 <= 0) {
          return 0;
        }
        init_tapeuni(tmadd, arg1, arg2);
        init_turing(tmadd);
      } else if (strncmp(buf + 2, "adb", 3) == 0) {
        if (getarg2(buf + 5, &arg1, &arg2) == 0 || arg1 <= 0 || arg2 <= 0) {
          return 0;
        }
        init_tapebin(tmaddbin, arg1, arg2);
        init_turing(tmaddbin);
      } else if (strncmp(buf + 2, "mul", 3) == 0) {
        if (getarg2(buf + 5, &arg1, &arg2) == 0 || arg1 <= 0 || arg2 <= 0) {
          return 0;
        }
        init_tapeuni(tmmul, arg1, arg2);
        init_turing(tmmul);
      } else if (strncmp(buf + 2, "div", 3) == 0) {
        if (getarg2(buf + 5, &arg1, &arg2) == 0 || arg1 <= 0 || arg2 <= 0) {
          return 0;
        }
        init_tapeuni(tmdiv, arg1, arg2);
        init_turing(tmdiv);
      } else if (strncmp(buf + 2, "euc", 3) == 0) {
        if (getarg2(buf + 5, &arg1, &arg2) == 0 || arg1 <= 0 || arg2 <= 0) {
          return 0;
        }
        init_tapeuni(tmeucrid, arg1, arg2);
        init_turing(tmeucrid);
      } else {
        return 0;
      }
      Serial.printf("demo turing-machine %s\n", buf + 2);
      return 1;
    } else if (buf[1] == 'l') {
      if (init_life(atoi(buf + 2), 0)) {
        demomode = 'l';
        Serial.printf("demo life (non-loop) #%s\n", buf + 2);
        return 1;
      }
    } else if (buf[1] == 'L') {
      if (init_life(atoi(buf + 2), 1)) {
        demomode = 'l';
        Serial.printf("demo life (loop world) #%s\n", buf + 2);
        return 1;
      }
    }
  } else if ((buf[0] == 's'  || buf[0] == 'S') &&
             buf[9] == '\0') {
    // set: sPPRRGGBB (P, R, G, B in [0-9A-F])
    int p = ch2hex(buf + 1);
    rgb_t col;
    if (0 <= p && p < NUM_LEDS && ch6hex(buf + 3, &col)) {
      write_led(p, col, ANIMFADECYCLE);
      Serial.printf("%02X set to (%02X, %02X, %02X)\n",
        p, (int)col.r, (int)col.g, (int)col.b);
      return 1;
    }
  } else if ((buf[0] == 'a'  || buf[0] == 'A') &&
             buf[7] == '\0') {  // all: aRRGGBB
    rgb_t col;
    if (ch6hex(buf + 1, &col)) {
      Serial.printf("all set to (%02X, %02X, %02X)\n", col.r, col.g, col.b);
      setall(col, ANIMFADECYCLE);
      return 1;
    }
  }
  return 0;
}


void
setup() {
  uint8_t macBT[6];

  Serial.begin(115200);
  delay(100);
  Serial.print("started...");

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) {
    ledfadecount[i] = -1;
    ledfadecycle[i] = 1;
  }
//  setall(0, 0, 0, 1); // quick change
  Serial.print("LED...");

  esp_read_mac(macBT, ESP_MAC_BT);
  Serial.printf(
    "Bluetooth ESPtape %02X:%02X:%02X:%02X:%02X:%02X...",
    macBT[0], macBT[1], macBT[2], macBT[3], macBT[4], macBT[5]);
  while (! SerialBT.begin(DEVICENAME)) {
    Serial.println("error initializing Bluetooth");
    delay(2000);
  }

  Serial.print("timer...");
  timer = timerBegin(0, TIMERDIV, true);
  timerAttachInterrupt(timer, &timer_intr, true);
  timerAlarmWrite(timer, ANIMDIV, true);
  timerAlarmEnable(timer);

  Serial.println("ready");
  init_autodemo();
}


void
loop() {
  static char buf[128];
  static int  bufp = 0;

  while (SerialBT.available()) {
    buf[bufp] = SerialBT.read();
    if (buf[bufp] == '\n' || buf[bufp] == '\r' ||
       bufp == sizeof(buf) - 1) {
      buf[bufp] = '\0';
      if (docmd(buf) == 1) { // valid and manual command
        isautodemo = 0;
      }
      bufp = 0;
    } else {
      bufp++;
    }
  }
  if (isupdate) {
    if (demomode == 's') {
      update_soliton();
    } else if (demomode == 't') {
      update_turing();
    } else if (demomode == 'l') {
      update_life();
    }
    update_led();
    portENTER_CRITICAL_ISR(&timerMux);
    isupdate = 0;
    portEXIT_CRITICAL_ISR(&timerMux);
  }
}
