//
// $Rev: 2272 $
// $LastChangedDate: 2007-06-06 18:45:49 -0400 (Wed, 06 Jun 2007) $
//
#include "_SuffixArraySearch.h"
#include "malloc.h"
#include "time.h"

#include <sys/mman.h>
#include "stdio.h"
#include "stdlib.h"
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

extern int SHOW_DEBUG_INFO;

C_SuffixArraySearch::C_SuffixArraySearch()
{
	this->level1Buckets = NULL;
	this->loadLevel1Bucket = false;	//by default construct the level1 bucket
	this->noVocabulary = false;	//by default, still load the vocabulary
	this->withDiscounting = false;	//by default, do not build discounting map

	this->reportMaxOccurrenceOfOneNgram = -1;
	this->shortestUnitToReport = 1;
	this->longestUnitToReport = -1;	//no constraint 
	
	this->suffixArrayIsVersion6 = false;	//default, not version 6

	this->maxFreqForDiscounting = 800;
	this->maxNForDiscounting = 6;
	this->discountingMap = NULL;
}

C_SuffixArraySearch::~C_SuffixArraySearch()
{
	if(this->level1Buckets!=NULL){
		free(this->level1Buckets);
	}

	if(this->discountingMap!=NULL){
		free(this->discountingMap);
	}
}

void C_SuffixArraySearch::setParam_highestFreqThresholdForReport(int highestFreqThresholdForReport)
{
	this->highestFreqThresholdForReport = highestFreqThresholdForReport;
}

void C_SuffixArraySearch::setParam_longestUnitToReport(int longestUnitToReport)
{
	this->longestUnitToReport = longestUnitToReport;
}

void C_SuffixArraySearch::setParam_reportMaxOccurrenceOfOneNgram(int reportMaxOccurrenceOfOneNgram)
{
	this->reportMaxOccurrenceOfOneNgram = reportMaxOccurrenceOfOneNgram;
}

void C_SuffixArraySearch::setParam_shortestUnitToReport(int shortestUnitToReport)
{
	this->shortestUnitToReport = shortestUnitToReport;
}

void C_SuffixArraySearch::setParam_maxNForDiscounting(int maxNForDiscounting)
{
	this->maxNForDiscounting = maxNForDiscounting;
}

void C_SuffixArraySearch::setParam_maxFreqForDiscounting(int maxFreqForDiscounting)
{
	this->maxFreqForDiscounting = maxFreqForDiscounting;
}

void C_SuffixArraySearch::setParam_memMapping(bool useMemMapping)
{
	this->useMemMapping = useMemMapping;
}

void C_SuffixArraySearch::setParam_loadLevel1Bucket(bool loadLevel1Bucket)
{
	this->loadLevel1Bucket = loadLevel1Bucket;
}

void C_SuffixArraySearch::loadData(const char *fileNameStem, bool noVoc, unsigned int stem_len)
{
	long ltime1, ltime2;

	this->noVocabulary = noVoc;

	char tmpString[1000];

	//the order of loading the data is important, do not change	    
	if(! this->noVocabulary){
		time( &ltime1 );
		cerr<<"Loading Vocabulary...\n";
		sprintf(tmpString,"%s.id_voc",fileNameStem);
		this->loadVoc(tmpString, stem_len);
		time( &ltime2);
		cerr<<"Vocabulary loaded in "<<ltime2-ltime1<<" seconds.\n";
	}
	

	time( &ltime1 );
	cerr<<"Loading corpus...\n";
	sprintf(tmpString,"%s.sa_corpus",fileNameStem);	
	this->loadCorpusAndInitMem(tmpString);
	time( &ltime2);
	cerr<<"Corpus loaded in "<<ltime2-ltime1<<" seconds.\n";
	
	time( &ltime1 );
	cerr<<"Loading suffix...\n";
	sprintf(tmpString,"%s.sa_suffix",fileNameStem);
	this->loadSuffix(tmpString);
	time( &ltime2);
	cerr<<"Suffix loaded in "<<ltime2-ltime1<<" seconds.\n";

	time( &ltime1 );
	cerr<<"Loading offset...\n";
	sprintf(tmpString,"%s.sa_offset",fileNameStem);
	this->loadOffset(tmpString);
	time( &ltime2);
	cerr<<"Offset loaded in "<<ltime2-ltime1<<" seconds.\n";
	
}

void C_SuffixArraySearch::loadData_v6(const char *fileNameStem, bool noVoc, bool noOffset, bool withDiscounting, unsigned int stem_len)
{
	long ltime1, ltime2;

	this->noVocabulary = noVoc;
	this->noOffset = noOffset;
	this->withDiscounting = withDiscounting;
	
	this->suffixArrayIsVersion6 = true;

	char tmpString[1000];

	//the order of loading the data is important, do not change
	if(! this->noVocabulary){
		time( &ltime1 );
		cerr<<"Loading Vocabulary...\n";
		sprintf(tmpString,"%s.id_voc",fileNameStem);
		this->loadVoc(tmpString, stem_len);
		time( &ltime2);
		cerr<<"Vocabulary loaded in "<<ltime2-ltime1<<" seconds.\n";
	}
	
	time( &ltime1 );
	cerr<<"Loading corpus...\n";
	sprintf(tmpString,"%s.sa_corpus_v6",fileNameStem);	
	this->loadCorpusAndInitMem(tmpString);
	time( &ltime2);
	cerr<<"Corpus loaded in "<<ltime2-ltime1<<" seconds.\n";
	
	time( &ltime1 );
	cerr<<"Loading suffix...\n";
	sprintf(tmpString,"%s.sa_suffix_v6",fileNameStem);
	this->loadSuffix(tmpString);
	time( &ltime2);
	cerr<<"Suffix loaded in "<<ltime2-ltime1<<" seconds.\n";
	
	if(this->loadLevel1Bucket){
		time( &ltime1 );
		cerr<<"Loading level1bucket ...\n";
		sprintf(tmpString,"%s.sa_level1b_v6",fileNameStem);
		this->loadLevel1BucketFromFile(tmpString);
		time( &ltime2);
		cerr<<"Level1buckets loaded in "<<ltime2-ltime1<<" seconds.\n";
	}
	else{
		time( &ltime1 );
		cerr<<"Construct the level1bucket ...\n";
		this->buildUpLevel1Bucket();
		time( &ltime2);
		cerr<<"Level1buckets constructed in "<<ltime2-ltime1<<" seconds.\n";
	}

	if(! this->noOffset){
		time( &ltime1 );
		cerr<<"Loading offset...\n";
		sprintf(tmpString,"%s.sa_offset_v6",fileNameStem);
		this->loadOffset(tmpString);
		time( &ltime2);
		cerr<<"Offset loaded in "<<ltime2-ltime1<<" seconds.\n";
	}


	if(this->withDiscounting){
		this->constructDiscountingTable();
	}
}

