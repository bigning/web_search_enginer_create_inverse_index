#include "generate_posting.h"
#include "util.h"
#include <sstream>

#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml++/libxml++.h>
#include <exception>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <zlib.h>

void GeneratePosting::test_parse_index_file() {
    std::vector<IndexInfo> indices;
    parse_index_file("../data/nz2_merged/9_index", indices);
    std::cout << "[INFO]: get " << indices.size() << " index" << std::endl;
}

void GeneratePosting::get_wet_filenames() {
    std::ifstream index_file(wet_file_list_.c_str());
    std::string index_file_name;
    while (index_file >> index_file_name) {
        wet_files_.push_back(index_file_name);
    }
    index_file.close();
}

void GeneratePosting::parse_index_file(const char* filename,
        std::vector<IndexInfo>& indices){
    std::ifstream file(filename, std::ios::in|std::ios::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);
    //boost::iostreams::copy(in, std::cout);
    
    std::istream input(&in);
    std::string url, ip, status;
    int tmp1 = 0, tmp2 = 0, size = 0, port = 0;
    int num = 0;
    while (input >> url >> tmp1 >> tmp2 >> size >> ip >> port >> status) {
        IndexInfo index_info(url, num, size, status);
        indices.push_back(index_info);
        num++;
    } 
    file.close();    
}

bool GeneratePosting::get_next_sector(std::istream& input, std::map<std::string, std::string>& res) {
    //std::map<std::string, std::string> res;
    std::string line;
    if (input.eof()) { 
        return false; 
    }
    while (std::getline(input, line)) {
        if (line.size() > 1) break;
        if (input.eof()) { 
            return false; 
        }
    }
    if (line == "WARC/1.0\r") {
        std::getline(input, line);
        if (input.eof()) { 
            return false; 
        }
    }
    int length = 0;
    int line_num = 0;
    std::string text;
    if (input.eof()) { 
        return false; 
    }
    while (true) {
        line_num++;
        std::vector<std::string> split_vec;
        boost::split(split_vec, line, boost::is_any_of(": "), boost::token_compress_on);
        boost::trim_left(split_vec[1]);
        if (split_vec[1].size() <= 1) {
            continue;
        }
        split_vec[1] = split_vec[1].substr(0, split_vec[1].size() - 1);
        if (split_vec[0] == "Content-Type") {
            res.insert(std::pair<std::string, std::string>(split_vec[0], 
                        split_vec[1]));
        }
        if (split_vec[0] == "WARC-Target-URI") {
            res.insert(std::pair<std::string, std::string>("url", line.substr(17, line.size() - 18)));
        }
        if (split_vec[0] == "Content-Length") {
            std::istringstream buffer(split_vec[1]);
            buffer >> length;
            std::getline(input, line);
            break;
        }
        if (!std::getline(input, line)) return false;
        if (line_num == 50) {
            std::cout << "[EROOR]: parse wet data error" << std::endl;
            break;
        }
    }

    if (input.eof()) {
        return false;
    }

    if (length <= 0) return false;
    char* text_char = new char[length];
    input.read(text_char, length);
    text = std::string(text_char);
    res.insert(std::pair<std::string, std::string>("text", text));
    delete[] text_char;
    return true;
}

void GeneratePosting::parse_data_file(int num) {

    // parse data file
    std::string data_file_name = wet_files_[num];
    std::ifstream file(data_file_name.c_str(), std::ios::in|std::ios::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);
    std::istream input(&in);
    std::string data_file_name_pure = "";
    int k = data_file_name.size() - 1;
    while (data_file_name[k] != '/') {
        k--;
    }
    data_file_name_pure = data_file_name.substr(k, data_file_name.size() - k);



    // output file
    char output_posting_filename[500];
    snprintf(output_posting_filename, 500, "%s%d_posting_vectors",
            intermedia_save_path_.c_str(), num);
    std::ofstream output_file(output_posting_filename);

    int num_a = 0;
    int valid_num = 0;
    int html_num = 0;
    while (true) {
        std::map<std::string, std::string> sector;
        try {
            if(!get_next_sector(input, sector)) {
                break;
            }
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            std::cout << "[error1]" << std::endl;
            exit(0);
        }
        if (sector.size() <= 1) break;
        //if (num_a % 1000 == 0 || num_a > 50000) std::cout << num_a << std::endl;
        if (sector["Content-Type"] != "text/plain") continue;
        //std::cout << num_a << "  " << sector["text"].size() << std::endl;

        std::string text = sector["text"];
        std::string url = sector["url"];
        if (text.size() > 0 && url.size() > 0) {
            num_a++;
        }
        int invalid_char = 0;
        for (int i = 0; i < text.size(); i++) {
            if (text[i] < 0 || text[i] > 128) {
                invalid_char++;
                text[i] = ' ';
            }
        }

        // only consider the ascii page
        if ((float)invalid_char > 0.1 * (float)text.size()) {
            valid_num++;
            continue;
        }

        // to lower case and replace all non alphanumeric char with space
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        for (int j = 0; j < text.size(); j++) {
            if (text[j] >= 'a' && text[j] <= 'z') continue;
            if (text[j] >= '0' && text[j] <= '9') continue;
            text[j] = ' ';
        }

        int words_num = 0;
        std::string new_text;
        std::vector<PostingFromDoc> postings;
        try {
            postings = generate_posting_from_doc(
                    text, doc_num_, words_num, new_text);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            std::cout << "[error1]" << std::endl;
            exit(0);
        }

        new_text_file_ << new_text;

        URLInfo url_info(doc_num_, url, words_num, 
                data_file_name_pure, start_pos_);
        url_table_.push_back(url_info);
        if (url_table_.size() % 1000 == 0) {
            std::cout << "[INFO]: url table size: " << url_table_.size() << std::endl;
        }

        if (url_table_.size() == 50000) {
            std::cout << "[INFO]: writing url table" << std::endl;
            std::cout << "[INFO]: doc_num: " << doc_num_ << std::endl;
            for (int i = 0; i < url_table_.size(); i++) {
                url_table_file_ << url_table_[i].doc_id << " " << url_table_[i].words_num <<" " << url_table_[i].url << " " << url_table_[i].data_file << " " 
                    << url_table_[i].start_pos <<  std::endl;
            }
            url_table_.clear();
        }

        for (int j = 0; j < postings.size(); j++) {
            output_file << postings[j].word_id << " " << postings[j].doc_id << " " << postings[j].pos << std::endl;
        }
        html_num += postings.size();
        //std::copy(postings.begin(), postings.end(), output_iter);        

        //std::cout << text << std::endl;
        //exit(-1);

        doc_num_++;
        if (doc_num_ >= total_doc_num_) {
            new_text_file_.close();
            break;
        }
        start_pos_ += new_text.size();
    }

    std::cout << "[INFO]: writing url table" << std::endl;
    std::cout << "[INFO]: doc_num: " << doc_num_ << std::endl;
    for (int i = 0; i < url_table_.size(); i++) {
        url_table_file_ << url_table_[i].doc_id << " " << url_table_[i].words_num <<" " << url_table_[i].url << " " << url_table_[i].data_file << " " 
            << url_table_[i].start_pos <<  std::endl;
    }
    url_table_.clear();

    std::cout << "[INFO]: parsing data file done! " << num << " " << data_file_name << std::endl; 
    output_file.close();
    //new_text_file_.close();

    std::cout << "[DEBUG]: total postings: " << html_num << std::endl << std::endl;;
    file.close();
}

