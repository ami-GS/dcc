/*
This is sort function validator.

Usage

>> make
>> dcc example/sort.c -srm

Sorted array can be seen by usin check_stack
The output line should be like
4:   STO    40    9 8 7 6 5 4 3 2 1 0
*/

int check_stack(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
  return 111;
}

void bubble_loop(int len) {
  int A[] = {9,8,7,6,5,4,3,2,1,0};
  int i, j, tmp;
  for (i = 0; i < len-1; i++) {
    for (j = len-1; j > i; j--) {
      if (A[j-1] > A[j]) {
	tmp = A[j-1];
	A[j-1] = A[j];
	A[j] = tmp;
      }
    }
  }
  //check_stack(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[8], A[9]);
}

void quick_loop(int len) {
  int A[] = {9,10,7,8,5,0,3,22,1,6};
  int stack[10];
  int l_base, r_base, l, r, tmp, pivot, idx = 0;
  stack[idx++] = 0;
  stack[idx++] = len-1;

  while (idx != 0) {
    // start
    r_base = stack[--idx];
    l_base = stack[--idx];
    
    l = l_base;
    r = r_base;
    pivot = A[(r+l)/2];
    
    while (1) {
      while (A[l] < pivot) ++l;
      while (A[r] > pivot) --r;
      if (l >= r)
	break;
      tmp = A[l];
      A[l++] = A[r];
      A[r--] = tmp;
    }
    if (r_base > r + 1) {
      // right side
      stack[idx++] = r+1;
      stack[idx++] = r_base;
    }
    if (l_base < l - 1) {
      // left side
      stack[idx++] = l_base;
      stack[idx++] = l-1;      
    }
  }
  check_stack(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[8], A[9]);
}

void quick_rec(int *list, int st, int en) {
  int i = st, j = en, tmp;
  int pivot;
  pivot = list[(st+en)/2];
  while (1) {
    while (list[i] < pivot) ++i;
    while (list[j] > pivot) --j;
    if (i >= j)
      break;
    tmp = list[i];
    list[i++] = list[j];
    list[j--] = tmp;
  }
  if (st < i - 1)
    quick_rec(list, st, i-1);
  if (en > j + 1)
    quick_rec(list, j+1, en);
}

int main() {
  int len = 10;
  int A[] = {9,10,7,8,5,0,3,22,1,6};
  bubble_loop(len);
  quick_loop(len);
  quick_rec(A, 0, len-1);
  check_stack(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7], A[8], A[9]);
  return 10101;
}
