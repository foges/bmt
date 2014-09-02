test: test_bmt.cpp bmt.h
	g++ $< -o $@
	./test

clean:
	rm test

