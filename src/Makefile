# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adherrer <adherrer@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/10/23 04:06:52 by adherrer          #+#    #+#              #
#    Updated: 2024/12/18 18:51:26 by adherrer         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = WEBS
CXX = c++
CXFLAGS = -Wall -Werror -Wextra -std=c++98 -g
SRCS = main.cpp
COVFLAGS = #-g -fPIC  -fprofile-arcs -ftest-coverage
INCLUDE = 

OBJS = ${SRCS:.cpp=.o}

# Regla por defecto

# Compilar el ejecutable
$(NAME): $(OBJS) $(INCLUDE)
			$(CXX) -o $(NAME) $(CXFLAGS) $(COVFLAGS) $(OBJS)
			
all: $(NAME)


# Regla para compilar los archivos .o
%.o:%.cpp $(INCLUDE)
		$(CXX) $(CXFLAGS) $(COVFLAGS)  -c $< -o $@

# Limpiar los archivos generados
clean:
	rm -f $(OBJS) *.gcov *.gcda logs *.gcno *.html *.css 
	rm -f $(OBJS)
	@echo "Archivos .o eliminados."

# Limpiar todo, incluyendo el ejecutable
fclean: clean
	rm -f $(NAME)
	@echo "Ejecutable $(NAME) eliminado."

# Recompilar
re: fclean all

cov: fclean all
	./$(NAME)
	@for src in $(SRCS); do \
		echo "Processing $$src"; \
		dir=$$(dirname "$$src"); \
		gcov --object-directory=$$dir "$$src"; \
	done
	gcovr -r . --html --html-details -o coverage.html
	open ./coverage.html


.PHONY: all clean fclean re #cov
 