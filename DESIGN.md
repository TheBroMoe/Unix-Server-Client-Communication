CMPUT 379: Assignment 2 Design Document

Mohammad Kebbi; kebbi; 1496572 Zachary Kist; zkist; 1508381

What is the internal data structure you use to map file names to file contents?

We Stored each file as a struct called dedupFile which stores a file's hashname, filenames the hash is known as, and the total number of files associated with the hash. For the server, we used an array for the dedupFile struct that was used for managing the file contents. Different methods were put together to make working with the struct straightforward and trivial. These included:

initFileStruct() and freeStruct() for initializing and closing the fileStructure used
printFileInfo() for printing all values stored within a file entry
searchHash() for checking if a given hash is in the file Struct
searchFile() for checking if a file is in the file struct
addHashEntry() adds a hash value to the strucure as well as the first file associated with the hash
addKnownAsEntry() adds a known as entry to it's respective hash
getTotalFilenames() returns the total amount of files we have
removeHashEntry() removes a file from the data structure
deleteKnownAsEntry() removes a knownas from it's respective hash
The Filestruct and all the of it's methods are used in serverFile.c

How do you ensure that the server is responding efficiently and quickly to client requests?

The server follows a step by step process based on each request the client sends, and instantly reads and sends responses through a buffer. To avoid deadlock, the server ensures that only one client is being handled at a time to assure proper storage of files is being maintained.

How do you ensure changes to your internal data structures are consistent when multiple threads try to perform such changes?" The data structure used for managing files is saved as a global structure used by the server and hence is accessible by each client. Every time a client uploads or deletes a file, the structure gets updated immediately after the change is made (assuming the request does not cause an error)

How are the contents of a file which is being uploaded stored before you can determine whether it matches the hash of already stored file(s)? The file is temporarily written to the directory for storing files. The file's hash is then computed and compared with the stored files' hashes. If the hash exists, the file is removed from the directory and stored in the hash's knownas array. Otherwise the an entry for the new found hash is made and stored.

How do you deal with clients that fail or appear to be buggy without harming the consistency of your server? If we had more time, we would have added a function that took care of buggy clients that are indefinitely holding up the server are closed by the server if they take too long. This is done by using a hard timer after a request is sent. After the max time is exceeded the socket of the buggy client is closed.

How do you handle the graceful termination of the server when it receives SIGTERM? Using the signal handler for SIGTERM, we write all stored values from within the datastructure to the .dedup file, then free all memory allocated for the iternal datastructure. After that all pthreads are closed on the server's end and the server closes.

What sanity checks do you perform when the server starts on a given directory? We begin by checking if a .dedup file exists within the specified directory. If such a file exists, the file's data is loaded into the internal data structure used for the server. Otherwise the internal data structure is initalized to have nothing stored inside it.

How do you deal with zero sized files? If the size of the file is zero, we use a special associated 'zero' hash to store to and then if a user requests the file we send the filename and a size of zero
