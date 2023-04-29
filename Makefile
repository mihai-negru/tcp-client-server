CC 			:= gcc
CFLAGS 		:= -g -Wall -Wextra

RM			:= rm
RFLAGS		:= -rf

SRC			:= src

EXEC_FILES	:= server subscriber
O_FILES		:= server.o server_utils.o subscriber.o subscriber_utils.o

.PHONY: clean
.PRECIOUS: %.o

all: $(EXEC_FILES)

%.o: $(SRC)/%.c
	@$(CC) $(CFLAGS) -c $<

%: %.o %_utils.o
	@$(CC) $^ -o $@

clean:
	@$(RM) $(RFLAGS) $(EXEC_FILES) $(O_FILES)
