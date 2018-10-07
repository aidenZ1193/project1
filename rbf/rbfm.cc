#include "rbfm.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
    _pf_manager = PagedFileManager::instance();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
    _pf_manager = NULL;
    //_rbf_manager = NULL;
}

RC RecordBasedFileManager::createFile(const string &fileName) {

    // check if file exists already
    //if(_pf_manager->fileExists(fileName.c_str()) == 0)
    //    return -1;

    if(_pf_manager->createFile(fileName) != 0)
        return -1;

    return 0;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {

    if(_pf_manager->destroyFile(fileName) != 0)
        return -1;

    return 0;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {

    // if fileHandle is valid:
    if(fileHandle.getFilePinter() != NULL || fileHandle.getFileName() != "")
        return -1;

    if(_pf_manager->openFile(fileName, fileHandle) != 0)
        return -1;

    return 0;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {

    if(_pf_manager->closeFile(fileHandle) != 0)
        return -1;
        
    return 0;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    
    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}

// 1. get data size
// 2. get free space size in current page
// 3. if enough space, write rid and return slot #
// 4. else, return -1
RC RecordBasedFileManager::findSlot(FileHandle &FileHandle, const void *data, RID &rid, const vector<Attribute> &recordDescriptor){

    if(data == NULL){
        return -1;
    }
    // get the actual size that the record needs
    int size = getDataSize(data, recordDescriptor);

    // go to current page, get free space addr from file dir
    void *currentPage = FileHandle.currentPage;
    // get the free space offset and n = num of slots
    int f_offset = *(int*)(currentPage + PAGE_SIZE - sizeof(int));
    int n = *(int*)(currentPage + PAGE_SIZE - 2*sizeof(int));

    // if the page doesn't have a file dir yet
    if(f_offset < 0 || f_offset > PAGE_SIZE){
        f_offset = size;
        // put free space offset in the end of page
        memcpy(currentPage+PAGE_SIZE-sizeof(int), &f_offset, sizeof(int));
        n = 0;
        memcpy(currentPage+PAGE_SIZE-2*(sizeof(int)), &n, sizeof(int));
        // write to rid and return
        rid.pageNum = FileHandle.currentPageNum;
        rid.slotNum = n;
    }else{
        // see if the page has enough free space
        // get actual free space first
        int free = getFreeSpace(currentPage);
        if(free >= size){
            // update the new f-offset, and see if any slot open
            f_offset += size;
            for(int i = 0; i < n; i++){

            }
        }
    }
    
}

// @return: the actual size that the record needs to be inserted in the page
RC RecordBasedFileManager::getDataSize(const void *data, const vector<Attribute> &recordDescriptor){

    int s = recordDescriptor.size();

    // offset = offset in data when moving the pointer and getting attr length
    int offset = ceil(s/CHAR_BIT);

    // number of attributes: s
    int dataSize  = s;
    // s fields that stores the actul size of attribute data
    dataSize += s*sizeof(int);

    // go through the type of recordDescriptor to get size of fields in data
    for(int i = 0; i < s; i++){
        // int field should have fixed length
        if(recordDescriptor[i].type == TypeInt){
            // sizeof(int) integer itself
            dataSize += sizeof(int);
            offset += sizeof(int);
        }else if(recordDescriptor[i].type == TypeReal){
            dataSize += sizeof(float);
            offset += sizeof(float);
        }else if(recordDescriptor[i].type == TypeVarChar){
            // use offset to get the length of varchar that stores in the data
            int len = *(int*)(data+offset);
            dataSize += len;
            // offset should move behind the int that represents the varchar length, and the varchar itself
            offset += (sizeof(int) + len);
        }
    }

    return -1;

}
