# 1. 定义编译器
CC = gcc

# 2. 定义编译选项 (加上 -lm 用来链接数学库)
CFLAGS = -lm

# 3. 定义最终生成的可执行文件名字
TARGET = test.elf

# 4. 自动抓取这三个文件夹下的所有 .c 文件
SRCS = $(wildcard game_logic/*.c sources/*.c hardware/*.c)

# 5. 默认的编译规则 (直接敲 make 就会跑这里)
all:
	$(CC) -o $(TARGET) $(SRCS) $(CFLAGS)

# 6. 清理规则 (敲 make clean 会删掉生成的 test.elf)
clean:
	rm -f $(TARGET)