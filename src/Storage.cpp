#include "Storage.h"

using namespace std;

Storage::Storage(string disk) : m_disk(disk), m_opened(false), m_diskSize(0)
{

}

Storage::~Storage()
{
    if (m_opened)
        close();
}

void Storage::open()
{
    if (m_opened)
        Storage::close();
    if ((m_fd = ::open(m_disk.c_str(), O_RDWR)) == -1)
        throw logic_error("Cannot open " + m_disk + ": " + strerror(errno));
    m_opened = true;
}

void Storage::close()
{
    if (m_opened)
        ::close(m_fd);
}

unsigned long Storage::getDiskSize()
{
    unsigned long blockCount(0);
    unsigned long blockSize(0);
    if (m_opened)
    {
        ioctl(m_fd, DKIOCGETBLOCKCOUNT, &blockCount);
        ioctl(m_fd, DKIOCGETBLOCKSIZE, &blockSize);

        return (blockCount * blockSize);
    }
    return (0);
}

int Storage::getFd()
{
    if (m_opened)
        return (m_fd);
    return (-1);
}
