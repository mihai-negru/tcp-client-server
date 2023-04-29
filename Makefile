CC 			:= 	gcc
CFLAGS 		:= 	-g -Wall -Wextra -Wpedantic \
				-Wformat=2 -Wno-unused-parameter \
				-Wshadow -Wwrite-strings -Wstrict-prototypes \
				-Wold-style-definition -Wredundant-decls \
				-Wnested-externs -Wmissing-include-dirs \
				-Wjump-misses-init -Wlogical-op -O2

RM			:= 	rm
RFLAGS		:= 	-rf

SRC			:= 	src

EXEC_FILES	:= 	server subscriber
O_FILES		:= 	utils.o poll_vec.o server.o server_utils.o subscriber.o subscriber_utils.o

.PHONY: clean
.PRECIOUS: %.o

all: $(EXEC_FILES)

%.o: $(SRC)/%.c
	@$(CC) $(CFLAGS) -c $<

%: %.o %_utils.o utils.o poll_vec.o
	@$(CC) $^ -o $@

clean:
	@$(RM) $(RFLAGS) $(EXEC_FILES) $(O_FILES)