void C_SuffixArraySearch::loadVoc(const char *filename, unsigned int stem_len)
{
	this->voc =  new C_IDVocabulary(filename, stem_len);
}

void C_SuffixArraySearch::loadCorpusAndInitMem(const char *filename)
{
	unsigned int dwRead = 0;
	FILE *  CorpusInputFile;

	if(this->useMemMapping){
		
		CorpusInputFile = fopen(filename, "rb");
		//first, read the size of the corpus
		dwRead = fread( &(this->corpusSize), sizeof(TextLenType), 1, CorpusInputFile);
		fclose(CorpusInputFile);

		int fd;
		if ((fd = open(filename, O_RDONLY)) == -1) {
			cerr<<"Corpus file: "<<filename<<" does not exist or can not be opened!\n";
			exit(-1);
		}		

		IndexType * tmpCorpusList;
		tmpCorpusList = ((IndexType *) mmap( (caddr_t)0, sizeof(TextLenType)+this->corpusSize*sizeof(IndexType), PROT_READ, MAP_SHARED, fd, 0));

		// check if mmap failde
		if( ((void*) tmpCorpusList) == ((void *) -1) ){
		  cerr<<"Can not map the corpus file!";
		  exit(-1); 
		}

		this->corpus_list = tmpCorpusList+1;	//ignore the corpus size part

	}
	else{	
		
		CorpusInputFile = fopen(filename, "rb");

		if(!CorpusInputFile){
			cerr<<"Corpus file: "<<filename<<" does not exist or can not be opened!\n";
			exit(-1);
		}
		
		//first, read the size of the corpus
		dwRead = fread( &(this->corpusSize), sizeof(TextLenType), 1, CorpusInputFile);

		//allocate memory for all data structure
		this->corpus_list = (IndexType *) malloc(sizeof(IndexType)*this->corpusSize);
		if(! this->corpus_list){
			cerr<<"Can not allocate memory to load the corpus!\n";
			exit(-1);
		}

		this->suffix_list = (TextLenType *) malloc(sizeof(TextLenType)*this->corpusSize);
		if(! this->suffix_list){
			cerr<<"Can not allocate memory to load the suffix!\n";
			exit(0);
		}

		if(! this->noOffset){
			this->offset_list = (unsigned char *) malloc(sizeof(unsigned char)*this->corpusSize);
			if(! this->offset_list){
				cerr<<"Can not allocate memory to load the offset!\n";
				exit(0);
			}
		}

		//read the corpus file
		unsigned int totalRead = 0;
		char * currentPosInCorpusList = (char *) this->corpus_list;
		while(! feof(CorpusInputFile) && (totalRead<this->corpusSize)){
			dwRead = fread( currentPosInCorpusList, sizeof(IndexType), SIZE_ONE_READ, CorpusInputFile);
			totalRead+=dwRead;
			currentPosInCorpusList+=sizeof(IndexType)*dwRead;
		}
		if(totalRead!=this->corpusSize){
			cerr<<"Expecting "<<this->corpusSize<<" words from the corpus, read-in "<<totalRead<<endl;
			exit(0);
		}
		fclose(CorpusInputFile);
	}

	this->sentIdStart = this->corpus_list[0];
	this->vocIdForSentStart = this->corpus_list[1];
	this->vocIdForCorpusEnd = this->corpus_list[this->corpusSize-1];
	this->vocIdForSentEnd = this->corpus_list[this->corpusSize-2];

	//in this corpus, we will have at most sentIdStart-1 word types
	//the index in the array correspond to the vocId, 0 is for <unk> and the last one is for <sentIdStart-1> which is the largest vocId observed in the data
	this->level1Buckets = (S_level1BucketElement *) malloc(sizeof(S_level1BucketElement)* this->sentIdStart);	
	
	//initialize the level1 buckets
	for(IndexType i=0;i<this->sentIdStart;i++){
		this->level1Buckets[i].first = (TextLenType) -1;
		this->level1Buckets[i].last = 0;
	}
}

void C_SuffixArraySearch::loadSuffix(const char *filename)
{
	unsigned int dwRead = 0;
	FILE *  SuffixInputFile = fopen(filename, "rb");
	if(!SuffixInputFile){
		cerr<<"Suffix file: "<<filename<<" does not exist!"<<endl;
		exit(0);
	}

	//first, read in the size of the suffix array
	TextLenType suffixArraySize;
	dwRead = fread( &suffixArraySize, sizeof(TextLenType), 1, SuffixInputFile);
	
	if(suffixArraySize!=this->corpusSize){
		cerr<<"Something wrong, the suffix array size is different from the corpus size.\n";
		cerr<<"Corpus has "<<this->corpusSize<<" words and suffix array reported: "<<suffixArraySize<<endl;
		exit(0);
	}

	if(this->useMemMapping){	//map the suffix array list into memory
		fclose(SuffixInputFile);

		int fd;
		if ((fd = open(filename, O_RDONLY)) == -1) {
			cerr<<"Suffix file: "<<filename<<" does not exist or can not be opened!\n";
			exit(-1);
		}		

		TextLenType * tmpSuffixArrayList;
		tmpSuffixArrayList = ((IndexType *) mmap( (caddr_t)0, sizeof(TextLenType)+this->corpusSize*sizeof(IndexType), PROT_READ, MAP_SHARED, fd, 0));

		// check if mmap failde
		if( ((void*) tmpSuffixArrayList) == ((void *) -1) ){
		  cerr<<"Can not map the suffix file!";
		  exit(-1); 
		}

		this->suffix_list = tmpSuffixArrayList+1;	//ignore the corpus size part
	}
	else{

		//read all the suffix into memory
		unsigned int totalRead = 0;
		char * currentPosInSuffixList = (char *) this->suffix_list;
		while(! feof(SuffixInputFile) && (totalRead<suffixArraySize)){
			dwRead = fread( currentPosInSuffixList, sizeof(TextLenType), SIZE_ONE_READ, SuffixInputFile);
			totalRead+=dwRead;
			currentPosInSuffixList+=sizeof(TextLenType)*dwRead;
		}	
		if(totalRead!=suffixArraySize){
			cerr<<"Expecting "<<suffixArraySize<<" words from the suffix list, read-in "<<totalRead<<endl;
			exit(0);
		}

		fclose(SuffixInputFile);
	}
}

