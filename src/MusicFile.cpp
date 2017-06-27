#include "MusicFile.h"

using namespace std;

MusicFile::MusicFile(string file) : MusicFile(file, "")
{

}

MusicFile::MusicFile(string file, string name) : m_stream(file, ifstream::binary),
    m_size(0), m_data(NULL), m_file(file)
{
    setName(name);
}

MusicFile::~MusicFile()
{
    if (!m_data)
        delete m_data;
}

void MusicFile::open()
{
    if (!m_stream)
        throw logic_error("Cannot open " + m_file + ": " + strerror(errno));

    // Get size
    m_stream.seekg(0, ios_base::end);
    m_size = m_stream.tellg();

    if (m_size > MUSIC_MAX_SIZE)
        throw logic_error(m_file + ": Maximum size is " + to_string(MUSIC_MAX_SIZE) + ", current " + to_string(m_size));

    m_stream.seekg(0, ios_base::beg);

    m_data = new uint8_t[m_size];
    m_stream.read((char *)m_data, m_size);
}

void MusicFile::setName(const string & name)
{
    m_name = name.substr(0, MUSIC_MAX_NAME);
}

long MusicFile::getSize() const
{
    return (m_size);
}

string MusicFile::getName() const
{
    return (m_name);
}

uint8_t *MusicFile::getData() const
{
    return (m_data);
}
