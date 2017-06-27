#include "Window.h"

using namespace std;

Window::Window(const string & disk, const vector<string> & files) : m_disk(disk), m_card(disk)
{
    initscr();
    start_color();
    curs_set(0);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if (COLS < TERM_MIN_WIDTH || LINES < TERM_MIN_HEIGHT)
    {
        resetTerm();
        cerr << "Term size minimum must be " << TERM_MIN_WIDTH << " X " << TERM_MIN_HEIGHT
            << " Currently: " << COLS << " X " << LINES << endl;
        exit(EXIT_FAILURE);
    }

    m_winHeader = subwin(stdscr, HEADER_HEIGHT, COLS, 0, 0);
    box(m_winHeader, 0, 0);

    m_winBody = subwin(stdscr, LINES - HEADER_HEIGHT, COLS, HEADER_HEIGHT, 0);
    box(m_winBody, 0, 0);

    wrefresh(stdscr);

    // Open sdcard
    try{
        m_card.open();
    }
    catch (const exception & e) {
        dialogError(disk, e.what());
        resetTerm();
        return ;
    }

    // Write files if needed
    if (files.size() > 0)
        writeFiles(files);

    updateHeader();
    showMenu();
}

Window::~Window()
{
    resetTerm();
}

void Window::writeFiles(const vector<string> & files)
{
    for (size_t i(0); i < files.size(); ++i)
    {
        MusicFile music(files[i]);

        try {
            music.open();
            music.setName(dialogTextInput("Enter a name for " + files[i], MUSIC_MAX_NAME));
        }
        catch (const exception & e) {
            dialogError("Open " + files[i], e.what());
            continue;
        }

        try {
            m_card.writeMusic(music);
            dialogInfo("Write " + files[i], "Done");
        }
        catch (const exception & e) {
            dialogError("Write " + files[i], e.what());
            continue;
        }
    }
}

void Window::updateHeader()
{
    uint8_t musicCount;
    bool formatted;
    uint8_t version;

    musicCount = m_card.getMusicCount();
    formatted = m_card.isFormatted();
    version = m_card.getVersion();

    wclear(m_winHeader);
    box(m_winHeader, 0, 0);
    mvwaddstr(m_winHeader, 1, ((COLS - 20) / 2) - (m_disk.size() / 2), m_disk.c_str());

    mvwprintw(m_winHeader, 1, (COLS - 20), "| Music:   %hhu / %hhu", musicCount, XFORMAT_FAT_MAXENTRY);
    mvwprintw(m_winHeader, 2, (COLS - 20), "| XFormat: %s", formatted ? "YES" : "NO");
    mvwprintw(m_winHeader, 3, (COLS - 20), "| Version: %hhu", version);

    wrefresh(m_winHeader);
}

void Window::showMenu()
{
    vector<string> items;
    size_t selected(0);
    int ch;

    items.push_back("       MUSIC       ");
    items.push_back("       FORMAT      ");
    items.push_back("        QUIT       ");

    while (1)
    {
        wclear(m_winBody);
        box(m_winBody, 0, 0);

        // Menu
        for (size_t i(0); i < items.size(); ++i)
        {
            if (selected == i)
                wattron(m_winBody, A_STANDOUT);
            mvwaddstr(m_winBody, (i * 3) + ((LINES - HEADER_HEIGHT) / 2) -
                ((items.size() * 3) / 2), (COLS / 2) - (items[i].size() / 2), items[i].c_str());
            if (selected == i)
                wattroff(m_winBody, A_STANDOUT);
        }

        wrefresh(m_winBody);
        ch = getch();

        if (ch == KEY_UP && selected > 0)
            --selected;
        else if (ch == KEY_DOWN && selected < items.size() - 1)
            ++selected;
        else if (ch == KEY_ENTER || ch == '\n')
        {
            switch (selected)
            {
                case 0: // Music
                    if (!m_card.isFormatted())
                        dialogError("Unknow format", "You must format the disk before access to music.");
                    else
                        showMusic();
                    break;

                case 1: // Format
                    if (dialogYesNo("You really want format the disk ? (All data lost)", "  Yes  ", "  No  "))
                    {
                        try {
                            m_card.format();
                            updateHeader();
                            dialogInfo("Format complete");
                        }
                        catch (const exception & e) {
                            dialogError("Format", e.what());
                        }
                    }
                    break;

                case 2: // Quit
                    resetTerm();
                    return ;
            }
        }
    }
}

