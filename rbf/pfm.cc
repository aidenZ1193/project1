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
    if(fileExists(fileName) != 0)
        return -1;

    FILE* filePointer = fopen(fileName.c_str(), "wb");
    if(filePointer = NULL){
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


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    return -1;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    return -1;
}

// @return: 0 when file exists
int fileExists(const string &filename)
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
    if(pageNum < 0 || pageNum > num)
        return -1;

    if(fseek(fp, PAGE_SIZE * pageNum, SEEK_SET) != 0)
        return -1;

    // clear whatever was in the data
    memset(data, 0, PAGE_SIZE);
    if(fread(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
        return -1;

    readPageCounter ++;

    cout<<"reading successed"<<endl;

    return 0;
}

// check if file is valid
// check if pageNUm is vald
// check if write is successful
RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    if(fp == NULL)
        return -1;
    
    if(pageNum > getNumberOfPages() || pageNum < 0)
        return -1;

    if(fseek(fp, PAGE_SIZE*pageNum, SEEK_SET) != 0)
        return -1;

    if(fwrite(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
        return -1;

    writePageCounter ++;

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

    if(fseek(fp, 0, SEEK_END) != 0)
        return -1;
    
    if(fwrite(data, 1, PAGE_SIZE, fp) != PAGE_SIZE)
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

    return ftell(fp)/PAGE_SIZE;

}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    readPageCount = readPageCounter;
    writePageCount = writePageCounter;
    appendPageCount = appendPageCounter;
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