GeneratePosting::GeneratePosting() {
    total_doc_num_ = 1000000;

    input_data_path_ = "../data/wet/";
    intermedia_save_path_ = "../data/postings_from_doc/";
    wet_file_list_ = "./wet_list.txt";

    word_num_ = 0;
    doc_num_ = 0;
    start_pos_ = 0;

    data_file_num_ = 100;

    // write url table and word2id_unsorted_
    char url_table_filename[500];
    snprintf(url_table_filename, 500, "%s/urltable",
            intermedia_save_path_.c_str());
    url_table_file_.open(url_table_filename, std::ofstream::out);

    char new_text_file_name[500];
    snprintf(new_text_file_name, 500, "%s/new_text",
            intermedia_save_path_.c_str());
    new_text_file_.open(new_text_file_name, std::ofstream::out);
}

std::vector<PostingFromDoc> GeneratePosting::generate_posting_from_doc(
        const std::string text, int doc_id, int& words_num_in_doc, std::string& new_text) {
    std::vector<std::string> split_vec;
    boost::split(split_vec, text, boost::is_any_of(" "), boost::token_compress_on);
    words_num_in_doc = split_vec.size();

    std::vector<PostingFromDoc> res;
    std::map<std::string, int>::iterator iter;
    for (int i = 0; i < split_vec.size(); i++) {
        if (split_vec[i].size() == 0) continue;
        iter = word2id_unsorted_.find(split_vec[i]);
        int word_id = 0;
        if (iter == word2id_unsorted_.end()) {
            word2id_unsorted_.insert(
                    std::pair<std::string, int>(split_vec[i], word_num_));
            word_id = word_num_;
            word_num_++;
        } else {
            word_id = iter->second;
        }

        res.push_back(PostingFromDoc(word_id, doc_id, i));    
    }
    for (int i = 0; i < split_vec.size(); i++) {
        new_text += split_vec[i] + " ";
    }

    return res;
}

void print_element_names(xmlNode * a_node, std::string& text){
    xmlNode *cur_node = NULL;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_TEXT_NODE) {
            //printf("node type: %d, name: %s\n",cur_node->type, cur_node->name);
            //printf("content:%s\n", cur_node->content);
            //printf("parent: %s\n", cur_node->parent->name);
            std::string tmp = std::string((char*)cur_node->content);
            if (tmp.substr(0, 8) == "HTTP/1.1") continue;
            //boost::trim_left(tmp);
            //boost::trim_right(tmp);
            text += tmp + " ";
        }
        print_element_names(cur_node->children, text);
    }
}

void GeneratePosting::generate_postings() {
    get_wet_filenames();
    for (int i = 0; i < wet_files_.size(); i++) {
    //for (int i = 0; i < 1; i++) {
        std::cout << "[INFO]: " << i << std::endl; 
        parse_data_file(i);
    }

    char word2id_unsorted_filename[500];
    snprintf(word2id_unsorted_filename, 500, "%s/word2id_unsorted", 
            intermedia_save_path_.c_str());
    std::ofstream word2id_unsorted_file(word2id_unsorted_filename);
    std::cout << "[INFO]: writing word2id info" << std::endl;
    std::map<std::string, int>::iterator iter;
    for (iter = word2id_unsorted_.begin(); iter != word2id_unsorted_.end(); iter++) {
        word2id_unsorted_file << iter->first << " " << iter->second << std::endl;
    }
    word2id_unsorted_file.close();

}


int main() {
    GeneratePosting* posting_generator = new GeneratePosting();
    posting_generator->generate_postings();

    // read posting file
    //std::ifstream input("../data/postings_from_doc/0_posting_vectors");
    //std::cout << "hello world" << std::endl;

    return 0;
}
