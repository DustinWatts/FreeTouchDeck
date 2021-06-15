#include "globals.hpp"
#include "Storage.h"
#include <FS.h> // Filesystem support header
#include "SPIFFS.h"
#ifdef SDDAT3
#include "SD.h"
#endif
namespace FreeTouchDeck
{
    using namespace fs;
    static const char *module = "Storage";
    FileSystem_t *ftdfs = NULL;

    FileSystem_t *FSInternal = new FileSystem_t(
        SPIFFS, "SPIFFS",
        []()
        { return SPIFFS.begin(); },
        []()
        { return (size_t)SPIFFS.totalBytes(); },
        []()
        { return (size_t)SPIFFS.usedBytes(); },
        []()
        { return "Internal"; },
        [](const char *path, const char *mode)
        { return SPIFFS.open(path, mode); },
        [](const char *name)
        { return SPIFFS.exists(name); },
        [](const char *name)
        { return SPIFFS.remove(name); },
        [](const char *from, const char *to)
        { return SPIFFS.rename(from, to); },
        [](const char *name)
        { return SPIFFS.mkdir(name); },
        [](const char *name)
        { return SPIFFS.rmdir(name); },
        [](const String &path, const char *mode)
        { return SPIFFS.open(path, mode); },
        [](const String &name)
        { return SPIFFS.exists(name); },
        [](const String &name)
        { return SPIFFS.remove(name); },
        [](const String &from, const String &to)
        { return SPIFFS.rename(from, to); },
        [](const String &name)
        { return SPIFFS.mkdir(name); },
        [](const String &name)
        { return SPIFFS.rmdir(name); },
        [](){ return true;},
        false
        );

