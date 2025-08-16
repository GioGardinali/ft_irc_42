
# Simple Makefile for ft_irc (C++98, poll())
NAME := ircserv
CXX := c++
CXXFLAGS := -std=c++98 -Wall -Wextra -Werror -pedantic -O2
INCLUDES := -Iinclude

SRCS := 	src/main.cpp 	src/Server.cpp 	src/Client.cpp 	src/Parser.cpp 	src/Command.cpp 	src/Utils.cpp 	src/Channel.cpp

OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
