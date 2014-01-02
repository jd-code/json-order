
PREFIX=/usr
# PREFIX=/opt/local
DESTBINDIR=$(PREFIX)/bin

all: json-order

clean:
	rm -f json-order

json-order: json-order.cpp
	g++ -Wall -o json-order json-order.cpp

vimtest: json-order
	cat test.json
	./json-order -impartial < test.json
	./json-order -impartial -nooutput test.json

install: json-order
	cp json-order $(DESTBINDIR)/json-order && chmod 755 ${DESTBINDIR}/json-order
	cp jsondiff ${DESTBINDIR}/jsondiff && chmod 755 $(DESTBINDIR)/jsondiff
	ln -sf $(DESTBINDIR)/jsondiff $(DESTBINDIR)/jsonvimdiff

