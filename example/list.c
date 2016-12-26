struct list {
  int val;
  struct list *nxt;
};

int main() {
  struct list LIST[5];
  struct list *lp;
  struct list *lps;
  int i;
  LIST[0].val = 11;
  lp = &LIST[0];
  lps = lp;
  for (i = 1; i < 5; i++) {
    LIST[i].val = 11*(i+1);
    lp->nxt = &LIST[i];
    lp = lp->nxt;
  }
  return lps->nxt->nxt->nxt->nxt->val;
}
