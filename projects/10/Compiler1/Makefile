CC := gcc
OBJS := main.o jacktokenizer.o compilationengine.o
TARGET := JackAnalyzer

$(TARGET):$(OBJS)
	$(CC) $^ -o $@

%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm -rf *.o $(TARGET)
