#include "log.h"
#include "format.h"
#include "linestream.h"
#include "geneOntology.h"
#include <gsl/gsl_randist.h>


/**
 *   \file geneOntology.c Gene Ontology (GO) routines. 
 *   Note this module depends on the GNU scientific library (http://www.gnu.org/software/gsl).  
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



#define ANALYSIS_MODE_GENE_ENRICHMENT 1
#define ANALYSIS_MODE_GENE_DEPLETION 2



typedef struct {
  char* db;
  char* dbGeneName;
  char* geneName;
  char* goId;
} GoGeneAssociationEntry;



static Array goTerms = NULL; // of type GoTerm
static Array goNodes = NULL; // of type GoNode
static Array genericGoSlimNodes = NULL; // of type GoNode*
static Array goGeneAssociations = NULL; // of type GoGeneAssociation
static Texta genesOfInterest = NULL;
static GoNode* biologicalProcessRoot = NULL;
static GoNode* molecularFunctionRoot = NULL;
static GoNode* cellularComponentRoot = NULL;




/**
 * Get the GoNode that points to the biological_process root.
 * @return A GoNode pointer
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
GoNode* geneOntology_getBiologicalProcessRoot (void)
{
  return biologicalProcessRoot;
}



/**
 * Get the GoNode that points to the molecular_function root.
 * @return A GoNode pointer
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
GoNode* geneOntology_getMolecularFunctionRoot (void)
{
  return molecularFunctionRoot;
}



/**
 * Get the GoNode that points to the cellular_component root.
 * @return A GoNode pointer
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
GoNode* geneOntology_getCellularComponentRoot (void)
{
  return cellularComponentRoot;
}



static Array getGoNodesByNameSpace (char* namespace)
{
  int i;
  GoNode *currGoNode;
  static Array subsetGoNodes = NULL;

  if (subsetGoNodes == NULL) {
    subsetGoNodes = arrayCreate (15000,GoNode*);
  }
  else {
    arrayClear (subsetGoNodes);
  }
  for (i = 0; i < arrayMax (goNodes); i++) {
    currGoNode = arrp (goNodes,i,GoNode);
    if (strEqual (currGoNode->goTerm->namespace,namespace) || 
	strEqual (namespace,"all")) {
      array (subsetGoNodes,arrayMax (subsetGoNodes),GoNode*) = currGoNode;
    }
  }
  return subsetGoNodes;
}



/**
 * Get the GoNodes that are part of the biological_process hierarchy.
 * @return An Array of GoNode pointers
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
Array geneOntology_getBiologicalProcessGoNodes (void)
{
  return getGoNodesByNameSpace ("biological_process");
}



/**
 * Get the GoNodes that are part of the molecular_function hierarchy.
 * @return An Array of GoNode pointers
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
Array geneOntology_getMolecularFunctionGoNodes (void)
{
  return getGoNodesByNameSpace ("molecular_function");
}



/**
 * Get the GoNodes that are part of the cellular_component hierarchy.
 * @return An Array of GoNode pointers
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
Array geneOntology_getCellularComponentGoNodes (void)
{
  return getGoNodesByNameSpace ("cellular_component");
}



/**
 * Get the GoNodes of the entire ontology.
 * @return An Array of GoNode pointers
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
Array geneOntology_getAllGoNodes (void)
{
  return getGoNodesByNameSpace ("all");
}



/**
 * Get the generic GO slim nodes.
 * @return An Array of GoNode pointers
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @note The memory belongs to this module
 */
Array geneOntology_getGenericGoSlimNodes (void)
{
  return genericGoSlimNodes;
}



/**
 * Get the number of associated genes.
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @pre The gene annotations have been mapped to the gene ontology. 
   See geneOntology_mapGeneAnnotationsToGeneOntology()
 */
int geneOntology_getNumberOfAssociatedGenes (void) 
{
  return arrayMax (goGeneAssociations);
}