void Window::showMusic()
{
    int selected(0);
    int scrollIndex(0);
    int ch;
    vector<MusicFAT> music;

    try {
        music = m_card.getListMusic();
    }
    catch (const exception & e) {
        dialogError("List music", e.what());
        return ;
    }

    while (1)
    {
        int end(-1);

        wclear(m_winBody);
        box(m_winBody, 0, 0);

        // Header
        init_pair(3, COLOR_RED, COLOR_BLACK);
        wattron(m_winBody, COLOR_PAIR(3));
        mvwaddstr(m_winBody, 1, (COLS / 2) - (strlen("Select music to edit. ESC for quit")
            / 2), "Select music to edit. ESC for quit");
        wattroff(m_winBody, COLOR_PAIR(3));

        // Menu
        for (size_t i(scrollIndex); i < music.size(); ++i)
        {
            int line = ((i - scrollIndex) * 1) + 3;

            if (line >= LINES - HEADER_HEIGHT - 1)
            {
                end = i;
                break;
            }
            if (selected == (int)i)
                wattron(m_winBody, A_STANDOUT);
            mvwprintw(m_winBody, line, 5, "%2i - %-*s", i + 1, COLS - 28,
                music[i].getName().substr(0, COLS - 28).c_str());
            mvwprintw(m_winBody, line, COLS - 18, " | Size: %4hu", music[i].getSize());
            if (selected == (int)i)
                wattroff(m_winBody, A_STANDOUT);
        }

        wrefresh(m_winBody);

        ch = getch();
        if (ch == 27) // ESC
            break ;
        else if (ch == KEY_DOWN && selected < (int)music.size() - 1)
        {
            ++selected;
            if (selected == end)
                ++scrollIndex;
        }
        else if (ch == KEY_UP && selected > 0)
        {
            if (selected == (scrollIndex))
                --scrollIndex;
            --selected;
        }
        else if ((ch == KEY_ENTER || ch == '\n') && music.size() > 0)
        {
            dialogMusicEdit(music[selected]);
            music = m_card.getListMusic();
            if (selected >= (int)music.size())
            {
                selected = music.size() - 1;
            }

        }
    }
}

void Window::resetTerm()
{
    endwin();
}

void Window::dialogMusicEdit(const MusicFAT & music)
{
    WINDOW *dialog;
    int bodyW, bodyH;
    int bodyX, bodyY;
    int dialogW, dialogH;
    int dialogX, dialogY;
    int ch;
    size_t selected(2);
    vector<string> items;
    string newstr;

    getmaxyx(m_winBody, bodyH, bodyW);
    getparyx(m_winBody, bodyY, bodyX);

    dialogW = 40;
    dialogH = 10;
    dialogX = (bodyW / 2) - (dialogW / 2);
    dialogY = bodyY + (bodyH / 2) - (dialogH / 2);

    dialog = subwin(m_winBody, dialogH, dialogW, dialogY, dialogX);
    wclear(dialog);
    init_pair(1, COLOR_WHITE, COLOR_YELLOW);
    wbkgd(dialog, COLOR_PAIR(1));

    // Header
    wborder(dialog, '#', '#', '#', '#', '#', '#', '#', '#');
    wattron(dialog, A_BOLD);
    mvwaddstr(dialog, 0, (dialogW / 2) - (music.getName().substr(0, dialogW - 2).size() / 2),
        music.getName().substr(0, dialogW - 2).c_str());
    wattroff(dialog, A_BOLD);

    // Menu
    items.push_back("   Rename   ");
    items.push_back("   Delete   ");
    items.push_back("   Cancel   ");

    while (1)
    {
        for (size_t i(0); i < items.size(); ++i)
        {
            int line = (i * 2) + 2;

            if (selected == i)
                wattron(dialog, A_STANDOUT);
            mvwprintw(dialog, line, (dialogW / 2) - (items[i].size() / 2), items[i].c_str());
            if (selected == i)
                wattroff(dialog, A_STANDOUT);
        }

        wrefresh(dialog);

        ch = getch();
        if (ch == KEY_DOWN && selected < items.size() -1)
            ++selected;
        else if (ch == KEY_UP && selected > 0)
            --selected;
        else if (ch == KEY_ENTER || ch == '\n')
        {
            switch (selected)
            {
                case 0: // Rename
                    if ((newstr = dialogTextInput(string("Enter new name for ")
                            + music.getName().substr(0, 30), MUSIC_MAX_NAME)).size())
                    {
                        try {
                            m_card.renameMusic(music.getSector(), newstr);
                        }
                        catch (const exception & e) {
                            dialogError(string("Rename ") + music.getName().substr(0, 30), e.what());
                        }
                    }
                    return;

                case 1: // Delete
                    if (dialogYesNo("Are you sure ?", "  Yes  ", "  No  "))
                    {
                        try {
                            m_card.removeMusic(music.getSector());
                            updateHeader();
                        }
                        catch (const exception & e) {
                            dialogError(string("Delete ") + music.getName().substr(0, 30), e.what());
                        }
                    }
                    return;

                case 2: // Cancel
                    delwin(dialog);
                    return;
            }
        }
    }
    delwin(dialog);
}