    FileSystem_t *FileSystems[] = {
#ifdef SDDAT3
        new FileSystem_t(
            SD, "SD Card",
            [](){ return SD.begin(SDDAT3); },
            [](){ return (size_t)SD.totalBytes(); },
            [](){ return (size_t)SD.usedBytes(); },
            [](){switch ((size_t)SD.cardType()){
                    ENUM_TO_STRING_HELPER_SIMPLE(CARD_NONE);
                    ENUM_TO_STRING_HELPER_SIMPLE(CARD_MMC);
                    ENUM_TO_STRING_HELPER_SIMPLE(CARD_SD);
                    ENUM_TO_STRING_HELPER_SIMPLE(CARD_SDHC);
                    ENUM_TO_STRING_HELPER_SIMPLE(CARD_UNKNOWN);
                default:
                    return "Error";
                    break;
                }
            },
            [](const char *path, const char *mode)
            { 
                checkStatusHold();
                return SD.open(path, mode); 
            },
            [](const char *name)
            { return checkStatusHold() && SD.exists(name); },
            [](const char *name)
            { return checkStatusHold() && SD.remove(name); },
            [](const char *from, const char *to)
            { return checkStatusHold() && SD.rename(from, to); },
            [](const char *name)
            { return checkStatusHold() && SD.mkdir(name); },
            [](const char *name)
            { return checkStatusHold() && SD.rmdir(name); },
            [](const String &path, const char *mode)
            { 
                checkStatusHold();
                return  SD.open(path, mode); 
                },
            [](const String &name)
            { return checkStatusHold() && SD.exists(name); },
            [](const String &name)
            { return checkStatusHold() && SD.remove(name); },
            [](const String &from, const String &to)
            { return checkStatusHold() && SD.rename(from, to); },
            [](const String &name)
            { return checkStatusHold() && SD.mkdir(name); },
            [](const String &name)
            { return checkStatusHold() && SD.rmdir(name); },
            [](){ 
                File f = SD.open("/config/menus.json", FILE_READ);
                if(f && f.size()>0)
                {
                    f.close();
                    return true;
                }
                return false;},
                true),
#endif
        FSInternal,
        NULL};
    void InitFileSystem()
    {
        int c = 0;
        do
        {
            FileSystem_t *currentFS = FileSystems[c++];
            if (!currentFS)
            {
                LOC_LOGI(module, "Reached the end of the file systems list");
                break;
            }
            else
            {
                LOC_LOGI(module, "Initializing storage %s", STRING_OR_DEFAULT(currentFS->Name, "N/A"));
                if (!currentFS->Begin())
                {
                    LOC_LOGI(module, "File system %s could not be initialized", (STRING_OR_DEFAULT(currentFS->Name, "")));
                }
                else
                {
                    currentFS->Initialized = true;
                    if (!ftdfs)
                    {
                        LOC_LOGI(module, "Using File system %s.", currentFS->Name);
                        ftdfs = currentFS;
                    }
                }
            }
        } while (true);
        if (!isStorageInitialized())
        {
            LOC_LOGE(module, "Unable to initialize storage after trying %d entries", c - 1);
            drawErrorMessage(true, module, "Unable to initialize storage");
            while (1)
                yield();
        }
        return;
    }
    bool checkStatusHold()
    {
        while(!ftdfs->inserted())
        {
            drawErrorMessage(true,module,"Please insert %s and press to continue");
        }

        return true;
    }
    bool checkErrorFileSystem(FileSystem_t *fileSystem)
    {
        if (!fileSystem || !fileSystem->Initialized)
        {
            LOC_LOGE(module, "File system %s is not initialized", fileSystem ? STRING_OR_DEFAULT(fileSystem->Name, "unknown") : "error");
            return false;
        }
        return true;
    }
    bool CreateDirectory(const char *path, FileSystem_t *fileSystem)
    {
        bool success = false;
        success = checkErrorFileSystem(fileSystem);

        if (success && !fileSystem->exists(path))
        {
            LOC_LOGI(module, "Creating directory %s", path);
            success = fileSystem->mkdir(path);
        }
        if (!success)
        {
            LOC_LOGE(module, "Unable to create directory %s", path);
        }
        return success;
    }
    bool checkExists(const char *filename, FileSystem_t *toSystem)
    {
        if (!checkErrorFileSystem(toSystem))
        {
            return false;
        }
        LOC_LOGD(module, "Checking if file %s exists on file system %s", STRING_OR_DEFAULT(filename, "n/a"), STRING_OR_DEFAULT(toSystem->Name, "n/a"));
        return checkErrorFileSystem(toSystem) && toSystem->exists(filename);
    }
    bool checkExists(const char *filename)
    {
        LOC_LOGD(module, "Checking if file %s exists", STRING_OR_DEFAULT(filename, "n/a"));
        return checkExists(filename, ftdfs);
    }
    bool CopyFile(File *source, FileSystem_t *toSystem)
    {
        bool success = true;
        success = checkErrorFileSystem(toSystem);
        if (success)
        {
            if (!source)
            {
                success = false;
                LOC_LOGE(module, "No source file passed");
            }
            else if (source->size() == 0)
            {
                success = false;
                LOC_LOGE(module, "File %s is empty", source->name());
            }
        }
        if (success && checkExists(source->name()))
        {
            LOC_LOGW(module, "File %s already exists on %s", source->name(), toSystem->Name);
            return true;
        }
        if (success)
        {
            File target = toSystem->open(source->name(), FILE_WRITE);
            if (target)
            {
                char buffer[501];
                size_t remaining = source->size();
                size_t readBytes = 0;
                while (remaining > 0 && success)
                {
                    readBytes = source->readBytes(buffer, sizeof(buffer));
                    if (readBytes == 0)
                    {
                        LOC_LOGE(module, "Could not read from source");
                    }
                    else
                    {
                        remaining -= readBytes;
                        if (target.write((uint8_t *)buffer, readBytes) != readBytes)
                        {
                            LOC_LOGE(module, "File copy error. Could not write %d bytes to %s on %s", readBytes, target.name(), STRING_OR_DEFAULT(toSystem->Name, "N/A"));
                            success = false;
                        }
                        else
                        {
                            target.flush();
                        }
                    }
                }
                target.close();
            }
            else
            {
                LOC_LOGE(module, "Open failed");
                return false;
            }
        }
        if (!success)
        {
            LOC_LOGE(module, "Unable to copy file %s to %s", STRING_OR_DEFAULT(source->name(), "N/A"), toSystem ? STRING_OR_DEFAULT(toSystem->Name, "") : "N/A");
        }
        return success;
    }
    bool CopyDirectory(const char *source, FileSystem_t *toSystem)
    {
        bool result = true;
        if (ISNULLSTRING(source))
        {
            LOC_LOGE(module, "No source file passed");
            return false;
        }
        if (!checkErrorFileSystem(toSystem))
        {
            LOC_LOGE(module, "Unable to copy directory %s. Target file system error.", STRING_OR_DEFAULT(source, "n/a"));
            return false;
        }
        if (!checkErrorFileSystem(FSInternal))
        {
            LOC_LOGE(module, "Unable to copy directory %s. Internal SPIFFS not initialized.", STRING_OR_DEFAULT(source, "n/a"));
            return false;
        }

        File root = FSInternal->open("/", FILE_READ);
        if (!root)
        {
            LOC_LOGE(module, "Error opening root folter ");
            return false;
        }
        LOC_LOGI(module, "Copying path %s to %s", STRING_OR_DEFAULT(source, "n/a"), STRING_OR_DEFAULT(toSystem->Name, "n/a"));
        File file = root.openNextFile();
        while (file)
        {
            LOC_LOGI(module, "Found file %s", file.name());
            if (strncmp(file.name(), source, strlen(source)) == 0)
            {
                LOC_LOGI(module, "Copying %s (%d bytes) to %s ", STRING_OR_DEFAULT(source,"n/a"),file.size(),STRING_OR_DEFAULT(toSystem->Name,"n/a"));
                if (!CopyFile(&file, toSystem))
                {
                    LOC_LOGE(module, "Stopping copy due to error.");
                    result = false;
                    break;
                }
            }
            else
            {
                LOC_LOGI(module, "Ignoring file %s", file.name());
            }
            file = root.openNextFile();
        }
        if (file)
        {
            file.close();
        }
        if (root)
        {
            root.close();
        }
        return result;
    }

