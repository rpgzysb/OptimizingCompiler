
int main()
{
	int x = 3;
	int y = 5;
	int z = x + y;
	int a1 = 0;
	int l = 0 + a1;
	int w = -y;
	w = x + w;
	int h = x + 10;

	for (int i = 1; i < 10; ++i) {
		x += i;
		y += i;
		z += i;
		w += x;
		h += z;
		l += 0;
	}

	return h;
}