#include "XFormat.h"
#include <iostream> // DEBUG
using namespace std;

XFormat::XFormat(const string & disk) : Storage(disk), m_isFormatted(false)
{

}

void XFormat::format()
{
    if (!m_opened)
        open();

    // Write header
    uint8_t header[] = {
        0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
        0x0, 0x0, 0x0, 0x0,
        0x42, 0x42, 0x42, 0x42,
        XFORMAT_VERSION
    };

    lseek(m_fd, 0, SEEK_SET);
    if (write(m_fd, header, 17) == -1)
        throw logic_error(string("Cannot format: ") + strerror(errno));

    for (uint16_t i(17); i < XFORMAT_SECTOR_SIZE; ++i)
    {
        uint8_t zero(0x0);

        if (write(m_fd, &zero, 1) == -1)
            throw logic_error(string("Cannot format: ") + strerror(errno));
    }

    // Write FAT
    uint8_t fatentry[] = {
        0x0, 0x9,
        0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    };

    lseek(m_fd, XFORMAT_SECTOR_SIZE * 1, SEEK_SET);

    for (uint8_t i(0); i < XFORMAT_FAT_MAXENTRY; ++i)
    {
        uint16_t sector(9 + (i * 4));

        fatentry[0] = sector >> 8;
        fatentry[1] = sector;

        if (write(m_fd, fatentry, 64) == -1)
            throw logic_error(string("Cannot format: ") + strerror(errno));
    }
}

void XFormat::writeMusic(const MusicFile & music)
{
    uint16_t sector;
    vector<uint8_t> fat;

    if (!(m_opened | m_isFormatted))
        return;
    if (!music.getName().size())
        throw logic_error("You must enter name for music");
    if (getMusicCount() >= XFORMAT_FAT_MAXENTRY)
        throw logic_error("No enough space for new music");

    // Read FAT
    fat = getSectors(1, 8);
    for (uint8_t entry(0); entry < XFORMAT_FAT_MAXENTRY; ++entry)
    {
        uint16_t index(entry * 64);

        // Empty entry
        if (!((((uint16_t)fat[index + 2]) << 8) | fat[index + 3]))
        {
            uint8_t musicSize[2];

            musicSize[0] = music.getSize() >> 8;
            musicSize[1] = music.getSize();

            sector = (((uint16_t)fat[index]) << 8) | fat[index + 1];

            // Write to FAT
            lseek(m_fd, XFORMAT_SECTOR_SIZE + (((entry / 8) * XFORMAT_SECTOR_SIZE)
                + ((entry % 8) * 64)) + 2, SEEK_SET);
            if (write(m_fd, &musicSize, 2) == -1)
                throw logic_error(string("Unable to write: ") + strerror(errno));
            if (write(m_fd, music.getName().c_str(), music.getName().size() + 1) == -1)
                throw logic_error(string("Unable to write: ") + strerror(errno));

            // Write to Sector
            lseek(m_fd, sector * XFORMAT_SECTOR_SIZE, SEEK_SET);
            if (write(m_fd, music.getData(), music.getSize()) == -1)
                throw logic_error(string("Unable to write: ") + strerror(errno));

            return ;
        }
    }

    throw logic_error("Unable to write the music.");
}

void XFormat::renameMusic(uint16_t sector, const string & name)
{
    vector<uint8_t> fat;
    string subname;

    if (!(m_opened | m_isFormatted))
        return;

    subname = name.substr(0, MUSIC_MAX_NAME);
    
    // Read FAT
    fat = getSectors(1, 8);
    for (uint8_t entry(0); entry < XFORMAT_FAT_MAXENTRY; ++entry)
    {
        uint16_t index(entry * 64);

        if (((((uint16_t)fat[index]) << 8) | fat[index + 1]) == sector)
        {
            if (!((((uint16_t)fat[index + 2]) << 8) | fat[index + 3]))
                throw logic_error("Attempt to rename empty entry");

            lseek(m_fd, XFORMAT_SECTOR_SIZE + (((entry / 8) * XFORMAT_SECTOR_SIZE)
                + ((entry % 8) * 64)) + 4, SEEK_SET);

            if (write(m_fd, subname.c_str(), subname.size() + 1) == -1)
                throw logic_error(string("Unable to rename: ") + strerror(errno));
        }
    }
}

