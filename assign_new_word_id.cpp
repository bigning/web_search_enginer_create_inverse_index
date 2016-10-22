#include <iostream>
#include <fstream>
#include <string>

void read_in_words() {
    std::string old_words_filename = "../data/postings_from_doc/word2id_unsorted";
    std::ifstream input_file(old_words_filename.c_str());
    std::string word;
    int old_index = 0;

    while (input_file >> word >> old_index) {
    }
};

int main() {
    std::cout << "hello world" << std::endl;
    return 0;
}