void C_SuffixArraySearch::loadLevel1BucketFromFile(char * filename)
{

	ifstream Level1BucketInputFile;
	Level1BucketInputFile.open(filename, ios::binary);

	if(!Level1BucketInputFile){
		cerr<<"Can not open "<<filename<<" for reading!\n";
		exit(-1);
	}

	//first, read the maxVoc+1 id
	IndexType maxVocIdPlus1;
	Level1BucketInputFile.read((char *) &maxVocIdPlus1, sizeof(IndexType));	//max voc Id in corpus

	if(maxVocIdPlus1!=this->sentIdStart){
		cerr<<"Content in Level1Bucket file does not match with the suffix file\n";
		exit(-1);
	}

	TextLenType startRange;
	TextLenType endRange;
	for(IndexType i=0;i<this->sentIdStart;i++){
		Level1BucketInputFile.read((char *) &startRange, sizeof(TextLenType));
		Level1BucketInputFile.read((char *) &endRange, sizeof(TextLenType));
		this->level1Buckets[i].first = startRange;
		this->level1Buckets[i].last = endRange;
	}

	Level1BucketInputFile.close();
}

void C_SuffixArraySearch::buildUpLevel1Bucket()
{
	//build level-1 bucket
	cerr<<"Initialize level-1 buckets...\n";
	IndexType currentVocId = 0;
	IndexType vocId;
	TextLenType pos;
	TextLenType lastSaIndex = 0;
	
	for(TextLenType i=0; i< this->corpusSize; i++){
		pos = this->suffix_list[i];
		
		//for level1 bucket
		vocId = this->corpus_list[pos];

		if(vocId<this->sentIdStart){	//is a meaningful word type
			if(vocId!=currentVocId){
				this->level1Buckets[currentVocId].last = lastSaIndex;	//for first word which is <unk> this does not matter
				this->level1Buckets[vocId].first = i;
				
				currentVocId=vocId;				
			}

			lastSaIndex = i;
		}	
	}

	//for the last word type
	this->level1Buckets[currentVocId].last = lastSaIndex;
}


void C_SuffixArraySearch::loadOffset(const char *filename)
{
	unsigned int dwRead = 0;
	FILE *  OffsetInputFile = fopen(filename, "rb");
	
	if(!OffsetInputFile){
		cerr<<"Offset file: "<<filename<<" does not exist!"<<endl;
		exit(0);
	}
		
	//first, read the size of the corpus	
	TextLenType offsetListLen;
	dwRead = fread( &offsetListLen, sizeof(TextLenType), 1, OffsetInputFile);	
	if(offsetListLen!=this->corpusSize){
		cerr<<"Text length is inconsistent with the length of the offset.\n";
		exit(0);
	}

	//read all the suffix into memory
	unsigned int totalRead = 0;
	char * currentOffsetListPos = (char *) this->offset_list;
	while(! feof(OffsetInputFile) && (totalRead < offsetListLen)){
		dwRead = fread( currentOffsetListPos, sizeof(unsigned char), SIZE_ONE_READ, OffsetInputFile);
		totalRead+=dwRead;
		currentOffsetListPos+=sizeof(unsigned char)*dwRead;

	}
	if(totalRead!=offsetListLen){
		cerr<<"Expecting "<<offsetListLen<<" words from the offset list, read-in "<<totalRead<<endl;
		exit(0);
	}
	fclose(OffsetInputFile);
	
}

//return 0 if w = text
//return 1 if w < text
//return 2 if w > text
//given that the prefix of lcp words are the same
char C_SuffixArraySearch::comparePhraseWithTextWithLCP(IndexType vocInWord, int lcp, TextLenType posInText) const
{	
	//in previsou versions, it is called: IndexType vocInText = this->accessTextString(posInText+lcp);
	//but in accessTextString() two operations will be applied, since this function is called very often
	//it might slow down the process
	//now access directly, assuming that we will not call it in a stupid way to access out of range positions

	IndexType vocInText = this->corpus_list[posInText+lcp];

	if(vocInWord == vocInText){
		return 0;
	}
	
	if(vocInWord < vocInText){
		return 1;
	}

	return 2;
}

vector<IndexType> C_SuffixArraySearch::convertStringToVocId(const char * sentText)
{
	if(this->noVocabulary){
		cerr<<"Vocabulary not available!\n";
		exit(0);
	}

	vector<IndexType> sentInVocId;

	char tmpToken[MAX_TOKEN_LEN];
	memset(tmpToken,0,MAX_TOKEN_LEN);

	int pos = 0;

	int inputLen = strlen(sentText);

	for(int posInInput = 0; posInInput<inputLen; posInInput++){
		char thisChar = sentText[posInInput];

		if((thisChar==' ')||(thisChar=='\t')){	//delimiters
			if(strlen(tmpToken)>0){
				tmpToken[pos] = '\0';				
				sentInVocId.push_back(this->voc->returnId(tmpToken));
				pos=0;
				tmpToken[pos] = '\0';
			}
		}
		else{
			tmpToken[pos] = thisChar;
			pos++;
			if(pos>=MAX_TOKEN_LEN){	//we can handle it
				fprintf(stderr,"Can't read tokens that exceed length limit %d. Quit.\n", MAX_TOKEN_LEN);
				exit(0);
			}
		}
	}

	tmpToken[pos] = '\0';
	if(strlen(tmpToken)>0){		
		sentInVocId.push_back(this->voc->returnId(tmpToken));
	}

	return sentInVocId;
}


//if know the range where the phrase is, search in this range for it
//position here are all positions in SA, not the positions in the textstring
//
//LCP indicates that all the suffixes in the range has the same prefix with LCP length with the proposed n-gram phrase
//only need to compare the "nextWord" at LCP+1 position
//
//return true if such phrase can be found inside the range, false if not
bool C_SuffixArraySearch::searchPhraseGivenRangeWithLCP(IndexType nextWord, int lcp, TextLenType rangeStartPos, TextLenType rangeEndPos, TextLenType &resultStartPos, TextLenType &resultEndPos) const
{
	TextLenType leftPos, rightPos, middlePos;

	//in case the phrase to be searched is beyond the bucket although the first LCP word is the same as this bucket
	//e.g. range correspondes to [ab, ad], but we are searching for (aa)
	//so first step is to make sure the lcp+next word is still in this range
	if(this->comparePhraseWithTextWithLCP(nextWord, lcp, this->suffix_list[rangeStartPos])==1){
		//phrase+next word < text corresponding rangeStart, we could not find it inside this range
		return false;
	}

	if(this->comparePhraseWithTextWithLCP(nextWord, lcp, this->suffix_list[rangeEndPos])==2){
		//phrase+next word > text corresponding to rangeEnd
		return false;
	}
	
	//now we are sure that text(SA[rangeStart]) <= phrase <= text(SA[rangeEnd])


	//search for left bound ( the pos in text which is the min(text>=w))
	//at any time, Left<w<=Right (actually Left<=w<=Right)
	leftPos = rangeStartPos;
	rightPos = rangeEndPos;	
	while( rightPos > (leftPos+1)){ //at the time when right = left +1, we should stop

		middlePos = (TextLenType)((leftPos + rightPos) / 2);
		if(((leftPos + rightPos) % 2) != 0){			
			middlePos++; //bias towards right
		}

		if(this->comparePhraseWithTextWithLCP(nextWord, lcp, this->suffix_list[middlePos]) != 2 ){ 
			// phrase <= middlePos in Text, go left
			rightPos = middlePos;
		}
		else{
			leftPos = middlePos;	//word > middle, go right
		}

	}
	//in previous implementation, we can gurantee that Left<w, because we take rangeStartPos-- from original range
	//here we can only guarantee that Left<=w, so need to check if Left==w at lcp
	if(this->comparePhraseWithTextWithLCP(nextWord, lcp, this->suffix_list[leftPos])==0){
		resultStartPos = leftPos;
	}
	else{
		resultStartPos = rightPos;
	}

	//search for right bound ( the value which is the max(text<=w))
	//at any time, Left<w<=Right (actually Left<=w<=Right)
	leftPos = rangeStartPos;
	rightPos = rangeEndPos;			
	while( rightPos > (leftPos+1)){	//stop when right = left + 1
		middlePos = (TextLenType) ((leftPos + rightPos) / 2 );	//bias towards left
		
		if(this->comparePhraseWithTextWithLCP(nextWord, lcp, this->suffix_list[middlePos]) != 1 ){ // phrase >= middlePos in Text, go right
			leftPos = middlePos;
		}
		else{
			rightPos = middlePos;	// ==1, phrase < middlePos
		}
	}
	//in previous implementation, we can gurantee that w<Right, because we take rangeEndPos++ from original range
	//here we can only guarantee that w<=Right, so need to check if Right==w at lcp
	if(this->comparePhraseWithTextWithLCP(nextWord, lcp, this->suffix_list[rightPos])==0){
		resultEndPos = rightPos;
	}
	else{
		resultEndPos = leftPos;
	}

	if(resultEndPos>=resultStartPos){
		return true;
	}

	return false;	//could not find this phrase
}


//constructing the n-gram search table
//memory allocated here, remember to free the memory when the table is not needed any more in the 
//calling function
//
//faster than constructNgramSearchTable4Sent because the suffixes in the range given by n-1 gram can 
//guaranteed to have the first n-1 words to be the same as the n-1 gram
//only needs to compare the following one word 
//
// for a sentence as:w1, w2,....
// cell [i,j] in the table is for n-gram from w_(j-1)...w_(j+i-1), that is a 
// (i+1)-gram starting at position j+1 in sentence
S_sentSearchTableElement * C_SuffixArraySearch::constructNgramSearchTable4SentWithLCP( vector<IndexType> & sentInVocId) const
{
    int sentLen = sentInVocId.size();
    S_sentSearchTableElement * table = (S_sentSearchTableElement *) malloc( sentLen * sentLen * sizeof(S_sentSearchTableElement));
    
    //for consistency, initialize all cells
    for(int c=0;c<(sentLen*sentLen);c++){
    	table[c].found = false;
    	table[c].startPosInSA = 0;
    	table[c].endingPosInSA = 0;
    }
	
    TextLenType startPos, endPos;

	//initialize word level elements
	for(int i=0;i<sentLen;i++){
		IndexType vocId = sentInVocId[i];
		//cout<<vocId<<" ";
		if((vocId==0)||(vocId>=this->sentIdStart)){	//vocId ==0 means this word is OOV <unk>, if vocId>=sentIdStart means for this corpus, we don't know this word
			table[i].found = false;
		}
		else{
			table[i].startPosInSA = this->level1Buckets[vocId].first;
			table[i].endingPosInSA = this->level1Buckets[vocId].last;

			if(table[i].startPosInSA<=table[i].endingPosInSA){
				table[i].found = true;
			}
			else{	//because vocabulary is built on top of an existing voc, this corpus may not have all the occurrences of all the words in the voc
				table[i].found = false;
			}
		}
	}
	

	//filling in the cells in the table row by row
	//basically this means we start by looking for smaller units first
	//if they are found, search for longer n-grams
	for(int n=1;n<sentLen;n++){	//finding n+1 gram. when n=sentLen-1, we are search for the occurrence of the whole sent
		int levelN_1_0 = (n - 1) * sentLen;	//map from two dimensional position to one-dimension
		int levelN_0 = n * sentLen;
		for(int j=0;j<= (sentLen - 1 - n); j++){	//possible starting point for n+1 gram
			//necessary conditions that this n+1 gram exist are:
			//the two sub n-gram all exist in the corpus			
			if( table[levelN_1_0 + j].found && table[levelN_1_0 + j +1].found){
				IndexType nextWord = sentInVocId[j+n]; //the last word of the n+1 gram								

				//n+1 gram has to be in the range of the n-gram in SA
				startPos = table[levelN_1_0 + j].startPosInSA;
				endPos = table[levelN_1_0 + j].endingPosInSA;

				TextLenType foundPosStart = 0;
				TextLenType foundPosEnd = 0;

				//the prefix of n words of all suffixes between [startPos, endPos] is the same as the
				//prefix of the n words in the proposed n+1 gram, no need to compare
				//only need to compare the n+1 word, which is "nextWord" here
				if(this->searchPhraseGivenRangeWithLCP(nextWord, n, startPos, endPos, foundPosStart, foundPosEnd)){					
					table[levelN_0 + j].found = true;
					table[levelN_0 + j].startPosInSA = 	foundPosStart;
					table[levelN_0 + j].endingPosInSA = foundPosEnd;
				}
				else{
					table[levelN_0 + j].found = false;
				}

			}
			else{
				table[levelN_0 + j].found = false;
			}
		}
	}
	return table;
}

