#include <iostream>
#include "fontconverter.h"

extern "C"
{
    const char *generateFont(char *fileName, int fontSize, int outType)
    {

        int firstChar = ' ', lastChar = '~';
        FontConverter f(outType);
        if (f.convert(fileName, fontSize, firstChar, lastChar) != 0)
        {
            return "Error";
        }

        return f.getCode();
    }
}

int main(int argc, char *argv[])
{

    int out, fontSize, firstChar = ' ', lastChar = '~';

    // Parse command line.  Valid syntaxes are:
    //   fontconvert [filename] [fontSize]
    //   fontconvert [filename] [fontSize] [last char]
    //   fontconvert [filename] [fontSize] [firstChar char] [last char]
    // Unless overridden, default firstChar and last chars are
    // ' ' (space) and '~', respectively

    if (argc < 3)
    {
        //fprintf(stderr, "Usage: %s fontfile size [first] [last]\n", argv[0]);
        return 0;
    }

    fontSize = atoi(argv[2]);

    if (argc == 4)
    {
        lastChar = atoi(argv[3]);
    }
    else if (argc == 5)
    {
        firstChar = atoi(argv[3]);
        lastChar = atoi(argv[4]);
    }

    FontConverter f(1);
    out = f.convert(argv[1], fontSize, firstChar, lastChar);
    std::cout << f.getCode() << std::endl;
    return out;
}