/**
 * Get the number of genes of interest that have been mapped to the gene ontology.
 * Note: duplicates gene names were removed.
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @pre The gene annotations have been mapped to the gene ontology. 
   See geneOntology_mapGeneAnnotationsToGeneOntology()
 * @pre The genes of interest have been mapped to the gene ontology. 
   See geneOntology_mapGenesOfInterestToGeneOntology()
 */
int geneOntology_getNumberOfGenesOfInterest (void)
{
  return arrayMax (genesOfInterest);
}



/**
 * Resets (performs arrayClear()) the genes of interest for every GoNode.
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @pre The gene annotations have been mapped to the gene ontology. 
   See geneOntology_mapGeneAnnotationsToGeneOntology()
 * @pre The genes of interest have been mapped to the gene ontology. 
   See geneOntology_mapGenesOfInterestToGeneOntology()
 * @post A new set of genes of interest can be mapped to the gene ontology by using
   geneOntology_mapGenesOfInterestToGeneOntology()
 */
void geneOntology_resetGenesOfInterest (void)
{
  int i;
  GoNode *currGoNode;
  
  for (i = 0; i < arrayMax (goNodes); i++) {
    currGoNode = arrp (goNodes,i,GoNode);
    if (currGoNode->genesOfInterest != NULL) {
      arrayClear (currGoNode->genesOfInterest);
    }
  }
}



static void readGoOntology (char* oboFileName)
{
  LineStream ls;
  char *line,*pos1,*pos2;
  int isGoTerm;
  GoTerm *currGoTerm;
  GoTagValue *currGoTagValue;
  
  goTerms = arrayCreate (10000,GoTerm);
  ls = ls_createFromFile (oboFileName);
  isGoTerm = 0;
  while (line = ls_nextLine (ls)) {
    if (strEqual (line,"[Term]")) {
      isGoTerm = 1;
      currGoTerm = arrayp (goTerms,arrayMax (goTerms),GoTerm);
      continue;
    }
    else if (line[0] == '\0') {
      isGoTerm = 0;
      continue;
    }
    if (isGoTerm == 1) {
      if (strStartsWithC (line,"id:")) {
	pos1 = strchr (line,' ');
	currGoTerm->id = hlr_strdup (pos1 + 1);
      }
      else if (strStartsWithC (line,"name:")) {
	pos1 = strchr (line,' ');
	currGoTerm->name = hlr_strdup (pos1 + 1);
      }
      else if (strStartsWithC (line,"namespace:")) {
	pos1 = strchr (line,' ');
	currGoTerm->namespace = hlr_strdup (pos1 + 1);
      }
      else if (strStartsWithC (line,"alt_id:")) {
	if (currGoTerm->altIds == NULL) {
	  currGoTerm->altIds = textCreate (10);
	}
	pos1 = strchr (line,' ');
	textAdd (currGoTerm->altIds,pos1 + 1);
      }
      else if (strStartsWithC (line,"def:")) {
	pos1 = strstr (line," [");
	if (pos1 == NULL) {
	  die ("Expected ' [' in def line: %s",line);
	}
	*pos1 = '\0';
	pos1 = strchr (line,' ');
	strTrim (pos1," \""," \"");
	currGoTerm->definition = hlr_strdup (pos1);
      }
      else if (strStartsWithC (line,"subset:")) {
	if (currGoTerm->subsets == NULL) {
	  currGoTerm->subsets = textCreate (10);
	}
	pos1 = strchr (line,' ');
	textAdd (currGoTerm->subsets,pos1 + 1);
	if (strEqual (pos1 + 1,"goslim_generic")) {
	  currGoTerm->isGenericGoSlim = 1;
	}
      }
      else if (strStartsWithC (line,"comment:")) {
	pos1 = strchr (line,' ');
	currGoTerm->comment = hlr_strdup (pos1 + 1);
      }
      else if (strEqual (line,"is_obsolete: true")) {
	currGoTerm->isObsolete = 1;
      }
      else if (strStartsWithC (line,"synonym:")) {
	if (currGoTerm->synonyms == NULL) {
	  currGoTerm->synonyms = textCreate (10);
	}
	pos1 = strrchr (line,'"');
	*pos1 = '\0';
	pos1 = strchr (line,'"');
	textAdd (currGoTerm->synonyms,pos1 + 1);
      }
      else if (strStartsWithC (line,"consider:")) {
	if (currGoTerm->considers == NULL) {
	  currGoTerm->considers = textCreate (10);
	}
	pos1 = strchr (line,' ');
	textAdd (currGoTerm->considers,pos1 + 1);
      }
      else if (strStartsWithC (line,"xref:")) {
	if (currGoTerm->xrefs == NULL) {
	  currGoTerm->xrefs = arrayCreate (10,GoTagValue);
	}
	currGoTagValue = arrayp (currGoTerm->xrefs,arrayMax (currGoTerm->xrefs),GoTagValue);
	pos1 = strchr (line,' ');
	pos2 = strchr (pos1 + 1,':');
	*pos2 = '\0';
	currGoTagValue->tag = hlr_strdup (pos1 + 1);
	currGoTagValue->value = hlr_strdup (pos2 + 1);
      }
      else if (strStartsWithC (line,"is_a:")) {
	if (currGoTerm->parents == NULL) {
	  currGoTerm->parents = textCreate (10);
	}
	if (pos1 = strchr (line,'!')) {
	  *(pos1 - 1) = '\0';
	}
	pos1 = strchr (line,' ');
	textAdd (currGoTerm->parents,pos1 + 1);
      }
      else if (strStartsWithC (line,"relationship:")) {
	if (currGoTerm->relationships == NULL) {
	  currGoTerm->relationships = arrayCreate (10,GoTagValue);
	}
	currGoTagValue = arrayp (currGoTerm->relationships,arrayMax (currGoTerm->relationships),GoTagValue);
	if (pos1 = strchr (line,'!')) {
	  *(pos1 - 1) = '\0';
	}
	pos1 = strchr (line,' ');
	pos2 = strchr (pos1 + 1,' ');
	*pos2 = '\0';
	currGoTagValue->tag = hlr_strdup (pos1 + 1);
	currGoTagValue->value = hlr_strdup (pos2 + 1);
      }
      else if (strStartsWithC (line,"replaced_by:")) {
	;
      }
      else if (strStartsWithC (line,"disjoint_from:")) {
	;
      }
      else {
	warn ("Unexpected line: %s",line);
      }
    }
  }
  ls_destroy (ls);
}



