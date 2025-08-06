## **************************************************************************** #
##                                                                              #
##                                                         :::      ::::::::    #
##    Makefile                                           :+:      :+:    :+:    #
##                                                     +:+ +:+         +:+      #
##    By: lchauvet <lchauvet@student.42.fr>          +#+  +:+       +#+         #
##                                                 +#+#+#+#+#+   +#+            #
##    Created: 2024/11/19 15:53:44 by mgendrot          #+#    #+#              #
##    Updated: 2025/01/31 17:09:37 by lchauvet         ###   ########.fr        #
##                                                                              #
## **************************************************************************** #

# Variables

NAME			= malloc
LIB_NAME		= libmalloc.so

INCLUDE			= include

TMP				= .tmp/
CC				= cc 
CFLAGS_Debug	= -fsanitize=address,undefined -g
CFLAGS			= -Wall -Wextra -Werror -g -O3 -fPIC

RM				= rm -f
MKDIR			= mkdir

LIBFT_DIR		= libft/
LIBFT_CFLAGS	= -L . -l ft
LIBFT			= libft.a
LIBFT_INC_H		= include/libft.h

# Test variables
TEST_DIR		= test/
TEST_RUNNER		= test_runner
TEST_SRC		= $(TEST_DIR)test_runner.c $(TEST_DIR)unit_tests.c
TEST_OBJ		= $(TEST_SRC:.c=.o)



# **************************************************************************** #
#                                   Colors                                     #
# **************************************************************************** #

