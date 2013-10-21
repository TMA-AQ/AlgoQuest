#ifndef __DATABASE_LOADER_H__
#define __DATABASE_LOADER_H__

#include <cstddef>
#include <aq/BaseDesc.h>

namespace aq
{

class DatabaseLoader
{
public:
  DatabaseLoader(const aq::base_t bd, const std::string& _loader, const std::string& _path, const size_t _packet_size, const char _end_of_field, bool _csv_format);
  void generate_ini();
  void run(size_t num_table, size_t num_column);

protected:
  void FileWriteEnreg(aq::symbole col_type, int col_size, char *my_field, FILE *fcol, int rowNr);
  char* nettoie_nom_vite( char* strval ); //return NULL - failure
  aq::symbole NbFieldInRecord ( const char * my_fic2 , char *my_record ,   int max_read, int *nb_fields  , int *nb_pack_col , int *nb_col_in_last_pack );
  void ChangeCommaToDot (  char *string );
  void display_base (  aq::base_t *base);
  void nettoie_nom (char *nom);
  void *safecalloc ( size_t nb, size_t size);
  void allocate_and_init_prefixe ( aq::s_prefixe *prefixe );
  void init_format_column ( char *format , aq::s_prefixe *prefixe );
  void init_format_column_short ( char *format , aq::s_prefixe *prefixe );
  void CleanSpaceAtEnd ( char *my_field );
  double MonAtoF  ( char *string ); // , int nb_decim );
  double str2num(char *c); //capable de lire toute les valeurs !
  double monatof_V0(char *string);

private:
	const aq::base_t my_base;
  std::string k_rep_racine;
  std::string k_rep_param;
  std::string k_batch_loader;
  std::string ini_filename;
  std::string rep_source;
  std::string rep_cible;
  std::string base_desc_file;
  size_t k_packet_size;
  char k_double_separator;
  unsigned char end_of_field;   
  bool csv_format;
};

}

#endif
