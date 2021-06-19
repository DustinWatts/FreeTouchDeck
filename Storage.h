#pragma once
#include "globals.hpp"
#include "UserConfig.h"
#include "FS.h"

namespace FreeTouchDeck
{
  class FileSystem_t;
  typedef size_t (*SizeCallback_t)();
  typedef bool (*BeginCallback_t)();
  typedef bool (*BooleanCallback_t)();
  typedef bool (*CheckStatusCallback_t)();
  typedef const char * (*StorageType_t)();
  typedef fs::File (*OpenCallback_t)(const char *path, const char *mode);

  typedef bool (*OperationCallback_t)(const char *path);
  typedef bool (*FromToOperation_cb)(const char *pathFrom, const char *pathTo);
  typedef bool (*OpenStringCallback_t)(const String &path, const char *mode);

  typedef fs::File (*StringOpenCallback_t)(const String &path, const char *mode);
  typedef bool (*StringOperationCallback_t)(const String &path);
  typedef bool (*StringFromToOprationCallback_t)(const String &pathFrom, const String &pathTo);

  class FileSystem_t
  {
  public:
    SizeCallback_t totalBytes = NULL;
    SizeCallback_t usedBytes = NULL;
    fs::FS &fileSystem;
    const char *Name;
    StorageType_t StorageType;
    BeginCallback_t Begin = NULL;
    bool Initialized = false;
    OpenCallback_t open;
    BooleanCallback_t end;
    OperationCallback_t exists;
    OperationCallback_t remove;
    FromToOperation_cb rename;
    OperationCallback_t mkdir;
    OperationCallback_t rmdir;
    StringOpenCallback_t stopen;
    StringOperationCallback_t stexists;
    StringOperationCallback_t stremove;
    StringFromToOprationCallback_t strename;
    StringOperationCallback_t stmkdir;
    StringOperationCallback_t strmdir;
    CheckStatusCallback_t inserted;
    
    bool External;
   FileSystem_t(fs::FS&_fileSystem, const char * _Name, BeginCallback_t _Begin, SizeCallback_t _totalBytes, SizeCallback_t _usedBytes, StorageType_t _StorageType, OpenCallback_t _open,
                 OperationCallback_t _exists,
                 OperationCallback_t _remove,
                 FromToOperation_cb _rename,
                 OperationCallback_t _mkdir,
                 OperationCallback_t _rmdir,
                 StringOpenCallback_t _str_open,
                 StringOperationCallback_t _str_exists,
                 StringOperationCallback_t _str_remove,
                 StringFromToOprationCallback_t _str_rename,
                 StringOperationCallback_t _str_mkdir,
                 StringOperationCallback_t _str_rmdir, CheckStatusCallback_t _inserted, bool _external, BooleanCallback_t _end );
  };
  extern FileSystem_t *ftdfs;
  inline bool isStorageInitialized() { return ftdfs && ftdfs->Initialized; }
  void InitFileSystem();
  bool InitializeStorage();
  bool checkStatusHold();
  /**
* @brief This function checks if a file exists and returns a boolean accordingly.
         It then prints a debug message to the serial as wel as the tft.
*
* @param filename (const char *)
*
* @return boolean True if succeeded. False otherwise.
*
* @note Pass the filename including a leading /
*/
  bool checkfile(const char *filename);
  void ShowDir();
  void ShowDir(FileSystem_t * targetFS);
  size_t GetFileSize(const char * fileName, FileSystem_t * fileSystem);
};
