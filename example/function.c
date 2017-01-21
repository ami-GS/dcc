/*
This is function caller

Usage

>> make
>> dcc example/function.c -srm

Final output line should be like
1: STOP 0 334
*/

int FUNC3(int d, int e, int f) {
  return 300 + d + e + f;
}

int FUNC1(int a);
int FUNC2(int b, int c);
int main() {
  int a;
  a = FUNC1(1);
  
  return a;
}

int FUNC2(int b, int c) {
  return 20 + FUNC3(2, b, c);
}

int FUNC1(int a) {
  return 10 + FUNC2(1, a);
}
