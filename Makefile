doc:
	texi2pdf -c design-spec.tex.

code:
	g++ -std=c++11 sequential-kmeans.cpp
