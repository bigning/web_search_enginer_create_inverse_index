#ifndef GENERATE_POSTING_H
#define GENERATE_POSTING_H

#include "util.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <fstream>

#include <string>
#include <vector>
#include <map>

class GeneratePosting{
public:
    GeneratePosting();
    void test_parse_index_file();
    void parse_data_file(int num);
    std::vector<PostingFromDoc> generate_posting_from_doc(
            const std::string text, int doc_id, int& words_num_in_doc,
            std::string& new_text);
    void generate_postings();
    void get_wet_filenames();
    std::map<std::string, std::string> get_next_sector(std::istream& input);

private:
    void parse_index_file(const char* filename, std::vector<IndexInfo>& indices);
    std::ofstream url_table_file_;
    std::ofstream new_text_file_;
    
    int debug_mode_;
    std::string input_data_path_;
    std::string intermedia_save_path_;
    std::string wet_file_list_;

    std::vector<std::string> wet_files_;
    std::vector<std::string> data_file_names_;

    std::map<std::string, int> word2id_unsorted_;
    std::map<std::string, int> word2id_sorted_;
    std::vector<URLInfo> url_table_;

    int word_num_;
    int doc_num_;
    int data_file_num_;
    int total_doc_num_;
    int start_pos_;
};

void print_element_names(xmlNode * a_node, std::string& text);

#endif
