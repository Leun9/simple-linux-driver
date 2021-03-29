src = $(wildcard *.cpp)
target = $(patsubst %.cpp, %, ${src})

.PHONY: all clean

%:%.c
	g++ $^ -o $@

all: ${target}

clean:
	rm -f ${target}