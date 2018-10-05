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
    return -1;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return -1;
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
