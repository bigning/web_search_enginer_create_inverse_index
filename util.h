#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>

struct IndexInfo {
    std::string url;
    int index;
    int size;
    std::string status;
    IndexInfo(std::string url_in, int index_in, int size_in,
            std::string status_in): url(url_in), index(index_in),
            size(size_in), status(status_in) {}
};

struct URLInfo {
    int doc_id;
    int words_num;
    std::string url;
    std::string data_file;
    int start_pos;
    URLInfo(int doc_id_, std::string url_, int words_num_, std::string data_file_,
            int start_pos_): 
        doc_id(doc_id_), url(url_), words_num(words_num_), data_file(data_file_),
        start_pos(start_pos_){}
};

struct PostingFromDoc{
    int word_id;
    int doc_id;
    int pos;
    //int frequency;
    //std::vector<int> positions;

    PostingFromDoc(int word_id_, int doc_id_, int pos_in_):
        word_id(word_id_), doc_id(doc_id_), pos(pos_in_)
    {}
    PostingFromDoc() {};
};

struct Word2List {
    std::string word;
    int starting_location;
    int doc_num;
};

#endif 
