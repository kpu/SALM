// _SingleCorpusSALM.cpp: implementation of the C_SingleCorpusSALM class.
//
// $Rev: 2296 $
// $LastChangedDate: 2007-06-27 15:09:33 -0400 (Wed, 27 Jun 2007) $
//
//////////////////////////////////////////////////////////////////////

#include "_SingleCorpusSALM.h"
#include "_SingleCorpusSALMSentenceState.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
C_SingleCorpusSALM::C_SingleCorpusSALM()
{

}

C_SingleCorpusSALM::~C_SingleCorpusSALM()
{
	
}

C_SingleCorpusSALM::C_SingleCorpusSALM(const char *corpusFileNameStem, unsigned int ngram_length)
{
  
	//-----------------------------------------------------------------------------
	//reading parameters
//	char paraName[1024];
	int maxFreqForDiscounting=-1;

	this->interpolationStrategy = 'e';		//default interpolation strategy: equally weighted n-gram conditional prob
	
	unsigned int stemLen = 0;		//default value, take whole word. If negative: take last n characters, if positive: take first n characters
       	
	this->maxLenOfNgramConsidered = ngram_length;         // default value; consider up to 5 words
		
	bool useMemMapping = false;		//default
	bool loadLevel1Bucket = false;
		
/*	while(!cfgFile.eof()){
		cfgFile>>paraName;

		if(strcmp(paraName,"CORPUS")==0){
			cfgFile>>corpusFileNameStem;
		}		
		else if(strcmp(paraName,"N")==0){
			cfgFile>>this->maxLenOfNgramConsidered;
		}
		else if(strcmp(paraName,"MAX_FREQ_DISC")==0){
			cfgFile>>maxFreqForDiscounting;
		}
		else if(strcmp(paraName,"STEM_LEN")==0){
			cfgFile>>stemLen;
		}
		else if(strcmp(paraName,"INTERPOLATION_STRATEGY")==0){
			cfgFile>>this->interpolationStrategy;
		}
		else if(strcmp(paraName,"MEM_MAPPING")==0){
			cfgFile>>useMemMapping;
		}
		else if(strcmp(paraName,"LOAD_LEVEL1_BUCKET")==0){
			cfgFile>>loadLevel1Bucket;
		}
		

		paraName[0]=0;
		
	}*/
		
	
	if(maxFreqForDiscounting<0){
		this->applyDiscounting = false;
	}
	else{
		this->applyDiscounting = true;
		this->saSearchObj.setParam_maxNForDiscounting(maxLenOfNgramConsidered);
		this->saSearchObj.setParam_maxFreqForDiscounting(maxFreqForDiscounting);
	}

	this->saSearchObj.setParam_memMapping(useMemMapping);
	this->saSearchObj.setParam_loadLevel1Bucket(loadLevel1Bucket);	//if there exist a file: corpusName.sa_level1b_v6 then we could load it to save time
	this->saSearchObj.loadData_v6(corpusFileNameStem, true, true, this->applyDiscounting, stemLen);	//no need of the voc, do not load the offset
	this->corpusSize = double (this->saSearchObj.returnCorpusSize());
	
	
	char idVocFileName[1024];
	sprintf(idVocFileName, "%s.id_voc", corpusFileNameStem);
	this->idVoc = new C_IDVocabulary(idVocFileName, stemLen);

	this->sentEndVocId = idVoc->returnId("_END_OF_SENTENCE_");

	if(this->sentEndVocId==0){	//could not find the voc ID for </s>
		cerr<<"VocId for _END_OF_SENTENCE_ could not be found in the vocabulary! Quit\n";
		exit(0);
	}

}

void C_SingleCorpusSALM::setParam_interpolationStrategy(char interpolationStrategy)
{
	this->interpolationStrategy = interpolationStrategy;
}

void C_SingleCorpusSALM::setParam_maxLenOfNgramConsidered(int maxLenOfNgramConsidered)
{
	this->maxLenOfNgramConsidered = maxLenOfNgramConsidered;
}

