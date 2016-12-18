.PHONY : all codecs clean

all: codecs

clean:
	@rm -f *~ *.aux *.log *.bak *.out *.toc *.dvi *.ps

archclean: clean
	@rm -f *.pdf

codecs:
	latex codecs
	dvips codecs.dvi
	ps2pdf codecs.ps
