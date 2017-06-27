#include <iostream>
#include <vector>
#include <string>
#include "Window.h"

using namespace std;

static void printUsage(const char *name)
{
    cout << "Usage: " << name << " [disk] {file_to_copy...}" << endl;
}

int main(int argc, char **argv)
{
    vector<string> files;
    if (argc < 2)
    {
        printUsage(argv[0]);
        return (EXIT_SUCCESS);
    }

    if (argc > 2)
    {
        for (int i(2); i < argc; ++i)
            files.push_back(argv[i]);
    }

    Window mainWin(argv[1], files);

    return (EXIT_SUCCESS);
}
