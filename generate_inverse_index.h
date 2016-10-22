#include "util.h"
#include <string>
#include <map>

class GenerateInverseIndex {
public:
    GenerateInverseIndex();
    void load_word2id();
    void run();

private:
    std::map<int, std::string> id2word_;
    std::map<std::string, Word2List> word2list_;
    // In the last writing-to-file, how many postings we have processed
    int last_write_at_;
    long long current_bytes_;
    int* write_buffer_; 
    int num_of_posting_in_buffer_;
    int write_to_file_interval_;

    int index_file_bytes_so_far_;
    std::string postings_filename_;
    std::string word2id_filename_;
    std::string save_filename_;
    std::string words_to_list_filename_;
};
