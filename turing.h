
typedef struct {
  int currstate;
  byte currsym;
  byte nextsym;
  int dir;
  int nextstate;
} turst_t;

typedef struct {
  int nsymbols;
  int nstates;
  int initpos;
  int initstate;
  byte sym0;  // tape is filled with sym0
  byte sym1;  // (unary encoding) input symbol
  byte sym2;  // (binary encoding) input symbol "0"/"1" is sym1/sym2
  int speed;
  turst_t *rule;
} tm_t;

rgb_t tmtcol[] = {
  COL_BLACK,
  COL_BLUE,
  COL_GREEN,
  COL_PURPLE,
  COL_ORANGE
};

#define TMSHALT (-1)
tm_t currmachine;
turst_t tmstate;
int tmheadpos;
int tmrun;
int tmcycle;
int tmcount;
int tmhaltcount;
int tmsteps;
byte tmtape[NUM_LEDS];
#define TPOS_CENTER (NUM_LEDS / 2)

#define TMCYCLE_STD (16) // #1: 100ms step (turbigcyc) for turing demo
#define TMCYCLE_S   (32) // #0: x 1/2 speed
#define TMCYCLE_M   (4)  // #2: x 4 speed
#define TMCYCLE_H   (1)  // #3: x16 speed

#define TMREWSTEP (5) // 500ms fade for rewrite tape
#define TMHALTSTEP (5) // (x2) TMCYCLE for halt blink
#define TMBLINKSTEP (8) // 0.00-0.80s for head blink 
#define TMREWRITESTEP (15) // -(rewrite)-1.50s)
#define TMAUTOHALTQUIT (8)

void disp_tape(void);
void init_tapezero(tm_t tm);
void init_tapeuni(tm_t tm, int arg1, int arg2);
int  tapebin1(int pos, int arg, byte sym0, byte sym1);
void init_tapebin(tm_t tm, int arg1, int arg2);
void init_turing(tm_t tm);
void update_turing(void);

//
// machines
//
// busy beavers: http://www.logique.jussieu.fr/~michel/ha.html#tm23
turst_t tmbb22r[] = {
    {0, 0, 1, 1, 1},
    {0, 1, 1, -1, 1},
    {1, 0, 1, -1, 0},
    {1, 1, 1, 1, -1},
    {-1, -1, -1, 0, -1}
};
tm_t tmbb22 = {
  2,  // nsymbols
  2,  // nstates
  30, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  0,  // speed
  tmbb22r
};

turst_t tmbb32r[] = {
  {0, 0, 1, 1, 1},
  {0, 1, 1, 1, -1},
  {1, 0, 1, -1, 1},
  {1, 1, 0, 1, 2},
  {2, 0, 1, -1, 2},
  {2, 1, 1, -1, 0},
  {-1, -1, -1, 0, -1}
};
tm_t tmbb32 = {
  2,  // nsymbols
  2,  // nstates
  30, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  1,  // speed
  tmbb32r
};

turst_t tmbb42r[] = {
  {0, 0, 1, 1, 1},
  {0, 1, 1, -1, 1},
  {1, 0, 1, -1, 0},
  {1, 1, 0, -1, 2},
  {2, 0, 1, 1, -1},
  {2, 1, 1, -1, 3},
  {3, 0, 1, 1, 3},
  {3, 1, 0, 1, 0},
  {-1, -1, -1, 0, -1}
};
tm_t tmbb42 = {
  2,  // nsymbols
  4,  // nstates
  30, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  2,  // speed
  tmbb42r
};

turst_t tmbb23r[] = {
  {0, 0, 1, 1, 1},
  {0, 1, 2, -1, 1},
  {0, 2, 1, 1, -1},
  {1, 0, 2, -1, 0},
  {1, 1, 2, 1, 1},
  {1, 2, 1, -1, 1},
  {-1, -1, -1, 0, -1}
};
tm_t tmbb23 = {
  3,  // nsymbols
  2,  // nstates
  30, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  1,  // speed
  tmbb23r
};
/*
turst_t turmac22[TSTATEMAX][TSYMBOLMAX] = {      // (2, 2) busy beavor
  {{TMT1, TMDR, TMSB}, {TMT1, TMDL, TMSB}, },
  {{TMT1, TMDL, TMSA}, {TMT1, TMDR, TMSHALT}, },
};
turst_t turmac32[TSTATEMAX][TSYMBOLMAX] = {      // (3, 2) busy beavor
  {{TMT1, TMDR, TMSB}, {TMT1, TMDR, TMSHALT}, },
  {{TMT0, TMDR, TMSC}, {TMT1, TMDR, TMSB}, },
  {{TMT1, TMDL, TMSC}, {TMT1, TMDL, TMSA}, },
};
turst_t turmac42[TSTATEMAX][TSYMBOLMAX] = {      // (4, 2) busy beavor
  {{TMT1, TMDR, TMSB}, {TMT1, TMDL, TMSB}, },
  {{TMT1, TMDL, TMSA}, {TMT0, TMDL, TMSC}, },
  {{TMT1, TMDR, TMSHALT}, {TMT1, TMDL, TMSD}, },
  {{TMT1, TMDR, TMSD}, {TMT0, TMDR, TMSA}}
};
turst_t turmac23[TSTATEMAX][TSYMBOLMAX] = {      // (2, 3) busy beavor
  {{TMT1, TMDR, TMSB}, {TMT2, TMDL, TMSB}, {TMT1, TMDR, TMSHALT}},
  {{TMT2, TMDL, TMSA}, {TMT2, TMDR, TMSB}, {TMT1, TMDL, TMSB}},
};
*/

