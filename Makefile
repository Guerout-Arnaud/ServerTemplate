##
## EPITECH PROJECT, 2018
## Makefile
## File description:
## make / clean / fclean / re
##

## --------- COLOR ------##

DEFAULT	=	"\033[00m"
GREEN	=	"\033[1;32m"
TEAL	=	"\033[1;36m"
YELLOW	=	"\033[1;7;25;33m"
MAGENTA	=	"\033[1;3;4;35m"
ERROR	=	"\033[5;7;1;31m"

ECHO	=	echo -e

## -------- COMPIL ----##

CC	=	gcc

## ------- DIR --------##

SRC_DIR	=	./src/server

TEST_DIR	=	./tests

RESSOURCE_DIR	=	./ressource

## -------- SRC -------##

SRC_TEST	=	$(TEST_DIR)/ \

MAIN	=	$(SRC_DIR)/main.c

SRC	=	$(SRC_DIR)/server/server.c	\
		$(SRC_DIR)/server/message.c	\
		$(SRC_DIR)/server/connection.c	\
		$(SRC_DIR)/game/game.c	\

OBJ	=	$(SRC:.c=.o)

OBJ_MAIN	=	$(MAIN:.c=.o)

OBJ_TEST	=	$(SRC_TEST:.c=.o)

## ------- FLAGS --------##

cflags.common	:=	-W -Wall -Wextra -Wno-unused-variable -Wno-unused-function -D_GNU_SOURCE -I./include/ -I./lib/include/
cflags.debug	:=	-g3  -save-temps=obj
cflags.release	:= 
cflags.tests	:=

ldflags.common	:= -L./lib/output -llog -llinked_list -pthread
ldflags.debug	:=
ldflags.release	:=
ldflags.tests	:=	-lcriterion -lgcov  --coverage

CFLAGS	:=	${cflags.${BUILD}} ${cflags.common}
LDFLAGS	:=	${ldflags.${BUILD}} ${ldflags.common}

## ------- BIN ----------##

BINNAME	= template_srv

TEST_BIN	=	$(BINNAME)_test

## ------- BUILD ----------##

BUILD   =   release

## --------- RULES --------##

all: 
	@make -s $(BINNAME) && \
	$(ECHO) $(GREEN) "[OK]"$(TEAL)"  Done : " $@ $(DEFAULT)  || \
	$(ECHO) $(ERROR) "[ERROR]" $(YELLOW) $(BINNAME) $(DEFAULT)

set_rules_common	:
			$(eval BUILD=common)
			$(eval CFLAGS=$(cflags.common))
			$(eval LDFLAGS=$(ldflags.common))

$(BINNAME)	:	set_rules_common $(OBJ) $(OBJ_MAIN)
				@make -s -C ./lib/
				@$(CC) -o $(BINNAME) $(OBJ) $(OBJ_MAIN) $(CFLAGS) $(LDFLAGS)

%.o	:	%.c
		@$(CC)  $(CFLAGS) -c $< -o $@ && \
		$(ECHO) $(GREEN) "[OK] " $(DEFAULT) $(TEAL) $<  $(DEFAULT)  " -----> " $(GREEN) $@ $(DEFAULT) || \
		$(ECHO) $(ERROR) " [ERROR] can't find " $(YELLOW) $^ $(DEFAULT)

set_rules_test	:
			$(eval BUILD=tests)
			$(eval CFLAGS=${cflags.tests} $(cflags.common))
			$(eval LDFLAGS=${ldflags.tests} $(ldflags.common))

$(TEST_BIN)	:	set_rules_test $(OBJ_TEST) $(OBJ)
			@make -s -C ./lib/
			@$(CC) -o $(TEST_BIN) $(OBJ_TEST) $(OBJ) $(CFLAGS) $(LDFLAGS)

tests_run	:	set_rules_test $(TEST_BIN)
			@./$(TEST_BIN) && \
			$(ECHO) $(GREEN) "[OK]"$(TEAL)"  Done : " $@ $(DEFAULT)  || \
			$(ECHO) $(ERROR) "[ERROR]" $(YELLOW) $(BINNAME) $(DEFAULT)

set_rules_debug	:
			$(eval BUILD=debug)
			$(eval CFLAGS=${cflags.debug} $(cflags.common))
			$(eval LDFLAGS=${ldflags.debug} $(ldflags.common))

debug	:	 set_rules_debug $(OBJ) $(OBJ_MAIN)
			@$(CC) -o $(BINNAME) $(OBJ) $(OBJ_MAIN) $(CFLAGS) $(LDFLAGS) && \
		        $(ECHO) $(GREEN) "[OK]"$(TEAL)"  Done : " $@ $(DEFAULT)  || \
			$(ECHO) $(ERROR) "[ERROR]" $(YELLOW) $(BINNAME) $(DEFAULT)

set_rules_release	:
			$(eval BUILD=release)
			$(eval CFLAGS=${cflags.release} $(cflags.common))
			$(eval LDFLAGS=${ldflags.release} $(ldflags.common))

release	:	 set_rules_release
		@make -s $(BINNAME) && \
		$(ECHO) $(GREEN) "[OK]"$(TEAL)"  Done : " $@ $(DEFAULT)  || \
		$(ECHO) $(ERROR) "[ERROR]" $(YELLOW) $(BINNAME) $(DEFAULT)
		
install	: 
		@make -s release && \
		$(ECHO) $(GREEN) "[OK]"$(TEAL)"  Done : " $@ $(DEFAULT)  || \
		$(ECHO) $(ERROR) "[ERROR]" $(YELLOW) $(BINNAME) $(DEFAULT)
		@[ -d $(INSTALL_DIR) ] || \
		($(ECHO) $(TEAL) "[INFO] $($(INSTALL_DIR)) folder not found. Generating..." $(DEFAULT)  && \
		$(CREATE_DIR) $(INSTALL_DIR))
		$(COPY) $(BINNAME).service $(SERVICE_DIR) 

clean:
	@make -s clean -C ./lib/
	@$(foreach i, $(OBJ), $(shell rm -rf $(i)) echo -e $(MAGENTA) "\tRemoved:  $(i)" $(DEFAULT);)
	@echo -e $(MAGENTA) "\tRemoved: $(OBJ_MAIN)" $(DEFAULT)
	@rm -rf $(OBJ_MAIN)
	@$(foreach k, $(OBJ_TEST), $(shell rm -rf $(i)) echo -e $(MAGENTA) "\tRemoved:  $(k)" $(DEFAULT);)
	@find -name "*.gcda" -delete
	@find -name "*.gcno" -delete
	@find -name "*.gcov" -delete && \
        $(ECHO) $(GREEN) "[OK]"$(TEAL)"  Done : " $@ $(DEFAULT)  || \
        $(ECHO) $(ERROR) "[ERROR]" $(YELLOW) $(BINNAME) $(DEFAULT)


fclean:	clean
	@make -s fclean -C ./lib/
	@rm -rf $(BINNAME)
	@rm -rf $(TEST_BIN) && \
        $(ECHO) $(GREEN) "[OK]"$(TEAL)"  Done : " $@ $(DEFAULT)  || \
        $(ECHO) $(ERROR) "[ERROR]" $(YELLOW) $(BINNAME) $(DEFAULT)


re:	fclean all