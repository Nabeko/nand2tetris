CC := gcc
OBJS := main.o parser.o codewriter.o
TARGET := VMTranslator

$(TARGET):$(OBJS)
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm -rf *.o $(TARGET)
