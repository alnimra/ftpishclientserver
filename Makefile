# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mray <marvin@42.fr>                        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2018/02/26 13:50:52 by mray              #+#    #+#              #
#    Updated: 2018/05/04 19:03:10 by mray             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME1	= server
NAME2	= client

# src / obj files
SRC1		= server.c\

SRC2		= client.c\


OBJ1		= $(addprefix $(OBJDIR),$(SRC1:.c=.o))
OBJ2		= $(addprefix $(OBJDIR),$(SRC2:.c=.o))

# compiler
CC		= gcc
CFLAGS	= -Wall -Wextra -Werror -g

# ft library
FT		= ./libft/
FT_LIB	= $(addprefix $(FT),libft.a)
FT_INC	= -I ./libft
FT_LNK	= -L ./libft -l ft

# directories
SRCDIR	= ./srcs/
INCDIR	= ./includes/
OBJDIR	= ./obj/

all: obj $(FT_LIB) $(NAME1) $(NAME2)

obj:
	mkdir -p $(OBJDIR)

$(OBJDIR)%.o:$(SRCDIR)%.c
	$(CC) $(CFLAGS) $(FT_INC) -I $(INCDIR) -o $@ -c $<

$(FT_LIB):
	make -C $(FT)

$(NAME1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) $(MLX_LNK) $(FT_LNK) -lm -o $(NAME1)

$(NAME2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) $(MLX_LNK) $(FT_LNK) -lm -o $(NAME2)

clean:
	rm -rf $(OBJDIR)
	make -C $(FT) clean

fclean: clean
	rm -rf $(NAME1)
	rm -rf $(NAME2)
	make -C $(FT) fclean

re: fclean all
