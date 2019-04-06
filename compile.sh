gppflags='-Wall -shared -m32 -fPIC -Ofast -s -static -std=c++0x'
time(
	echo "Compiling"
	g++ $gppflags main.cpp patternscan.cpp -o firebulletsfix.so
	echo "Done")