string Window::dialogTextInput(const string & msg, int maxsize)
{
    WINDOW *dialog;
    int bodyW, bodyH;
    int bodyX, bodyY;
    int dialogW, dialogH;
    int dialogX, dialogY;
    int ch;
    int cursor(0);
    bool yes(false);
    string input;

    getmaxyx(m_winBody, bodyH, bodyW);
    getparyx(m_winBody, bodyY, bodyX);

    dialogW = bodyW - 6;
    dialogH = 10;
    dialogX = 3;
    dialogY = bodyY + (bodyH / 2) - (dialogH / 2);

    dialog = subwin(m_winBody, dialogH, dialogW, dialogY, dialogX);
    wclear(dialog);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    wbkgd(dialog, COLOR_PAIR(1));

    // Header
    wborder(dialog, '#', '#', '#', '#', '#', '#', '#', '#');
    wattron(dialog, A_BOLD);
    mvwaddstr(dialog, 0, (dialogW / 2) - (strlen(" TEXT INPUT ") / 2), " TEXT INPUT ");
    wattroff(dialog, A_BOLD);

    // Message
    mvwaddstr(dialog, 3, (dialogW / 2) - (msg.size() / 2), msg.c_str());

    while (1)
    {
        // Textbox
        init_pair(4, COLOR_WHITE, COLOR_BLACK);
        wattron(dialog, COLOR_PAIR(4));
        mvwprintw(dialog, 5, 2, "%-*s", dialogW - 4, input.c_str());
        wattron(dialog, A_STANDOUT);
        mvwaddstr(dialog, 5, 2 + cursor, " ");
        wattroff(dialog, COLOR_PAIR(4) | A_STANDOUT);

        // Button
        if (yes)
            wattron(dialog, A_STANDOUT);
        mvwaddstr(dialog, dialogH - 2, (dialogW / 4) - (strlen("  Apply  ") / 2),
            "  Apply  ");
        if (yes)
            wattroff(dialog, A_STANDOUT);

        if (!yes)
            wattron(dialog, A_STANDOUT);
        mvwaddstr(dialog, dialogH - 2, ((dialogW / 4) * 3) - (strlen("  Cancel  ") / 2),
            "  Cancel  ");
        if (!yes)
            wattroff(dialog, A_STANDOUT);

        wrefresh(dialog);
        ch = getch();

        if (ch >= 32 && ch <= 126 && (int)input.size() < maxsize)
        {
            input += ch;
            ++cursor;
        }
        else if (ch == 127 && cursor > 0)
        {
            input.pop_back();
            --cursor;
        }
        else if (ch == KEY_LEFT && !yes)
            yes = true;
        else if (ch == KEY_RIGHT && yes)
            yes = false;
        else if (ch == KEY_ENTER || ch == '\n')
        {
            delwin(dialog);
            return (yes ? input : "");
        }
    }
    delwin(dialog);
    return (input);
}

bool Window::dialogYesNo(const string & question, const string & btnyes, const string & btnno)
{
    WINDOW *dialog;
    int bodyW, bodyH;
    int bodyX, bodyY;
    int dialogW, dialogH;
    int dialogX, dialogY;
    int ch;
    bool yes(false);

    getmaxyx(m_winBody, bodyH, bodyW);
    getparyx(m_winBody, bodyY, bodyX);

    dialogW = bodyW - 6;
    dialogH = 10;
    dialogX = 3;
    dialogY = bodyY + (bodyH / 2) - (dialogH / 2);

    dialog = subwin(m_winBody, dialogH, dialogW, dialogY, dialogX);
    wclear(dialog);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    wbkgd(dialog, COLOR_PAIR(1));

    // Header
    wborder(dialog, '#', '#', '#', '#', '#', '#', '#', '#');
    wattron(dialog, A_BOLD);
    mvwaddstr(dialog, 0, (dialogW / 2) - (strlen(" QUESTION ") / 2), " QUESTION ");
    wattroff(dialog, A_BOLD);

    // Question
    mvwaddstr(dialog, 4, (dialogW / 2) - (question.size() / 2), question.c_str());

    while (1)
    {
        if (yes)
            wattron(dialog, A_STANDOUT);
        mvwaddstr(dialog, dialogH - 2, (dialogW / 4) - (btnyes.size() / 2), btnyes.c_str());
        if (yes)
            wattroff(dialog, A_STANDOUT);

        if (!yes)
            wattron(dialog, A_STANDOUT);
        mvwaddstr(dialog, dialogH - 2, ((dialogW / 4) * 3) - (btnno.size() / 2), btnno.c_str());
        if (!yes)
            wattroff(dialog, A_STANDOUT);

        wrefresh(dialog);

        ch = getch();
        if (ch == KEY_RIGHT && yes)
            yes = false;
        else if (ch == KEY_LEFT && !yes)
            yes = true;
        else if (ch == KEY_ENTER || ch == '\n')
            break;
    }
    delwin(dialog);
    return (yes);
}

