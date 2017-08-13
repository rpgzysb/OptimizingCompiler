int dead_func(int a, int b, int c, int d) {
  int j = b;
  int sum = 0;
  for (int i = c; i < d; ++i) {
    sum += j;
    j++;
  }
  return a + b + c + d;
}

int main() {
  int num;
  for (int i = 0; i < 10000; ++i) {
    num = dead_func(1, 2, 3, 4);
  }
  return num;
}