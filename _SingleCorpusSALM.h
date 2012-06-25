// _SingleCorpusSALM.h: interface for the C_SingleCorpusSALM class.
//
// $Rev: 2296 $
// $LastChangedDate: 2007-06-27 15:09:33 -0400 (Wed, 27 Jun 2007) $
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__SINGLECORPUSSALM_H__004675FC_A75A_4CED_A1A5_0811F29865A8__INCLUDED_)
#define AFX__SINGLECORPUSSALM_H__004675FC_A75A_4CED_A1A5_0811F29865A8__INCLUDED_

#include "salm_shared.h"
#include "math.h"

#include "LMInterface.h"

#include "_IDVocabulary.h"
#include "_SuffixArraySearch.h"

//const double SALM_PROB_UNK = 0.00000000023283064365386962890625; // 1/4G
//const double SALM_LOG_PROB_UNK = log(SALM_PROB_UNK);
//const double SALM_LOG_0 = -20;
// These three commented out by GH -- already defined in salm_shared.h, which replaces shared.h.


typedef struct s_cached_SA_access_key{
	TextLenType currentMatchStart;
	unsigned char currentMatchLen;
	IndexType nextWord;
}S_CachedSA_Access_Key;

typedef struct s_cached_SA_access_info{
	TextLenType updatedMatchingStart;
	unsigned char updatedMatchingLen;
	double logProb;
	long lastTimedUsed;
}S_Cached_SA_Access_Info;

struct lt_s_cached_SA_access_key
{
  bool operator()(S_CachedSA_Access_Key a, S_CachedSA_Access_Key b) const{
		if(a.currentMatchStart<b.currentMatchStart){
			return true;
		}

		if(a.currentMatchStart>b.currentMatchStart){
			return false;
		}

		if(a.currentMatchLen<b.currentMatchLen){
			return true;
		}

		if(a.currentMatchLen>b.currentMatchLen){
			return false;
		}

		if(a.nextWord<b.nextWord){
			return true;
		}

		return false;	
	}
};

class C_SingleCorpusSALM : public CLM  
{
public:
	void setParam_maxLenOfNgramConsidered(int maxLenOfNgramConsidered);
	void setParam_interpolationStrategy(char interpolationStrategy);

	LMWordIndex VocabularySize() const;
	LMWordIndex returnSentEndVocId();
	double LogProb(TextLenType currentMatchStart, unsigned char currentMatchLen, IndexType nextWord, TextLenType &updatedMatchingStart, unsigned char &updatedMatchingLen) const;

	unsigned int Order() const {
		return maxLenOfNgramConsidered;
	}

	// Added by kheafiel.  Yes, this does reveal internals, with the point of making a better wrapper.
	const C_IDVocabulary &GetVocabulary() const { return *idVoc; }

	CLMSentenceState * NewSentence() const;

	C_SingleCorpusSALM(const char *file_stem, unsigned int ngram_length);
	C_SingleCorpusSALM();
	virtual ~C_SingleCorpusSALM();

protected:
 	C_IDVocabulary * idVoc;
  
private:	
	
	bool applyDiscounting;

	double calcLogProb(const double *freq) const;
	double calcLogProb_equalWeightedInterpolation(const double * freq) const;
	double calcLogProb_ibmHeuristicInterpolation(const double *freq) const;
	int calcNGramOrder(const double *freq) const; // GH ADD

	double corpusSize;
	int maxLenOfNgramConsidered;

	char interpolationStrategy;

	LMWordIndex sentEndVocId;

	C_SuffixArraySearch saSearchObj;


};

#endif // !defined(AFX__SINGLECORPUSSALM_H__004675FC_A75A_4CED_A1A5_0811F29865A8__INCLUDED_)