// calculators

turst_t tmaddr[] = {
  {0, 0, 0, 1, 0},
  {0, 1, 2, 1, 1},
  {1, 0, 0, 1, 2},
  {1, 1, 1, 1, 1},
  {2, 0, 0, 1, 3},
  {2, 1, 1, 1, 2},
  {3, 0, 1, -1, 4},
  {3, 1, 1, 1, 3},
  {4, 0, 0, -1, 5},
  {4, 1, 1, -1, 4},
  {5, 0, 0, -1, 7},
  {5, 1, 1, -1, 5},
  {5, 2, 2, 1, 6},
  {6, 0, 0, -1, 11},
  {6, 1, 2, 1, 2},
  {7, 1, 1, -1, 8},
  {7, 2, 2, 1, 9},
  {8, 0, 0, 1, 10},
  {8, 1, 1, -1, 8},
  {8, 2, 2, 1, 0},
  {9, 0, 0, 1, 6},
  {10, 0, 0, 1, -1},
  {10, 2, 1, -1, 10},
  {11, 0, 0, -1, 10},
  {11, 2, 1, -1, 11},
  {-1, -1, -1, 0, -1}
};
tm_t tmadd = {
  3,  // nsymbols
  12,  // nstates
  0, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  3,  // speed
  tmaddr
};

// add binary: # http://www.mlb.co.jp/linux/science/gturing/BinaryAdd.tur
turst_t tmaddbinr[] = {
  {0, 0, 0, 1, 0},
  {0, 1, 1, 1, 1},
  {0, 2, 2, 1, 1},
  {1, 0, 0, 1, 2},
  {1, 1, 1, 1, 1},
  {1, 2, 2, 1, 1},
  {1, 3, 3, 1, 1},
  {1, 4, 4, 1, 1},
  {2, 0, 0, -1, 3},
  {2, 1, 1, 1, 2},
  {2, 2, 2, 1, 2},
  {3, 0, 0, -1, 6},
  {3, 1, 0, -1, 4},
  {3, 2, 0, -1, 5},
  {4, 0, 0, -1, 7},
  {4, 1, 1, -1, 4},
  {4, 2, 2, -1, 4},
  {5, 0, 0, -1, 8},
  {5, 1, 1, -1, 5},
  {5, 2, 2, -1, 5},
  {6, 0, 0, -1, -1},
  {6, 1, 1, -1, -1},
  {6, 2, 2, -1, -1},
  {6, 3, 1, -1, 6},
  {6, 4, 2, -1, 6},
  {7, 0, 3, 1, 1},
  {7, 1, 3, 1, 1},
  {7, 2, 4, 1, 1},
  {7, 3, 3, -1, 7},
  {7, 4, 4, -1, 7},
  {8, 0, 4, 1, 1},
  {8, 1, 4, 1, 1},
  {8, 2, 3, -1, 9},
  {8, 3, 3, -1, 8},
  {8, 4, 4, -1, 8},
  {9, 0, 2, 1, 1},
  {9, 1, 2, 1, 1},
  {9, 2, 1, -1, 9},
  {-1, -1, -1, 0, -1}
};
tm_t tmaddbin = {
  5,  // nsymbols
  10,  // nstates
  TPOS_CENTER, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  2,  // sym2
  2,  // speed
  tmaddbinr
};

turst_t tmmulr[] = {
  {0, 0, 0, 1, 0},
  {0, 1, 2, 1, 1},
  {1, 0, 0, 1, 4},
  {1, 1, 1, 1, 1},
  {2, 0, 0, 1, -1},
  {2, 2, 1, -1, 2},
  {3, 0, 0, -1, 2},
  {3, 1, 2, 1, 1},
  {3, 2, 2, 1, 3},
  {4, 0, 0, -1, 5},
  {4, 1, 2, 1, 6},
  {4, 2, 2, 1, 4},
  {5, 0, 0, -1, 10},
  {5, 2, 1, -1, 5},
  {6, 0, 0, 1, 7},
  {6, 1, 1, 1, 6},
  {7, 0, 1, -1, 8},
  {7, 1, 1, 1, 7},
  {8, 0, 0, -1, 9},
  {8, 1, 1, -1, 8},
  {9, 1, 1, -1, 9},
  {9, 2, 2, 1, 4},
  {10, 0, 0, 1, 3},
  {10, 1, 1, -1, 10},
  {10, 2, 2, -1, 10},
  {-1, -1, -1, 0, -1}
};
tm_t tmmul = {
  3,  // nsymbols
  11,  // nstates
  0, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  2,  // speed
  tmmulr
};

