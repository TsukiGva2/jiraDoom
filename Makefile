
.PHONY: all
all: jiraDoom

jiraDoom: main.c
	gcc -Ofast -std=c99 -Wshadow -fsanitize=address,undefined -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable main.c -o jiraDoom -I./raylib-linux/include -L./raylib-linux/lib -l:libraylib.a -lm

.PHONY: debug
debug: main.c
	gcc -g -std=c99 -Wshadow -fsanitize=address,undefined -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -DDEBUG_BUILD main.c -o jiraDoom -I./raylib-linux/include -L./raylib-linux/lib -l:libraylib.a -lm

.PHONY: clean
clean:
	rm -f jiraDoom

