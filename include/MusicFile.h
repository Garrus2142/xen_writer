#ifndef MUSICFILE_H
#define MUSICFILE_H

#include <string>
#include <fstream>
#include <sys/types.h>
#include <stdexcept>
#include <errno.h>

#define MUSIC_MAX_SIZE 2048
#define MUSIC_MAX_NAME 59

class MusicFile
{
private:
    std::string m_name;
    std::string m_file;
    std::ifstream m_stream;
    long m_size;
    uint8_t *m_data;

public:
    MusicFile(std::string file);
    MusicFile(std::string file, std::string name);
    ~MusicFile();

    void open();
    void setName(const std::string & name);

    long getSize() const;
    std::string getName() const;
    uint8_t *getData() const;
};

#endif
