#define NUM_LEDS (60)

typedef struct {
  byte r, g, b;
} rgb_t;
#define COL_BLACK  ((rgb_t){0, 0, 0})
#define COL_RED    ((rgb_t){200, 0, 0})
#define COL_GREEN  ((rgb_t){0, 200, 0})
#define COL_BLUE   ((rgb_t){0, 0, 200})
#define COL_WHITE  ((rgb_t){200, 200, 200})
#define COL_YELLOW ((rgb_t){200, 200, 0})
#define COL_PURPLE ((rgb_t){200, 0, 200})
#define COL_CYAN   ((rgb_t){0, 200, 200})
#define COL_ORANGE ((rgb_t){220, 150, 0})

void IRAM_ATTR timer_intr(void);
rgb_t xfade(rgb_t x, rgb_t y, int fadecount, int fadecycle);
int write_led(int pos, rgb_t col, int fadecycle);
int vcorr(int v);
void update_led(void);
void setall(rgb_t col, int fadecycle);

void init_soliton(void);
void update_soliton(void);

int init_life(int ruleno, int isloop);
void update_life(void);

int ch2hex(char *buf);
int ch6hex(char *buf, rgb_t *colp);
void init_autodemo(void);
int docmd(char *buf);
