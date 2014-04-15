#ifndef DEF_GENE_ONTOLOGY_H
#define DEF_GENE_ONTOLOGY_H


/**
 *   \file geneOntology.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */


#define MULTIPLE_TESTING_CORRECTION_BENJAMINI_HOCHBERG 1
#define MULTIPLE_TESTING_CORRECTION_BONFERRONI 2



/**
 * GoTagValue.
 */
typedef struct {
  char* tag;
  char* value;
} GoTagValue;



/**
 * GoTerm.
 */
typedef struct {
  char* id;
  char* name;
  char* namespace;
  Texta altIds;
  char* definition;
  Texta synonyms;
  Texta subsets;
  int isGenericGoSlim;
  char* comment;
  int isObsolete;
  Texta considers;
  Array xrefs; // of type TagValue
  Array relationships; // of type TagValue
  Texta parents;
} GoTerm;



/**
 * GoNode.
 */
typedef struct {
  char* id;
  GoTerm *goTerm;
  Array parents; // of type GoNode*
  Array children; // of type GoNode*
  Texta associatedGenes;
  Texta genesOfInterest;
} GoNode;



/**
 * GoGeneAssociation.
 */
typedef struct {
  char* db;
  char* dbGeneName;
  char* geneName;
  Texta goIds;
} GoGeneAssociation;



/**
 * GoStatistic.
 */
typedef struct {
  GoNode* goNode;
  Texta genesOfInterest; // Names of the genes of interest of goNode and its children
  int numberOfAnnotatedGenes;
  int numberOfGenesOfInterest;
  double pvalue;
  double pvalueCorrected;
} GoStatistic;



extern void geneOntology_init (char* geneOntologyFileName);
extern void geneOntology_mapGeneAnnotationsToGeneOntology (char* goGeneAssociationFileName);
extern Texta geneOntology_mapGenesOfInterestToGeneOntology (Texta genesOfInterest);
extern GoNode* geneOntology_getBiologicalProcessRoot (void);
extern GoNode* geneOntology_getMolecularFunctionRoot (void);
extern GoNode* geneOntology_getCellularComponentRoot (void);
extern Array geneOntology_getBiologicalProcessGoNodes (void);
extern Array geneOntology_getMolecularFunctionGoNodes (void);
extern Array geneOntology_getCellularComponentGoNodes (void);
extern Array geneOntology_getGenericGoSlimNodes (void);
extern Array geneOntology_getAllGoNodes (void);
extern int geneOntology_getNumberOfAssociatedGenes (void);
extern int geneOntology_getNumberOfGenesOfInterest (void);
extern void geneOntology_resetGenesOfInterest (void);
extern GoNode* geneOntology_findGoNode (char* id);
extern GoGeneAssociation* geneOntology_findGoGeneAssociation (char* geneName);
extern Array geneOntology_calculateGeneEnrichment (Array goNodePointers, int multipleTestingCorrectionMethod);
extern Array geneOntology_calculateGeneDepletion (Array goNodePointers, int multipleTestingCorrectionMethod);
extern Array geneOntology_getChildrenAtSpecifiedHierarchyLevel (GoNode* currGoNode, int level);



#endif
