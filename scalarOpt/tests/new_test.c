int a, b, c;
int fa (int n)
{
  a += n;
  return a;
}
int fb (int n)
{
  b *= n;
  return b;
}
int fc (int n)
{
  c += n*2;
  return c;
}
int fact(int n) {
  // factorial
  if (n == 1 || n == 0)
    return n;
  return n*fact(n-1);

}
int recur(int a1, int a2, int a3)
{
  if ((fa(a1)+fb(a2)+fc(a3)) % 2 == 0)
    return fact(fa(a1)+fb(a2)+fc(a3));
  else 
    return fact(fa(a1)+fb(a2));
}

