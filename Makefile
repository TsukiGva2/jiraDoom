
.PHONY: all
all: jiraDoom

jiraDoom: main.c
	gcc -Ofast -std=c99 -Wno-shadow -fsanitize=address,undefined -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable main.c -o jiraDoom -I./raylib-linux/include -L./lua-linux/src -I./lua-linux/src -L./raylib-linux/lib -l:liblua.a -l:libraylib.a -lm

.PHONY: clean
clean:
	rm -f jiraDoom

