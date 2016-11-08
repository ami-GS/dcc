/*
This is fibonacci calculator.

Usage

>> make
>> dcc example/fib.c -sm

Final output line should be like
1: STOP 0 34
*/

int fib_recursive(int c) {
  // recursive implementation (slow)
  if (c == 0)
    return 0;
  else if (c == 1)
    return 1;
  else
    return fib_recursive(c-2) + fib_recursive(c-1);
}

int fib_loop(int c) {
  // loop implementation (much faster than recursive)
  int bef = 0, af = 1, tmp;
  while (c > 0) {
    tmp = af;
    af += bef;
    bef = tmp;
    --c;
  }
  return bef;
}

int main() {
  int level = 11;
  return fib_loop(level) - fib_recursive(level-1);
  // 89 - 55 = 34
}
