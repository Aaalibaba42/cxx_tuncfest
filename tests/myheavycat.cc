#include <iostream>
#include <fstream>
#include <vector>

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
    std::vector<char> stdout_vec(0);
    std::vector<char> stderr_vec(0);
    while (inputStream->get(ch))
    {
        stdout_vec.push_back(ch);
        stdout_vec.shrink_to_fit();
        stderr_vec.push_back(ch);
        stderr_vec.shrink_to_fit();
    }

    for (char const& c : stdout_vec)
        std::cout << c << std::flush;
    for (char const& c : stderr_vec)
        std::cout << c << std::flush;

    stdout_vec.clear();
    stderr_vec.clear();

    if (inputFile.is_open())
    {
        inputFile.close();
    }
    return 0;
}
