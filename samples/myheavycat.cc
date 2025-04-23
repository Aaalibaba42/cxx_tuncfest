#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
    std::istream* inputStream = &std::cin;
    std::ifstream inputFile;

    if (argc == 2)
    {
        inputFile.open(argv[1]);
        if (!inputFile.is_open())
        {
            std::cerr << "Could not open " << argv[1] << std::endl;
            return 1;
        }
        inputStream = &inputFile;
    }
    else if (argc > 2)
    {
        std::cerr << "Usage: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }

    char ch;
    int i = 0;
    while (inputStream->get(ch))
    {
        std::cout << ch << std::flush;
        ++i;
        for (int j = 0; j < i * 1000; ++j)
        {
            ;
        }
    }

    if (inputFile.is_open())
    {
        inputFile.close();
    }
    return 0;
}
