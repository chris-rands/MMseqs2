#include "IndexTable.h"

IndexTable::IndexTable (int alphabetSize, int kmerSize, int skip)
{
    this->alphabetSize = alphabetSize;
    this->kmerSize = kmerSize;
    this->size = 0;
    this->skip = skip;
    
    tableSize = ipow(alphabetSize, kmerSize);
    
    sizes = new int[tableSize];
    memset(sizes, 0, sizeof(int) * tableSize);
    
    currPos = new int[tableSize];
    std::fill_n(currPos, tableSize, 1); // needed because of size at beginning
    
    table = new int*[tableSize];
    
    idxer = new Indexer(alphabetSize, kmerSize);
    
    this->tableEntriesNum = 0;
}
IndexTable::~IndexTable(){
    delete[] entries;
    delete[] table;
    delete idxer;
}

int* IndexTable::getSizes(){
    if(sizes == NULL)
        std::cerr << "AAAAAH" << std::endl;
    return sizes;
}

void IndexTable::addKmerCount (Sequence* s){
    unsigned int kmerIdx;
    s->resetCurrPos();
    idxer->reset();
    
    while(s->hasNextKmer(kmerSize)){
        kmerIdx = idxer->getNextKmerIndex(s->nextKmer(kmerSize), kmerSize);
        sizes[kmerIdx]++;
        tableEntriesNum++;
        for (int i = 0; i < skip && s->hasNextKmer(kmerSize); i++){
            idxer->getNextKmerIndex(s->nextKmer(kmerSize), kmerSize);
        }
    }
    this->s = s;
}

void IndexTable::initMemory(){
    // allocate memory for the sequence id lists
    // tablesSizes is added to put the Size of the entry infront fo the memory
    entries = new int[tableEntriesNum+tableSize];
}


void IndexTable::init(){
    int* it = entries;
    // set the pointers in the index table to the start of the list for a certain k-mer
    for (int i = 0; i < tableSize; i++){
        table[i] = it;
        table[i][0] = sizes[i];
        it += sizes[i] + 1; // +1 for sizes element
    }
}

int * IndexTable::getEntries(){
    return entries;
}

void IndexTable::addSequence (Sequence* s){
    // iterate over all k-mers of the sequence and add the id of s to the sequence list of the k-mer (tableDummy)
    unsigned int kmerIdx;
    this->size++; // amount of sequences added
    s->resetCurrPos();
    this->s = s;
    idxer->reset();
    
    while(s->hasNextKmer(kmerSize)){
        kmerIdx = idxer->getNextKmerIndex(s->nextKmer(kmerSize), kmerSize);
        table[kmerIdx][currPos[kmerIdx]++] = s->getId();
        for (int i = 0; i < skip && s->hasNextKmer(kmerSize); i++){
            idxer->getNextKmerIndex(s->nextKmer(kmerSize), kmerSize);
        }
    }
}

void IndexTable::removeDuplicateEntries(){
    
    delete[] currPos;

    
    for (int e = 0; e < tableSize; e++){
        if (sizes[e] == 0)
            continue;
        int* entries = table[e] + 1; // because of size at position 1
        int size = sizes[e];
        
        std::sort(entries, entries + size);
        // remove duplicates in-place
        int boundary = 1;
        for (int i = 1; i < size; i++){
            if (entries[i] != entries[i-1])
                entries[boundary++] = entries[i];
        }
        size = boundary;
        table[e][0]  = size;
    }
    delete[] sizes;

}

void IndexTable::print(){
    for (int i = 0; i < tableSize; i++){
        if (table[i][0] > 0){
            idxer->printKmer(i, kmerSize, s->int2aa);
            std::cout << "\n";
            for (int j = 0; j < table[i][0]; j++){
                std::cout << "\t" << table[i][j+1] << "\n";
            }
        }
    }
}



void IndexTable::initTableByExternalData(uint64_t tableEntriesNum,
                                         int * sizes, int * pentries, unsigned int sequenzeCount){
    this->tableEntriesNum = tableEntriesNum;
    this->size = sequenzeCount;
    initMemory();
    memcpy ( this->entries , pentries, sizeof(unsigned int) * (this->tableEntriesNum  + this->tableSize));
    int* it = this->entries;
    // set the pointers in the index table to the start of the list for a certain k-mer
    for (int i = 0; i < tableSize; i++){
        table[i] = it;
        it += sizes[i] + 1; // +1 for sizes element
    }
    delete [] this->sizes;
    this->sizes = NULL;
}



int IndexTable::ipow (int base, int exponent){
    int res = 1;
    for (int i = 0; i < exponent; i++)
        res = res*base;
    return res;
}