static int sortGoNodesById (GoNode* a, GoNode* b)
{
  return strcmp (a->id,b->id);
}



/**
 * Find a GoNode.
 * @param[in] id A GO term identifier. Example: GO:0065003
 * @return A pointer to the GoNode, NULL if the GO term was not found
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
GoNode* geneOntology_findGoNode (char* id) 
{
  GoNode testGoNode;
  int index,foundGoNode;

  testGoNode.id = hlr_strdup (id);
  foundGoNode = arrayFind (goNodes,&testGoNode,&index,(ARRAYORDERF)sortGoNodesById);
  hlr_free (testGoNode.id);
  if (foundGoNode == 1) {
    return arrp (goNodes,index,GoNode);
  }
  return NULL;
}



static void addGoNode (Array a, GoNode* n)
{
  int i;
  GoNode *currGoNode;
  
  for (i = 0; i < arrayMax (a); i++) {
    currGoNode = arru (a,i,GoNode*);
    if (currGoNode == n) {
      return;
    }
  }
  array (a,arrayMax (a),GoNode*) = n;
}



static void goTerms2goNodes (void)
{
  int i,j;
  GoTerm *currGoTerm;
  GoNode *currGoNode,*parentGoNode;

  goNodes = arrayCreate (arrayMax (goTerms),GoNode);
  genericGoSlimNodes = arrayCreate (500,GoNode*);
  for (i = 0; i < arrayMax (goTerms); i++) {
    currGoTerm = arrp (goTerms,i,GoTerm);
    if (currGoTerm->isObsolete == 1) {
      continue;
    }
    currGoNode = arrayp (goNodes,arrayMax (goNodes),GoNode);
    currGoNode->goTerm = currGoTerm;
    currGoNode->id = hlr_strdup (currGoTerm->id);
    if (currGoNode->goTerm->isGenericGoSlim == 1) {
      array (genericGoSlimNodes,arrayMax (genericGoSlimNodes),GoNode*) = currGoNode;
    }
    if (strEqual (currGoNode->goTerm->name,"biological_process")) {
      biologicalProcessRoot = currGoNode;
    }
    else if (strEqual (currGoNode->goTerm->name,"molecular_function")) {
      molecularFunctionRoot = currGoNode;
    }
    else if (strEqual (currGoNode->goTerm->name,"cellular_component")) {
      cellularComponentRoot = currGoNode;
    }
  }
  arraySort (goNodes,(ARRAYORDERF)sortGoNodesById);
  for (i = 0; i < arrayMax (goNodes); i++) {
    currGoNode = arrp (goNodes,i,GoNode);
    if (currGoNode->goTerm->parents == NULL) {
      continue;
    }
    if (currGoNode->parents == NULL) {
      currGoNode->parents = arrayCreate (10,GoNode*);
    }
    for (j = 0; j < arrayMax (currGoNode->goTerm->parents); j++) {
      parentGoNode = geneOntology_findGoNode (textItem (currGoNode->goTerm->parents,j));
      if (parentGoNode == NULL) {
	die ("Expected to find %s in GO ontology: %s",textItem (currGoNode->goTerm->parents,j));
      }
      addGoNode (currGoNode->parents,parentGoNode);
      if (parentGoNode->children == NULL) {
	parentGoNode->children = arrayCreate (10,GoNode*);
      }
      addGoNode (parentGoNode->children,currGoNode);
    }
  }
}



static int sortGoGeneAssociationEntriesByGeneName (GoGeneAssociationEntry *a, GoGeneAssociationEntry *b)
{
  return strcmp (a->geneName,b->geneName);
}



static int sortGoGeneAssociationsByGeneName (GoGeneAssociation *a, GoGeneAssociation *b)
{
  return strcmp (a->geneName,b->geneName);
}



static void readGoAnnotations (char* goAnnotationFileName)
{
  LineStream ls;
  char *line;
  WordIter w;
  Array goGeneAssociationEntries;
  GoGeneAssociationEntry *currGoGeneAssociationEntry,*nextGoGeneAssociationEntry;
  GoGeneAssociation *currGoGeneAssociation;
  int i,j;

  goGeneAssociationEntries = arrayCreate (100000,GoGeneAssociationEntry);
  ls = ls_createFromFile (goAnnotationFileName); 
  while (line = ls_nextLine (ls)) {
    if (line[0] == '\0' || line[0] == '!') {
      continue;
    }
    currGoGeneAssociationEntry = arrayp (goGeneAssociationEntries,arrayMax (goGeneAssociationEntries),GoGeneAssociationEntry);
    w = wordIterCreate (line,"\t",0);
    currGoGeneAssociationEntry->db = hlr_strdup (wordNext (w));
    currGoGeneAssociationEntry->dbGeneName = hlr_strdup (wordNext (w));
    currGoGeneAssociationEntry->geneName = hlr_strdup (wordNext (w));
    wordNext (w); // optional column [qualifier], not considered
    currGoGeneAssociationEntry->goId = hlr_strdup (wordNext (w));
    wordIterDestroy (w);
  }
  ls_destroy (ls);
  arraySort (goGeneAssociationEntries,(ARRAYORDERF)sortGoGeneAssociationEntriesByGeneName);
  goGeneAssociations = arrayCreate (10000,GoGeneAssociation);
  i = 0;
  while (i < arrayMax (goGeneAssociationEntries)) {
    currGoGeneAssociationEntry = arrp (goGeneAssociationEntries,i,GoGeneAssociationEntry);
    currGoGeneAssociation = arrayp (goGeneAssociations,arrayMax (goGeneAssociations),GoGeneAssociation);
    currGoGeneAssociation->db = hlr_strdup (currGoGeneAssociationEntry->db);
    currGoGeneAssociation->dbGeneName = hlr_strdup (currGoGeneAssociationEntry->dbGeneName);
    currGoGeneAssociation->geneName = hlr_strdup (currGoGeneAssociationEntry->geneName);
    currGoGeneAssociation->goIds = textCreate (10);
    textAdd (currGoGeneAssociation->goIds,currGoGeneAssociationEntry->goId);
    j = i + 1;
    while (j < arrayMax (goGeneAssociationEntries)) {
      nextGoGeneAssociationEntry = arrp (goGeneAssociationEntries,j,GoGeneAssociationEntry);
      if (strEqual (currGoGeneAssociationEntry->geneName,nextGoGeneAssociationEntry->geneName)) {
	textAdd (currGoGeneAssociation->goIds,nextGoGeneAssociationEntry->goId);
      }
      else {
	break;
      }
      j++;
    }
    i = j;
    textUniqKeepOrder (currGoGeneAssociation->goIds); 
  }
  arraySort (goGeneAssociations,(ARRAYORDERF)sortGoGeneAssociationsByGeneName);
}



static void mapAnnotatedGenesToGoOntology (void)
{
  int i,j;
  GoGeneAssociation *currGoGeneAssociation;
  GoNode *currGoNode;

  for (i = 0; i < arrayMax (goGeneAssociations); i++) {
    currGoGeneAssociation = arrp (goGeneAssociations,i,GoGeneAssociation);
    for (j = 0; j < arrayMax (currGoGeneAssociation->goIds); j++) {
      currGoNode = geneOntology_findGoNode (textItem (currGoGeneAssociation->goIds,j));
      if (currGoNode == NULL) {
	die ("Expected to find %s in GO ontology: %s",textItem (currGoGeneAssociation->goIds,j));
      }
      if (currGoNode->associatedGenes == NULL) {
	currGoNode->associatedGenes = textCreate (50);
      }
      textAdd (currGoNode->associatedGenes,currGoGeneAssociation->geneName);
    }
  }
}



/**
 * Find a GoGeneAssociation.
 * @param[in] geneName A geneName or geneSymbol as specified in the geneAssociation file. 
   See http://www.geneontology.org/GO.format.annotation.shtml for details. 
   This geneName refers to the third column labeled as DB_Object_Symbol.
 * @return A pointer to the GoGeneAssociation, NULL if GoGeneAssociation was not found 
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @pre The gene annotations have been mapped to the gene ontology. 
   See geneOntology_mapGeneAnnotationsToGeneOntology()
 */
