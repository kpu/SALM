#if !defined (_HEADER_SUFFIX_ARRAY_SEARCH_V6_)
#define _HEADER_SUFFIX_ARRAY_SEARCH_V6_

#include "salm_shared.h"
#include "_IDVocabulary.h"

#include <vector>

using namespace std;

//const int SIZE_ONE_READ = 4096;	//when loading the data, each I/O read in SIZE_ONE_READ data points // GH comment out -- conflicting value in salm_shared.h, which is replacing shared.h.

typedef struct level1BucketElement
{
	TextLenType first;
	TextLenType last;
} S_level1BucketElement;

typedef struct sentSearchTableElement
{
	bool found;
	TextLenType startPosInSA;
	TextLenType endingPosInSA;
}S_sentSearchTableElement;

typedef struct simplePhraseLocationElement
{
	TextLenType sentIdInCorpus;
	unsigned char posInSentInCorpus;
}S_SimplePhraseLocationElement;

typedef struct phraseLocationElement
{
	unsigned char posStartInSrcSent;
	unsigned char posEndInSrcSent;
	TextLenType sentIdInCorpus;
	unsigned char posInSentInCorpus;
}S_phraseLocationElement;

typedef struct ngramMatchingInfoTokenFreqOnlyElement
{
	unsigned char startPos;
	unsigned char n;	
	unsigned int tokenFreq;	

}S_ngramMatchingInfoTokenFreqOnlyElement;

//for discounting
typedef struct s_nGramScanningSimpleInfoElement
{
	IndexType vocId;
	TextLenType freqSoFar;
}S_nGramScanningSimpleInfoElement;

class C_SuffixArraySearch
{
public:
	void calcNgramMatchingInfoTokenFreqOnlyExtendingCurrentMatch(TextLenType currentMatchStart, unsigned char currentMatchLen, IndexType nextWord, int maxLenOfNgramConsidered, double * freqTable, TextLenType & updatedMatchingStart,  unsigned char & updatedMatchingLen) const;
	
	TextLenType returnCorpusSize() const;
	
	TextLenType freqOfExactPhraseMatch(const char * phrase);
	TextLenType freqOfExactPhraseMatch(vector<IndexType> phrase);

	vector<S_SimplePhraseLocationElement> locateExactPhraseInCorpus(const char * phrase);
	vector<S_SimplePhraseLocationElement> locateExactPhraseInCorpus(vector<IndexType> phrase);

	vector<S_phraseLocationElement> findPhrasesInASentence(const char * srcSent);
	vector<S_phraseLocationElement> findPhrasesInASentence(vector<IndexType> srcSentAsVocIDs);

	void displayNgramMatchingFreq4Sent(const char *);
	void displayNgramMatchingFreq4Sent(vector<IndexType>);

	unsigned int calcNgramMatchingInfoTokenFreqOnly(vector<IndexType> & sentInVocId, vector<S_ngramMatchingInfoTokenFreqOnlyElement> & nGramInfo);
	void calcNgramMatchingInfoTokenFreqOnly(vector<IndexType> & sentInVocId, double * freqTable);

	S_sentSearchTableElement * constructNgramSearchTable4SentWithLCP( vector<IndexType> & sentInVocId) const;
		
	void loadData(const char * fileNameStem, bool noVoc, unsigned int stem_len);
	void loadData_v6(const char *fileNameStem, bool noVoc, bool noOffset, bool noDiscounting, unsigned int stem_len);

	vector<IndexType> convertStringToVocId(const char * sentText);

	void setParam_reportMaxOccurrenceOfOneNgram(int reportMaxOccurrenceOfOneNgram);
	void setParam_highestFreqThresholdForReport(int highestFreqThresholdForReport);
	void setParam_longestUnitToReport(int longestUnitToReport);
	void setParam_shortestUnitToReport(int shortestUnitToReport);
	void setParam_memMapping(bool useMemMapping);
	void setParam_loadLevel1Bucket(bool loadLevel1Bucket);
	//for discounting
	void setParam_maxNForDiscounting(int maxNForDiscounting);
	void setParam_maxFreqForDiscounting(int maxFreqForDiscounting);

	C_SuffixArraySearch();
	~C_SuffixArraySearch();
private:

	void oneDimensionTableIndexToTwoDimension(unsigned int index, unsigned int sentLen, unsigned int &posInSrcSent, unsigned int &n);

	bool locateSAPositionRangeForExactPhraseMatch(vector<IndexType> phrase, TextLenType & rangeStart, TextLenType & rangeEnd);
	
	void locateSendIdFromPos(TextLenType pos, TextLenType & sentId, unsigned char & offset);
	void locateSendIdFromPos(TextLenType pos, TextLenType & sentId, unsigned char & offset, unsigned char & sentLen);

	bool searchPhraseGivenRangeWithLCP(IndexType nextWord, int lcp, TextLenType rangeStartPos, TextLenType rangeEndPos, TextLenType & resultStartPos, TextLenType & resultEndPos) const;
	char comparePhraseWithTextWithLCP(IndexType, int, TextLenType) const;

	bool loadLevel1Bucket;
	void buildUpLevel1Bucket();	//if there is no offline level1bucket file, build when reading the suffix
	void loadLevel1BucketFromFile(char * filename); //load from external file

	TextLenType corpusSize;
	bool useMemMapping;
	void loadVoc(const char * filename, unsigned int stem_len);
	void loadOffset(const char * filename);
	void loadSuffix(const char * filename);
	void loadCorpusAndInitMem(const char * filename);

	bool noVocabulary;
	bool noOffset;
	bool withDiscounting;

	//for discounting
	void constructDiscountingTable();
	double discountFreq(int n, unsigned int observedFreq) const;
	int maxNForDiscounting;
	int maxFreqForDiscounting;
	double * discountingMap;

	C_IDVocabulary * voc;
	IndexType sentIdStart;
	IndexType vocIdForSentStart;
	IndexType vocIdForSentEnd;
	IndexType vocIdForCorpusEnd;

	IndexType * corpus_list;

	unsigned char * offset_list;
	TextLenType * suffix_list;

	S_level1BucketElement * level1Buckets;

	int reportMaxOccurrenceOfOneNgram;
	int highestFreqThresholdForReport;
	int longestUnitToReport;
	int shortestUnitToReport;

	bool suffixArrayIsVersion6;	//in version 6, there is an additional <s> after <sentId>, when using offset to get the word position in sentence, need to deduct 1

	
};

#endif
