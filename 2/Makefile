all: proj2

proj2:
	latex $@.tex
	latex $@.tex
	dvips -t a4 $@.dvi
	ps2pdf $@.ps

clean:
	rm proj2.{aux,dvi,log,pdf,ps,out} xdobro23-fit.zip 2> /dev/null || true
	
