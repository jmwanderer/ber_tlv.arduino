all: test example

.PHONY: test
test: tlv_tests
	./tlv_tests

tlv_tests: tlv.cc tlv_tests.cc
	g++ -fsanitize=address -o tlv_tests tlv.cc tlv_tests.cc

example: tlv.cc example.cc
	g++ -o example tlv.cc example.cc

.PHONY: clean
clean:
	rm -f example tlv_tests

