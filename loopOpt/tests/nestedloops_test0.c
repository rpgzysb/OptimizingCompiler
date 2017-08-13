int nest (int a, int b, int c)
{
  int i, j, k, res = 0;
  for (i = 0; i < a; ++i) {
    int x0 = a + b; // i loop inv
    int x1 = c * 5; // i loop inv
    res += i + x1;
    for (j = i; j < b; ++j) {
      int x2 = a + c; // i loop inv
      int x3 = x0 + c; // i loop inv
      int x4 = x1 + i; // j loop inv 
      int x5 = i + j; // non inv
      res += j + x4;
      for (k = j; k < c; ++k) {
        int x6 = x1 - x5; // k loop inv
        int x7 = x4 - x3; // j loop inv
        int x8 = x1 + x2; // i loop inv
        int x9 = x1 + k; // non inv
        res += x9 - 2 * k;
      }
    }
    
  }
  return res;
}

int main() {
  return nest(50000, 10, 15);
}