turst_t tmdivr[] = {
  {0, 0, 0, 1, 0},
  {0, 1, 1, 1, 1},
  {1, 0, 0, 1, 2},
  {1, 1, 1, 1, 1},
  {2, 0, 0, 1, 3},
  {2, 1, 1, 1, 2},
  {3, 0, 3, -1, 4},
  {4, 0, 0, -1, 5},
  {5, 0, 0, 1, 6},
  {5, 1, 1, -1, 5},
  {6, 1, 2, 1, 7},
  {6, 2, 2, 1, 6},
  {7, 0, 0, -1, 9},
  {7, 1, 1, -1, 8},
  {8, 0, 0, -1, 10},
  {8, 2, 2, -1, 8},
  {9, 0, 0, -1, 14},
  {9, 2, 1, -1, 9},
  {10, 0, 0, 1, 11},
  {10, 1, 1, -1, 10},
  {10, 2, 2, 1, 11},
  {10, 3, 3, 1, 11},
  {11, 0, 0, -1, 13},
  {11, 1, 2, 1, 12},
  {12, 0, 0, 1, 6},
  {12, 1, 1, 1, 12},
  {13, 0, 0, 1, 21},
  {13, 2, 2, -1, 13},
  {13, 3, 3, 1, 21},
  {14, 0, 0, 1, 15},
  {14, 1, 1, -1, 14},
  {14, 2, 2, 1, 15},
  {14, 3, 3, 1, 15},
  {15, 0, 0, -1, 13},
  {15, 1, 3, 1, 16},
  {16, 0, 0, 1, 17},
  {16, 1, 1, 1, 16},
  {17, 0, 0, 1, 18},
  {17, 1, 1, 1, 17},
  {18, 0, 1, -1, 19},
  {18, 1, 1, 1, 18},
  {18, 3, 1, -1, 19},
  {19, 0, 0, -1, 20},
  {19, 1, 1, -1, 19},
  {20, 0, 0, 1, 6},
  {20, 1, 1, -1, 20},
  {21, 0, 0, 1, -1},
  {21, 2, 3, 1, 22},
  {22, 0, 0, 1, 23},
  {22, 2, 2, 1, 22},
  {23, 0, 0, 1, 24},
  {23, 1, 1, 1, 23},
  {23, 2, 1, 1, 23},
  {24, 0, 0, 1, 25},
  {24, 1, 1, 1, 24},
  {24, 3, 3, 1, 24},
  {25, 0, 1, -1, 26},
  {25, 1, 1, 1, 25},
  {26, 0, 0, -1, 27},
  {26, 1, 1, -1, 26},
  {27, 0, 0, -1, 28},
  {27, 1, 1, -1, 27},
  {27, 3, 3, -1, 27},
  {28, 0, 0, -1, 13},
  {28, 1, 1, -1, 28},
  {28, 2, 2, -1, 28},
  {-1, -1, -1, 0, -1}
};
tm_t tmdiv = {
  4,  // nsymbols
  29,  // nstates
  0, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  3,  // speed
  tmdivr
};

turst_t tmeucridr[] = {
  {0, 0, 0, 1, 0},
  {0, 1, 1, 1, 1},
  {1, 0, 0, 1, 2},
  {1, 1, 1, 1, 1},
  {2, 1, 2, 1, 3},
  {2, 2, 2, 1, 2},
  {3, 0, 0, -1, 5},
  {3, 1, 1, -1, 4},
  {4, 0, 0, -1, 6},
  {4, 2, 2, -1, 4},
  {5, 0, 0, -1, 9},
  {5, 2, 1, -1, 5},
  {6, 0, 0, 1, 7},
  {6, 1, 1, -1, 6},
  {6, 2, 2, 1, 7},
  {6, 3, 3, 1, 7},
  {7, 0, 0, -1, 8},
  {7, 1, 2, 1, 1},
  {8, 0, 0, 1, 12},
  {8, 2, 2, -1, 8},
  {8, 3, 3, 1, 12},
  {8, 4, 4, 1, 12},
  {9, 0, 0, 1, 10},
  {9, 1, 1, -1, 9},
  {9, 2, 2, 1, 10},
  {9, 3, 3, 1, 10},
  {10, 0, 0, -1, 8},
  {10, 1, 3, 1, 11},
  {11, 0, 0, 1, -1},
  {11, 1, 1, 1, 1},
  {12, 0, 0, 1, 1},
  {12, 2, 4, 1, 13},
  {13, 0, 0, 1, 14},
  {13, 2, 2, 1, 13},
  {14, 0, 0, 1, 15},
  {14, 1, 1, 1, 14},
  {14, 2, 1, 1, 14},
  {15, 0, 1, -1, 16},
  {15, 1, 1, 1, 15},
  {16, 0, 0, -1, 17},
  {16, 1, 1, -1, 16},
  {17, 0, 0, -1, 8},
  {17, 1, 1, -1, 17},
  {-1, -1, -1, 0, -1}
};
tm_t tmeucrid = {
  5,  // nsymbols
  18,  // nstates
  0, // initpos
  0,  // initstate
  0,  // sym0
  1,  // sym1
  0,  // sym2
  3,  // speed
  tmeucridr
};
