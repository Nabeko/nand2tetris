CC := gcc
OBJS := main.o jacktokenizer.o compilationengine.o symboltable.o vmwriter.o
TARGET := JackCompiler

$(TARGET):$(OBJS)
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm -rf *.o $(TARGET)
