CFLAGS ?= -O2 -g

CFLAGS += -std=gnu99

CFLAGS += -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self \
        -Wswitch-default -Wpointer-arith -Wtype-limits -Wempty-body \
        -Wstrict-prototypes -Wold-style-declaration -Wold-style-definition \
        -Wmissing-parameter-type -Wmissing-field-initializers -Wnested-externs \
        -Wstack-usage=4096 -Wmissing-prototypes -Wfloat-equal -Wabsolute-value

CFLAGS += -fsanitize=undefined -fsanitize-undefined-trap-on-error

CC += -m32 -no-pie -fno-pie

LDLIBS = -lm

.PHONY: all clean
all: integral

integral: f.o
	$(CC) $(LDLIBS) -o integral f.o main.c

f.o: f.asm
	nasm -felf32 -o f.o f.asm

clean:
	rm -f *.o integral

.PHONY: test
test: integral
	./integral --test-root 1:2:-5.0:-0.1:0.0001:-0.3718
	./integral --test-root 1:3:-5:0:1.0:0.0001:-4.027
	./integral --test-root 2:3:-5.0:-0.1:0.0001:-1.822
	./integral --test-integral 1:1.0:2.0:0.0001:6.67077
	./integral --test-integral 2:-5.0:-4.0:0.0001:0.22314
	./integral --test-integral 3:-1.0:0.0:0.0001:-0.33333

