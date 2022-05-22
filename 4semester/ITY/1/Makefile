all: proj1

proj1:
	latex $@.tex
	latex $@.tex
	dvips -t a4 $@.dvi
	ps2pdf $@.ps

clean:
	rm proj1.{aux,dvi,log,pdf,ps,out} xdobro23-fit.zip 2> /dev/null || true
	
