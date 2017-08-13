int c;

int fc (int n)
{ 
  int c1 = 3, c2 = 8, c3 = 4, c4 = 15;
  int c5 = 16, c6 = 32, c7 = 64;
  
  c -= c5 * c6;
  c += c1 + c2 + c3;
  c += c1 + c4;
  c += c5 + c1 * c5;
  c += c3 / c3;
  c += c2 / c4;

  c += n + 0;
  c += 0 + n;
  c += 2 * n - 0;
  c += n - n;
  c += n * 1;
  c += 1 * n;
  c -= n / 1;
  c -= n / n;

  c += n * c7;
  c -= n / c6;
  c -= c5 * n;
  return c;
}
/*
int fact(int n) {
  // factorial
  if (n == 1 || n == 0)
    return n;
  return n*fact(n-1);

}
int recur(int a1, int a2, int a3)
{
  return fact(fc(a1 / a1 + a2 + a3));
}
*/