void C_SuffixArraySearch::displayNgramMatchingFreq4Sent(const char * sent)
{
	vector<IndexType> sentInVocId = this->convertStringToVocId(sent);
	this->displayNgramMatchingFreq4Sent(sentInVocId);
}

void C_SuffixArraySearch::displayNgramMatchingFreq4Sent(vector<IndexType> sentInVocId)
{
	int sentLen = sentInVocId.size();
	
	int i,j;
	//construct the n-gram search table
	//S_sentSearchTableElement * table = constructNgramSearchTable4Sent(sentInVocId);
	S_sentSearchTableElement * table = constructNgramSearchTable4SentWithLCP(sentInVocId);
  
	//show sentence
	cout<<"\t";
	for(i=0;i<sentLen;i++){
		cout<<this->voc->getText(sentInVocId[i])<<"\t";
	}
	cout<<endl;

	//show frequency of each n-gram
	i=0;
	bool stillMatch = true;
	while(stillMatch &&( i<sentLen)){
		cout<<i+1<<"\t";
		int startForRow = i*sentLen;
		bool anyGood = false;
		for(j=0;j<= (sentLen - 1 - i); j++){
			if(table[startForRow+j].found){
				//this is for regular case				
				if(table[startForRow+j].endingPosInSA>=table[startForRow+j].startPosInSA){	//more than one occurrence
					cout<<table[startForRow+j].endingPosInSA-table[startForRow+j].startPosInSA + 1;
					anyGood = true;
				}
				else{
					cout<<"0";
				}
				
				
				//if(table[startForRow+j].endingPosInSA>table[startForRow+j].startPosInSA){	//more than one occurrence
				//	cout<<table[startForRow+j].endingPosInSA-table[startForRow+j].startPosInSA;
				//	anyGood = true;
				//}
				//else{
				//	cout<<"0";
				//}
			}
			else{
				cout<<"0";
			}
			cout<<"\t";
		}

		stillMatch = anyGood;
		cout<<endl;
		i++;
	}
	
	free(table);
}

//given the pos of a word in corpus, return its offset in the sentence
//and the sentence ID
void C_SuffixArraySearch::locateSendIdFromPos(TextLenType pos, TextLenType & sentId, unsigned char & offset)
{
	offset = this->offset_list[pos];
	sentId = this->corpus_list[pos-offset] - this->sentIdStart + 1;
	
	if(this->suffixArrayIsVersion6){
		offset--;
	}
}

void C_SuffixArraySearch::locateSendIdFromPos(TextLenType pos, TextLenType & sentId, unsigned char & offset, unsigned char & sentLen)
{
	offset = this->offset_list[pos];
	sentLen = this->offset_list[pos-offset];
	sentId = this->corpus_list[pos-offset] - this->sentIdStart + 1;
	
	if(this->suffixArrayIsVersion6){
		offset--;
	}
}

vector<S_phraseLocationElement> C_SuffixArraySearch::findPhrasesInASentence(vector<IndexType> srcSentAsVocIDs)
{
	if(srcSentAsVocIDs.size()>255){
		cerr<<"Sorry, I prefer to handle sentences with less than 255 words. Please cut the sentence short and try it again.\n";
		exit(0);
	}

	unsigned char sentLen = (unsigned char) srcSentAsVocIDs.size();

	//construct the n-gram search table	
	S_sentSearchTableElement * table = constructNgramSearchTable4SentWithLCP(srcSentAsVocIDs);

	//Now, we know all the n-grams we are looking for
	//output the results
	vector<S_phraseLocationElement> allFoundNgrams;
	S_phraseLocationElement tmpNode;	

	int longestUnitToReportForThisSent = sentLen;
	if(this->longestUnitToReport!=-1){
		longestUnitToReportForThisSent = this->longestUnitToReport;
	}
	
	for(unsigned char r = this->shortestUnitToReport - 1; r< longestUnitToReportForThisSent; r++){
		int firstPosInRow = r*sentLen;
		for(unsigned char c=0; c<= (sentLen - 1 - r); c++){
			if(table[firstPosInRow + c].found){	//at this position the ngram was found
				tmpNode.posStartInSrcSent = c + 1;	//position starts from 1
				tmpNode.posEndInSrcSent = r + c + 1;

				//now for all ocurrences, find their sentId and realative positions
				TextLenType startPosInSA = table[firstPosInRow + c].startPosInSA;
				TextLenType endPosInSA = table[firstPosInRow + c].endingPosInSA;
				
				if( (this->highestFreqThresholdForReport < 0) ||	//no limit
					( (this->highestFreqThresholdForReport > 0 ) && ( (endPosInSA - startPosInSA) < this->highestFreqThresholdForReport ))
				){	
					// we don't want to retrieve high-freq n-gram which is very time consuming
					//and meaningless for translation, such as 1M occurrences of "of the" in the corpus
										

					if((this->reportMaxOccurrenceOfOneNgram > 0) && ( (endPosInSA - startPosInSA +1) > this->reportMaxOccurrenceOfOneNgram) ){
						//and for each n-gram, report only a limited amount of occurrences
						endPosInSA = startPosInSA + this->reportMaxOccurrenceOfOneNgram - 1;
					}

					TextLenType sentId;
					unsigned char posInSent;
					for(TextLenType iterator =startPosInSA; iterator <=endPosInSA; iterator++ ){
						this->locateSendIdFromPos(this->suffix_list[iterator], sentId, posInSent);
						tmpNode.sentIdInCorpus = sentId;
						tmpNode.posInSentInCorpus = posInSent;

						allFoundNgrams.push_back(tmpNode);
					}
				}
			}

		}
	}
	
	free(table);

	return allFoundNgrams;	
}

vector<S_phraseLocationElement> C_SuffixArraySearch::findPhrasesInASentence(const char * srcSent)
{
	//use the vocabulary associated with this corpus to convert words to vocIDs
	vector<IndexType> srcSentAsVocIDs = this->convertStringToVocId(srcSent);

	return this->findPhrasesInASentence(srcSentAsVocIDs);
}


bool C_SuffixArraySearch::locateSAPositionRangeForExactPhraseMatch(vector<IndexType>  phrase, TextLenType & rangeStart, TextLenType & rangeEnd)
{
	int phraseLen = phrase.size();

	//first check if there are any <unk> in the phrase
	for(int i=0;i<phrase.size();i++){
		if((phrase[i]==0)||(phrase[i]>=this->sentIdStart)){
			return false;	//return empty matching result
		}
	}

	TextLenType currentRangeStart, currentRangeEnd;
	TextLenType narrowedRangeStart, narrowedRangeEnd;
	IndexType vocId;

	//for word 1
	vocId = phrase[0];
	currentRangeStart = this->level1Buckets[vocId].first;
	currentRangeEnd = this->level1Buckets[vocId].last;

	if(currentRangeStart>currentRangeEnd){
		return false;	//even this 1-gram does not exist
	}

	int posInPhrase = 1;	
	while( posInPhrase<phraseLen ){
		vocId = phrase[posInPhrase];
		bool stillExist = this->searchPhraseGivenRangeWithLCP(vocId, posInPhrase, currentRangeStart, currentRangeEnd, narrowedRangeStart, narrowedRangeEnd);

		if(! stillExist){
			return false;
		}
		
		currentRangeStart = narrowedRangeStart;
		currentRangeEnd = narrowedRangeEnd;

		posInPhrase++;
	}

	//we find the range of matching phrase, now get the sentId
	rangeStart = currentRangeStart;
	rangeEnd = currentRangeEnd;

	return true;
}

//similar to construct the freq table
//but only search for the exact phrase matching
vector<S_SimplePhraseLocationElement> C_SuffixArraySearch::locateExactPhraseInCorpus(vector<IndexType> phrase)
{
	vector<S_SimplePhraseLocationElement> matchingResult;

	TextLenType rangeStart, rangeEnd;

	if(this->locateSAPositionRangeForExactPhraseMatch(phrase, rangeStart, rangeEnd)){
		//we find some match
		S_SimplePhraseLocationElement tmpNode;
		for(TextLenType saPos = rangeStart; saPos <= rangeEnd; saPos++){
			this->locateSendIdFromPos(this->suffix_list[saPos], tmpNode.sentIdInCorpus, tmpNode.posInSentInCorpus);
			matchingResult.push_back(tmpNode);
		}
	}

	return matchingResult;
}

vector<S_SimplePhraseLocationElement> C_SuffixArraySearch::locateExactPhraseInCorpus(const char *phrase)
{
	//use the vocabulary associated with this corpus to convert words to vocIds
	vector<IndexType> phraseAsVocIDs = this->convertStringToVocId(phrase);

	return this->locateExactPhraseInCorpus(phraseAsVocIDs);
}


TextLenType C_SuffixArraySearch::freqOfExactPhraseMatch(vector<IndexType> phrase)
{
	TextLenType rangeStart, rangeEnd;

	if(this->locateSAPositionRangeForExactPhraseMatch(phrase, rangeStart, rangeEnd)){
		return rangeEnd - rangeStart + 1;
	}

	return 0;
}

TextLenType C_SuffixArraySearch::freqOfExactPhraseMatch(const char *phrase)
{
	//use the vocabulary associated with this corpus to convert words to vocIds
	vector<IndexType> phraseAsVocIDs = this->convertStringToVocId(phrase);

	return this->freqOfExactPhraseMatch(phraseAsVocIDs);
}

TextLenType C_SuffixArraySearch::returnCorpusSize() const
{
	return this->corpusSize;
}

//given src sentence length, convert the index in one-dimensional table to pair<startingPosInSrcSent, n>
//startingPosInSrcSent starts at 0, n is the n-gram length
void C_SuffixArraySearch::oneDimensionTableIndexToTwoDimension(unsigned int index, unsigned int sentLen, unsigned int &posInSrcSent, unsigned int &n)
{
	n = index / sentLen + 1;
	posInSrcSent = index % sentLen;
}

//fill in information of each matched n-gram freq
unsigned int C_SuffixArraySearch::calcNgramMatchingInfoTokenFreqOnly(vector<IndexType> & sentInVocId, vector<S_ngramMatchingInfoTokenFreqOnlyElement> & nGramInfo)
{
	unsigned int i;	
	int sentLen = sentInVocId.size();
	nGramInfo.clear();

	//construct the n-gram search table	
	S_sentSearchTableElement * table = this->constructNgramSearchTable4SentWithLCP(sentInVocId);

	for(i=0;i<(sentLen*sentLen);i++){			

		if(table[i].found){
			unsigned int n = 0;
			unsigned int posInSrcSent = 0;

			this->oneDimensionTableIndexToTwoDimension(i, sentLen, posInSrcSent, n);
			unsigned int totalOccurrences = table[i].endingPosInSA - table[i].startPosInSA + 1;
			S_ngramMatchingInfoTokenFreqOnlyElement newNode;
			newNode.n = n;
			newNode.startPos = posInSrcSent;
			newNode.tokenFreq = totalOccurrences;

			if(newNode.tokenFreq>0){
				nGramInfo.push_back(newNode);
			}
		}
	}

	free(table);
	return sentLen;
}

void C_SuffixArraySearch::calcNgramMatchingInfoTokenFreqOnly(vector<IndexType> & sentInVocId, double * freqTable)
{
	unsigned int i;	
	int sentLen = sentInVocId.size();

	//construct the n-gram search table	
	S_sentSearchTableElement * table = this->constructNgramSearchTable4SentWithLCP(sentInVocId);

	for(i=0;i<(sentLen*sentLen);i++){
		unsigned int n = 0;
		unsigned int posInSrcSent = 0;
		
		if(table[i].found){
			this->oneDimensionTableIndexToTwoDimension(i, sentLen, posInSrcSent, n);		
			unsigned int totalOccurrences = table[i].endingPosInSA - table[i].startPosInSA + 1;
			
			freqTable[i] = (double)totalOccurrences;
		
		}
	}

	free(table);	
}

//if currently matched an n-gram at corpus position [currentMatchStart, currentMatchStart+currentMatchLen-1]
//get the freq for [currentMatchStart, currentMatchStart+currentMatchLen-1] + nextWord
//only need to get freq(w_n | history) of different history
//return in freq table, freq(history+Wn, history) for all the matched n
//freq: 1-gram Freq, corpusSize, 2-gram freq, freq of 2-gram history
//	    3-gram freq, freq of 3-gram history
//freqTable should have length of 2*n
//return the longest match with this updated n-gram
void C_SuffixArraySearch::calcNgramMatchingInfoTokenFreqOnlyExtendingCurrentMatch(TextLenType currentMatchStart, unsigned char currentMatchLen, IndexType nextWord, int maxLenOfNgramConsidered, double *freqTable, TextLenType &updatedMatchingStart, unsigned char &updatedMatchingLen) const
{
	vector<IndexType> nGram;	

	if(currentMatchStart!=(TextLenType) -1){	//-1 will be <unk>
		if(currentMatchLen==maxLenOfNgramConsidered){	//we consider only up to maxLenOfNgramConsidered for the extended n-gram
			currentMatchStart++;
			currentMatchLen--;
		}

		for(TextLenType pos=currentMatchStart; pos<(currentMatchStart+currentMatchLen); pos++){
			nGram.push_back(this->corpus_list[pos]);
		}
	}

	nGram.push_back(nextWord);

	int sentLen = nGram.size();
	
	//construct the n-gram search table	
	S_sentSearchTableElement * table = this->constructNgramSearchTable4SentWithLCP(nGram);

	int startPosForNgram;
	int startPosForLongestMatchingWithNextWord;
	int cellIndexForLongestMatchingWithNextWord;

	bool stillMatched = true;
	bool atLeastOneMatched = false;

	int indexForNgram;
	int indexForHistory;

	unsigned int totalOccurrences;
	unsigned int totalOccurrencesOfHistory;

	//for unigram
	indexForNgram = sentLen - 1;
	if(table[indexForNgram].found){
		totalOccurrences = table[indexForNgram].endingPosInSA - table[indexForNgram].startPosInSA + 1;
		if(this->withDiscounting){
			freqTable[0] = this->discountFreq(1, totalOccurrences);
		}
		else{
			freqTable[0] = totalOccurrences; 
		}

		freqTable[1] = this->returnCorpusSize();
		cellIndexForLongestMatchingWithNextWord = indexForNgram;
		startPosForLongestMatchingWithNextWord = sentLen-1;
		atLeastOneMatched = true;
	}
	else{
		stillMatched = false;
	}

	int n=2;	//considering 2-gram and longer n-gram now
	startPosForNgram = sentLen - 2;
	while((stillMatched)&&(startPosForNgram>=0)){
		
		indexForNgram = (n-1) * sentLen + startPosForNgram;
		int indexForHistory = (n-2) * sentLen +  startPosForNgram;
		
		if(table[indexForNgram].found){
						
			totalOccurrences = table[indexForNgram].endingPosInSA - table[indexForNgram].startPosInSA + 1;	
			totalOccurrencesOfHistory = table[indexForHistory].endingPosInSA - table[indexForHistory].startPosInSA + 1;

			
			if(this->withDiscounting){
				freqTable[2*n-2] = this->discountFreq(n, totalOccurrences);				
			}
			else{
				freqTable[2*n-2] = (double)totalOccurrences;
			}

			freqTable[2*n-1] = (double) totalOccurrencesOfHistory;	//do not discount the history
			
			if(n<maxLenOfNgramConsidered){	//new history is at most maxLenOfNgramConsidered-1 words long
				cellIndexForLongestMatchingWithNextWord = indexForNgram;
				startPosForLongestMatchingWithNextWord = startPosForNgram;
			}
		}
		else{
			stillMatched = false;
		}

		startPosForNgram--;
		n++;
	}

	if(atLeastOneMatched){	//at least one n-gram can be matched with 'nextWord'
		updatedMatchingStart = this->suffix_list[table[cellIndexForLongestMatchingWithNextWord].startPosInSA];
		updatedMatchingLen = (unsigned char) (sentLen - startPosForLongestMatchingWithNextWord);
	}
	else{
		updatedMatchingStart = (TextLenType) -1;
		updatedMatchingLen = 0;
	}

	free(table);

}


//construct the Good-Turing discounting table
void C_SuffixArraySearch::constructDiscountingTable()
{
	cerr<<"Constructing discounting information...\n";
	int i,j;
	int c;
	int totalCellInTable = this->maxFreqForDiscounting*this->maxNForDiscounting;

	//start to scan the suffix array to build up the discounting table
	//first, initialize the scan list
	S_nGramScanningSimpleInfoElement * nGramScanningList = (S_nGramScanningSimpleInfoElement *) malloc(sizeof(S_nGramScanningSimpleInfoElement)*this->maxNForDiscounting);

	//initialize the scanning list
	for(i=0;i<this->maxNForDiscounting;i++){
		nGramScanningList[i].freqSoFar=0;
		nGramScanningList[i].vocId = 0;		
	}

	//initialize the count of counts table
	unsigned int * countOfCountsTable = (unsigned int *) malloc(sizeof(unsigned int)*totalCellInTable);
	if(countOfCountsTable==NULL){
		cerr<<"Count of counts table can not be initialized. Exit\n";
		exit(0);
	}
	for(c=0;c<totalCellInTable;c++){
		countOfCountsTable[c]=0;
	}

	//scan the suffix array list	
	bool stillMeaningful = true;	
	TextLenType saPos=0;

	while(stillMeaningful && ( saPos<this->corpusSize ) ){

		TextLenType posInCorpus = this->suffix_list[saPos];
		IndexType wordInCorpus = this->corpus_list[posInCorpus];

		if(wordInCorpus<this->sentIdStart){	//SA positions pointing to sentID are not interesting
			
			if((wordInCorpus!=this->vocIdForSentStart)&&(wordInCorpus!=this->vocIdForSentEnd)){	//n-grams start with <s> and </s> are not interested
				bool quit =false;
				i=0;

				while(!quit && (i<this->maxNForDiscounting)){
					wordInCorpus = this->corpus_list[posInCorpus+i];
					if(						
						(wordInCorpus<this->sentIdStart)&&
						(wordInCorpus!=this->vocIdForSentEnd)&&
						(wordInCorpus!=this->vocIdForSentStart)&&
						(wordInCorpus==nGramScanningList[i].vocId)){	//still match

						nGramScanningList[i].freqSoFar++;
					}
					else{	//we will have new (i+1) and longer n-grams soon, before that check if we should increase the count of counts for n because of this n-gram type
											
						for(j=i;j<this->maxNForDiscounting;j++){				
							
							unsigned int freqSoFar = nGramScanningList[j].freqSoFar;
							if( (freqSoFar > 0) && ( freqSoFar <= this->maxFreqForDiscounting) ){
								//increase the count for (j+1)-gram with freq freqSoFar
								countOfCountsTable[j*this->maxFreqForDiscounting+freqSoFar-1]++;
							}

							//finished output, now clear the list from point of i
							wordInCorpus = this->corpus_list[posInCorpus+j];
							if((wordInCorpus>=this->sentIdStart)||(wordInCorpus==this->vocIdForSentEnd)||(wordInCorpus==this->vocIdForSentStart)){
								wordInCorpus=0;	//write 0 for <sentId>, <s> and </s>
								nGramScanningList[j].freqSoFar = 0;
							}
							else{
								nGramScanningList[j].freqSoFar = 1;
							}

							nGramScanningList[j].vocId = wordInCorpus;							
						}

						quit=true;	//at i+1 gram, already not match, no need to check for longer
					}

					i++;
				}
			}
		}
		else{
			stillMeaningful = false;	//once vocID is getting larger/equal than sentIdStart, everything follows it are <sentId> and no actual text
		}

		saPos++;
	}

	//at the end of corpus (according to suffix order)		
	for(i=0;i<this->maxNForDiscounting;i++){		
		unsigned int freqSoFar = nGramScanningList[j].freqSoFar;
		if( (freqSoFar > 0) && ( freqSoFar <= this->maxFreqForDiscounting) ){
			//increase the count for (j+1)-gram with freq freqSoFar
			countOfCountsTable[j*this->maxFreqForDiscounting+freqSoFar-1]++;
		}
	}

	//now, use Good-Turing discounting to create frequency mapping
	//still assign N*Freq table for simplicity, even though that for each N, only maxFreq-1 freq type will be discounted
	this->discountingMap = (double *) malloc(sizeof(double) * this->maxNForDiscounting * this->maxFreqForDiscounting);
	
	for(i=0;i<this->maxNForDiscounting;i++){
		//for (i+1)-gram
		
		unsigned int * ccTableForThisN = countOfCountsTable + i*this->maxFreqForDiscounting;
		double * discountingMapForThisN = this->discountingMap + i*this->maxFreqForDiscounting;

		for(int freq=0;freq<(this->maxFreqForDiscounting-1);freq++){	//only goes to maxFreq-1, because we can not discount maxFreq
			//for all (freq+1) ngrams
			if((ccTableForThisN[freq]>0)&&(ccTableForThisN[freq+1]>0)){	//both freq exists
				discountingMapForThisN[freq] = (double)(ccTableForThisN[freq+1]*(freq+2))/(double)(ccTableForThisN[freq]);			
			}
			else{
				discountingMapForThisN[freq] = -1;
			}
		}

		discountingMapForThisN[this->maxFreqForDiscounting-1] = -1;	//won't be used, just for consistency
	}

	free(countOfCountsTable);
	free(nGramScanningList);
}


//given observedFreq of n-gram, return discounted freq using Good-Turing smoothing
double C_SuffixArraySearch::discountFreq(int n, unsigned int observedFreq) const
{
	if(n>=this->maxNForDiscounting){	//do not discount
		return (double) observedFreq;
	}

	if(observedFreq>=(this->maxFreqForDiscounting-1)){	//no discounting for high freq
		return (double) observedFreq;
	}

	//else, check the discount map
	double discountedFreq = this->discountingMap[ (n-1) * this->maxFreqForDiscounting + observedFreq -1];

	if(discountedFreq>0){
		return discountedFreq;
	}

	//else, no discounting
	return (double) observedFreq;
}
