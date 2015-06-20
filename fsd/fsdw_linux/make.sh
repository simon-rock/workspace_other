if [ -z $1 ]; then
	echo client only
else
	killall -9 fsserver
fi
rm -f *.o
g++ -c common.cpp
g++ -c fsserver.cpp
g++ -c fsclient.cpp
g++ -c cfg.cpp
g++ common.o fsserver.o cfg.o -o fsserver 
g++ common.o fsclient.o cfg.o -o fsclient -l pthread
