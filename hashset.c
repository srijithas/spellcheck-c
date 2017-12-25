// $Id: hashset.c,v 1.2 2016-03-11 23:52:31-08 - - $
// Srijitha Somangili
// ssomangi@ucsc.edu
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "debug.h"
#include "hashset.h"
#include "strhash.h"

#define HASH_NEW_SIZE 15

typedef struct hashnode hashnode;
struct hashnode {
   char *word;
   hashnode *link;
};

struct hashset {
   size_t size;
   size_t load;
   hashnode **chains;
};

hashset *new_hashset (void) {
   hashset *this = malloc (sizeof (struct hashset));
   assert (this != NULL);
   this->size = HASH_NEW_SIZE;
   this->load = 0;
   size_t sizeof_chains = this->size * sizeof (hashnode *);
   this->chains = malloc (sizeof_chains);
   assert (this->chains != NULL);
   memset (this->chains, 0, sizeof_chains);
   DEBUGF ('h', "%p -> struct hashset {size = %zd, chains=%p}\n",
                this, this->size, this->chains);
   return this;
}

void free_hashset (hashset *this) {
   DEBUGF ('h', "free (%p)\n", this)
   for(size_t ind = 0; ind < this->size; ++ind){
        hashnode *set = this->chains[ind];
        while(set != NULL){
             hashnode *old = set;
             set = set->link;
             free(old->word);
             free(old);
        }
   } 
   free(this->chains);
   free(this);   
}

void debugprint( hashset *this){
  printf("%zu words in the hash set\n",this->load);
  printf("%zu size of the hash array\n",this->size);
  int length[15] ={0};
  for(size_t ind = 0; ind < this-> size; ++ind){
     int count = 0;
     for(hashnode *curr = this->chains[ind]; curr != NULL;
               curr= curr->link){
          count++;
     }
     length[count]++;
  }
  for(int i= 1; i <15; ++i){
    if(length[i] != 0){
        printf("%5d chains of size %d\n",length[i],i); 
     }
   } 
}

void dumpprint(hashset *this){
   for(size_t ind = 0; ind < this->size; ++ind){
       if(this->chains[ind]){
         printf("array[%10zu] = %20lu \"%s\"\n", ind,
                 strhash(this->chains[ind]->word),
                 this->chains[ind]->word);
         for(hashnode *curr = this->chains[ind]->link; curr != NULL;
                curr= curr->link){
              for(int space =0; space < 18; ++space) printf(" ");
              printf("= %20lu \"%s\"\n",strhash(curr->word),
                                  curr->word);
          }
       }
    }
} 

bool has_hashset (hashset *this, const char *item) {
  // STUBPRINTF ("hashset=%p, item=%s\n", this, item);
   size_t hash_index = strhash(item) % this->size;
   hashnode *temp = this->chains[hash_index];
   for(hashnode *curr = temp; curr != NULL;curr= curr->link){
        if(strcmp(curr->word,item) == 0) return true;
   }
   return false;
}

void doublesize(hashset *this){
     size_t newsize = ((this->size *2) +1);
     hashnode **new_chain = malloc((newsize) *sizeof (hashnode *));
     assert(new_chain != NULL);
     memset(new_chain, 0, newsize *sizeof (hashnode *));
     for(size_t ind =0; ind< this->size; ++ind){
          hashnode *set = this->chains[ind];
          while(set != NULL){
               hashnode *old = set;
               set = set->link;
               size_t nhash_ind = strhash (old->word) % newsize;
               hashnode *nchain = new_chain[nhash_ind];
               old->link = nchain;
               new_chain[nhash_ind] = old;
           }
      }
      free(this->chains);
      this->chains = new_chain;
      this->size = newsize;
}

void put_hashset (hashset *this, const char *item) {
  // STUBPRINTF ("hashset=%p, item=%s\n", this, item);
   if((this->load * 2) > this->size) doublesize(this);
   size_t hash_index = strhash (item) % this-> size;
   if(has_hashset(this,item)) return;
   hashnode *temp = malloc(sizeof (struct hashnode));
   assert(temp != NULL);
   temp->word = strdup(item);
   assert(temp->word != NULL);
   temp->link = this->chains[hash_index];
   this->chains[hash_index] = temp;
   this->load++;
}
