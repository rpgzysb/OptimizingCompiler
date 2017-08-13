
#include <stdio.h>

int main()
{
  int a, h, m, n, r, q, x, y, z;
  
  y = 0;
  z = 4;
  a = y + z;
  x = z + 3;

  for (int i = 0; i < z + 40000; ++i) {

    int seconddepth = z + a;
    m = seconddepth + i;
    n = seconddepth + x;

    for (int i = 0; i < a + 4; ++i) {     
      int newd = x + a;
      q = newd + 3;
      y += i;
    }
  }
  return 0;
}