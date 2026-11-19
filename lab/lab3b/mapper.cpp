#include <iostream>
#include <string>
#include <sstream>

int main() {
    std::string line;
    // Read line-by-line from stdin
    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        std::string word;
        // Tokenize by whitespace
        while (ss >> word) {
            // Write "word\t1" to stdout
            std::cout << word << "\t1\n";
        }
    }
    return 0;
}
