#include "generate_inverse_index.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <set>

GenerateInverseIndex::GenerateInverseIndex() {
    postings_filename_ = "../data/postings_from_doc/all_postings";
    word2id_filename_ = "../data/postings_from_doc/word2id_unsorted";
    save_filename_ = "../data/postings_from_doc/inverted_list";
    words_to_list_filename_ = "../data/postings_from_doc/word_to_list";

    index_file_bytes_so_far_ = 0;
    last_write_at_ = -1;
    current_bytes_ = 0;

    num_of_posting_in_buffer_ = 1100000;
    write_buffer_ = new int[num_of_posting_in_buffer_ * 3];
    write_to_file_interval_ = 1000000;
}

void GenerateInverseIndex::run() {

    load_word2id();

    std::ofstream output(save_filename_.c_str(), std::ios::binary);
    std::ofstream words_to_list(words_to_list_filename_.c_str());

    std::ifstream posting_input(postings_filename_.c_str());
    int word_id, doc_id, position;
    int last_word_id = 0, last_doc_id = 0;
    long long last_word_start = 0;
    std::vector<int> positions; 

    int buffer_index = 0;
    long long verify_bytes = 0;

    int process_num = 0;
    std::set<int> doc_ids_contains_this_word;
    while (posting_input >> word_id >> doc_id >> position) {
        //std::cout << process_num << std::endl;
        if (word_id == last_word_id && doc_id == last_doc_id) {
            positions.push_back(position);
        }
        doc_ids_contains_this_word.insert(doc_id);
        if (word_id != last_word_id || doc_id != last_doc_id) {
            write_buffer_[buffer_index++] = last_doc_id;
            write_buffer_[buffer_index++] = positions.size();
            for (int i = 0; i < positions.size(); i++) {
                write_buffer_[buffer_index++] = positions[i];
            }
            /*
            std::cout << "buffer_index: " << buffer_index 
                << ", posisions.size " << positions.size() << std::endl;
            */
            current_bytes_ += (2 + positions.size()) * sizeof(int);

            positions.clear();
            positions.push_back(position);
            if (process_num - last_write_at_ >= write_to_file_interval_) {
                // write buffer to file
                output.write((const char*)write_buffer_, buffer_index*sizeof(int)); 
                verify_bytes += buffer_index * sizeof(int);
                if (verify_bytes != current_bytes_) {
                    std::cout << "[ERROR]: current_bytes_: " << current_bytes_
                        << " verify bytes: " << verify_bytes << std::endl;
                    exit(-1);
                }
                buffer_index = 0;
                last_write_at_ = process_num;
                std::cout << "[INFO]: writing to file: " << process_num << std::endl;
            }
        }

        if (word_id != last_word_id) {
            words_to_list << id2word_[last_word_id] << " " <<
                last_word_start << " " <<
                doc_ids_contains_this_word.size() << std::endl;
            last_word_start = current_bytes_;
            doc_ids_contains_this_word.clear();
        }

        last_word_id = word_id;
        last_doc_id = doc_id;
        process_num++;
    }

    write_buffer_[buffer_index++] = doc_id;
    write_buffer_[buffer_index++] = positions.size();
    for (int i = 0; i < positions.size(); i++) {
        write_buffer_[buffer_index++] = positions[i];
    }
    
    output.write((const char*)write_buffer_, buffer_index*sizeof(int));
    words_to_list << id2word_[last_word_id] << " " <<
        last_word_start << " " << doc_ids_contains_this_word.size() << std::endl;

    posting_input.close();
    words_to_list.close();
}

void GenerateInverseIndex::load_word2id() {
    std::ifstream input(word2id_filename_.c_str());
    std::string word;
    int id;
    while (input >> word >> id) {
        id2word_.insert(std::pair<int, std::string>(id, word));
    }
    input.close();
}

int main() {
    GenerateInverseIndex* p_index_generator = new GenerateInverseIndex;
    p_index_generator->run();
    std::cout << "hello world!" << std::endl;
    return 0;
}
