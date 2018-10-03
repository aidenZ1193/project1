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


RC PagedFileManager::createFile(const string &fileName)
{
    return -1;
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    return -1;
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    return -1;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
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

    int num = getNumberOfPages();
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


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    return -1;
}


RC FileHandle::appendPage(const void *data)
{
    return -1;
}


unsigned FileHandle::getNumberOfPages()
{
    return -1;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    return -1;
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
