#include "globals.hpp"
#include "Storage.h"
#include <FS.h> // Filesystem support header
#include "SPIFFS.h"
#ifdef SDDAT3
#include "SD.h"
time_t sd_start_op;
#endif
namespace FreeTouchDeck
{
    using namespace fs;
    static const char *module = "Storage";
    FileSystem_t *ftdfs = NULL;

    FileSystem_t *FSInternal = new FileSystem_t(
        SPIFFS, "SPIFFS",
        []()
        {
            LOC_LOGV(module, "Starting SPIFFS");
            return SPIFFS.begin();
        },
        []()
        {
            LOC_LOGV(module, "SPIFFS, returning total bytes");
            return (size_t)SPIFFS.totalBytes();
        },
        []()
        {
            LOC_LOGV(module, "SPIFFS, returning Used bytes");
            return (size_t)SPIFFS.usedBytes();
        },
        []()
        { return "Internal"; },
        [](const char *path, const char *mode)
        {
            LOC_LOGV(module, "SPIFFS handler Opening file %s", path);
            return SPIFFS.open(path, mode);
        },
        [](const char *name)
        {
            LOC_LOGV(module, "SPIFFS checking if %s exists", name);
            return SPIFFS.exists(name);
        },
        [](const char *name)
        {
            LOC_LOGV(module, "SPIFFS handler removing file %s", name);
            return SPIFFS.remove(name);
        },
        [](const char *from, const char *to)
        {
            LOC_LOGV(module, "SPIFFS handler renaming %s to %s", from, to);
            return SPIFFS.rename(from, to);
        },
        [](const char *name)
        {
            LOC_LOGV(module, "SPIFFS handler creating directory %s", name);
            return SPIFFS.mkdir(name);
        },
        [](const char *name)
        {
            LOC_LOGV(module, "SPIFFS handler removing directory %s", name);
            return SPIFFS.rmdir(name);
        },
        [](const String &path, const char *mode)
        {
            LOC_LOGV(module, "Opening file %s", path.c_str());
            return SPIFFS.open(path, mode);
        },
        [](const String &name)
        {
            LOC_LOGV(module, "SPIFFS checking if %s exists", name.c_str());
            return SPIFFS.exists(name);
        },
        [](const String &name)
        {
            LOC_LOGV(module, "SPIFFS handler removing file %s", name.c_str());
            return SPIFFS.remove(name);
        },
        [](const String &from, const String &to)
        {
            LOC_LOGV(module, "SPIFFS handler renaming %s to %s", from.c_str(), to.c_str());
            return SPIFFS.rename(from, to);
        },
        [](const String &name)
        {
            LOC_LOGV(module, "SPIFFS handler creating directory %s", name.c_str());
            return SPIFFS.mkdir(name);
        },
        [](const String &name)
        {
            LOC_LOGV(module, "SPIFFS handler removing directory %s", name.c_str());
            return SPIFFS.rmdir(name);
        },
        []()
        {
            LOC_LOGV(module, "SPIFFS handler inserted true");
            return true;
        },
        false,
        []()
        {
            LOC_LOGV(module, "Stopping SPIFFS");
            SPIFFS.end();
            return true;
        });

