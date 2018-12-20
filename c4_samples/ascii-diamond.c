//v;main(i){for(;i<307;putchar(i++%18?v>8?32:57-v:10))v=abs(i%18-9)+abs(i/18-8);}

int v;

int abs(int n) { return n < 0 ? n * -1 : n; }
void putchar(char c) { printf("%c", c); }


int main(int argc, char **arv){
	int i;
	i = argc;
	while(i < 307) {
		v = abs((i % 18) - 9) + abs(i / 18 - 8);
		putchar(i++ % 18 ? v > 8 ? 32 : 57 - v : 10);
	}
	return 0;
}
