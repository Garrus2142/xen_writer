#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <String>
#include <ncurses.h>
#include <vector>
#include "XFormat.h"

#define TERM_MIN_WIDTH 100
#define TERM_MIN_HEIGHT 15

#define HEADER_HEIGHT 5
#define HEADER_TITLE "XEN COMPILER"

class Window
{
private:
    std::string m_disk;
    WINDOW *m_winHeader;
    WINDOW *m_winBody;
    XFormat m_card;

public:
    Window(const std::string & disk, const std::vector<std::string> & files);
    ~Window();

private:
    void resetTerm();
    void initBody();

    void updateHeader();
    void showMenu();
    void showMusic();
    void writeFiles(const std::vector<std::string> & files);

    std::string dialogTextInput(const std::string & msg, int maxsize);
    void dialogInfo(const std::string & infomsg, const std::string & infomsg2 = "");
    void dialogError(const std::string & errmsg, const std::string & errmsg2 = "");
    bool dialogYesNo(const std::string & question, const std::string & btnyes, const std::string & btnno);
    void dialogMusicEdit(const MusicFAT & music);
};

#endif
