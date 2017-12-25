// $Id: spellchk.c,v 1.2 2016-03-11 23:52:31-08 - - $
// Srijitha Somamngili
// ssomangi@ucsc.edu
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#include "debug.h"
#include "hashset.h"
#include "yyextern.h"

#define STDIN_NAME       "-"
#define DEFAULT_DICTNAME \
        "/afs/cats.ucsc.edu/courses/cmps012b-wm/usr/dict/words"
#define DEFAULT_DICT_POS 0
#define EXTRA_DICT_POS   1
#define NUMBER_DICTS     2

void print_error (const char *object, const char *message) {
   fflush (NULL);
   fprintf (stderr, "%s: %s: %s\n", program_name, object, message);
   fflush (NULL);
   exit_status = NUMBER_DICTS;
}

FILE *open_infile (const char *filename) {
   FILE *file = fopen (filename, "r");
   if (file == NULL) print_error (filename, strerror (errno));
   DEBUGF ('m', "filename = \"%s\", file = 0x%p\n", filename, file);
   return file;
}

void spellcheck (const char *filename, hashset *hashset) {
   yylineno = 1;
   DEBUGF ('m', "filename = \"%s\", hashset = 0x%p\n",
                filename, hashset);
   for (;;) {
      int token = yylex ();
      if (token == 0) break;
      DEBUGF ('m', "line %d, yytext = \"%s\"\n", yylineno, yytext);
      if(!has_hashset(hashset,yytext)){
           char *word = strdup(yytext);
           yytext[0] = tolower(yytext[0]);
           if(!has_hashset(hashset,yytext)){
               printf("%s: %d: %s\n",filename,yylineno,word);
               exit_status = 1;
           }
           free(word);
      }
   }
}

void load_dictionary (const char *dictionary_name, hashset *hashset) {
   if (dictionary_name == NULL) return;
   DEBUGF ('m', "dictionary_name = \"%s\", hashset = %p\n",
           dictionary_name, hashset);
   FILE *dictionary = open_infile(dictionary_name);
   char buff[64];
   size_t linenr = 1;
   for(;;++linenr){
       char *buf = fgets (buff, sizeof buff, dictionary);
       if (buf == NULL) break;
      char *nlpos = strchr(buff, '\n');
       if (nlpos != NULL){
           *nlpos = '\0';
       }
       put_hashset(hashset, buf);
   }
   fclose(dictionary);
}

void scan_options (int argc, char** argv,
                   char **default_dictionary,
                   char **user_dictionary,
                   bool *debug, bool *all) {
   // Scan the arguments and set flags.
   opterr = false;
   for (;;) {
      int option = getopt (argc, argv, "nxyd:@:");
      if (option == EOF) break;
      switch (option) {
         char optopt_string[16]; // used in default:
         case 'd': *user_dictionary = optarg;
                   break;
         case 'n': *default_dictionary = NULL;
                   break;
         case 'x': 
                  if(*debug == false) *debug= true;
                      else *all = true;
                   break;
         case 'y': yy_flex_debug = true;
                   break;
         case '@': set_debug_flags (optarg);
                   if (strpbrk (optarg, "@y")) yy_flex_debug = true;
                   break;
         default : sprintf (optopt_string, "-%c", optopt);
                   print_error (optopt_string, "invalid option");
                   break;
      }
   }
}

int main (int argc, char **argv) {
   program_name = basename (argv[0]);
   char *default_dictionary = DEFAULT_DICTNAME;
   char *user_dictionary = NULL;
   bool debug= false;
   bool all = false;
   hashset *hashset = new_hashset ();
   yy_flex_debug = false;
   scan_options (argc, argv, &default_dictionary, &user_dictionary,
                           &debug,&all);
   
   if(default_dictionary == NULL && user_dictionary == NULL){
      print_error(user_dictionary, strerror(errno));
      exit_status = 2;
      return exit_status;
   }   
   // Load the dictionaries into the hash table.
   load_dictionary (default_dictionary, hashset);
   load_dictionary (user_dictionary, hashset);

   if(debug == true){ 
      debugprint(hashset);
      if(all == true){
          dumpprint(hashset);
       }
       return EXIT_SUCCESS;
    }
   // Read and do spell checking on each of the files.
   if (optind >= argc) {
      yyin = stdin;
      spellcheck (STDIN_NAME, hashset);
   }else {
      for (int fileix = optind; fileix < argc; ++fileix) {
         DEBUGF ('m', "argv[%d] = \"%s\"\n", fileix, argv[fileix]);
         char *filename = argv[fileix];
         if (strcmp (filename, STDIN_NAME) == 0) {
            yyin = stdin;
            spellcheck (STDIN_NAME, hashset);
         }else {
            yyin = open_infile (filename);
            if (yyin == NULL) continue;
            spellcheck (filename, hashset);
            fclose (yyin);
         }
      }
   }
   free_hashset(hashset); 
   yylex_destroy ();
   return exit_status;
}

