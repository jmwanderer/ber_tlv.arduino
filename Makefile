all: test example

.PHONY: test
test: tlv_tests
	./tlv_tests

tlv_tests: tlv.cpp tlv_tests.cpp
	g++ -fsanitize=address -o tlv_tests tlv.cpp tlv_tests.cpp

example: tlv.cpp example.cpp
	g++ -o example tlv.cpp example.cpp

.PHONY: clean
clean:
	rm -f example tlv_tests

