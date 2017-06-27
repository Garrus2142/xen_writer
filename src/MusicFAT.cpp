#include "MusicFAT.h"

using namespace std;

MusicFAT::MusicFAT(string name, uint16_t size, uint16_t sector) : m_name(name),
    m_size(size), m_sector(sector)
{

}

string MusicFAT::getName()
{
    return (m_name);
}

uint16_t MusicFAT::getSize()
{
    return (m_size);
}

uint16_t MusicFAT::getSector()
{
    return (m_sector);
}
