.PHONY : all clean archclean single

all: single

clean:
	@rm -f *~ *.aux *.log *.bak *.out *.toc *.dvi *.ps

archclean: clean
	@rm -f *.pdf

single:
	pdflatex manual

manual.pdf: preface.tex background.tex defect-reporting.tex howtos.tex manual.tex qa.tex burning-compatibility.tex download.tex local.tex overview.tex
	pdflatex manual
	pdflatex manual
	pdflatex manual
