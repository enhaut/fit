all: proj3

proj3:
	latex $@.tex
	latex $@.tex
	dvips -t a4 $@.dvi
	ps2pdf -sPAPERSIZE=a4 $@.ps

clean:
	rm proj1.{aux,dvi,log,pdf,ps,out} xdobro23-fit.zip 2> /dev/null || true
	
