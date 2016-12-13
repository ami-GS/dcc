int factorial_rec(int n) {
  if (n == 1)
    return 1;
  else
    return n * factorial_rec(n-1);
}

int euclidean(int a, int b) {
  int r = a % b; // a > b
  while (r != 0) {
    a = b;
    b = r;
    r = a % b;
  }
  return b;
}


int main () {
  // 120 - 24 = 96
  return factorial_rec(5) - euclidean(72, 48);
}
