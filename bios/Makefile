include $(BIOINFOCONFDIR)/biosdefs.make
.SUFFIXES:
SHELL=/bin/sh



D=$(BIOSOBJ)

LIBS=$D/libbios.a

MODS = $D/array.o $D/format.o $D/log.o $D/hlrmisc.o $D/plabla.o $D/linestream.o $D/html.o $D/common.o $D/dlist.o $D/numUtil.o $D/stringUtil.o $D/fasta.o $D/bits.o $D/seq.o $D/geneOntology.o $D/htmlLinker.o $D/intervalFind.o $D/blatParser.o $D/blastParser.o $D/elandMultiParser.o $D/elandParser.o $D/bowtieParser.o $D/bgrParser.o $D/exportPEParser.o

MODS_H = array.h format.h log.h hlrmisc.h plabla.h linestream.h html.h common.h dlist.h numUtil.h stringUtil.h fasta.h bits.h seq.h geneOntology.h htmlLinker.h intervalFind.h blatParser.h blastParser.h elandMultiParser.h elandParser.h bowtieParser.h bgrParser.h exportPEParser.h

MODS_DOC = array.txt log.txt format.txt



# ------------------------------
all: dirs dev

dirs:
	test -d $D || mkdir $D
	test -d $(BIOSLIBLOC) || mkdir $(BIOSLIBLOC)

dev: $(LIBS) 
	@echo "bios: made libbios."
	@echo "bios: use 'make prod' to release into production."

doc: $(MODS_HDOC) $(MODS_DOC)
	@cp $(MODS_HDOC) $(MODS_DOC) $(BIOSLIBLOC)

libs: $(LIBS) doc  
	@echo "bios: cp libbios.a ... $(BIOSLIBLOC)."
	@cp $(LIBS) $(LIBS_H) $(BIOSLIBLOC)
	@cp $(MODS_H) $(BIOSLIBLOC)
	@echo "bios: libbios made and released."

client: libs

prod: client

