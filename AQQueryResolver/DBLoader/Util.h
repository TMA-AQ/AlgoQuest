#ifndef __AQ_TOOLBOX_H__
#define __AQ_TOOLBOX_H__

#include <aq/BaseDesc.h>

#define k_batch_size_max 8192   // batch_name
#define k_record_size_max 40960   // record in a file
#define k_field_size_max 4096   // record in a file  
#define k_file_name_size_max 4096    // nom de fichier 
#define k_taille_paquet 1048576;  // nbre max d'enreg dans un paquet
// #define k_max_open_file 15 // nbre de fichiers ouverts en meme temps
#define  k_taille_message  4096 

extern char k_rep_racine[k_file_name_size_max]; //  = ".";          // a passer en argument
extern char k_rep_param[k_file_name_size_max]; // = "parameters/"; //  a passer en argument
// FLOAT INFOS to get from file descriptor or ini.properties
extern char k_double_separator ; //= ',';


extern char *rep_source;
extern char *rep_cible;
extern char *base_desc_file;
extern unsigned char end_of_field; //field separator
extern int csv_format; //0 - not csv format, 1 - csv format "field1","123","42.5"
//extern char empty_field [ 2 + 1 ] ; //field empty  2011/02/22

// file name for log
extern char name_log [ k_taille_message ];
extern char a_message [ k_taille_message ];

// for loader
extern char k_batch_loader [ k_batch_size_max ];

//symbole ReadRecord ( FILE *fp2 , char *my_fic2 , char *my_record ,  int max_read );
void FileWriteEnreg(aq::symbole col_type, int col_size, char *my_field, FILE *fcol, int rowNr);
void die_with_error( char *errorMessage );
char* nettoie_nom_vite( char* strval ); //return NULL - failure

aq::symbole  NbFieldInRecord ( char *my_fic2 , char *my_record ,   int max_read, int *nb_fields  , int *nb_pack_col , int *nb_col_in_last_pack );

void redirect_std_out ( char *name_out, FILE *f_out );

void ChangeCommaToDot (  char *string );

//void my_copie_secure ( char * source, char *cible );
//-------------------------------------------------------------------------------
void display_base (  aq::base_t *base);
// -----------------------------------------------------------------------------------------------------------
void nettoie_nom (char *nom);


// -----------------------------------------------------------------------------------------------------------
//int count_nb_enreg ( char *my_fic);
//-------------------------------------------------------------------------------
void *safecalloc ( size_t nb, size_t size);
//-------------------------------------------------------------------------------
void allocate_and_init_prefixe ( aq::s_prefixe *prefixe );
//-------------------------------------------------------------------------------
void init_format_column ( char *format , aq::s_prefixe *prefixe );
void init_format_column_short ( char *format , aq::s_prefixe *prefixe );
//-------------------------------------------------------------------------------
void   CleanSpaceAtEnd ( char *my_field );
//-------------------------------------------------------------------------------
// long long MonAtoF_LongLong(char *string , int nb_decim );
//-------------------------------------------------------------------------------
double MonAtoF  ( char *string ); // , int nb_decim );
double str2num(char *c); //capable de lire toute les valeurs !
//-------------------------------------------------------------------------------
double monatof_V0(char *string);
void check_args(const char * iniFilename, size_t table, size_t column);
//-------------------------------------------------------------------------------
FILE* fopenUTF8(const char* filename, const char* mode);

#endif
