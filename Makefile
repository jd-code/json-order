
all: json-order

clean:
	rm json-order

json-order: json-order.cpp
	g++ -Wall -o json-order json-order.cpp

vimtest: json-order
	cat test.json
	./json-order < test.json
	./json-order test.json

