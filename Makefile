all: paper code

paper:
	texi2pdf -c design-spec.tex

code:
	g++ -std=c++11 sequential-kmeans.cpp

.PHONY: clean
clean: 
	rm *.pdf
	rm *.out
