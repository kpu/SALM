#include "_IDVocabulary.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <string.h>

using namespace std;

C_IDVocabulary::C_IDVocabulary(const char * fileName, unsigned int stem_len) {
	stem_len_ = stem_len;
	LoadFromFile(fileName);
}

C_IDVocabulary::~C_IDVocabulary() {
	// StringPieces will have lingering pointers otherwise
	word_id_.clear();
	for (std::vector<char*>::iterator i = id_word_.begin(); i != id_word_.end(); ++i) {
		delete [] *i;
	}
}

/// Return the vocID of word "text" if it exist in the vocabulary
/// Otherwise return 0
IndexType C_IDVocabulary::returnId(const StringPiece &text) const {
	StringPiece stemmed(text);
	if (stem_len_ > 0) { //take first stem_len_ characters
	    stemmed = text.substr(stem_len_);
        } else if ((stem_len_ < 0) && (text.length() > -stem_len_)) {//take last stem_len_ characters
	   stemmed = text.substr(text.length() + stem_len_, -stem_len_);
	} //otherwise leave as it is

	WordId::const_iterator iter = word_id_.find(stemmed);

	if(iter == word_id_.end()) {
		return 0;
	} else {
		return iter->second;
	}
}

const char *C_IDVocabulary::getText(IndexType index) const {
	if (id_word_.size() <= index) {
		return "<UNK>";
	}
	const char * ret = id_word_[index];
	if (ret) {
		return ret;
	} else {
		return "<UNK>";
	}
}

IndexType C_IDVocabulary::getSize() {
	return word_id_.size();
}


/// Load the vocabulary file into memory
/// The format of the vocabulary file is:
///		word vocID
//	in each line.
void C_IDVocabulary::LoadFromFile(const char *fileName) {

	ifstream existingVocFile;
	existingVocFile.open(fileName);

	if(!existingVocFile){
		cerr<<"Can not open existing vocabulary file "<<fileName<<endl;
		exit(0);
	}

	cerr<<"Loading existing vocabulary file: "<<fileName<<endl;

	string line;
	char delimit[] = " \t\r\n";
	
	for(getline(existingVocFile, line); ; getline(existingVocFile, line)) {
		if (!existingVocFile) {
			if (existingVocFile.eof()) break;
			cerr<<"Error reading from "<<fileName<<endl;
			break;
		}
		if (line.empty()) continue;
		size_t delimiter = line.find_first_of(delimit);
		if (delimiter == string::npos) {
			cerr<<"Line missing delimiter: "<<line<<endl;
		}
		if (line.find_first_of(delimit, delimiter + 1) != string::npos) {
			cerr<<"Too many delimiters: "<<line<<endl;
		}
		const char *start = line.c_str() + delimiter + 1;
		char *endptr;
		IndexType vocId = strtol(start, &endptr, 10);
		if (!start[0] || endptr[0]) {
			cerr<<"Bad number in second column: "<<line<<endl;
		}
		Insert(StringPiece(line.c_str(), delimiter), vocId);
	}
}

/// Return the maximum ID from all words in the vocabulary
/// Usually equals to the size of the vocabulary if the vocabulary is created from this corpus only.
/// If the vocabulary includes words from other corpora and the vocabulary only lists words in this corpus,
/// then max voc ID could be different from the vocabulary size
IndexType C_IDVocabulary::returnMaxID() const
{
	return id_word_.size() - 1;
}

IndexType C_IDVocabulary::returnNullWordID() const
{
	return 0;
}

/// Reserver vocID 0-NUMBER_OF_RESERVED_WORDS_IN_VOC for special words that might be useful for applications
/// Here we reserved 5 words:
/// _SENT_ID_PLACEHOLDER_ 1
/// _END_OF_SENTENCE_ 2
/// _TOO_LONG_TOKEN_ 3
/// _SENTENCE_START_ 4
/// _END_OF_CORPUS_ 5
/// You can add other special words to the list as long as the assignment of vocID and its interpretation is consistent between application and indexing
void C_IDVocabulary::AddReservedWords() {
	Insert("_SENT_ID_PLACEHOLDER_", 1);
	Insert("_END_OF_SENTENCE_", 2);
	Insert("_TOO_LONG_TOKEN_", 3);
	Insert("_SENTENCE_START_", 4);
	Insert("_END_OF_CORPUS_", 5);
	
	char reservedWord[20];
	for(int i=6; i<=NUMBER_OF_RESERVED_WORDS_IN_VOC; i++){
		memset(reservedWord, 0, 20);
		sprintf(reservedWord, "_RESERVED_WORDS_%d", i);
		Insert(reservedWord, i);
	}
}

void C_IDVocabulary::Insert(const StringPiece &word, IndexType id) {
	if (id_word_.size() < id) {
		id_word_.resize(id + 1);
	}
	char *&place = id_word_[id];
	if (place) {
		StringPiece existing(place);
		if (existing != word) {
      std::cerr << "Duplicate SALM vocab id " << id << ": " << existing << " and " << word << std::endl;
      abort();
		} else {
			return;
		}
	}
	place = new char[word.length() + 1];
	memcpy(place, word.data(), word.length());
	place[word.length()] = 0;
	std::pair<WordId::iterator, bool> result(word_id_.insert(make_pair(StringPiece(place, word.length()), id)));
	if (!result.second) {
    std::cerr << "SALM words " << word << " and " << result.first->second << " have the same id " << id << std::endl;
    abort();
	}
}
