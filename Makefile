CC = riscv64-unknown-elf-gcc

CFLAGS = -Wall -g -O1 -ffunction-sections -fno-inline -gdwarf-2 \
         -march=rv32imf_zicsr_zicbom -mabi=ilp32f \
         -I. -Igame_logic -Ihardware -Isources \
         -I/usr/lib/picolibc/riscv64-unknown-elf/include

LDFLAGS =

TARGET = test.elf

SRCS = start.s $(wildcard game_logic/*.c sources/*.c hardware/*.c)

all:
	$(CC) $(CFLAGS) -nostdlib -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)