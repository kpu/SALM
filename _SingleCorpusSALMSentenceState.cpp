// _SingleCorpusSALMSentenceState.cpp: implementation of the C_SingleCorpusSALMSentenceState class.
//
// $Rev: 2299 $
// $LastChangedDate: 2007-06-28 20:23:27 -0400 (Thu, 28 Jun 2007) $
//
//////////////////////////////////////////////////////////////////////

#include "_SingleCorpusSALMSentenceState.h"
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C_SingleCorpusSALMSentenceState::C_SingleCorpusSALMSentenceState(const C_SingleCorpusSALM * salm)
{
	this->salm = (C_SingleCorpusSALM *) salm;
	this->sentEndVocId = this->salm->returnSentEndVocId();
}

C_SingleCorpusSALMSentenceState::~C_SingleCorpusSALMSentenceState()
{

}

//start a new sentence now, clear up the sentence LM state
CLMState C_SingleCorpusSALMSentenceState::BeginOfSentenceState()
{
	this->resetLmStates();
	this->initialLmState();		

	return 0;
}

void C_SingleCorpusSALMSentenceState::initialLmState()
{
	//add sentence start
	S_LMStateInfo sentStartNode;
	sentStartNode.locationInCorpus.posInCorpus = 1;	//if corpus is indexed correctly position 1 should be <s>
	sentStartNode.locationInCorpus.len = 1;
	sentStartNode.cachedNextWordExtension.clear();

	this->allLMStates.push_back(sentStartNode);
	this->ngramLocation2LmStateId.insert(make_pair(sentStartNode.locationInCorpus, 0));
}

void C_SingleCorpusSALMSentenceState::resetLmStates()
{	
	this->allLMStates.clear();
	this->ngramLocation2LmStateId.clear();
}

void C_SingleCorpusSALMSentenceState::LogProb(CLMTransition & LMTransition)
{
	double totalCost = 0.0;
	CLMState currentState, nextState;

	currentState = LMTransition.OldState;
	int totalWords = LMTransition.Words.size();
	for(int i=0; i<totalWords; i++){
		double logProb = this->LogProb(currentState, LMTransition.Words[i], nextState);
		totalCost+= logProb;
		currentState = nextState;
	}

	LMTransition.Prob = totalCost;
	LMTransition.NewState = currentState;

}

double C_SingleCorpusSALMSentenceState::LogProb(CLMState lmstate, const LMWordIndex nextWord, CLMState & nextstate)
{
	if(lmstate>=this->allLMStates.size()){
		cerr<<"Invalid LM State: "<<lmstate<<endl;
		exit(-1);
	}

	//first check if we have already seen this next word before
	map< IndexType, S_CachedLmInfo>::iterator iterNextWordExtensionCache;
	iterNextWordExtensionCache = this->allLMStates[lmstate].cachedNextWordExtension.find( nextWord );

	if(iterNextWordExtensionCache==this->allLMStates[lmstate].cachedNextWordExtension.end()){ //we haven't seen this lmstate+word yet

		//search for it in the corpus
		S_NgramLocationInCorpus correspondingNgramLocation = this->allLMStates[lmstate].locationInCorpus;
		S_NgramLocationInCorpus updatedNgramLocation;
		
		double logProb = this->salm->LogProb(
			correspondingNgramLocation.posInCorpus, 
			correspondingNgramLocation.len,
			nextWord, 
			updatedNgramLocation.posInCorpus,
			updatedNgramLocation.len
			);

		int updatedLmStateId;
		map<S_NgramLocationInCorpus, int, lt_ngramLocationInCorpus>::iterator iterNgramLocation2LmStateId;
		iterNgramLocation2LmStateId = this->ngramLocation2LmStateId.find(updatedNgramLocation);
		if(iterNgramLocation2LmStateId==this->ngramLocation2LmStateId.end()){	//this updated lm state does not exist yet
			S_LMStateInfo newLmStateNode;

			newLmStateNode.locationInCorpus = updatedNgramLocation;
			newLmStateNode.cachedNextWordExtension.clear();
			
			this->allLMStates.push_back(newLmStateNode);
			updatedLmStateId = this->allLMStates.size() -1 ;
			this->ngramLocation2LmStateId.insert(make_pair(updatedNgramLocation, updatedLmStateId));
		}
		else{
			updatedLmStateId = iterNgramLocation2LmStateId->second;
		}

		//cache this
		S_CachedLmInfo cachedLmInfo;
		cachedLmInfo.logProb = logProb;
		cachedLmInfo.nextState = updatedLmStateId;

		this->allLMStates[lmstate].cachedNextWordExtension.insert(make_pair(nextWord, cachedLmInfo));

		//updated next state
		nextstate = updatedLmStateId;
		
		return logProb;
	}

	nextstate = iterNextWordExtensionCache->second.nextState;

	return iterNextWordExtensionCache->second.logProb;
}

double C_SingleCorpusSALMSentenceState::LogProb(CLMState lmstate, const LMWordIndex nextWord )
{
  CLMState NewState;
  return LogProb( lmstate, nextWord, NewState );
}

double C_SingleCorpusSALMSentenceState::LogProbEnd(CLMState lmstate)
{
	CLMState dummyNextState;
	return this->LogProb(lmstate, this->sentEndVocId, dummyNextState);
}

double C_SingleCorpusSALMSentenceState::LogProbEnd(CLMTransition & LMTransition)
{
	CLMState dummyNextState;
	return this->LogProb(LMTransition.OldState, this->sentEndVocId, dummyNextState);
}

CLMState C_SingleCorpusSALMSentenceState::AppendWord( CLMState LMState, const LMWordIndex WordIndex )
{
	CLMState nextState;

	this->LogProb(LMState, WordIndex, nextState);

	return nextState;
}
