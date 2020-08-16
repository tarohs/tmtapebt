
void
disp_tape(void)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    write_led(i, tmtcol[tmtape[i]], 1);
    Serial.printf("%d", (int)tmtape[i]);
  }
  Serial.printf("\n");
  return;
}


int
tmgetrule(tm_t tm, turst_t *statep)
{
  for (int i = 0; tm.rule[i].currstate != -1; i++) {
    if (statep -> currstate == tm.rule[i].currstate &&
        statep -> currsym   == tm.rule[i].currsym) {
      statep -> nextsym   = tm.rule[i].nextsym;
      statep -> dir       = tm.rule[i].dir;
      statep -> nextstate = tm.rule[i].nextstate;
      return 1;
    }
  }
  return 0;
}


void
init_tapezero(tm_t tm)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    tmtape[i] = tm.sym0;
  }
  return;
}


void
init_tapeuni(tm_t tm, int arg1, int arg2)
{
  init_tapezero(tm);
  for (int i = 1; i < 1 + arg1; i++) {
    tmtape[i] = tm.sym1;
  }
  for (int i = 1 + arg1 + 1; i < 1 + arg1 + 1 + arg2; i++) {
    tmtape[i] = tm.sym1;
  }
  return;
}


int
tapebin1(int pos, int arg, byte sym0, byte sym1)
{
  int len = -1;
  for (int mask = (1 << 8); 0 < mask; mask >>= 1) {  // 8bit max
    if  ((arg & mask) != 0) {
      tmtape[pos] = sym1;
      if (len < 0) {
        len = 0;
      }
    } else {
      tmtape[pos] = sym0;
    }
    if (0 <= len) {
      len++;
      pos++;
    }
  }
  return len;
}


void
init_tapebin(tm_t tm, int arg1, int arg2)
{
  int len1;

  init_tapezero(tm);
  len1 = tapebin1(TPOS_CENTER + 1, arg1, tm.sym1, tm.sym2);
  tapebin1(TPOS_CENTER + 1 + len1 + 1, arg2, tm.sym1, tm.sym2);
  return;
}


void
init_turing(tm_t tm)
{
  currmachine = tm;
  setall(COL_BLACK, 1);
  tmheadpos = tm.initpos;
  tmstate.currstate = tm.initstate;
  tmstate.currsym   = tmtape[tmheadpos];
  switch (tm.speed) {
  case 0:
    tmcycle = TMCYCLE_S;
    break;
  case 2:
    tmcycle = TMCYCLE_M;
    break;
  case 3:
    tmcycle = TMCYCLE_H;
    break;
  default:
    tmcycle = TMCYCLE_STD;
  }
  tmrun = 1;
  tmsteps = 0;
  tmcount = 0;
  disp_tape();
  Serial.print("run...\n");
  return;
}


void
halt_turing(void)
{
  tmrun = 0;
  Serial.print("halt\n");
  tmcycle = TMCYCLE_STD;
  tmhaltcount = 0;
  tmcount = 0;
  return;
}


void
update_turing(void)
{
  static int nextheadpos;
  int tmbigcyc;

  if (0 < tmcount % tmcycle) {
    tmcount++;
    return;
  }
  tmbigcyc = tmcount / tmcycle;
  if (tmrun == 0) {
    if (tmbigcyc == 0) {
      write_led(tmheadpos, COL_RED, tmcycle * TMHALTSTEP);
    } else if (tmbigcyc == TMHALTSTEP) {
      write_led(tmheadpos, tmtcol[tmtape[tmheadpos]],
                tmcycle * TMHALTSTEP);
    } else if (tmbigcyc == TMHALTSTEP * 2) {
      tmcount = 0;
      tmhaltcount++;
      if (isautodemo && TMAUTOHALTQUIT <= tmhaltcount) {
        init_autodemo();
      }
      return;
    }
    tmcount++;
    return;
  }
  if (tmbigcyc < TMBLINKSTEP) {   // 0.00-1.00s
    if (tmbigcyc % 2 == 0) {
      write_led(tmheadpos, COL_YELLOW, 1);
    } else {
      write_led(tmheadpos, tmtcol[tmtape[tmheadpos]], 1);
    }
  }
  switch (tmbigcyc) {
  case 0:   // 0.00s (hold -1.00s)
    tmstate.currsym = (int)tmtape[tmheadpos];
    if (tmgetrule(currmachine, &tmstate) == 0) {
      Serial.printf("error: no (state %d, symbol %d). ", tmstate.currstate, tmstate.currsym);
      halt_turing();
      return;
    }
    nextheadpos = tmheadpos + tmstate.dir;
    Serial.printf("step %d: state %d->%d, head @%d->@%d: symbol %d->%d\n",
                  tmsteps,
                  tmstate.currstate, tmstate.nextstate, tmheadpos, nextheadpos,
                  tmstate.currsym, tmstate.nextsym); 
    break;
  case TMBLINKSTEP - 1:
    write_led(tmheadpos, tmtcol[tmtape[tmheadpos]], 1);
    break;
  case TMBLINKSTEP:
    write_led(tmheadpos, tmtcol[tmstate.nextsym], tmcycle * TMREWSTEP);
    break;
  case TMREWRITESTEP:
    tmtape[tmheadpos] = (byte)tmstate.nextsym;
    if (nextheadpos < 0 || NUM_LEDS <= nextheadpos ||
        tmstate.nextstate == -1) {
      halt_turing();
      return;
    }
    tmheadpos = nextheadpos;
    tmstate.currstate = tmstate.nextstate;
    tmsteps++;
    tmcount = 0;
    return;    
  }
  tmcount++;
  return;
}
