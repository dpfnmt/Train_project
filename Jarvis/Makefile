CC=g++
bin=Jarvis
src=Jarvis.cc
INCLUDE=-Ispeech/
LIB=-ljsoncpp -lcurl -lcrypto -lpthread
STD=-std=c++11 
FLAGS=-D_LOG_
$(bin):$(src)
	$(CC) -o $@ $^ $(INCLUDE) $(LIB) $(FLAGS) $(STD)

.PHONY:clear
clear:
	rm -f $(bin)
