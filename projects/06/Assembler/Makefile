CC := gcc
OBJS := main.o parser.o code.o symboltable.o
TARGET := HackAssembler

HackAssembler:$(OBJS)
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm -rf *.o $(TARGET)
