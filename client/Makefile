CC = g++
CXXFLAGS = -std=c++17 -ggdb -g3 -Wall -mrdrnd
LDFLAGS =  -pthread -lboost_system -lcryptopp

appname = test.out

default: compile clean

compile:
	$(CC) $(CXXFLAGS) -c protocol/types.cpp -o protocol_types.o $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c protocol/response.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c protocol/request.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c protocol/exceptions.cpp -o protocol_exceptions.o $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c crypto/asymmetric.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c crypto/symmetric.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c session/exceptions.cpp -o session_exceptions.o $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c session/types.cpp -o session_types.o $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c radix.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c session/session.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c config.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c tempfile.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c ui.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c client.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -c main.cpp $(LDFLAGS)
	$(CC) $(CXXFLAGS) -o $(appname) *.o $(LDFLAGS)

clean:
	rm *.o