INCLUDE=czechiso.bst
all: proj4

proj4: proj4.tex
	pdflatex $^
	bibtex proj4
	pdflatex $^
	pdflatex $^

clean:
	rm proj4.{aux,dvi,log,pdf,ps,out,bbl,blg} xdobro23-fit.zip 2> /dev/null || true
	
