#include <String>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/disk.h>
#include <sys/ioctl.h>

class Storage
{
protected:
    std::string m_disk;
    bool m_opened;
    int m_fd;
    unsigned long m_diskSize;

public:
    Storage(std::string disk);
    ~Storage();

    void open();
    void close();

   int getFd();

    unsigned long getDiskSize();

};
