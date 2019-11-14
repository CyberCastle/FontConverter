#include "fontconverter.h"
#include <iostream>

extern "C"
{
    const char *convert(char *fileName, int fontSize, int outType, int firstChar, int lastChar) {

        if (firstChar == NULL) {
            firstChar = ' ';
        }

        if (lastChar == NULL) {
            lastChar = '~';
        }

        // Idea obtain from here: https://stackoverflow.com/a/5419388/11454077
        std::stringstream buffer;
        std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());

        FontConverter<&std::cout> f(outType);
        int result = f.convert(fileName, fontSize, firstChar, lastChar);

        std::cout.rdbuf(old);
        if (result != 0) {
            std::cerr << buffer.str() << std::endl;
            return "Error";
        }

        return f.getCode();
    }
}

int main(int argc, char *argv[]) {

    int out,
        fontSize, firstChar = ' ', lastChar = '~';

    // Parse command line.  Valid syntaxes are:
    //   fontconvert [filename] [fontSize]
    //   fontconvert [filename] [fontSize] [last char]
    //   fontconvert [filename] [fontSize] [firstChar char] [last char]
    // Unless overridden, default firstChar and last chars are
    // ' ' (space) and '~', respectively

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << "  fontfile size [first] [last]" << std::endl;
        return 0;
    }

    fontSize = atoi(argv[2]);

    if (argc == 4) {
        lastChar = atoi(argv[3]);
    } else if (argc == 5) {
        firstChar = atoi(argv[3]);
        lastChar = atoi(argv[4]);
    }

    FontConverter<&std::cerr> f(1);
    out = f.convert(argv[1], fontSize, firstChar, lastChar);
    if (out == 0)
        std::cout << f.getCode() << std::endl;

    return out;
}
