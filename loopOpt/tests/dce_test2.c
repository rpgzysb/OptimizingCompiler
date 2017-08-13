int dead_func(int a, int b, int c, int d) {
  int j, k;
  for (int i = 0; i < 233; ++i) {
    j = b + c - i;
    k = b + c + d;
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