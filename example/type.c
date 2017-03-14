char f_char() {
  char c, *cc;
  c = 256;
  cc = &c;
  return *cc;
}

short f_short() {
  short s;
  s = 256;
  return s;
}

float f_float() {
  float f;
  f = 123.4567;
  return f;
}

double f_double() {
  double d;
  d = 987.654;
  return d;
}

int f_int() {
  int i, *ii;
  i = 12;
  ii = &i;
  return *ii;
}

int main() {
  char c;
  short s;
  float f;
  int i;
  double d;
  c = f_char();
  s = f_short();
  f = f_float();
  i = f_int();
  d = f_double();
  return 123;
}
