#include <iostream>
#include <string>

int main() {
    std::string line;
    std::string current_word = "";
    int current_count = 0;

    // Read tab-separated "word\t1" lines from stdin
    while (std::getline(std::cin, line)) {
        size_t tab_pos = line.find('\t');
        if (tab_pos == std::string::npos) continue;

        std::string word = line.substr(0, tab_pos);
        int count = std::stoi(line.substr(tab_pos + 1));

        // Aggregate if it matches the current word
        if (word == current_word) {
            current_count += count;
        } else {
            // Print the previous word's total (if it exists)
            if (!current_word.empty()) {
                std::cout << current_word << "\t" << current_count << "\n";
            }
            current_word = word;
            current_count = count;
        }
    }

    // Print the final remaining word
    if (!current_word.empty()) {
        std::cout << current_word << "\t" << current_count << "\n";
    }
    return 0;
}
