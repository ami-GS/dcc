int main() {
  int a=4, b=2, c=0;
  c += a + b;  //6
  c += a - b;  //8
  c += a * b;  //16
  c += a / b;  //18
  c += a % b;  //18
  c += a == b; //18
  c += a != b; //19
  c += a < b;  //19
  c += a <= b; //19
  c += a > b;  //20
  c += a >= b; //21
  c += a && b; //22
  c += a || b; //23
  c += a & b;  //23
  c += a | b;  //29
  c += a ^ b;  //35
  c += a << b; //51
  c += a >> b; //52
  c += -a;     //48
  c += +a;     //52
  c += ~a;     //47
  c += ++a;    //52
  c += a++;    //57
  c += --a;    //62
  c += a--;    //67
  return c;
}