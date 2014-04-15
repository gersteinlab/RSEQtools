# required input from environment:
# $(BIOINFOCONFDIR) and $(BICOSN)

# ---------------- setup symbols needed in the rest of the file --------

include $(BIOINFOCONFDIR)/biosdefs.make
.SUFFIXES:
SHELL = /bin/sh

DOXYGEN_BIN_DIR=/home1/lh372/SOFT/doxygen-1.5.7.1/bin
DOC_DIR=/nfs/web/lh372/public_html/SOFT/mrf


# ----------------------- entry points --------------

PROGRAMS=psl2mrf bowtie2mrf singleExport2mrf mrfSubsetByTargetName mrfQuantifier mrfAnnotationCoverage mrf2wig mrf2gff mrfSampler mrf2bgr wigSegmenter mrfMappingBias mrfSelectRegion mrfSelectSpliced mrfSelectAnnotated createSpliceJunctionLibrary gff2interval export2fastq mergeTranscripts interval2gff interval2sequences bed2interval interval2bed mrf2sam sam2mrf mrfValidate bgrQuantifier bgrSegmenter mrfCountRegion


MODULES=mrf.o segmentationUtil.o sam.o

all: allprogs 

allprogs: $(MODULES) $(PROGRAMS) 

clean: 
	/bin/rm -f $(PROGRAMS) $(MODULES)