DEF_COLOR		= \033[0;39m
GRAY			= \033[0;90m
RED				= \033[0;91m
GREEN			= \033[0;92m
YELLOW			= \033[0;93m
BLUE			= \033[0;94m
MAGENTA			= \033[0;95m
CYAN			= \033[0;96m
WHITE			= \033[0;97m
BOLD			= $'\x1b[1m'
END				= $'\x1b[0m'
TERM_UP			= \033[1A
TERM_CLEAR_LINE	= \033[2K\r

# **************************************************************************** #
#                                   Sources                                    #
# **************************************************************************** #

SRC_DIR		= src/

SRC = $(shell find ./src -iname "*.c")


OBJ_DIR			= $(TMP)obj/
OBJ				= $(patsubst %.c, $(OBJ_DIR)%.o, $(SRC))

DEPS			= $(OBJ:.o=.d)

# **************************************************************************** #
#                             progress_update                                  #
# **************************************************************************** #

TOTAL_FILES     := $(words $(OBJ))
COMPILED_COUNT  = 0

define progress_update
    @if [ ! -f .compiled_count ]; then \
        echo 0 > .compiled_count; \
        printf "\n"; \
    fi; \
    COMPILED_COUNT=$$(cat .compiled_count); \
    COMPILED_COUNT=$$((COMPILED_COUNT + 1)); \
    echo $$COMPILED_COUNT > .compiled_count; \
    printf "$(TERM_UP)$(TERM_CLEAR_LINE)$(CYAN)Compilation progress: \
        $$((COMPILED_COUNT * 100 / $(TOTAL_FILES)))%% $(BLUE) ->$(1) $(DEF_COLOR)\n"; \
    if [ $$COMPILED_COUNT -eq $(TOTAL_FILES) ]; then \
        rm -f .compiled_count; \
    fi
endef
# **************************************************************************** #\
#                                   Rules                                      #
# **************************************************************************** #

all: lib $(LIB_NAME)
	@printf "$(GREEN)All targets compiled successfully!$(DEF_COLOR)\n"

# **************************************************************************** #
#                                   Library                                    #
# **************************************************************************** #

$(LIB_NAME): $(OBJ)
	@$(CC) -shared $(CFLAGS) $(OBJ) -o $(LIB_NAME)
	@printf "$(GREEN)Dynamic library %s compiled!$(DEF_COLOR)\n" $(LIB_NAME)

$(NAME): $(LIB_NAME)
	@ln -sf $(LIB_NAME) lib$(NAME).so
	@printf "$(GREEN)Symlink created: lib$(NAME).so -> $(LIB_NAME)$(DEF_COLOR)\n"

# **************************************************************************** #
#                                   libift                                     #
# **************************************************************************** #

cleanlibs:
	@$(MAKE) clean -C $(LIBFT_DIR)	-s
	
fcleanlibs:
	@$(MAKE) fclean -C $(LIBFT_DIR) -s
	@$(RM) $(LIBFT)
	@$(RM) $(LIBFT_INC_H)

lib : $(LIBFT)

$(LIBFT):
	@printf "$(MAGENTA)Making archive $(TERM_BLUE)\"%s\"$(MAGENTA)...$(DEF_COLOR)\n" $@
	@$(MAKE) -C $(LIBFT_DIR) -s
	@cp -u $(LIBFT_DIR)/$(LIBFT_INC_H) $(INCLUDE)
	@cp $(LIBFT_DIR)/$(LIBFT) ./ 
	@printf "$(TERM_CLEAR_LINE)$(GREEN)Done copying archive $(BLUE)\"%s\"$(DEF_COLOR) !\n$(TERM_RESET)" $@

relibft: fcleanlibs lib

# **************************************************************************** #
#                                   lem-in                                     #
# **************************************************************************** #

# Remove the old executable compilation rule since we're building a library now

$(OBJ_DIR)%.o: %.c | $(OBJF)
	@mkdir -p $(dir $@)
	$(call progress_update,$(notdir $@))
	@$(CC) $(CFLAGS) -I $(INCLUDE) -c $< -o $@

-include $(DEPS)

%.d: %.c
	@$(CC) $(CFLAGS) -MM $< -MF $@

$(OBJF):
	@$(MKDIR) -p $(OBJ_DIR)

# **************************************************************************** #
#                                   Tests                                      #
# **************************************************************************** #

test: $(LIB_NAME) $(TEST_RUNNER)
	@printf "$(CYAN)Running unit tests...$(DEF_COLOR)\n"
	@LD_LIBRARY_PATH=. ./$(TEST_RUNNER)

$(TEST_RUNNER): $(TEST_SRC) $(LIB_NAME)
	@printf "$(MAGENTA)Compiling test runner...$(DEF_COLOR)\n"
	@$(CC) $(CFLAGS) -I $(INCLUDE) $(TEST_SRC) -L. -l:$(LIB_NAME) -o $(TEST_RUNNER)

test-clean:
	@$(RM) $(TEST_RUNNER) $(TEST_OBJ)
	@printf "$(YELLOW)Test files cleaned$(DEF_COLOR)\n"

# Test with debug flags
test-debug: CFLAGS += -DDEBUG -fsanitize=address,undefined
test-debug: test

# Run tests with valgrind (if available)
test-valgrind: $(LIB_NAME) $(TEST_RUNNER)
	@printf "$(CYAN)Running tests with valgrind...$(DEF_COLOR)\n"
	@LD_LIBRARY_PATH=. valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_RUNNER)

# Install the library (optional)
install: $(LIB_NAME)
	@printf "$(MAGENTA)Installing library...$(DEF_COLOR)\n"
	@sudo cp $(LIB_NAME) /usr/local/lib/
	@sudo ldconfig
	@printf "$(GREEN)Library installed successfully$(DEF_COLOR)\n"

# Help target
help:
	@printf "$(YELLOW)Available targets:$(DEF_COLOR)\n"
	@printf "  $(BLUE)all$(DEF_COLOR)         - Build the library\n"
	@printf "  $(BLUE)test$(DEF_COLOR)        - Run unit tests\n"
	@printf "  $(BLUE)test-debug$(DEF_COLOR)  - Run tests with debug flags\n"
	@printf "  $(BLUE)test-valgrind$(DEF_COLOR) - Run tests with valgrind\n"
	@printf "  $(BLUE)clean$(DEF_COLOR)       - Clean object files\n"
	@printf "  $(BLUE)fclean$(DEF_COLOR)      - Clean everything\n"
	@printf "  $(BLUE)re$(DEF_COLOR)          - Rebuild everything\n"
	@printf "  $(BLUE)install$(DEF_COLOR)     - Install library system-wide\n"
	@printf "  $(BLUE)help$(DEF_COLOR)        - Show this help\n"

clean:
	@if [ -d $(OBJ_DIR) ]; then \
		$(RM) -r $(OBJ_DIR); \
		printf "$(YELLOW)Removing %d objects from \"%s\" folder.$(DEF_COLOR)\n" $(words $(OBJ)) $(OBJ_DIR);\
	fi
	@$(MAKE) test-clean
	
fclean: clean
	@if [ -e $(LIBFT) ]; then \
		printf "$(YELLOW)Removing \"%s\"...$(DEF_COLOR)\n" $(LIBFT);\
		$(RM) $(LIBFT);\
		$(MAKE) fclean -C $(LIBFT_DIR) -s;\
	fi
	@if [ -e $(LIBFT_INC_H) ]; then \
		printf "$(YELLOW)Removing \"%s\"...$(DEF_COLOR)\n" $(INC_DIR)/$(LIBFT_INC_H); \
		$(RM) $(LIBFT_INC_H);\
	fi
	@if [ -e $(LIB_NAME) ]; then \
		printf "$(YELLOW)Removing \"%s\"...$(DEF_COLOR)\n" $(LIB_NAME);\
		$(RM) $(LIB_NAME);\
	fi
	@if [ -e lib$(NAME).so ]; then \
		printf "$(YELLOW)Removing symlink \"lib$(NAME).so\"...$(DEF_COLOR)\n";\
		$(RM) lib$(NAME).so;\
	fi
	@if [ -e $(NAME) ]; then \
		printf "$(YELLOW)Removing \"%s\"...$(DEF_COLOR)\n" $(NAME);\
		$(RM) -r $(NAME);\
	fi

re: fclean all

fcleanall: fcleanlibs
	@$(RM) -rf $(TMP)
	@printf "$(RED)All files removed!$(DEF_COLOR)\n"

.PHONY: all clean fclean re norminette cleanlibs fcleanlibs relibft fcleanall test test-clean test-debug test-valgrind install help