void XFormat::removeMusic(uint16_t sector)
{
    vector<uint8_t> fat;

    if (!(m_opened | m_isFormatted))
        return;

    // Read FAT
    fat = getSectors(1, 8);
    for (uint8_t entry(0); entry < XFORMAT_FAT_MAXENTRY; ++entry)
    {
        uint16_t index(entry * 64);

        if (((((uint16_t)fat[index]) << 8) | fat[index + 1]) == sector)
        {
            lseek(m_fd, XFORMAT_SECTOR_SIZE + (((entry / 8) * XFORMAT_SECTOR_SIZE)
                + ((entry % 8) * 64)) + 2, SEEK_SET);

            for (uint8_t i(0); i < 60; ++i)
            {
                uint8_t zero(0);

                if (write(m_fd, &zero, 1) == -1)
                    throw logic_error("Unable to remove music");
            }
        }
    }
}

uint8_t XFormat::getMusicCount()
{
    vector<uint8_t> fat;
    uint8_t count(0);

    if (!(m_opened | m_isFormatted))
        return (0);

    // Read FAT
    fat = getSectors(1, 8);
    for (uint8_t entry(0); entry < XFORMAT_FAT_MAXENTRY; ++entry)
    {
        uint16_t index(entry * 64);

        if ((((uint16_t)fat[index + 2]) << 8) | fat[index + 3])
            ++count;
    }

    return (count);
}

vector<MusicFAT> XFormat::getListMusic()
{
    vector<uint8_t> fat;
    vector<MusicFAT> musics;

    if (!(m_opened | m_isFormatted))
        return (musics);

    // Read FAT
    fat = getSectors(1, 8);
    for (uint8_t entry(0); entry < XFORMAT_FAT_MAXENTRY; ++entry)
    {
        uint16_t index(entry * 64);

        // has music
        if ((((uint16_t)fat[index + 2]) << 8) | fat[index + 3])
        {
            uint16_t sector;
            uint16_t size;
            string name;

            sector = (((uint16_t)fat[index]) << 8) | fat[index + 1];
            size = (((uint16_t)fat[index + 2]) << 8) | fat[index + 3];
            for (uint8_t i(0); i < 60; ++i)
            {
                if (!fat[index + 4 + i])
                    break;
                name += fat[index + 4 + i];
            }

            musics.push_back(MusicFAT(name, size, sector));
        }
    }

    return (musics);
}

bool XFormat::isFormatted()
{
    vector<uint8_t> data;

    if (!m_opened)
        return (false);

    if (m_isFormatted)
        return (true);

    data = getSectors(0);

    // Vérification du header
    for (int i(0); i < 8; ++i)
    {
        if (data[i] != 0x42)
            return (false);
    }
    for (int i(0); i < 4; ++i)
    {
        if (data[i + 8] != 0x0)
            return (false);
    }
    for (int i(0); i < 4; ++i)
    {
        if (data[i + 12] != 0x42)
            return (false);
    }

    // Vérification de la version
    if (data[16] != XFORMAT_VERSION)
        return (false);

    return (true);
}

uint8_t XFormat::getVersion()
{
    vector<uint8_t> data;

    if (!(m_opened || m_isFormatted))
        return (0);

    data = getSectors(0);

    return (data[16]);
}

vector<uint8_t> XFormat::getSectors(uint16_t sector, int length)
{
    uint8_t buffer[XFORMAT_SECTOR_SIZE];
    vector<uint8_t> data;

    lseek(m_fd, sector * XFORMAT_SECTOR_SIZE, SEEK_SET);

    for (int i(0); i < length; ++i)
    {
        if (read(m_fd, buffer, XFORMAT_SECTOR_SIZE) == -1)
            throw logic_error("Unable to read sector");
        for (ssize_t u(0); u < XFORMAT_SECTOR_SIZE; ++u)
        {
            data.push_back(buffer[u]);
        }
    }
    return (data);
}
