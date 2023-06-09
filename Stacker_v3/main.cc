#include <iostream>
#include <string>
#include <vector>

int main(int argc, char const *argv[])
{
    if (argc < 3) {
        std::cerr << "Stacker expects at least 1 argument. Usage: ./stacker_exec <input root> [Settingfile] [Options]" << std::endl;
        exit(1);
    }

    std::vector<std::string> argvStr(&argv[1], &argv[0]+argc);

    // styling force
    
    // loop options
    
    // book options
    // etcetc
    return 0;
}
