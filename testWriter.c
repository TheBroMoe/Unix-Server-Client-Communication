#include <stdio.h>
#include <string.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#if defined(LIBXML_WRITER_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

#define MY_ENCODING "ISO-8859-1"

void testXmlwriterFilename(const char *uri);
xmlChar *ConvertInput(const char *in, const char *encoding);



int main(void){
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */

    /* first, the file version */
    testXmlwriterFilename(".dedup"); // THIS WILL NEED TO BE NORMAL FILE PATH WRT SERVER

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
    return 0;
}

/**
 * testXmlwriterFilename:
 * @uri: the output URI
 *
 * test the xmlWriter interface when writing to a new file
 */
void
testXmlwriterFilename(const char *uri)
{
    int rc;
    xmlTextWriterPtr writer;
    xmlChar *tmp;

    /* Create a new XmlWriter for uri, with no compression. */
    writer = xmlNewTextWriterFilename(uri, 0);
    if (writer == NULL) {
        printf("testXmlwriterFilename: Error creating the xml writer\n");
        return;
    }

    /* Start the document with the xml default for the version,
     * encoding ISO 8859-1 and the default for the standalone
     * declaration. */
    rc = xmlTextWriterStartDocument(writer, NULL, NULL, NULL);
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterStartDocument\n");
        return;
    }

    /* Start an element named "repository". Since thist is the first
     * element, this will be the root element of the document. */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "repository");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return;
    }

    /* Start an element named "ORDER" as child of repository. */
    rc = xmlTextWriterStartElement(writer, BAD_CAST "file");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return;
    }

    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hashname",
                                         "0123U124128451209358923");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteFormatElement\n");
        return;
    }


    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "foo.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "bar.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return;
    }

    rc = xmlTextWriterStartElement(writer, BAD_CAST "file");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return;
    }

    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hashname",
                                         "9923U124128451209358923");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteFormatElement\n");
        return;
    }


    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "fofffo.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "bafffr.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return;
    }

     rc = xmlTextWriterStartElement(writer, BAD_CAST "file");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterStartElement\n");
        return;
    }

    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hashname",
                                         "29232124228252202328222");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteFormatElement\n");
        return;
    }


    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "Spongebob.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "squid.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "Patrick.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterWriteElement(writer, BAD_CAST "knownas", "Star.txt");
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterWriteElement\n");
        return;
    }

    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterEndElement\n");
        return;
    }
    /* Here we could close the elements ORDER and repository using the
     * function xmlTextWriterEndElement, but since we do not want to
     * write any other elements, we simply call xmlTextWriterEndDocument,
     * which will do all the work. */
    rc = xmlTextWriterEndDocument(writer);
    if (rc < 0) {
        printf
            ("testXmlwriterFilename: Error at xmlTextWriterEndDocument\n");
        return;
    }

    xmlFreeTextWriter(writer);
}


#else
int main(void) {
    fprintf(stderr, "Writer or output support not compiled in\n");
    exit(1);
}
#endif