GoGeneAssociation* geneOntology_findGoGeneAssociation (char* geneName) 
{
  GoGeneAssociation testGoGeneAssociation;
  int index,foundGoGeneAssociation;

  testGoGeneAssociation.geneName = hlr_strdup (geneName);
  foundGoGeneAssociation = arrayFind (goGeneAssociations,&testGoGeneAssociation,&index,(ARRAYORDERF)sortGoGeneAssociationsByGeneName);
  hlr_free (testGoGeneAssociation.geneName);
  if (foundGoGeneAssociation == 1) {
    return arrp (goGeneAssociations,index,GoGeneAssociation);
  }
  return NULL;
}



/**
 * Map genes of interest to the gene ontology.
 * @param[in] genesNamesOfInterest Texta that contains the gene names of the genes of interest. 
   The gene names must be consistent with the genes names found in the gene annotation file.  
 * @return Texta of invalid gene names. If the number of invalid gene names is zero, then
   an empty Texta is returned. 
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @pre The gene annotations have been mapped to the gene ontology. 
   See geneOntology_mapGeneAnnotationsToGeneOntology()
 */
Texta geneOntology_mapGenesOfInterestToGeneOntology (Texta geneNamesOfInterest) 
{
  int i,j;
  GoGeneAssociation *currGoGeneAssociation;
  GoNode *currGoNode;
  static Texta invalidGeneNames = NULL;
  
  textCreateClear (invalidGeneNames,100);
  genesOfInterest = textCreate (100);
  textUniqKeepOrder (geneNamesOfInterest);
  for (i = 0; i < arrayMax (geneNamesOfInterest); i++) {
    currGoGeneAssociation = geneOntology_findGoGeneAssociation (textItem (geneNamesOfInterest,i));
    if (currGoGeneAssociation != NULL) {
      textAdd (genesOfInterest,textItem (geneNamesOfInterest,i));
      for (j = 0; j < arrayMax (currGoGeneAssociation->goIds); j++) {
	currGoNode = geneOntology_findGoNode (textItem (currGoGeneAssociation->goIds,j));
	if (currGoNode == NULL) {
	  die ("Expected to find %s in GO ontology: %s",textItem (currGoGeneAssociation->goIds,j));
	}
	if (currGoNode->genesOfInterest == NULL) {
	  currGoNode->genesOfInterest = textCreate (10);
	}
	textAdd (currGoNode->genesOfInterest,textItem (geneNamesOfInterest,i));
      }
    }
    else {
      textAdd (invalidGeneNames,textItem (geneNamesOfInterest,i));
    }
  }
  return invalidGeneNames;
}