void Window::dialogError(const string & errmsg, const string & errmsg2)
{
    WINDOW *dialog;
    int bodyW, bodyH;
    int bodyX, bodyY;
    int dialogW, dialogH;
    int dialogX, dialogY;
    int ch;

    getmaxyx(m_winBody, bodyH, bodyW);
    getparyx(m_winBody, bodyY, bodyX);

    dialogW = bodyW - 6;
    dialogH = 10;
    dialogX = 3;
    dialogY = bodyY + (bodyH / 2) - (dialogH / 2);

    dialog = subwin(m_winBody, dialogH, dialogW, dialogY, dialogX);
    wclear(dialog);
    init_pair(1, COLOR_WHITE, COLOR_RED);
    wbkgd(dialog, COLOR_PAIR(1));

    // Header
    wborder(dialog, '#', '#', '#', '#', '#', '#', '#', '#');
    wattron(dialog, A_BOLD);
    mvwaddstr(dialog, 0, (dialogW / 2) - (strlen(" ERROR ") / 2), " ERROR ");
    wattroff(dialog, A_BOLD);

    // Errmsg 1
    mvwaddstr(dialog, 3, (dialogW / 2) - (errmsg.size() / 2), errmsg.c_str());
    // Errmsg 2
    if (errmsg2.size())
        mvwaddstr(dialog, 4, (dialogW / 2) - (errmsg2.size() / 2), errmsg2.c_str());

    // Button
    wattron(dialog, A_STANDOUT);
    mvwaddstr(dialog, dialogH - 2, (dialogW / 2) - (strlen("   OK   ") / 2), "   OK   ");
    wattroff(dialog, A_STANDOUT);

    wrefresh(dialog);

    while (1)
    {
        ch = getch();
        if (ch == KEY_ENTER || ch == '\n')
            break;
    }
    delwin(dialog);
}

void Window::dialogInfo(const std::string & infomsg, const std::string & infomsg2)
{
    WINDOW *dialog;
    int bodyW, bodyH;
    int bodyX, bodyY;
    int dialogW, dialogH;
    int dialogX, dialogY;
    int ch;

    getmaxyx(m_winBody, bodyH, bodyW);
    getparyx(m_winBody, bodyY, bodyX);

    dialogW = bodyW - 6;
    dialogH = 10;
    dialogX = 3;
    dialogY = bodyY + (bodyH / 2) - (dialogH / 2);

    dialog = subwin(m_winBody, dialogH, dialogW, dialogY, dialogX);
    wclear(dialog);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    wbkgd(dialog, COLOR_PAIR(1));

    // Header
    wborder(dialog, '#', '#', '#', '#', '#', '#', '#', '#');
    wattron(dialog, A_BOLD);
    mvwaddstr(dialog, 0, (dialogW / 2) - (strlen(" INFO ") / 2), " INFO ");
    wattroff(dialog, A_BOLD);

    // Errmsg 1
    mvwaddstr(dialog, 3, (dialogW / 2) - (infomsg.size() / 2), infomsg.c_str());
    // Errmsg 2
    if (infomsg2.size())
        mvwaddstr(dialog, 4, (dialogW / 2) - (infomsg2.size() / 2), infomsg2.c_str());

    // Button
    wattron(dialog, A_STANDOUT);
    mvwaddstr(dialog, dialogH - 2, (dialogW / 2) - (strlen("   OK   ") / 2), "   OK   ");
    wattroff(dialog, A_STANDOUT);

    wrefresh(dialog);

    while (1)
    {
        ch = getch();
        if (ch == KEY_ENTER || ch == '\n')
            break;
    }
    delwin(dialog);
}
