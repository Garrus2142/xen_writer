#Nom de l'exécutable
NAME = xen_writer
#Fichiers source (.cpp)
SRC = main.cpp Window.cpp Storage.cpp XFormat.cpp MusicFAT.cpp MusicFile.cpp
#Flags de compilation
FLAGS = -Werror -Wall -Wextra

OBJ = $(addprefix obj/, $(SRC:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ)
	@g++ -o $(NAME) -std=c++11 -lncurses $(OBJ)

obj/%.o: src/%.cpp
	@echo "Compilation > $@"
	@mkdir -p $(dir $@)
	@g++ $(FLAGS) -std=c++11 -I include -c $^ -o $@

clean:
	@rm -rf obj
	@rm -f $(NAME)

re: clean all

.PHONY:
