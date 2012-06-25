//#include <config.h>
//#include <fortify.h>

#include <math.h>
#include <string.h>

#include "LMInterface.h"


CLMSentenceState::~CLMSentenceState()
{}



CLMState
CLMSentenceState::BeginOfSentenceState()
{
  return CLM_INITIALSTATE;
}



//! GH ADD!
int
CLMSentenceState::HighestOrderNGram( const CLMState,
									 const LMWordIndex WordIndex )
{
	//This is the default implementation:
	return -1;
}



//! GH ADD!
int
CLMSentenceState::HighestOrderNGramEnd( CLMState lmstate )
{
	//This is the default implementation:
	return -1;
}



double
CLMSentenceState::Prob( const CLMState LMState, const LMWordIndex WordIndex ) 
{
  return exp( LogProb( LMState, WordIndex ) );
} 



//! get many probabilities and new LM states in one call - for batch mode LM
void
CLMSentenceState::LogProbs( vector< CLMTransition > &Transitions )
{
  for ( int i = 0; i <= Transitions.size(); i++ ) {
    LogProb( Transitions[ i ] );
  }

}



void
CLMSentenceState::LogProb( CLMTransition &Transition )
{
  CLMState LMState = Transition.OldState;
  vector< LMWordIndex > &Words = Transition.Words;
  float SumLogProb = 0.0;

  for ( int i = 0; i < Words.size(); i++ ) {
    SumLogProb += LogProb( LMState, Words[ i ] );
    LMState = AppendWord( LMState, Words[ i ] );
  }

  Transition.Prob = SumLogProb;
  Transition.NewState = LMState;


}
 


double
CLMSentenceState::LogProb( vector< string > Words, CLMState& LMStateLeft, CLMState& LMStateRight )
{
  LMStateLeft  = CLM_INITIALSTATE;
  LMStateRight = CLM_INITIALSTATE;
  return 0.0;
}



double
CLMSentenceState::LogProbMerge( CLMState &LMStateLeftLeft, CLMState LMStateLeftRight,
                                CLMState LMStateRightLeft, CLMState &LMStateRightRight)
{
  return 0.0;
}


double
CLMSentenceState::ProbEnd( CLMState LMState )
{
  return exp( LogProbEnd( LMState ) );
}



double
CLMSentenceState::LogProbEnd( CLMTransition &LMTransition )
{
  LMTransition.Prob = LogProbEnd( LMTransition.OldState );
  return LMTransition.Prob;
}



void
CLMSentenceState::DeleteState( CLMState LMState )
{}



void
CLMSentenceState::AllProb( CLMState LMState, double Probs[] )
{
  LMWordIndex VocSize = lm->VocabularySize();

  AllLogProb( LMState, Probs );

  for( LMWordIndex i = 0; i <= VocSize; ++i) {
    Probs[ i ] = exp( Probs[ i ] );
  }
}



inline double
CLMSentenceState::LogProb( CLMState LMState, const LMWordIndex WordIndex, CLMState &NewLMState )
{

  NewLMState = AppendWord( LMState, WordIndex );
  return LogProb( LMState, WordIndex );
}



inline double
CLMSentenceState::LogProb( CLMState LMState, const vector< LMWordIndex > WordIndices, CLMState &NewLMState )
{
  double LogP = 0.0;

  for ( int i = 0; i < WordIndices.size(); i++ ) {
    LogP += LogProb( LMState, WordIndices[ i ], NewLMState );
    LMState = NewLMState;
    //cout << "word index = " << WordIndices[ i ] << endl;
    //cout << "LogP = " << LogP << " NewLMState = " << NewLMState << endl;
  }
  
  return LogP;
}



void
CLMSentenceState::AllLogProb( CLMState LMState, double LogProbs[] )
{
  LMWordIndex VocSize = lm->VocabularySize();

  for( LMWordIndex i = 0; i <= VocSize; ++i )
    LogProbs[ i ] = LogProb( LMState, i );

}



void
CLMSentenceState::NBestProb( CLMState LMState, unsigned int N, 
                             LMWordIndex WordIndices[], double NBestProbs[] )
{
  unsigned int i;
  
  NBestLogProb( LMState, N, WordIndices, NBestProbs );

  for( i = 0; i < N; ++i ) {
    NBestProbs[ i ] = exp( NBestProbs[ i ] );
  }
}



void
CLMSentenceState::NBestLogProb( CLMState LMState, unsigned int N, 
                                LMWordIndex WordIndices[], 
                                double NBestLogProbs[] )
{
  LMWordIndex VocSize = lm->VocabularySize();
  LMWordIndex *idx = new LMWordIndex[ VocSize + 1 ];
  double *p = new double[ VocSize + 1 ];

  LMWordIndex i; 

  for( i = 0;i <= VocSize; ++i ) {
    p[ i ] = LogProb( LMState, i );
    idx[ i ] = i;
  }

  qsort(p, 0, VocSize, idx );

  if ( N > (unsigned) VocSize ) 
    N = VocSize + 1;

  memcpy( NBestLogProbs, p, ( (N+1) * sizeof( double ) ) );
  memcpy( WordIndices, idx,( (N+1) * sizeof( LMWordIndex ) ) );

  delete[] p;
  delete[] idx;
}



/** Get N probabilities for N LM states; used for batch mode access to the LM.
    Provide the old LM states and the words, return the probabilities and new states */
void
CLMSentenceState::LogProbs( CLMState OldLMStates[],
                            LMWordIndex WordIndices[], 
                            double LogProbs[],
                            CLMState NewLMStates[],
                            unsigned int N )
{
  for ( int n = 0; n < N; n++ ) {
    LogProbs[ n ] = LogProb( OldLMStates[ n ], WordIndices[ n ] );
    NewLMStates[ n ] = AppendWord(  OldLMStates[ n ], WordIndices[ n ] );
  }
}



				   
void
CLMSentenceState::qsort( double a[], int l, int r, LMWordIndex index[] ) const
{
  double v, t;
  int i, j;
  LMWordIndex v_idx, t_idx;

  if ( r > l ) {
    v = a[ r ];
    v_idx = index[ r ];
    i = l - 1;
    j = r;
    
    for(;;) {
      while( a[ ++i ] > v );
      while( a[ --j ] < v);

      if ( i >= j )
        break;

      t = a[ i ];
      t_idx = index[ i ];
      a[ i ] = a[ j ];   
      index[ i ] = index[ j ];
      a[ j ] = t;
      index[ j ] = t_idx;
    }

    t = a[ i ];
    t_idx = index[ i ];
    a[ i ] = a[ r ];
    index[ i ] = index[ r ];
    a[ r ] = t;
    index[ r ] = t_idx;
    
    qsort( a, l, i - 1, index );
    qsort( a, i + 1, r, index );
  }

}



CLM::~CLM()
{ }
 


void
CLM::DeleteSentence( CLMSentenceState* SentenceState ) const
{
  if ( SentenceState != NULL)
    delete SentenceState;
}



LMWordIndex
CLM::GetVocabularyIndex( string Word )
{ 
  return GetVocabularyIndexMap[ Word ]; 
}



LMWordIndex
CLM::GetVocabularyIndex( string Word ) const
{ 
  return GetVocabularyIndexMap.find( Word )->second;
}
