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
    if(fileHandle.getFilePointer() != NULL || fileHandle.getFileName() != "")
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

// 1. check if findSlot finds a spot, or need to append a new page
// 2. if yes, memcpy data to page, and update f, n, slot data
// 3. if no, append a page and set it up with file dir at the end of page, and mamcpy data, update necessaries
RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    
    // append a new page
    if(findSlot(fileHandle,data, rid, recordDescriptor) == -1){
        
    }

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
    int re = -1;     // if ends up with no space in current file, need to append a new page. use -1 to represent that

    // get the actual size that the record needs
    int size = getDataSize(data, recordDescriptor);

    // go to current page, get free space addr from file dir
    void *currentPage = FileHandle.currentPage;
    // get the free space offset and n = num of slots
    int f_offset = *(int*)((char*)currentPage + PAGE_SIZE - sizeof(int));
    int n = *(int*)((char*)currentPage + PAGE_SIZE - 2*sizeof(int));

    // if the page doesn't have a file dir yet, set it up
    if(f_offset < 0 || f_offset > PAGE_SIZE){
        f_offset = size;
        // put free space offset in the end of page
        memcpy((char*)currentPage+PAGE_SIZE-sizeof(int), &f_offset, sizeof(int));
        n = 0;
        memcpy((char*)currentPage+PAGE_SIZE-2*(sizeof(int)), &n, sizeof(int));
        // write to rid and return
        rid.pageNum = FileHandle.currentPageNum;
        rid.slotNum = n;
        re = 0;
        return re;
    }else{
        // see if the page has enough free space
        // get actual free space first
        int free_s = getFreeSpace(FileHandle);

        if(free_s >= size){
            // update the new f-offset, and see if any slot open
            f_offset += size;
            int offset = 0;
            int i = 0;
            for(i = 0; i < n; i++){
                // from dir's end, get offset of the slot
                memcpy(&offset, (char*)currentPage+PAGE_SIZE-(2+i)*sizeof(int), sizeof(int));
                // -1 means open slot
                if(offset == -1){
                    rid.pageNum = FileHandle.currentPageNum;
                    rid.slotNum = i;
                    //break;
                    re = 0;
                    return re;
                }
            }
            // if reached the last slot with a valid offset, means no previous open slot
            if(i == (n-1) && offset != -1){
                // make a new slot
                rid.slotNum = i+1;
                rid.pageNum = FileHandle.currentPageNum;
                re = 0;
                return re;
            }
            // go to page #0, starts to find a page with open slot
        }else{
            // if no enough free space in current page: go to page 0 and search for open slot
            int sizeOfFile = FileHandle.getNumberOfPages();
            void *page = malloc(PAGE_SIZE);
            for(int i = 0; i < sizeOfFile; i++){
                FileHandle.readPage(i, page);
                // see if the free space in page is enough
                // set the page i to current page, so getFreeSpace can function
                FileHandle.currentPage = page;
                FileHandle.currentPageNum = i;
                free_s = getFreeSpace(FileHandle);
                if(free_s >= size){
                    // repear of the upper if - should be reconstucted
                    int offset = 0;
                    int j = 0;
                    for(j = 0; j < n; j++){
                        // from dir's end, get offset of the slot
                        memcpy(&offset, (char*)currentPage+PAGE_SIZE-(2+j)*sizeof(int), sizeof(int));
                        // -1 means open slot
                        if(offset == -1){
                            rid.pageNum = FileHandle.currentPageNum;
                            rid.slotNum = j;
                            re = 0;
                            break;
                        }
                    }
                }
            }
            free(page);  
        }
    }
    return re;
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
            int len = *(int*)((char*)data+offset);
            dataSize += len;
            // offset should move behind the int that represents the varchar length, and the varchar itself
            offset += (sizeof(int) + len);
        }
    }

    return -1;

}

// @return the actual free space size in current page
// 1. get N - num of slots in the file dir
// 2. page_size - free_offset - size_of_file_dir
int RecordBasedFileManager::getFreeSpace(FileHandle &FileHandle){

    int n = 0;          // num of slots in the page
    memcpy(&n, (char*)FileHandle.currentPage + PAGE_SIZE - sizeof(int), sizeof(int));

    // calculate size of file dir
    int size = 2*(sizeof(int));
    // now add the size of the slot's offset&length
    size += n*2*sizeof(int);

    int f_offset = 0;
    memcpy(&f_offset, (char*)FileHandle.currentPage + PAGE_SIZE - sizeof(int), sizeof(int));

    return PAGE_SIZE - f_offset - size;
}
