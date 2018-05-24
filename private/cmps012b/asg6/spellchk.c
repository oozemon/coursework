// $Id: spellchk.c,v 1.9 2014-05-15 21:07:47-07 - - $
//usman zahid and alexander hoyt
//uzahid@ucsc.edu and adhoyt@ucsc.edu	


#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "debug.h"
#include "hashset.h"
#include "yyextern.h"

#define STDIN_NAME "-"
#define DEFAULT_DICTNAME \
       "/afs/cats.ucsc.edu/courses/cmps012b-wm/usr/dict/words"
#define DEFAULT_DICT_POS 0
#define EXTRA_DICT_POS 1
#define NUMBER_DICTS 2

bool debug_bool=false;

volatile int debug;

// prints out the error messages
void print_error (const char *object, const char *message) {
  fflush (NULL);
  fprintf (stderr, "%s: %s: %s\n", program_name, object, message);
  fflush (NULL);
  exit_status = EXIT_FAILURE;
}

// opens the file
FILE *open_infile (const char *filename) {
  FILE *file = fopen (filename, "r");
  if (file == NULL) {
    print_error (filename, strerror (errno));
    exit_status = 2;
  }
  return file;
}

// checks for spelling by looking for word in hashtable
void spellcheck (const char *filename, hashset *hashset) {
  yylineno = 1;
  for (;;) {
    int token = yylex ();
    if (token == 0) break;
    //text = strdup(yytext);
    // initial lookup
     char* text= yytext;
    if(!has_hashset(hashset, text)){
      text[0] = tolower(text[0]);
        if(!has_hashset(hashset,text)){
    // upper case lookup
   // if(has_hashset(hashset, text)) continue;
    // if both lookups fail print error messages
      //printf("%s", filename);
      printf("%s: %d: %s\n", filename, yylineno, yytext);
      exit_status = 1;
      }
    }
  }
}

//Loads the dictionary and puts each word into the hashtable

void load_dictionary (const char *dictionary_name, hashset *hashset) {
   if (dictionary_name == NULL) return;
   //DEBUGF ('m', "dictionary_name = \"%s\", hashset = %p\n",
        //   dictionary_name, hashset);
   FILE *dictionary = open_infile(dictionary_name);
   char c[1024];
   for(;;){
     if (fgets(c, sizeof(c), dictionary) != NULL){
        char *nlptr = strchr(c, '\n');
        if (nlptr) *nlptr = '\0';
        put_hashset(hashset, c);
        //printf("Good job it works!\n");
     } else {
        break;
     }
   }
   fclose(dictionary);
}
/*void load_dictionary (char *dictionary_name, hashset *hashset) {
  if (dictionary_name == NULL) return;
  FILE *dictionary = open_infile(dictionary_name);
  char item[100];
  while (fgets (item, 100, dictionary) != NULL){
    // replaces new line characters with null
    item[strlen(item) - 1] = '\0';
    put_hashset(hashset , item);
  }
  fclose(dictionary);
}*/


void scan_options (int argc, char** argv,
                   char **default_dictionary,
                   char **user_dictionary) {
    debug=0;
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
         case 'x': ++debug;
                   debug_bool=true;
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
   hashset *hashset = new_hashset ();
   yy_flex_debug = false;
   scan_options (argc, argv, &default_dictionary, &user_dictionary);
   load_dictionary (default_dictionary, hashset);
   load_dictionary (user_dictionary, hashset);
   //printf("debug: %d\n", debug);
   if(debug_bool)debugopt (hashset, debug);
   // Read and do spell checking on each of the files.
   if (optind >= argc) {
      yyin = stdin;
      spellcheck (STDIN_NAME, hashset);
   }else {
      for (int fileix = optind; fileix < argc; ++fileix) {
         //DEBUGF ('m', "argv[%d] = \"%s\"\n", fileix, argv[fileix]);
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

