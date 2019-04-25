deps:
	sudo apt install liblua5.3-dev liblua5.3-0

all:
	g++ lua.cpp -o test -I /usr/include/lua5.3/ -llua5.3 -std=c++11	
