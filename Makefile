all: paper code

paper: design-spec.tex
	texi2pdf -c design-spec.tex

code: sequential-kmeans.cpp 
	g++ -std=c++11 sequential-kmeans.cpp

.PHONY: clean
clean: 
	rm *.pdf
	rm *.out