    bool InitializeStorage()
    {
        LOC_LOGW(module, "Initializing storage for first use");
        if (!FSInternal || !FSInternal->Initialized)
        {
            drawErrorMessage(true, module, "Internal storage is not initialized. Did you upload the data folder correctly?");
        }
        else
        {
            if (!CreateDirectory("/config", ftdfs))
            {
                return false;
            }
            if (!CreateDirectory("/logos", ftdfs))
            {
                return false;
            }
            if (!CopyDirectory("/", ftdfs))
            {
                drawErrorMessage(true, module, "Error initializing storage.");
            }
        }
        return true;
    }

    void ShowDir()
    {
        if (!checkErrorFileSystem(ftdfs))
            return;
        File root = ftdfs->open("/", FILE_READ);
        File file = root.openNextFile();
        LOC_LOGI(module, "Name\tSize");
        while (file)
        {
            LOC_LOGI(module, "%s\t%d", file.name(), file.size());
            file = root.openNextFile();
        }
        root.close();
        LOC_LOGI(module,"Total Size %0.3fMB", (float)ftdfs->totalBytes()/1024.0f);
        LOC_LOGI(module,"Used space %0.3fMB", (float)ftdfs->usedBytes()/1024.0f);
        LOC_LOGI(module,"Remaining space %0.3fMB", ((float)ftdfs->totalBytes()-(float)ftdfs->usedBytes())/1024.0f);
    }
    
    FileSystem_t::FileSystem_t(
        fs::FS &_fileSystem,
        const char *_Name,
        BeginCallback_t _Begin,
        SizeCallback_t _totalBytes,
        SizeCallback_t _usedBytes,
        StorageType_t _StorageType,
        OpenCallback_t _open,
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
        StringOperationCallback_t _str_rmdir, CheckStatusCallback_t _inserted,bool _external) : fileSystem(_fileSystem)
    {
        totalBytes = _totalBytes;
        usedBytes = _usedBytes;
        Begin = _Begin;
        Name = _Name;
        StorageType = _StorageType;
        Initialized = false;
        open = _open;
        exists = _exists;
        remove = _remove;
        rename = _rename;
        mkdir = _mkdir;
        rmdir = _rmdir;
        stopen = _str_open;
        stexists = _str_exists;
        stremove = _str_remove;
        strename = _str_rename;
        stmkdir = _str_mkdir;
        strmdir = _str_rmdir;
        inserted = _inserted;
        External = _external;
    }
};
