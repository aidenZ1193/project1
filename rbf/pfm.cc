#include "pfm.h"

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}

// check if the file already exists
// check if it can be opened correctly
// check if it can be closed properly
RC PagedFileManager::createFile(const string &fileName)
{
    if(fileExists(fileName) == 0)
        return -1;

    FILE* filePointer = fopen(fileName.c_str(), "wb");
    if(filePointer == NULL){
        cout<<"open failed"<<endl;
        return -1;
    }

    if(fclose(filePointer) != 0)
        return -1;

    return 0;
}

// check if the name already exists
// delete the file
RC PagedFileManager::destroyFile(const string &fileName)
{
    if(fileExists(fileName) != 0)
        return -1;

    if(remove(fileName.c_str()) != 0)
        return -1;

    return 0;
}

// check if fileHandle is valid
// check if file exists
// open file
RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    if(fileHandle.getFilePinter() != NULL || fileHandle.getFileName() != "")
        return -1;

    if(fileExists(fileName) != 0)
        return -1;
    
    // check if opens correctly
    FILE *file = fopen(fileName.c_str(), "rb+w");
    if(file == NULL)
        return -1;

    fileHandle.setFileName(fileName);
    fileHandle.setFilePOinter(file);

    return 0;
    
}

// check if the handle is connected to a valid file pointer
// re-intialize the file pointer and file name in the handle
RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    if(fileHandle.getFilePinter() == NULL)
        return -1;
 
    if(fclose(fileHandle.getFilePinter()) != 0)
        return -1;

    fileHandle.setFileName("");
    fileHandle.setFilePOinter(NULL);

    return 0;
}

// @return: 0 when file exists
int PagedFileManager::fileExists(const string &filename)
{
    struct stat fileBuffer;
    if(stat(filename.c_str(), &fileBuffer) == 0)
        return 0;
    return -1;
}


FileHandle::FileHandle()
{
    readPageCounter = 0;
    writePageCounter = 0;
    appendPageCounter = 0;
    fp = NULL;
    filename = "";
}


FileHandle::~FileHandle()
{
    delete fp;
}

// check if file is valid&open
// check if page number is valid
// check if fseek correct
RC FileHandle::readPage(PageNum pageNum, void *data)
{
    if(fp == NULL)
        return -1;

    unsigned num = getNumberOfPages();
    if(pageNum < 0 || pageNum+2 > num)  // -2 because the index of pageNum starts at 0
        return -1;

    // pageNum+1 because of the hidden page
    if(fseek(fp, PAGE_SIZE * (pageNum+1), SEEK_SET) != 0)
        return -1;

    // clear whatever was in the data
    memset(data, 0, PAGE_SIZE);
    if(fread(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
        return -1;

    readPageCounter ++;

    if(writeCounters() != 0)
        return -1;

    cout<<"reading successed"<<endl;

    return 0;
}

// check if file is valid
// check if pageNUm is vald
// check if write is successful

// for hidden page:
// check if the writeCOunter is 0
// if yes, then make a page for the counters
RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    if(fp == NULL)
        return -1;
    
    cout<<pageNum<<"and actual page-2"<<getNumberOfPages()-2<<endl;

    if(pageNum+2 > getNumberOfPages() || pageNum < 0)
        return -1;

    // use pageNum+1 because of the existance of hidden page
    if(fseek(fp, PAGE_SIZE*(pageNum+1), SEEK_SET) != 0)
        return -1;

    if(fwrite(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
        return -1;

    writePageCounter ++;
    writeCounters();

    cout<<"writing successed"<<endl;

    return 0;
}

// adds a new page to the end of file
// move to the end of file
// write data to the file
RC FileHandle::appendPage(const void *data)
{
    if(fp == NULL)
        return -1;

    // now for the hidden page:
    // if file is empty, then make the hidden page first
    if(writePageCounter == 0){
        void *hidden = malloc(PAGE_SIZE);
        memset(hidden, '\0', sizeof(byte)*PAGE_SIZE);
        fwrite(hidden, 1, PAGE_SIZE, fp);
    }
    
    // move to the end of file
    if(fseek(fp, 0, SEEK_END) != 0)
        return -1;
    
    if(fwrite(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
        return -1;

    if(writeCounters() != 0)
        return -1;

    appendPageCounter ++;

    return 0;
}

// get size of file and divided by page size
unsigned FileHandle::getNumberOfPages()
{
    if(fp == NULL)
        return -1;

    if(fseek(fp, 0, SEEK_END) != 0)
        return -1;

    int page = ftell(fp)/PAGE_SIZE;

    if(page == 0)
        return 0;

    cout<<page<<endl;

    return page-1;

}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    readPageCount = readPageCounter;
    writePageCount = writePageCounter;
    appendPageCount = appendPageCounter;
    return 0;
}

// write counters to the hidden page of file
RC FileHandle::writeCounters()
{
    if(fseek(fp, 0, SEEK_END) != 0)
        return -1;

    void *hidden = malloc(100);
    memset(hidden, '\0', 100);
    string s = "readPageCount "+std::to_string(readPageCounter);
    s = s + "writePageCount "+std::to_string(writePageCounter);
    s = s + "appendPageCount "+std::to_string(appendPageCounter);
    strcpy((char*)hidden, s.c_str());

    // write to file
    if(fwrite(hidden, 1, 100, fp) <= 0)
        return -1;

    // debug time
    cout<<hidden<<endl;
    return 0;

}


FILE* FileHandle::getFilePinter()
{
    return fp;
}
void FileHandle::setFilePOinter(FILE* filePointer)
{
    fp = filePointer;
}

string FileHandle::getFileName()
{
    return filename;
}
void FileHandle::setFileName(string name)
{
    filename = name;
}
