#ifndef MUSICFAT_H
#define MUSICFAT_H

#include <String>
#include <sys/types.h>

class MusicFAT
{
protected:
    std::string m_name;
    uint16_t m_size;
    uint16_t m_sector;

public:
    MusicFAT(std::string name, uint16_t size, uint16_t sector);

    std::string getName() const;
    uint16_t getSize() const;
    uint16_t getSector() const;
};

#endif
