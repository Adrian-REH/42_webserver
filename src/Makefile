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
CXXFLAGS = -Wall -Werror -Wextra -D LOG_LEVEL=3 -std=c++98 -g3 
COVFLAGS = #-g -fPIC  -fprofile-arcs -ftest-coverage
INCLUDE = -I. -Iutils
SRCS = main.cpp Server.cpp HttpServerManager.cpp Location.cpp \
	   ParserServer.cpp \
	   utils/endsWith.cpp utils/extractStrBetween.cpp utils/randomID.cpp \
	   utils/readFd.cpp utils/readFileName.cpp utils/split.cpp utils/startsWith.cpp \
	   utils/getAllDirs.cpp utils/strtrim.cpp utils/toString.cpp
OBJS = ${SRCS:.cpp=.o}

# Regla por defecto

# Compilar el ejecutable
$(NAME): $(OBJS) 
			$(CXX) $(CXXFLAGS) -o $(NAME) $(COVFLAGS) $(INCLUDE) $(OBJS) 
			
all: $(NAME)


# Regla para compilar los archivos .o
%.o:%.cpp $(INCLUDE)
		$(CXX) $(CXXFLAGS) $(COVFLAGS)  -c $< -o $@

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
 