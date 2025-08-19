
.PHONY: all
all: jiraDoom

jiraDoom: main.c
	gcc -O2 -std=c99 -Wshadow -fsanitize=address,undefined -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable main.c -o jiraDoom -I./raylib-linux/include -L./raylib-linux/lib -l:libraylib.a -lm

.PHONY: clean
clean:
	rm -f jiraDoom

