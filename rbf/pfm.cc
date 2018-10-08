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
    delete _pf_manager;
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
    if(fileHandle.getFilePointer() != NULL || fileHandle.getFileName() != "")
        return -1;

    if(fileExists(fileName) != 0)
        return -1;
    
    // check if opens correctly
    FILE *file = fopen(fileName.c_str(), "rb+w");
    if(file == NULL)
        return -1;

    fileHandle.setFileName(fileName);
    fileHandle.setFilePointer(file);

    if(fileHandle.getNumberOfPages() > 0){
        // set current page to the end of file
        fileHandle.currentPageNum = fileHandle.getNumberOfPages();
        // use data to read page, and be the current page
        void* data = malloc(PAGE_SIZE);
        if(fileHandle.readPage(fileHandle.currentPageNum, data) != -1)
            fileHandle.currentPage = data;
        free(data);
    }

    return 0;
    
}

// check if the handle is connected to a valid file pointer
// re-intialize the file pointer and file name in the handle
RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    if(fileHandle.getFilePointer() == NULL)
        return -1;
 
    if(fclose(fileHandle.getFilePointer()) != 0)
        return -1;

    fileHandle.setFileName("");
    fileHandle.setFilePointer(NULL);

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
    currentPageNum = 0;
    currentPage = NULL;
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
    if(pageNum < 0 || pageNum+1 > num){  // -2 because the index of pageNum starts at 0
        //cout<<"reading page is "<<pageNum<<" ,total pages = "<<num<<endl;
        return -1;
    }

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
    
    //cout<<pageNum<<"and actual page-2 = "<<getNumberOfPages()-2<<endl;

    if(pageNum+1 > getNumberOfPages() || pageNum < 0){
        cout<<"index "<<pageNum<<" doesn't exist, returning -1"<<endl;
        return -1;
    }

    // use pageNum+1 because of the existance of hidden page
    if(fseek(fp, PAGE_SIZE*(pageNum+1), SEEK_SET) != 0)
        return -1;

    if(fwrite(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
        return -1;

    writePageCounter ++;
    writeCounters();

    currentPageNum = pageNum;

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
    if(appendPageCounter == 0){
        void *hidden = malloc(PAGE_SIZE);
        memset(hidden, '\0', PAGE_SIZE);
        fwrite(hidden, 1, PAGE_SIZE, fp);
        free(hidden);
    }
    
    // move to the end of file
    if(fseek(fp, 0, SEEK_END) != 0)
        return -1;
    
    if(fwrite(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
        return -1;

    if(writeCounters() != 0)
        return -1;

    appendPageCounter ++;
    currentPageNum = getNumberOfPages();

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
    if(fseek(fp, 0, SEEK_SET) != 0)
        return -1;

    void *hidden = malloc(100);
    memset(hidden, '\0', 100);
    string s = "readPageCount "+std::to_string(readPageCounter);
    s = s + "writePageCount "+std::to_string(writePageCounter);
    s = s + "appendPageCount "+std::to_string(appendPageCounter);
    strcpy((char*)hidden, s.c_str());

    // write to file
    if(fwrite(hidden, 1, 100, fp) <= 0){
        free(hidden);
        return -1;
    }
    free(hidden);
    return 0;

}


FILE* FileHandle::getFilePointer()
{
    return fp;
}
void FileHandle::setFilePointer(FILE* filePointer)
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
