CC 			:= gcc
CFLAGS 		:= -g -Wall -Wextra

RM			:= rm
RFLAGS		:= -rf

SRC			:= src

EXEC_FILES	:= server subscriber
O_FILES		:= server.o subscriber.o

.PHONY: clean

all: $(EXEC_FILES)

%.o: $(SRC)/%.c
	-@$(CC) $(CFLAGS) -c $<

server: server.o
	-@$(CC) $< -o $@

subscriber: subscriber.o
	-@$(CC) $< -o $@

clean:
	-@$(RM) $(RFLAGS) $(EXEC_FILES) $(O_FILES)

