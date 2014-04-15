#include "log.h"
#include "format.h"
#include "html.h"
#include "htmlLinker.h"



/**
 *   \file htmlLinker.c Module that generates links (URLs) to various sources
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */




/**
 * Generates a link to KnownGene description page.
 * Example: http://genome.ucsc.edu/cgi-bin/hgGene?db=rn4&hgg_gene=NM_012824&hgg_chrom=chr1&hgg_start=78996678&hgg_end=78999875
 */
char* htmlLinker_generateLinkToGeneDescriptionPageAtUCSC (char* database, char* geneName, 
							  char* chromosome, int start, int end)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://genome.ucsc.edu/cgi-bin/hgGene?db=%s&hgg_gene=%s&hgg_chrom=%s&hgg_start=%d&hgg_end=%d",
		database,geneName,chromosome,start,end);
  return string (buffer);
}



/**
 * Generates a link to the UCSC track element description page. 
 * Example: http://genome.ucsc.edu/cgi-bin/hgc?db=hg18&g=intronEst&i=BI755927&c=chrX&l=149673899&r=152783129
 */
char* htmlLinker_generateLinkToTrackElementDescriptionPageAtUCSC (char* database, char* trackName, char* elementName, 
								  char* chromosome, int start, int end)
{
  static Stringa buffer = NULL;
 
  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://genome.ucsc.edu/cgi-bin/hgc?db=%s&g=%s&i=%s&c=%s&l=%d&r=%d",
		database,trackName,elementName,chromosome,start,end);
  return string (buffer);
}



/**
 * Generates a link to the UCSC genome browser.
 * Example: http://genome.ucsc.edu/cgi-bin/hgTracks?db=mm9&clade=vertebrate&org=Mouse&position=chr12:10000-20000
 */
char* htmlLinker_generateLinkToGenomeBrowserAtUCSC (char* database, char* clade, char* organism, 
						    char* chromosome, int start, int end)
{
  static Stringa buffer = NULL;
  
  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://genome.ucsc.edu/cgi-bin/hgTracks?db=%s&clade=%s&org=%s&position=%s:%d-%d",
		database,clade,organism,chromosome,start,end);
  return string (buffer);
}



/**
 * Generates a link to the Yale human pseudogene site.
 * Example: http://tables.pseudogene.org/human/200550
 */
char* htmlLinker_generateLinkToHumanPseudogenePageAtYale (char* pseudogeneId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://tables.pseudogene.org/human/%s",pseudogeneId);
  return string (buffer);
}



/**
 * Generates a link to FlyBase gene description page.
 * Example: http://flybase.bio.indiana.edu/reports/FBgn0033837.html
 */
char* htmlLinker_generateLinkToFlyBaseGeneDescriptionPage (char* flyBaseId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://flybase.bio.indiana.edu/reports/%s.html",flyBaseId);
  return string (buffer);
}



/**
 * Generates a link to WormBase gene description page. 
 * Both the WormBase gene ID or the gene symbol can be used. 
 * Example: http://www.wormbase.org/db/gene/gene?name=WBGene00002239 or 
   http://www.wormbase.org/db/gene/gene?name=ksr-1
 */
char* htmlLinker_generateLinkToWormBaseGeneDescriptionPage (char* wormBaseGeneId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://www.wormbase.org/db/gene/gene?name=%s",wormBaseGeneId);
  return string (buffer);
}



/**
 * Generates a link to Uniprot.
 * Example: http://www.pir.uniprot.org/cgi-bin/upEntry?id=Q91V24 or 
   http://www.pir.uniprot.org/cgi-bin/upEntry?id=ADA1A_BOVIN
 */
char* htmlLinker_generateLinkToUniProt (char* uniProtId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://www.pir.uniprot.org/cgi-bin/upEntry?id=%s",uniProtId);
  return string (buffer);
}



/**
 * Generates a link to Mouse Genome Informatics (MGI) gene description page.
 * Example: http://www.informatics.jax.org/searches/accession_report.cgi?id=MGI:1351646
 * @note Instructions for linking to MGI can be found at 
   http://www.informatics.jax.org/mgihome/other/link_instructions.shtml
 */

char* htmlLinker_generateLinkToMouseGeneDescriptionPage (char* mgiId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://www.informatics.jax.org/searches/accession_report.cgi?id=%s",mgiId);
  return string (buffer);
}



/**
 * Generates a link to Rat Genome Database (RGD) gene description page.
 * Example: http://rgd.mcw.edu/tools/genes/genes_view.cgi?id=727972
 * @note The rgdId must be numeric.
 */
char* htmlLinker_generateLinkToRatGeneDescriptionPage (char* rgdId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://rgd.mcw.edu/tools/genes/genes_view.cgi?id=%s",rgdId);
  return string (buffer);
}



/**
 * Generates a link to Saccharomyces Genome Database (SGD) gene description page.
 * Both the SGD gene ID or the gene symbol can be used. 
 * Example: http://db.yeastgenome.org/cgi-bin/locus.pl?locus=AAC3 or
   http://db.yeastgenome.org/cgi-bin/locus.pl?locus=S000000289
 */
char* htmlLinker_generateLinkToYeastGeneDescriptionPage (char* sgdId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://db.yeastgenome.org/cgi-bin/locus.pl?locus=%s",sgdId);
  return string (buffer);
}



/**
 * Generates a link to Pfam.
 * Example: http://pfam.sanger.ac.uk/family?acc=PF09582
 */
char* htmlLinker_generateLinkToPfam (char* pfamId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://pfam.sanger.ac.uk/family?acc=%s",pfamId);
  return string (buffer);
}



/**
 * Generates a link to InterPro.
 * Example: http://www.ebi.ac.uk/interpro/DisplayIproEntry?ac=IPR001922
 */
char* htmlLinker_generateLinkToInterPro (char* interProId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://www.ebi.ac.uk/interpro/DisplayIproEntry?ac=%s",interProId);
  return string (buffer);
}



/**
 * Generates a link to EntrezGene.
 * Example: http://www.ncbi.nlm.nih.gov/sites/entrez?db=gene&cmd=search&term=NP_001069323 or 
   http://www.ncbi.nlm.nih.gov/sites/entrez?db=gene&cmd=search&term=1812
 */
char* htmlLinker_generateLinkToEntrezGene (char* term)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://www.ncbi.nlm.nih.gov/sites/entrez?db=gene&cmd=search&term=%s",term);
  return string (buffer);
}



/**
 * Generates a link to PubMed.
 * Example: http://www.ncbi.nlm.nih.gov/pubmed/18276894
 */
char* htmlLinker_generateLinkToPubmed (char* pmid)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://www.ncbi.nlm.nih.gov/pubmed/%s",pmid);
  return string (buffer);
}



/**
 * Generates a link to the PDB.
 * Example: http://www.rcsb.org/pdb/explore/explore.do?structureId=1HDC
 */
char* htmlLinker_generateLinkToPDB (char* pdbId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://www.rcsb.org/pdb/explore/explore.do?structureId=%s",pdbId);
  return string (buffer);
}



/**
 * Generates a link to AmiGO. 
 * Example: http://amigo.geneontology.org/cgi-bin/amigo/term-details.cgi?term=GO:0051240
 */
char* htmlLinker_generateLinkToAmiGO (char* goId)
{
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"http://amigo.geneontology.org/cgi-bin/amigo/term-details.cgi?term=%s",goId);
  return string (buffer);
}
