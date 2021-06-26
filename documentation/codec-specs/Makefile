.PHONY : all clean

all: codecs.pdf

clean:
	@rm -f *~ *.aux *.log *.bak *.out *.toc *.dvi *.ps

archclean: clean
	@rm -f *.pdf

# run latex twice to get the proper number of pages, fig numbers and index table
codecs.pdf: *.tex *.fig *.eps
	latex codecs
	latex codecs
	dvips codecs.dvi
	ps2pdf codecs.ps