/**
 * Initialize the geneOntology module.
 * @param[in] geneOntologyFileName File that describes the entire gene ontology. This file has to be in OBO format. 
   See http://www.geneontology.org/GO.format.obo-1_2.shtml for details.
 */
void geneOntology_init (char* geneOntologyFileName)
{
  readGoOntology (geneOntologyFileName);
  goTerms2goNodes (); 
}



/**
 * Map gene annotations to the gene ontology.
 * @param[in] goGeneAssociationFileName File that maps the genes of a specific organism to the gene ontology. 
   This file has to be in GO Annotation format. See http://www.geneontology.org/GO.format.annotation.shtml for details.
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
void geneOntology_mapGeneAnnotationsToGeneOntology (char* goGeneAssociationFileName)
{
  readGoAnnotations (goGeneAssociationFileName);
  mapAnnotatedGenesToGoOntology ();
}



static void countGenes (GoNode* n, Texta goNodeAnnotatedGenes, Texta goNodeGenesOfInterest)
{
  int i;
  GoNode *currGoNode;

  if (n->children != NULL) {
    for (i = 0; i < arrayMax (n->children); i++) {
      currGoNode = arru (n->children,i,GoNode*);
      countGenes (currGoNode,goNodeAnnotatedGenes,goNodeGenesOfInterest);
    }
  }
  if (n->associatedGenes != NULL) {
    for (i = 0; i < arrayMax (n->associatedGenes); i++) {
      textAdd (goNodeAnnotatedGenes,textItem (n->associatedGenes,i));
    }
  }
  if (n->genesOfInterest != NULL) {
    for (i = 0; i < arrayMax (n->genesOfInterest); i++) {
      textAdd (goNodeGenesOfInterest,textItem (n->genesOfInterest,i));
    }
  }
}



static double calculatePvalueForEnrichment (int k, int n1, int n2, int t)
{
  double pvalue;
  int i;
  
  pvalue = 0.0;
  for(i = k; i <= n1; i++) {
    pvalue += gsl_ran_hypergeometric_pdf (i,n1,n2,t);
  }
  return pvalue;
}



static double calculatePvalueForDepletion (int k, int n1, int n2, int t)
{
  double pvalue;
  int i;
  
  pvalue = 0.0;
  for(i = 0; i <= k; i++) {
    pvalue += gsl_ran_hypergeometric_pdf (i,n1,n2,t);
  }
  return pvalue;
}



static Texta calculateGeneEnrichmentOrDepletionForGoTerm (GoNode* goNode, int* numberOfAnnotatedGenes, 
							  int* numberOfGenesOfInterest, 
							  double* pvalue, int analysisMode)
{ 
  static Texta goNodeAnnotatedGenes = NULL;
  static Texta goNodeGenesOfInterest = NULL;

  textCreateClear (goNodeAnnotatedGenes,1000);
  textCreateClear (goNodeGenesOfInterest,1000);
  countGenes (goNode,goNodeAnnotatedGenes,goNodeGenesOfInterest);
  arraySort (goNodeAnnotatedGenes,(ARRAYORDERF)arrayStrcmp);
  arraySort (goNodeGenesOfInterest,(ARRAYORDERF)arrayStrcmp);
  arrayUniq (goNodeAnnotatedGenes,NULL,(ARRAYORDERF)arrayStrcmp);
  arrayUniq (goNodeGenesOfInterest,NULL,(ARRAYORDERF)arrayStrcmp);
  *numberOfAnnotatedGenes = arrayMax (goNodeAnnotatedGenes);
  *numberOfGenesOfInterest = arrayMax (goNodeGenesOfInterest);
  if (analysisMode == ANALYSIS_MODE_GENE_ENRICHMENT) {
    *pvalue = calculatePvalueForEnrichment (*numberOfGenesOfInterest,*numberOfAnnotatedGenes,
					    arrayMax (goGeneAssociations) - (*numberOfAnnotatedGenes),
					    arrayMax (genesOfInterest));
  }
  else if (analysisMode == ANALYSIS_MODE_GENE_DEPLETION) {
    *pvalue = calculatePvalueForDepletion (*numberOfGenesOfInterest,*numberOfAnnotatedGenes,
					   arrayMax (goGeneAssociations) - (*numberOfAnnotatedGenes),
					   arrayMax (genesOfInterest));
  }
  else {
    die ("Unknown analysis mode: %d",analysisMode);
  }
  return goNodeGenesOfInterest;
}



static int sortGoStatisticsByPvalue (GoStatistic *a, GoStatistic *b)
{
  if (a->pvalue < b->pvalue) {
    return -1;
  }
  if (a->pvalue > b->pvalue) {
    return 1;
  }
  return 0;
}



static Array calculateGeneEnrichmentOrDepletion (Array goNodePointers, int multipleTestingCorrectionMethod, int analysisMode)
{
  static Array goStatistics = NULL;
  GoStatistic *currGoStatistic;
  int i;
  GoNode* currGoNode;
  int numberOfAnnotatedGenes,numberOfGenesOfInterest;
  double pvalue;
  Texta genesOfInterest;

  if (goStatistics == NULL) {
    goStatistics = arrayCreate (10000,GoStatistic);
  }
  else {
    for (i = 0; i < arrayMax (goStatistics); i++) {
      currGoStatistic = arrp (goStatistics,i,GoStatistic);
      textDestroy (currGoStatistic->genesOfInterest);
    }
    arrayClear (goStatistics);
  }
  for (i = 0; i < arrayMax (goNodePointers); i++) {
    currGoNode = arru (goNodePointers,i,GoNode*);
    genesOfInterest = calculateGeneEnrichmentOrDepletionForGoTerm (currGoNode,&numberOfAnnotatedGenes,
								   &numberOfGenesOfInterest,&pvalue,analysisMode);
    currGoStatistic = arrayp (goStatistics,arrayMax (goStatistics),GoStatistic);
    currGoStatistic->goNode = currGoNode;
    currGoStatistic->numberOfAnnotatedGenes = numberOfAnnotatedGenes;
    currGoStatistic->numberOfGenesOfInterest = numberOfGenesOfInterest;
    currGoStatistic->pvalue = pvalue;
    currGoStatistic->genesOfInterest = textClone (genesOfInterest);
  }
  arraySort (goStatistics,(ARRAYORDERF)sortGoStatisticsByPvalue);
  for (i = 0; i < arrayMax (goStatistics); i++) {
    currGoStatistic = arrp (goStatistics,i,GoStatistic);
    if (multipleTestingCorrectionMethod == MULTIPLE_TESTING_CORRECTION_BENJAMINI_HOCHBERG) {
      currGoStatistic->pvalueCorrected  = MIN (currGoStatistic->pvalue * arrayMax (goStatistics) / (i + 1),1.0); 
    }
    else if (multipleTestingCorrectionMethod == MULTIPLE_TESTING_CORRECTION_BONFERRONI) {
      currGoStatistic->pvalueCorrected =  MIN (currGoStatistic->pvalue * arrayMax (goStatistics),1.0);
    }
    else {
      die ("Unknown multiple testing correction method: %d",multipleTestingCorrectionMethod);
    }
  }
  return goStatistics;
}



/**
 * Calculate the gene enrichment for every GoNode in goNodePointers.
 * @param[in] goNodePointers Array of goNode pointers. Gene enrichment is calculated for every goNode in this Array.
 * @param[in] multipleTestingCorrectionMethod Specifies the multiple testing correction method. Use either 
   MULTIPLE_TESTING_CORRECTION_BENJAMINI_HOCHBERG or MULTIPLE_TESTING_CORRECTION_BONFERRONI.
 * @return Array of type GoStatistic. This array is sorted by pvalue in ascending order.
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @pre The gene annotations have been mapped to the gene ontology. 
   See geneOntology_mapGeneAnnotationsToGeneOntology()
 * @pre The genes of interest have been mapped to the gene ontology. 
   See geneOntology_mapGenesOfInterestToGeneOntology()
 */