    FileSystem_t *FileSystems[] = {
#ifdef SDDAT3
        new FileSystem_t(
            SD, "SD Card",
            []()
            {
                LOC_LOGV(module, "SD Card Handler: Begin");
                return SD.begin(SDDAT3);;
            },
            []()
            {
                LOC_LOGV(module, "SD Card Handler: totalBytes");
                return (size_t)SD.totalBytes();
            },
            []()
            {
                LOC_LOGV(module, "SD Card Handler: usedBytes");
                return (size_t)SD.usedBytes();
            },
            []()
            {
                LOC_LOGV(module, "SD Card Handler: cardType");
                switch ((size_t)SD.cardType())
                {
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
                LOC_LOGV(module, "SD Card Handler: Open");
                sd_start_op = millis();
                fs:File f = SD.open(path, mode);
                checkStatusHold();
                return f;
            },
            [](const char *name)
            {
                LOC_LOGV(module, "SD Card Handler: Exists");
                sd_start_op = millis();
                return  SD.exists(name) && checkStatusHold();
            },
            [](const char *name)
            {
                LOC_LOGV(module, "SD Card Handler: remove");
                sd_start_op = millis();
                return SD.remove(name) && checkStatusHold();
            },
            [](const char *from, const char *to)
            {
                LOC_LOGV(module, "SD Card Handler: rename");
                sd_start_op = millis();
                return SD.rename(from, to) && checkStatusHold();
            },
            [](const char *name)
            {
                LOC_LOGV(module, "SD Card Handler: mkdir");
                sd_start_op = millis();
                return SD.mkdir(name) && checkStatusHold();
            },
            [](const char *name)
            {
                LOC_LOGV(module, "SD Card Handler: rmdir");
                sd_start_op = millis();
                return SD.rmdir(name) && checkStatusHold();
            },
            [](const String &path, const char *mode)
            {
                LOC_LOGV(module, "SD Card Handler: String Open");
                sd_start_op = millis();
                fs:File f = SD.open(path, mode);
                checkStatusHold();
                return f;
            },
            [](const String &name)
            {
                LOC_LOGV(module, "SD Card Handler: string exists");
                sd_start_op = millis();
                return SD.exists(name) && checkStatusHold();
            },
            [](const String &name)
            {
                LOC_LOGV(module, "SD Card Handler: string remove");
                sd_start_op = millis();
                return SD.remove(name) && checkStatusHold();
            },
            [](const String &from, const String &to)
            {
                LOC_LOGV(module, "SD Card Handler: string rename");
                sd_start_op = millis();
                return SD.rename(from, to) && checkStatusHold();
            },
            [](const String &name)
            {
                LOC_LOGV(module, "SD Card Handler: string mkdir");
                sd_start_op = millis();
                return SD.mkdir(name) && checkStatusHold();
            },
            [](const String &name)
            {
                LOC_LOGV(module, "SD Card Handler: string rmdir");
                sd_start_op = millis();
                return SD.rmdir(name) && checkStatusHold();
            },
            []()
            {
                // If the media was ejected, some operations will take longer 
                // to execute
                return millis()-sd_start_op<600;
            },
            true,
            []()
            {
                LOC_LOGD(module, "Stopping SD");
                SD.end();
                return true;
            }),
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
                PrintMemInfo(__FUNCTION__, __LINE__);
                if (!currentFS->Begin())
                {
                    LOC_LOGI(module, "File system %s could not be initialized", (STRING_OR_DEFAULT(currentFS->Name, "")));
                }
                else
                {
                    LOC_LOGI(module, "File system %s Initialized. Storage type is %s", (STRING_OR_DEFAULT(currentFS->Name, "")), currentFS->StorageType());
                    currentFS->Initialized = true;
                    if (!ftdfs)
                    {
                        LOC_LOGI(module, "Using File system %s.", currentFS->Name);
                        ftdfs = currentFS;
                    }
                }
                PrintMemInfo(__FUNCTION__, __LINE__);
            }
        } while (true);
        if (!isStorageInitialized())
        {
            PrintScreenMessage(false, "Unable to initialize any storage after trying %d entries", c - 1);
            WaitTouchReboot();
        }
        return;
    }
    bool checkStatusHold()
    {
        while (!ftdfs->inserted())
        {
            PrintScreenMessage(true, "Please insert %s", ftdfs->Name);
            WaitTouchReboot();
        }
        return true;
    }
    bool checkErrorFileSystem(FileSystem_t *fileSystem)
    {
        if (!fileSystem || !fileSystem->Initialized)
        {
            PrintScreenMessage(false, "File system %s is not initialized", fileSystem ? STRING_OR_DEFAULT(fileSystem->Name, "unknown") : "error");
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
            PrintScreenMessage(false, "Unable to create directory %s", path);
        }
        return success;
    }
    size_t GetFileSize(const char *filename, FileSystem_t *toSystem)
    {
        size_t fileSize = 0;
        fs::File target = toSystem->open(filename, FILE_READ);
        if (target)
        {
            fileSize = target.size();
            LOC_LOGD(module, "File %s on %s is %d bytes. Closing file", filename, toSystem->Name, target.size());
            target.close();
        }
        else
        {
            LOC_LOGE(module, "Error opening file %s on %s", filename, toSystem->Name);
        }
        return fileSize;
    }

    bool checkExists(const char *filename, FileSystem_t *toSystem)
    {
        if (!checkErrorFileSystem(toSystem))
        {
            return false;
        }
        LOC_LOGD(module, "Checking if file %s exists on file system %s", STRING_OR_DEFAULT(filename, "n/a"), STRING_OR_DEFAULT(toSystem->Name, "n/a"));
        return toSystem->exists(filename);
    }
    bool checkExists(const char *filename)
    {
        LOC_LOGD(module, "Checking if file %s exists", STRING_OR_DEFAULT(filename, "n/a"));
        return checkExists(filename, ftdfs);
    }
    bool CopyFile(fs::File *source, FileSystem_t *toSystem)
    {
        return CopyFile(source, source->name(), toSystem);
    }
    bool CopyFile(fs::File *source, const char *targetName, FileSystem_t *toSystem)
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
                success = true;
                LOC_LOGE(module, "File %s is empty", source->name());
                PrintScreenMessage(false, "File %s is empty. Ignoring", source->name());
                return success;
            }
            source->seek(0);
        }
        if (success)
        {
            PrintScreenMessage(false, "Copying %s (%d bytes) to %s on %s ", STRING_OR_DEFAULT(source->name(), "n/a"), source->size(), targetName, STRING_OR_DEFAULT(toSystem->Name, "n/a"));
        }
        if (success && checkExists(targetName))
        {
            size_t targetSize = GetFileSize(targetName, toSystem);
            if (targetSize == 0)
            {
                PrintScreenMessage(false, "Removing empty file on %s", toSystem->Name);
                toSystem->remove(targetName);
            }
            else
            {
                PrintScreenMessage(false, "File %s exists on %s", targetName, toSystem->Name);
                return true;
            }
        }
        if (success)
        {
            File target = toSystem->open(targetName, FILE_WRITE);
            if (target)
            {
                char buffer[501] = {0};
                size_t remaining = source->size();
                size_t readBytes = 0;
                while (remaining > 0 && success)
                {
                    readBytes = source->readBytes(buffer, sizeof(buffer));
                    if (readBytes == 0)
                    {
                        PrintScreenMessage(false, "Could not read from %s! (size=%d bytes), buffer size is (%d), file position is %d", source->name(), source->size(), sizeof(buffer), source->position());
                        success = false;
                        break;
                    }
                    else
                    {
                        remaining -= readBytes;
                        if (target.write((uint8_t *)buffer, readBytes) != readBytes)
                        {
                            PrintScreenMessage(false, "File copy error. Could not write %d bytes to %s on %s", readBytes, target.name(), STRING_OR_DEFAULT(toSystem->Name, "N/A"));
                            success = false;
                        }
                        LOC_LOGD(module, "%d/%d bytes copied", source->size() - remaining, source->size());
                    }
                }
                target.flush();
                target.close();
            }
            else
            {
                PrintScreenMessage(false, "Opening %s failed on target %s", targetName, toSystem->Name);
                return false;
            }
        }
        if (!success)
        {
            PrintScreenMessage(false, "Unable to copy file %s to %s on %s", STRING_OR_DEFAULT(source->name(), "N/A"), targetName, toSystem ? STRING_OR_DEFAULT(toSystem->Name, "") : "N/A");
        }
        return success;
    }
    bool CopyFile(const char *source, FileSystem_t *fromSystem, FileSystem_t *toSystem)
    {
        return CopyFile(source, source, fromSystem, toSystem);
    }
    bool CopyFile(const char *source, const char *target, FileSystem_t *fromSystem, FileSystem_t *toSystem)
    {
        bool success = false;
        fs::File sourceFile = fromSystem->open(source, FILE_READ);
        if (!sourceFile || sourceFile.size() == 0 || sourceFile.isDirectory())
        {
            if (sourceFile)
            {
                sourceFile.close();
            }
            LOC_LOGE(module, "Unable to open source file, or source file is empty");
            return false;
        }
        success = CopyFile(&sourceFile, target, toSystem);
        sourceFile.close();
        return success;
    }
    bool CopyDirectory(const char *source, FileSystem_t *toSystem)
    {
        bool result = true;
        if (ISNULLSTRING(source))
        {
            PrintScreenMessage(false, "No source file passed");
            return false;
        }
        if (!checkErrorFileSystem(toSystem))
        {
            PrintScreenMessage(false, "Unable to copy directory %s. Target file system error.", STRING_OR_DEFAULT(source, "n/a"));
            return false;
        }
        if (!checkErrorFileSystem(FSInternal))
        {
            PrintScreenMessage(false, "Unable to copy directory %s. Internal SPIFFS not initialized.", STRING_OR_DEFAULT(source, "n/a"));
            return false;
        }

        File root = FSInternal->open("/", FILE_READ);
        if (!root)
        {
            PrintScreenMessage(false, "Error opening root folter ");
            return false;
        }
        PrintScreenMessage(false, "Copying path %s to %s", STRING_OR_DEFAULT(source, "n/a"), STRING_OR_DEFAULT(toSystem->Name, "n/a"));
        File file = root.openNextFile();
        while (file)
        {
            if (strncmp(file.name(), source, strlen(source)) == 0)
            {
                LOC_LOGI(module, "Found file %s, size %d", file.name(), file.size());
                if (!CopyFile(&file, toSystem))
                {
                    PrintScreenMessage(false, "Stopping copy due to error.");
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
        if (!FSInternal || !FSInternal->Initialized)
        {
            drawErrorMessage(true, module, "Internal storage is not initialized. Did you upload the data folder correctly?");
        }
        else
        {
            LOC_LOGD(module, "Showing directory content");
            ShowDir();
            PrintScreenMessage(true, "Initializing %s for first use. Creating directory /config", ftdfs->Name);
            PrintScreenMessage(false, "Creating directory /config");
            if (!CreateDirectory("/config", ftdfs))
            {
                return false;
            }

            PrintScreenMessage(false, "Creating directory /logos");
            if (!CreateDirectory("/logos", ftdfs))
            {
                return false;
            }
            PrintScreenMessage(false, "Copying internal storage to %s", ftdfs->Name);
            if (!CopyDirectory("/", ftdfs))
            {
                PrintScreenMessage(false, "Error initializing storage.");
                WaitTouchReboot();
            }
        }
        return true;
    }

    void ShowDir(FileSystem_t *targetFS)
    {
        if (!checkErrorFileSystem(targetFS))
            return;
        File root = targetFS->open("/", FILE_READ);
        File file = root.openNextFile();
        LOC_LOGI(module, "Name\tSize");
        while (file)
        {
            LOC_LOGI(module, "%s\t%d", file.name(), file.size());
            file = root.openNextFile();
        }
        root.close();
        LOC_LOGI(module, "Total Size %0.3fMB", (float)targetFS->totalBytes() / 1024.0f);
        LOC_LOGI(module, "Used space %0.3fMB", (float)targetFS->usedBytes() / 1024.0f);
        LOC_LOGI(module, "Remaining space %0.3fMB", ((float)targetFS->totalBytes() - (float)targetFS->usedBytes()) / 1024.0f);
    }
    void ShowDir()
    {
        ShowDir(ftdfs);
    }
    bool ShowFileContent(const char * fileName, FileSystem_t * fileSystem)
    {
        bool success = false;
        char buffer[55]={0};
        fs::File sourceFile = fileSystem->open(fileName, FILE_READ);
        if (!sourceFile || sourceFile.size() == 0 || sourceFile.isDirectory())
        {
            if (sourceFile)
            {
                sourceFile.close();
            }
            LOC_LOGE(module, "Unable to open source file, or source file is empty");
            return false;
        }
        size_t readBytes=0;
        do
        {
            readBytes=sourceFile.readBytes(buffer, sizeof(buffer)-1);
            buffer[readBytes]='\0';
            Serial.print(buffer);
        }while(readBytes>0);
        sourceFile.close();
        Serial.println();
        return success;        
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
        StringOperationCallback_t _str_rmdir, CheckStatusCallback_t _inserted, bool _external, BooleanCallback_t _end) : fileSystem(_fileSystem)
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
        end = _end;
    }
};