doxygen: 
	@echo "mrf: documenting code using doxygen."
	@rm -rf html/*
	$(DOXYGEN_BIN_DIR)/doxygen Doxyfile
	rsync -a --delete ./html/ lh372@gw.gersteinlab.org:$(DOC_DIR)
	@echo "mrf: documentation finished and released to $(DOC_DIR)."




psl2mrf: psl2mrf.c mrf.h $(BIOSLIB)
	-@/bin/rm -f psl2mrf
	$(CC) $(CFLAGSO) $(BIOSINC) psl2mrf.c -o psl2mrf $(BIOSLNK)

bowtie2mrf: bowtie2mrf.c mrf.h $(BIOSLIB)
	-@/bin/rm -f bowtie2mrf
	$(CC) $(CFLAGSO) $(BIOSINC) bowtie2mrf.c -o bowtie2mrf $(BIOSLNK)

mrfSubsetByTargetName: mrfSubsetByTargetName.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfSubsetByTargetName
	$(CC) $(CFLAGSO) $(BIOSINC) mrfSubsetByTargetName.c mrf.o -o mrfSubsetByTargetName $(BIOSLNK)

mrfQuantifier: mrfQuantifier.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfQuantifier
	$(CC) $(CFLAGSO) $(BIOSINC) mrfQuantifier.c mrf.o -o mrfQuantifier $(BIOSLNK) -lm

bgrQuantifier: bgrQuantifier.c $(BIOSLIB)
	-@/bin/rm -f bgrQuantifier
	$(CC) $(CFLAGSO) $(BIOSINC) bgrQuantifier.c -o bgrQuantifier $(BIOSLNK) -lm

mrfAnnotationCoverage: mrfAnnotationCoverage.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfAnnotationCoverage
	$(CC) $(CFLAGSO) $(BIOSINC) mrfAnnotationCoverage.c mrf.o -o mrfAnnotationCoverage $(BIOSLNK) -lm

mrfCountRegion: mrfCountRegion.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfCountRegion
	$(CC) $(CFLAGSO) $(BIOSINC) mrfCountRegion.c mrf.o -o mrfCountRegion $(BIOSLNK) -lm

mrf2wig: mrf2wig.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrf2wig
	$(CC) $(CFLAGSO) $(BIOSINC) mrf2wig.c mrf.o -o mrf2wig $(BIOSLNK) -lm

mrf2bgr: mrf2bgr.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrf2bgr
	$(CC) $(CFLAGSO) $(BIOSINC) mrf2bgr.c mrf.o -o mrf2bgr $(BIOSLNK) -lm

mrf2gff: mrf2gff.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrf2gff
	$(CC) $(CFLAGSO) $(BIOSINC) mrf2gff.c mrf.o -o mrf2gff $(BIOSLNK)

mrfSampler: mrfSampler.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfSampler
	$(CC) $(CFLAGSO) $(BIOSINC) mrfSampler.c mrf.o -o mrfSampler $(BIOSLNK)

wigSegmenter: wigSegmenter.c segmentationUtil.o $(BIOSLIB)
	-@/bin/rm -f wigSegmenter
	$(CC) $(CFLAGSO) $(BIOSINC) wigSegmenter.c segmentationUtil.o -o wigSegmenter $(BIOSLNK)

bgrSegmenter: bgrSegmenter.c segmentationUtil.o $(BIOSLIB)
	-@/bin/rm -f bgrSegmenter
	$(CC) $(CFLAGSO) $(BIOSINC) bgrSegmenter.c segmentationUtil.o -o bgrSegmenter $(BIOSLNK)

mrfMappingBias: mrfMappingBias.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfMappingBias
	$(CC) $(CFLAGSO) $(BIOSINC) mrfMappingBias.c mrf.o -o mrfMappingBias $(BIOSLNK) -lm

mrfSelectRegion: mrfSelectRegion.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfSelectRegion
	$(CC) $(CFLAGSO) $(BIOSINC) mrfSelectRegion.c mrf.o -o mrfSelectRegion $(BIOSLNK) -lm

mrfSelectSpliced: mrfSelectSpliced.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfSelectSpliced
	$(CC) $(CFLAGSO) $(BIOSINC) mrfSelectSpliced.c mrf.o -o mrfSelectSpliced $(BIOSLNK)

mrfSelectAnnotated: mrfSelectAnnotated.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfSelectAnnotated
	$(CC) $(CFLAGSO) $(BIOSINC) mrfSelectAnnotated.c mrf.o -o mrfSelectAnnotated $(BIOSLNK) -lm

createSpliceJunctionLibrary: createSpliceJunctionLibrary.c $(BIOSLIB)
	-@/bin/rm -f createSpliceJunctionLibrary
	$(CC) $(CFLAGSO) $(BIOSINC) createSpliceJunctionLibrary.c -o createSpliceJunctionLibrary $(BIOSLNK) -lm

gff2interval: gff2interval.c $(BIOSLIB)
	-@/bin/rm -f gff2interval
	$(CC) $(CFLAGSO) $(BIOSINC) gff2interval.c -o gff2interval $(BIOSLNK)

export2fastq: export2fastq.c $(BIOSLIB)
	-@/bin/rm -f export2fastq
	$(CC) $(CFLAGSO) $(BIOSINC) export2fastq.c -o export2fastq $(BIOSLNK)

mergeTranscripts: mergeTranscripts.c $(BIOSLIB)
	-@/bin/rm -f mergeTranscripts
	$(CC) $(CFLAGSO) $(BIOSINC) mergeTranscripts.c -o mergeTranscripts $(BIOSLNK) -lm

mrfValidate: mrfValidate.c mrf.o $(BIOSLIB)
	-@/bin/rm -f mrfValidate
	$(CC) $(CFLAGSO) $(BIOSINC) mrfValidate.c mrf.o -o mrfValidate $(BIOSLNK) 

interval2gff: interval2gff.c $(BIOSLIB)
	-@/bin/rm -f interval2gff
	$(CC) $(CFLAGSO) $(BIOSINC) interval2gff.c -o interval2gff $(BIOSLNK) -lm

interval2sequences: interval2sequences.c $(BIOSLIB)
	-@/bin/rm -f interval2sequences
	$(CC) $(CFLAGSO) $(BIOSINC) interval2sequences.c -o interval2sequences $(BIOSLNK) -lm

bed2interval: bed2interval.c mrfUtil.o $(BIOSLIB)
	-@/bin/rm -f bed2interval
	$(CC) $(CFLAGSO) $(BIOSINC) bed2interval.c mrfUtil.o -o bed2interval $(BIOSLNK) -lm

interval2bed: interval2bed.c $(BIOSLIB)
	-@/bin/rm -f interval2bed
	$(CC) $(CFLAGSO) $(BIOSINC) interval2bed.c -o interval2bed $(BIOSLNK) -lm

mrf2sam: mrf2sam.c mrf.o sam.o $(BIOSLIB)
	-@/bin/rm -f mrf2sam
	$(CC) $(CFLAGSO) $(BIOSINC) mrf2sam.c mrf.o sam.o -o mrf2sam $(BIOSLNK)

sam2mrf: sam2mrf.c mrf.o sam.o $(BIOSLIB)
	-@/bin/rm -f sam2mrf
	$(CC) $(CFLAGSO) $(BIOSINC) sam2mrf.c mrf.o sam.o -o sam2mrf $(BIOSLNK)

singleExport2mrf: singleExport2mrf.c mrf.o $(BIOSLIB)
	-@/bin/rm -f singleExport2mrf
	$(CC) $(CFLAGSO) $(BIOSINC) singleExport2mrf.c mrf.o -o singleExport2mrf $(BIOSLNK)


sam.o: sam.c sam.h $(BIOSLIB)
	-@/bin/rm -f $O/sam.o
	$(CC) $(CFLAGSO) $(BIOSINC) sam.c -c -o sam.o

mrf.o: mrf.c mrf.h $(BIOSLIB)  
	-@/bin/rm -f $O/mrf.o
	$(CC) $(CFLAGSO) $(BIOSINC) mrf.c -c -o mrf.o

segmentationUtil.o: segmentationUtil.c segmentationUtil.h $(BIOSLIB)  
	-@/bin/rm -f $O/segmentationUtil.o
	$(CC) $(CFLAGSO) $(BIOSINC) segmentationUtil.c -c -o segmentationUtil.o

mrfUtil.o: mrfUtil.c mrfUtil.h $(BIOSLIB)  
	-@/bin/rm -f $O/mrfUtil.o
	$(CC) $(CFLAGSO) $(BIOSINC) mrfUtil.c -c -o mrfUtil.o
