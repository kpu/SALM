#ifndef SALM_VOCABULARY_H__
#define SALM_VOCABULARY_H__

#include "salm_shared.h"

#include "string_piece.h"

#include <boost/noncopyable.hpp>
#include <boost/unordered/unordered_map.hpp>

#include <exception>
#include <string>
#include <vector>

// Almost complete rewrite of original SALM vocabulary class.  Original by joy, rewrite by kheafiel.
class C_IDVocabulary : boost::noncopyable {
	public:
		C_IDVocabulary(const char *file_name, unsigned int stem_len);
		~C_IDVocabulary();

		IndexType returnNullWordID() const;
		IndexType returnMaxID() const;
		IndexType returnId(const StringPiece &text) const;

		const char *getText(IndexType index) const;

		IndexType getSize();

	private:
		void LoadFromFile(const char * fileName);

		void AddReservedWords();

		void Insert(const StringPiece &word, IndexType id);

		size_t stem_len_;

		std::vector<char *> id_word_;

		typedef boost::unordered_map<StringPiece, IndexType> WordId;

		WordId word_id_;
};

#endif
