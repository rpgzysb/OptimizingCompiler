int dead_func(int a, int b, int c, int d) {
  int i, j, k, sum;
  i = 1;
  j = 2;
  k = 3;
  sum = i + j + k;
  return a + b + c + d;
}

int main() {
  int num;
  for (int i = 0; i < 10000; ++i) {
  	num = dead_func(1, 2, 3, 4);
  }
  return num;
}