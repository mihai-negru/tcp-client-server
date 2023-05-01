CC 			:= 	gcc
CFLAGS 		:= 	-g -Wall -Wextra -Wpedantic 					\
				-Wformat=2 -Wno-unused-parameter 				\
				-Wshadow -Wwrite-strings -Wstrict-prototypes 	\
				-Wold-style-definition -Wredundant-decls 		\
				-Wnested-externs -Wmissing-include-dirs 		\
				-Wjump-misses-init -Wlogical-op -O2

RM			:= 	rm
RFLAGS		:= 	-rf

SRC			:= 	src
SRC_FILES	:= $(wildcard $(SRC)/*.c)

EXEC_FILES	:= 	server subscriber
O_FILES		:= 	$(patsubst $(SRC)/%.c,%.o,$(SRC_FILES))

.PHONY: clean
.PRECIOUS: %.o

all: $(EXEC_FILES)

%.o: $(SRC)/%.c
	@$(CC) $(CFLAGS) -c $<

server: server.o server_utils.o utils.o poll_vec.o udp_type.o tcp_type.o client_vec.o
	@$(CC) $^ -o $@

subscriber: subscriber.o subscriber_utils.o utils.o poll_vec.o tcp_type.o
	@$(CC) $^ -o $@

# %: %.o %_utils.o utils.o poll_vec.o
# 	@$(CC) $^ -o $@

clean:
	@$(RM) $(RFLAGS) $(EXEC_FILES) $(O_FILES)

clean_o:
	@$(RM) $(RFLAGS) $(O_FILES)
