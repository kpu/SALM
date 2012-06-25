/**
* Revision $Rev: 2333 $
* Last Modified $LastChangedDate: 2007-01-15 03:49:17 -0500 (Mon, 15 Jan 2007) $
**/
#if !defined(_SA_common_h)
#define _SA_common_h

#include <math.h>

#include "text_length.h"

//Typedefs:
typedef unsigned int IndexType;
typedef unsigned int TextLenType;
typedef unsigned short int SearchLenType;
typedef double WordPairValueType;

//    Typedef for parsing tree (from shared.h):
typedef unsigned short NodeIdInParsingTreeType;		//each tree can have at most 65536 nodes

//Constants:
const int SIZE_ONE_READ = 16384;	//when loading the data, each I/O read in SIZE_ONE_READ data points
const int MAX_TOKEN_LEN = 1024;		//length of the longest word
const int NUMBER_OF_RESERVED_WORDS_IN_VOC = 100;

//  Stuff from (older) shared.h:
const unsigned int MAX_VALID_TOKEN_LEN = 30;
const unsigned int MAX_SENT_SIZE = 4096;
const double NORMALIZED_BIG_N_CONST	 = 100000.0;
const double SMALL_FLOAT_VALUE =  1e-10;
const double SMALL_DOUBLE_VALUE = 1e-10;
const unsigned int NULL_TGT_WORD_POSITION = (unsigned int) -100;
const unsigned int NULL_SRC_WORD_POSITION = (unsigned int) -100;
const unsigned short NULL_TGT_WORD_POSITION_SHORT = (unsigned short) -100;
const unsigned short NULL_SRC_WORD_POSITION_SHORT = (unsigned short) -100;

//    Constants used in infoNet:
const unsigned int INFO_NET_MAX_NUMBER_ITEMS_EACH_SAMPLE = 10000;	//to speed up sample, at most this number of items to be sampled
const double INFO_NET_SAMPLE_RATE = 0.01;
const unsigned int INFO_NET_SAMPLE_TIMES = 10;

//  For language modeling:
const double SALM_PROB_UNK = 0.00000000023283064365386962890625; // 1/4G
const double SALM_LOG_PROB_UNK = log(SALM_PROB_UNK);
const double SALM_LOG_0 = -20;

/**
* \ingroup scan
**/
typedef struct s_nGramScanningInfoElement
{
	IndexType vocId;
	TextLenType freqThreshForOutput;
	TextLenType freqSoFar;
}S_nGramScanningInfoElement;

#endif