CLMSentenceState * C_SingleCorpusSALM::NewSentence() const
{
	return new C_SingleCorpusSALMSentenceState(this);

}


//extend the current matched n-gram with next word, calculate the prob and update the updated range
//the n-gram is represented by its position in the suffix array and the length
double C_SingleCorpusSALM::LogProb(TextLenType currentMatchStart, unsigned char currentMatchLen, IndexType nextWord, TextLenType &updatedMatchingStart, unsigned char &updatedMatchingLen) const
{
	double logProb;

	//else, need to calculate the log prob from searching SA
	double * freqTable = (double *) malloc(sizeof(double)*2*(this->maxLenOfNgramConsidered));
	memset(freqTable, 0, 2*this->maxLenOfNgramConsidered*sizeof(double));

	this->saSearchObj.calcNgramMatchingInfoTokenFreqOnlyExtendingCurrentMatch(currentMatchStart, currentMatchLen, nextWord, this->maxLenOfNgramConsidered, freqTable, updatedMatchingStart, updatedMatchingLen);

	logProb = this->calcLogProb(freqTable);

	free(freqTable);

	return logProb;

}

double C_SingleCorpusSALM::calcLogProb(const double *freq) const
{
	switch(this->interpolationStrategy){
	case 'e':
		return this->calcLogProb_equalWeightedInterpolation(freq);
		break;
	case 'i':
		return this->calcLogProb_ibmHeuristicInterpolation(freq);
		break;
	default:
		cerr<<"Unknown interpolation strategy!\n";
		exit(0);
	}
}

double C_SingleCorpusSALM::calcLogProb_equalWeightedInterpolation(const double *freq) const
{
	double prob = 0.0;

	
	if(freq[0]>0){

		int i=0;
		bool stillMatched = true;

		while(stillMatched && (i<this->maxLenOfNgramConsidered)){
			if(freq[2*i]>0){
				prob+=freq[2*i]/freq[2*i+1];
			}
			else{
				stillMatched = false;
			}

			i++;
		}

		return log(prob/(double)this->maxLenOfNgramConsidered);
	}
	else{	//unknown word
		return SALM_LOG_PROB_UNK;
	}
}

double C_SingleCorpusSALM::calcLogProb_ibmHeuristicInterpolation(const double *freq) const
{
	double prob = 0.0;
	if(freq[0]==0){	//unknown word
		return SALM_LOG_PROB_UNK;
	}

	double remainingWeightSum = 1.0;

	//find the first non-zero match
	int i = this->maxLenOfNgramConsidered - 1;

	while(freq[2*i]==0){	//will stop for sure because freq[0]!=0
		i--;
	}

	for(int j=i;j>=0;j--){
		//for (j+1)-gram
		double historyFreq = freq[2*j+1];
		double logHistoryFreq = log(historyFreq);
		if(logHistoryFreq>1){
			logHistoryFreq = 1.0;	//cap it to 1
		}

		double reliability = 0.1*logHistoryFreq+0.3;	//heuristics for reliability of the history
		double adjustedWeights = remainingWeightSum *  reliability;
		
		prob+=adjustedWeights * freq[2*i]/freq[2*i+1];

		remainingWeightSum -= adjustedWeights;
	}

	return log(prob);	
}

// GH ADD!
int C_SingleCorpusSALM::calcNGramOrder(const double *freq) const
{
	if(freq[0]>0){

		int i=0;
		int maxi = 0;

		while(i<this->maxLenOfNgramConsidered){
			if(freq[2*i]>0){
				maxi = i + 1;
			} else {
				break;
			}

			i++;
		}

		return maxi;
	}
	else{	//unknown word
		return 0;
	}
}


LMWordIndex C_SingleCorpusSALM::returnSentEndVocId()
{
	return this->sentEndVocId;
}

LMWordIndex C_SingleCorpusSALM::VocabularySize() const
{
	return this->idVoc->getSize();
}
