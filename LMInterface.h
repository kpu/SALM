//#include <config.h>
//#include <fortify.h>

#ifndef __LM__
#define __LM__

#include <string>
#include <vector>
#include <map>

typedef unsigned int LMWordIndex;

using namespace std;


//! initial LM state
#define CLM_INITIALSTATE 0

/* Normally the code runs on 64-bit, so sizeof(long long) == sizeof(int) == 8.
 * The code assumes in bitshift operations sizeof(CLMstate) == 8.  I don't know
 * why, but somebody changed this from long long to int, breaking 32 bit.  I
 * reverted it to long long. --kheafiel
 */
typedef unsigned long long CLMState;

//typedef unsigned int  LMWordIndex;
//typedef unsigned int  CLMState;

class CLM;
class CLMSentenceState;
class CLMTransition;



/**
   class CLMTransition

   this class was addded to deal with a LM server, where we want to
   get the probability for an entire word sequence in one call,
   to avoid communication overhead.

   The transition is from one state over a number of words (given
   by their index, into a new state, generating the probability.
*/

class CLMTransition
{
public:
  CLMState OldState;
  vector< LMWordIndex > Words;
  CLMState NewState;
  double Prob;
};




/** For each sentence an instance of this class is needed. */
class CLMSentenceState
{

public:
  virtual ~CLMSentenceState();


  //! Get initial state
  virtual CLMState BeginOfSentenceState();


  //! Get the highest order n-gram found in the corpus for a current state
  //! and the next word:  // GH ADD!
  virtual int HighestOrderNGram( const CLMState LMState, const LMWordIndex WordIndex );
  virtual int HighestOrderNGramEnd( CLMState lmstate );


  //! Get the probability of sentence end.
  virtual double ProbEnd( CLMState LMState );

  //! Get the logarithmic probability of sentence end.
  virtual double LogProbEnd( CLMState LMState ) = 0;


  //! Get the probability of a single word.
  virtual double Prob( const CLMState LMState, const LMWordIndex WordIndex );

  //! Get the logarithmic probability of a single word.
  virtual double LogProb( CLMState LMState, const LMWordIndex WordIndex ) = 0;

  //! Append a word to the state.
  virtual CLMState AppendWord( CLMState LMState, const LMWordIndex WordIndex ) = 0;



  //! Get the logarithmic probability of a single word, also return the new state.
  virtual double LogProb( CLMState LMState, const LMWordIndex WordIndex, CLMState &NewLMState );

  //! Get the logarithmic probability of a word sequence, also return the new state.
  //virtual double LogProb( CLMState LMState, const vector< string > Words, CLMState &NewLMState );
  virtual double LogProb( CLMState LMState, const vector< LMWordIndex > WordIndices, CLMState &NewLMState );


  //! The same, but now with a LM transitions
  virtual void LogProb( CLMTransition &LMTransition );


  //! Get the logarithmic probability of the sentence end, with a LMTransition
	virtual double LogProbEnd( CLMTransition &LMTransition );


  //! get many probabilities and new LM states in one call - for batch mode LM
  virtual void LogProbs( vector< CLMTransition > &Transitions );
  

  /** Get N probabilities for N LM states; used for batch mode access to the LM.
      Provide the old LM states and the WordIndexs, return the probabilities and new states */
  virtual void LogProbs( CLMState OldLMStates[],
                         LMWordIndex WordIndexIndices[],
                         double LogProbs[],
                         CLMState NewLMStates[],
                         unsigned int N );


  


  // the following methods are for a chart parser based decoder
  
  //! Get log prob and initial states (left and right) for a word sequence
  virtual double LogProb( vector< string > Words, CLMState& LMStateLeft, CLMState& LMStateRight );
  //virtual double LogProb( vector< LMWordIndex > Words, CLMState& LMStateLeft, CLMState& LMStateRight );

  /** Get the logarithmic probability by extending lmstate by words
	  which led to the creation of lmstate2
	  update the outer lmstates, which is necessary for word sequences
	  shorter than the history length */
  virtual double LogProbMerge( CLMState &LMStateLeftLeft, CLMState LMStateLeftRight,
                               CLMState LMStateRightLeft, CLMState &LMStateRightRight);


  //! Release a LMState that's no longer needed.
  virtual void DeleteState( CLMState LMState );



  // The following methods needn't be overloaded, but may be
  // overloaded to improve performance:

  /** Get the probabilities of all WordIndexs. The result is written into
      the array provided by the caller which must have size of vocabulary. */
  virtual void AllProb( CLMState LMState, double p[] );

  /** Get the logarithmic probabilities of all WordIndexs. The result is
      written into the array provided by the caller which must have
      size of vocabulary. */
  virtual void AllLogProb( CLMState LMState, double logp[] );

  /** Get the first N most likely WordIndexs. The WordIndexindices of the N best 
      WordIndexs and the probabilities are written into the arrays provided 
      by the caller which must have size N. */
  virtual void NBestProb( CLMState LMState, unsigned int N, 
                          LMWordIndex WordIndices[], double nbestp[] );

  /** Get the first N most likely WordIndexs. The WordIndexindices of the N best
      WordIndexs and the logarithmic probabilities are written into the
      arrays provided by the caller which must have size N. */
  virtual void NBestLogProb( CLMState LMState, unsigned int N,
                             LMWordIndex WordIndices[], 
                             double nbestlogp[]);



protected:
  CLM *lm;

  void qsort(double a[], int l, int r, LMWordIndex index[]) const;
};



//! This class represents a (left-to-right) language model.
class CLM
{
public:
  // the constructors for CLM's deferred classed should look like this:
  // CLM(const char *paramfilename); // construct a LM with parameters read
  // from parameter file
  virtual ~CLM();

  //! create new CLMSentenceState
  virtual CLMSentenceState * NewSentence() const =0;

  // delete CLMSentenceState 
  // if a LM's NewSentence does not use 'new' this function must be overloaded !!
  virtual void DeleteSentence(CLMSentenceState* st) const  ;

  virtual LMWordIndex VocabularySize() const =0;

  //! return VocabularyIndex of WordIndex
  virtual LMWordIndex GetVocabularyIndex( string WordIndex );

  //! the constant version for classes who make const calls
  virtual LMWordIndex GetVocabularyIndex( string WordIndex ) const;


protected:
  // map WordIndexs of corpus on idxs
  map< string, LMWordIndex > GetVocabularyIndexMap;

};




#endif