doxygen: 
	@echo "bios: documenting code using doxygen."
	@rm -rf html/*
	$(BIOINFODOXYGENBINDIR)/doxygen Doxyfile
	rsync -a --delete $(BIOINFOCONFDIR)/../html/ $(BIOINFOCONFDIR)/../format.txt $(BIOINFOCONFDIR)/../array.txt $(USER)@$(BIOINFOWEBSERVER):$(BIOINFODOXYGENDOCDIR)
	@echo "bios: documentation finished and released to $(BIOINFODOXYGENDOCDIR)."

clean: 
	-@if test -d $D ; then echo "bios: cleaning $D and $(BIOSLIBLOC)." ; rm -rf $D/* $(BIOSLIBLOC)/* ; else echo "PROBLEM: $D: no such directory." ; fi

# ----------------------------

$D/libbios.a: $(MODS) $(BIOINFOCONFDIR)/biosdefs.h 
	@-/bin/rm -f $D/libbios.a
	@ar sr $D/libbios.a $(MODS)
	@/bin/ls -l $D/libbios.a

# use
#   ar tv libbios.a
# to see module list in library
# use
#   nm libbios.a
# to see symbols defined



# ------------------- Core libbios modules -----------------

$D/plabla.o: plabla.c plabla.h $(BIOINFOCONFDIR)/plabla_conf.h
	$(CC) $(CFLAGSO) $(BICOSINC) plabla.c -c -o $D/plabla.o

$D/log.o: log.c log.h $D/plabla.o
	$(CC) $(CFLAGSO) $(BICOSINC) log.c -c -o $D/log.o

$D/hlrmisc.o: hlrmisc.c hlrmisc.h
	$(CC) $(CFLAGSO) hlrmisc.c -c -o $D/hlrmisc.o

$D/array.o: array.c array.h $D/log.o
	$(CC) $(CFLAGSO) array.c -c -o $D/array.o

$D/format.o: format.c format.h $D/array.o $D/log.o $D/hlrmisc.o $D/plabla.o
	$(CC) $(CFLAGSO) $(BICOSINC) format.c -c -o $D/format.o

$D/linestream.o: linestream.c linestream.h $D/array.o $D/log.o $D/hlrmisc.o $D/plabla.o $D/format.o
	$(CC) $(CFLAGSO) $(BICOSINC) linestream.c -c -o $D/linestream.o

$D/html.o: html.c html.h $D/array.o $D/log.o $D/hlrmisc.o $D/plabla.o $D/format.o $D/linestream.o
	$(CC) $(CFLAGSO) $(BICOSINC) html.c -c -o $D/html.o



# ------------------- Added libbios modules -----------------

$D/common.o: common.c common.h $D/log.o $D/format.o $D/linestream.o 
	$(CC) $(CFLAGSO) $(BIOSINC) common.c -c -o $D/common.o

$D/dlist.o: dlist.c dlist.h $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) dlist.c -c -o $D/dlist.o

$D/numUtil.o: numUtil.c numUtil.h $D/log.o $D/format.o
	$(CC) $(CFLAGSO) $(BIOSINC) numUtil.c -c -o $D/numUtil.o

$D/stringUtil.o: stringUtil.c stringUtil.h $D/common.o $D/log.o $D/format.o
	$(CC) $(CFLAGSO) $(BIOSINC) stringUtil.c -c -o $D/stringUtil.o

$D/fasta.o: fasta.c fasta.h $D/log.o $D/format.o $D/linestream.o $D/seq.o $D/stringUtil.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) fasta.c -c -o $D/fasta.o

$D/seq.o: seq.c seq.h $D/log.o $D/format.o $D/bits.o $D/common.o $D/numUtil.o
	$(CC) $(CFLAGSO) $(BIOSINC) seq.c -c -o $D/seq.o

$D/bits.o: bits.c bits.h $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) bits.c -c -o $D/bits.o

$D/geneOntology.o: geneOntology.c geneOntology.h $D/log.o $D/format.o $D/linestream.o 
	$(CC) $(CFLAGSO) $(BIOSINC) geneOntology.c -I$(BIOINFOGSLDIR)/include -c -o $D/geneOntology.o

$D/htmlLinker.o: htmlLinker.c htmlLinker.h $D/log.o $D/format.o  
	$(CC) $(CFLAGSO) $(BIOSINC) htmlLinker.c -c -o $D/htmlLinker.o

$D/intervalFind.o: intervalFind.c intervalFind.h $D/log.o $D/format.o $D/linestream.o $D/numUtil.o 
	$(CC) $(CFLAGSO) $(BIOSINC) intervalFind.c -c -o $D/intervalFind.o

$D/blatParser.o: blatParser.c blatParser.h $D/log.o $D/format.o $D/linestream.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) blatParser.c -c -o $D/blatParser.o

$D/blastParser.o: blastParser.c blastParser.h $D/log.o $D/format.o $D/linestream.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) blastParser.c -c -o $D/blastParser.o

$D/elandMultiParser.o: elandMultiParser.c elandMultiParser.h $D/log.o $D/format.o $D/linestream.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) elandMultiParser.c -c -o $D/elandMultiParser.o

$D/elandParser.o: elandParser.c elandParser.h $D/log.o $D/format.o $D/linestream.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) elandParser.c -c -o $D/elandParser.o

$D/bowtieParser.o: bowtieParser.c bowtieParser.h $D/log.o $D/format.o $D/linestream.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) bowtieParser.c -c -o $D/bowtieParser.o

$D/bgrParser.o: bgrParser.c bgrParser.h $D/log.o $D/format.o $D/linestream.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) bgrParser.c -c -o $D/bgrParser.o

$D/exportPEParser.o: exportPEParser.c exportPEParser.h $D/log.o $D/format.o $D/linestream.o $D/common.o
	$(CC) $(CFLAGSO) $(BIOSINC) exportPEParser.c -c -o $D/exportPEParser.o
