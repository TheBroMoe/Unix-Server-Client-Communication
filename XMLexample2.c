#include <stdio.h>
#include <libxml/xmlreader.h>
#include <string.h>
#ifdef LIBXML_READER_ENABLED


#define MAX_SIZE 50
#define STORAGE_SIZE 40
#define BUFFER_LENGTH 2000
#define FILES_PER_HASH 20
#define CHAR_LENGTH 256


// DATA STRUCTURE FOR FILE STORAGE
struct fileStructure{
	unsigned char *hashName; // Hashname for file
	char *knownAs[FILES_PER_HASH]; // Char array containing names of files for same hash
	int fileCount; // Current number of files related to hash
};

struct fileStructure dedupFile[STORAGE_SIZE];
int structCount = 0; // Global index for dedupFile
int flag = 0;

void initFileStruct(){
	for(int i = 0; i < STORAGE_SIZE; i++){
		dedupFile[i].fileCount = 0;	
		dedupFile[i].hashName = malloc(33);
		for(int j = 0; j < FILES_PER_HASH; j++){
			dedupFile[i].knownAs[j] = malloc(BUFFER_LENGTH);	
		}
	}
}


static void
processNode(xmlTextReaderPtr reader) {
	const xmlChar *name;//, *value;
	char* namething;
	char *value;
	name = xmlTextReaderConstName(reader);
	if (name == NULL)
		name = BAD_CAST "--";

	value = xmlTextReaderValue(reader);
	if ((strcmp(name,"hashname")==0) && (xmlTextReaderNodeType(reader) == 1)) {
		if (flag == 1) {
			structCount++;
		}
		flag = 0;

	}
	if (xmlTextReaderDepth(reader) == 3) {
		if (flag == 0){
			dedupFile[structCount].hashName = value;
			flag = 1;
		} else if (flag == 1) {
			//printf("Value in flag ==1 is %s\n", value);
			dedupFile[structCount].knownAs[dedupFile[structCount].fileCount] = value;
			dedupFile[structCount].fileCount++;		
		}

		/*printf("%d %d %s %d %d", 
		  xmlTextReaderDepth(reader),
		  xmlTextReaderNodeType(reader),
		  name,
		  xmlTextReaderIsEmptyElement(reader),
		  xmlTextReaderHasValue(reader));
		  if (value == NULL) {
		  printf("NULL\n");
		  } else {
		  if (xmlStrlen(value) > 40) {
		  printf(" %.40s...\n", value);
		  printf("VALUE IS %s\n", value);
		  }	else {
		  printf(" %s<<<\n", value);
		  printf("value is lower %s\n", value);
		  }
		  }*/
	}
}
static void
streamFile(const char *filename) {
	xmlTextReaderPtr reader;
	int ret;


	/*
	 * Pass some special parsing options to activate DTD attribute defaulting,
	 * entities substitution and DTD validation
	 */
	reader = xmlReaderForFile(filename, NULL,
			XML_PARSE_DTDATTR); // |  /* default DTD attributes */
	//	XML_PARSE_NOENT);// |    /* substitute entities */
	//XML_PARSE_DTDVALID); /* validate with the DTD */
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			processNode(reader);
			ret = xmlTextReaderRead(reader);
		}
		/*
		 * Once the document has been fully parsed check the validation results
		 */
		if (xmlTextReaderIsValid(reader) != 1) {
			fprintf(stderr, "Document %s does not validate\n", filename);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			fprintf(stderr, "%s : failed to parse\n", filename);
		}
	} else {
		fprintf(stderr, "Unable to open %s\n", filename);
	}
}

int main(int argc, char **argv) {
	if (argc != 2)
		return(1);

	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION

		streamFile(argv[1]);

	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	int i = 0;
	while (i <= structCount) {
		struct fileStructure *file = &dedupFile[i];
		printf("Printing Structure info\n");
		printf("Hash Name\n");
		printf("%s\n", file->hashName);
		printf("\nFile Names\n");
		for(int i = 0; i < file->fileCount; i++) printf("%s\n", file->knownAs[i]);
		printf("\nNumber of files: %d\n", file->fileCount);
		i++;
	}

	return(0);
}

#else
int main(void) {
	fprintf(stderr, "XInclude support not compiled in\n");
	exit(1);
}
#endif
