CXX      := c++
CXXFLAGS := -std=c++98 -Wall -Wextra -Werror -pedantic -O2 -Iinclude

NAME := ircserv

SRCS := \
	src/main.cpp \
	src/Server.cpp \
	src/ServerConn.cpp \
	src/ServerIO.cpp \
	src/ServerHelpers.cpp \
	src/ServerSession.cpp \
	src/CmdAuth.cpp \
	src/CmdChannel.cpp \
	src/CmdMessage.cpp \
	src/Client.cpp \
	src/Parser.cpp \
	src/Command.cpp \
	src/Utils.cpp \
	src/Channel.cpp

OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re