Array geneOntology_calculateGeneEnrichment (Array goNodePointers, int multipleTestingCorrectionMethod)
{
  return calculateGeneEnrichmentOrDepletion (goNodePointers,multipleTestingCorrectionMethod,
					     ANALYSIS_MODE_GENE_ENRICHMENT);
}



/**
 * Calculate the gene depletion for every GoNode in goNodePointers.
 * @param[in] goNodePointers Array of goNode pointers. Gene depletion is calculated for every goNode in this Array.
 * @param[in] multipleTestingCorrectionMethod Specifies the multiple testing correction method. Use either 
   MULTIPLE_TESTING_CORRECTION_BENJAMINI_HOCHBERG or MULTIPLE_TESTING_CORRECTION_BONFERRONI.
 * @return Array of type GoStatistic. This array is sorted by pvalue in ascending order.
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 * @pre The gene annotations have been mapped to the gene ontology. 
   See geneOntology_mapGeneAnnotationsToGeneOntology()
 * @pre The genes of interest have been mapped to the gene ontology. 
   See geneOntology_mapGenesOfInterestToGeneOntology()
 */
Array geneOntology_calculateGeneDepletion (Array goNodePointers, int multipleTestingCorrectionMethod)
{
  return calculateGeneEnrichmentOrDepletion (goNodePointers,multipleTestingCorrectionMethod,
					     ANALYSIS_MODE_GENE_DEPLETION);
}



