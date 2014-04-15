#ifndef DEF_HTML_LINKER
#define DEF_HTML_LINKER


/**
 *   \file htmlLinker.h 
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */


extern char* htmlLinker_generateLinkToGeneDescriptionPageAtUCSC (char* database, char* geneName, char* chromosome, int start, int end);
extern char* htmlLinker_generateLinkToTrackElementDescriptionPageAtUCSC (char* database, char* trackName, char* elementName, char* chromosome, int start, int end);
extern char* htmlLinker_generateLinkToGenomeBrowserAtUCSC (char* database, char* clade, char* organism, char* chromosome, int start, int end);
extern char* htmlLinker_generateLinkToHumanPseudogenePageAtYale (char* pseudogeneId);
extern char* htmlLinker_generateLinkToFlyBaseGeneDescriptionPage (char* flyBaseId);
extern char* htmlLinker_generateLinkToWormBaseGeneDescriptionPage (char* wormBaseGeneId);
extern char* htmlLinker_generateLinkToUniProt (char* uniProtId);
extern char* htmlLinker_generateLinkToMouseGeneDescriptionPage (char* mgiId);
extern char* htmlLinker_generateLinkToRatGeneDescriptionPage (char* rgdId);
extern char* htmlLinker_generateLinkToYeastGeneDescriptionPage (char* sgdId);
extern char* htmlLinker_generateLinkToPfam (char* pfamId);
extern char* htmlLinker_generateLinkToInterPro (char* interProId);
extern char* htmlLinker_generateLinkToEntrezGene (char* term);
extern char* htmlLinker_generateLinkToPubmed (char* pmid);
extern char* htmlLinker_generateLinkToAmiGO (char* goId);


#endif
