// _SingleCorpusSALMSentenceState.h: interface for the C_SingleCorpusSALMSentenceState class.
//
// $Rev: 2296 $
// $LastChangedDate: 2007-06-27 15:09:33 -0400 (Wed, 27 Jun 2007) $
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__SINGLECORPUSSALMSENTENCESTATE_H__85191BDA_ADFF_4B93_A513_34811BD796E0__INCLUDED_)
#define AFX__SINGLECORPUSSALMSENTENCESTATE_H__85191BDA_ADFF_4B93_A513_34811BD796E0__INCLUDED_

#include "LMInterface.h"
#include "_SingleCorpusSALM.h"

typedef struct s_cachedLmInfo{
	int nextState;
	double logProb;
}S_CachedLmInfo;

typedef struct s_NgramLocationInCorpus{
	TextLenType posInCorpus;
	unsigned char len;
}S_NgramLocationInCorpus;

typedef struct s_lmStateInfo{
	S_NgramLocationInCorpus locationInCorpus;
	map<IndexType, S_CachedLmInfo> cachedNextWordExtension;	//cached information of this LMState extended by the next word
}S_LMStateInfo;


struct lt_ngramLocationInCorpus
{
  bool operator()(S_NgramLocationInCorpus a, S_NgramLocationInCorpus b) const{
		if(a.posInCorpus<b.posInCorpus){
			return true;
		}

		if(a.posInCorpus>b.posInCorpus){
			return false;
		}

		if(a.len<b.len){
			return true;
		}

		return false;	
	}
};
	
using namespace std;

class C_SingleCorpusSALMSentenceState : public CLMSentenceState  
{
public:
	void setParam_useNextWordCache(bool useNextWordCache);
	CLMState BeginOfSentenceState();

	//! Get the logarithmic probability of a single word or phrase
	void LogProb( CLMTransition &LMTransition );
	double LogProb(CLMState lmstate, const LMWordIndex nextWord, CLMState & nextstate);	
	virtual double LogProb( CLMState LMState, const LMWordIndex WordIndex );
	
	//! Get the logarithmic probability of sentence end.
	double LogProbEnd(CLMState lmstate);
	virtual double LogProbEnd(CLMTransition &LMTransition );

	//just need to declare these functions as used in the interface
	CLMState AppendWord( CLMState LMState, const LMWordIndex WordIndex );	

	C_SingleCorpusSALMSentenceState(const C_SingleCorpusSALM * salm);	
	virtual ~C_SingleCorpusSALMSentenceState();

private:
	void resetLmStates();
	void initialLmState();	

	vector<S_LMStateInfo> allLMStates;

	map<S_NgramLocationInCorpus, int, lt_ngramLocationInCorpus> ngramLocation2LmStateId;

	C_SingleCorpusSALM * salm;
	LMWordIndex sentEndVocId;
};

#endif // !defined(AFX__SINGLECORPUSSALMSENTENCESTATE_H__85191BDA_ADFF_4B93_A513_34811BD796E0__INCLUDED_)