static void getChildrenAtHierarchyLevel (GoNode* n, Array resultGoNodes, int currentLevel, int specifiedLevel)
{
  int i;
  GoNode* currGoNode;

  if (currentLevel == specifiedLevel) {
    i = 0; 
    while (i < arrayMax (resultGoNodes)) {
      if (arru (resultGoNodes,i,GoNode*) == n) {
        break;
      }
      i++;
    }
    if (i == arrayMax (resultGoNodes)) {
      array (resultGoNodes,arrayMax (resultGoNodes),GoNode*) = n;
    }
    return;
  }
  if (n->children != NULL) {
    for (i = 0; i < arrayMax (n->children); i++) {
      currGoNode = arru (n->children,i,GoNode*);
      getChildrenAtHierarchyLevel (currGoNode,resultGoNodes,currentLevel + 1,specifiedLevel);
    }
  }
}




/**
 * Get the children of a GoNode at a specified hiearchy level. 
 * @param[in] currGoNode Get the children of this GoNode at the specified hiearchy level.
 * @param[in] level Specifies the level within the gene ontology relative to the currGoNode. 
   The level of currGoNode is 0.
 * Example: To get the grand-children of currGoNode the level would be 2.
 * @return An Array of GoNode pointers.
 * @pre The geneOntolgy module has been initialized. See geneOntology_init()
 */
Array geneOntology_getChildrenAtSpecifiedHierarchyLevel (GoNode* currGoNode, int level)
{
  static Array resultGoNodes = NULL;

  if (resultGoNodes == NULL) {
    resultGoNodes = arrayCreate (1000,GoNode*);
  }
  else {
    arrayClear (resultGoNodes);
  }
  getChildrenAtHierarchyLevel (currGoNode,resultGoNodes,0,level);
  return resultGoNodes;
}
