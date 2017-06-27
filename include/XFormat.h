#ifndef XENFORMAT_H
#define XENFORMAT_H

#include <vector>
#include <stdint.h>
#include "Storage.h"
#include "MusicFAT.h"
#include "MusicFile.h"

#define XFORMAT_VERSION 1
#define XFORMAT_SECTOR_SIZE 512
#define XFORMAT_FAT_MAXENTRY 64

class XFormat : public Storage
{
private:
    bool m_isFormatted;

public:
    XFormat(const std::string & disk);

    void format();

    bool isFormatted();
    uint8_t getVersion();
    uint8_t getMusicCount();
    std::vector<MusicFAT> getListMusic();
    
    void writeMusic(const MusicFile & music);
    void removeMusic(uint16_t sector);
    void renameMusic(uint16_t sector, const std::string & name);

private:
    std::vector<uint8_t> getSectors(uint16_t sector, int length = 1);
};

#endif
