/*
  * Unitex 
  *
  * Copyright (C) 2001-2006 Universit� de Marne-la-Vall�e <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */

/* Created by Agata Savary (agata.savary@univ-tours.fr)
 * Last modification on Sept 08 2005
 */
//---------------------------------------------------------------------------

/********************************************************************************/
/* INFLECTION OF A DELAC FILE INTO A DELACF                                     */
/********************************************************************************/


#include "unicode.h"
#include "MF_DLC_inflect.h"
#include "MF_SU_morpho.h"
#include "MF_LangMorpho.h"
#include "MF_Util.h"
#include "MF_DicoMorpho.h"
#include "Error.h"
#include "DELA.h"

//Alphabet of the current language
extern Alphabet* alph;

int DLC_line2entry(unichar* line, DLC_entry_T* entry);
int DLC_scan_unit(SU_id_T* u,unichar* line);
int DLC_scan_codes(unichar* codes[MAX_CODES],unichar* line);
int DLC_scan_comment(unichar** comment,unichar* line);
l_class_T* DLC_class_para(unichar* para);
int DLC_print_entry(DLC_entry_T* entry);
int DLC_print_unit(SU_id_T* unit);
int DLC_format_form(unichar* entry, int max, MU_f_T f, DLC_entry_T* dlc_entry);
void DLC_delete_entry(DLC_entry_T* entry);


/////////////////////////////////////////////////////////////////////////////////
// Inflect a DELAS/DELAC into a DELAF/DELACF.
// On error returns 1, 0 otherwise.
int inflect(char* DLC, char* DLCF) {
FILE *dlc, *dlcf;  //DELAS/DELAC and DELAF/DELACF files
unichar input_line[DIC_LINE_SIZE];  //current DELAS/DELAC line 
unichar output_line[DIC_LINE_SIZE];  //current DELAF/DELACF line 
int l;  //length of the line scanned
DLC_entry_T* dlc_entry;
MU_forms_T* MU_forms;  //inflected forms of the MWU
int err;
unichar nl[3];  //a newline
u_strcpy_char(nl,"\n");
//Open DELAS/DELAC
dlc = u_fopen(DLC, U_READ);
if (!dlc) {
   error("Unable to open file: '%s' !\n", DLC);
   return 1;
}
//Open DELAF/DELACF
dlcf = u_fopen(DLCF, U_WRITE);
if (!dlcf) {
   error("Unable to open file: '%s' !\n", DLCF);
   return 1;
}
//Inflect one entry at a time
l = u_fgets(input_line,DIC_LINE_SIZE-1,dlc);
//Omit the final newline
if (u_strlen(input_line)>0 && input_line[u_strlen(input_line)-1]==(unichar)'\n') {
   input_line[u_strlen(input_line)-1] = (unichar)'\0';
}
int flag=0;
//If a line is empty the file is not necessarily finished. 
//If the last entry has no newline, we should noty skip this entry
struct dela_entry* DELAS_entry;
while (l || !feof(dlc)) {
   if ((DELAS_entry=is_strict_DELAS_line(input_line,alph))!=NULL) {
      /* If we have a strict DELAS line, that is to say, one with
       * a simple word */
      SU_forms_T* forms=(SU_forms_T*)malloc(sizeof(SU_forms_T));
      if (!forms) {
         fatal_error("Not enough memory in function inflect\n");
      }
      forms->no_forms=0;
      forms->forms=NULL;
      char inflection_code[1024];
      unichar code_gramm[1024];
      /* We take the first grammatical code, and we extract from it the name
       * of the inflection transducer to use */
      get_inflection_code(DELAS_entry->semantic_codes[0],inflection_code,code_gramm);
      /* And we inflect the word */
      err=SU_inflect(DELAS_entry->lemma,inflection_code,forms);
      /* Then, we print its inflected forms to the output */
      for (int i=0;i<forms->no_forms;i++) {
         u_fprints(forms->forms[i].form,dlcf);
         u_fprints_char(",",dlcf);
         u_fprints(DELAS_entry->lemma,dlcf);
         u_fprints_char(".",dlcf);
         u_fprints(code_gramm,dlcf);
         /* We add the semantic codes, if any */
         for (int j=1;j<DELAS_entry->n_semantic_codes;j++) {
            u_fprints_char("+",dlcf);
            u_fprints(DELAS_entry->semantic_codes[j],dlcf);
         }
         u_fprints_char(":",dlcf);
         u_fprints(forms->forms[i].raw_features,dlcf);
         u_fprints_char("\n",dlcf);
      }
      free_dela_entry(DELAS_entry);
      /* End of simple word case */
   } else {  
      /* If we have not a simple word DELAS line, we try to analyse it
       * as a compound word DELAC line */
      if (config_files_status!=CONFIG_FILES_ERROR) {
         /* If this is a compound word, we process it if and only if the 
          * configuration files have been correctly loaded */
         dlc_entry=(DLC_entry_T*)malloc(sizeof(DLC_entry_T));
         if (!dlc_entry) {
            fatal_error("Not enough memory in function inflect\n");
         }
         /* Convert a DELAC entry into the internal multi-word format */
         err=DLC_line2entry(input_line,dlc_entry);
         if (!err) {
            //Inflect the entry
            MU_forms=(MU_forms_T*)malloc(sizeof(MU_forms_T));
            if (!MU_forms) {
               fatal_error("Not enough memory in function inflect\n");
            }
            err=MU_inflect(dlc_entry->lemma,MU_forms);
            if (!err) {
	            int f;  //index of the current inflected form
	            //Inform the user if no form generated 
	            if (MU_forms->no_forms == 0) {
	               error("No inflected form could be generated for ");
	               DLC_print_entry(dlc_entry);
               }
	            //Print inflected forms
	            for (f=0; f<MU_forms->no_forms; f++) {
	               //Format the inflected form to the DELACF format
	               err=DLC_format_form(output_line,DIC_LINE_SIZE-1,MU_forms->forms[f],dlc_entry);
                  if (!err) {
                     //Print one inflected form at a time to the DELACF file
                     u_fprints(output_line,dlcf);
                     u_fprints(nl,dlcf);
                  }
               }
            }
            MU_delete_inflection(MU_forms);      
            DLC_delete_entry(dlc_entry);
         }
      } else {
         /* We try to inflect a compound word whereas the "Morphology" and/or
          * "Equivalences" file(s) has/have not been loaded */
         if (!flag) {
            /* We use a flag to print the error message only once */
            error("WARNING: Compound words won't be inflected because configuration files\n");
            error("         have not been correctly loaded.\n");
            flag=1;
         }
      }
   }
   //Get next entry
   l = u_fgets(input_line,DIC_LINE_SIZE-1,dlc);
   //Omit the final newline
   if (u_strlen(input_line)>0 && input_line[u_strlen(input_line)-1]==(unichar)'\n') {
      input_line[u_strlen(input_line)-1] = (unichar)'\0';
   }
}
u_fclose(dlc);
u_fclose(dlcf);
return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Converts a DELAC line ('line') into a structured DELAC entry ('entry').
// 'line' is non terminated by a newline.
// Initially, entry has its space allocated but is empty.
// Returns 1 if 'line' is empty, 2 if its format is incorrect, -1 if memory allocation problems, 0 otherwise. 
int DLC_line2entry(unichar* line, DLC_entry_T* entry) {
  int l;  //length of the scanned sequence
  int pos;  //index of the next character to be read
  SU_id_T* unit;

  pos = 0;
  if (!line[pos])   //Empty line
    return 1;

  //Initalize the lemma
  entry->lemma = (MU_lemma_T*) malloc(sizeof(MU_lemma_T));
  if (!entry->lemma) { 
    fatal_error("Not enough memory in function DLC_line2entry\n");
  }
  entry->lemma->no_units = 0;

  //Scan the single units
  while (line[pos] && line[pos] != (unichar) ',') {  //Each DELAC line must contain a comma
    unit = (SU_id_T*) malloc(sizeof(SU_id_T));
    if (!unit) { 
      fatal_error("Not enough memory in function DLC_line2entry\n");
    }
    l = DLC_scan_unit(unit,&(line[pos]));
    if (l <= 0) {
      free(unit);
      MU_delete_lemma(entry->lemma);
      return 2;
    }
    entry->lemma->units[entry->lemma->no_units] = unit;
    entry->lemma->no_units++;
    pos += l;
  }
  
  if (line[pos] != (unichar) ',') {
    error("Comma missing in DELAC line: ");
    u_fprints(line,stderr);
    error(" !\n");
    MU_delete_lemma(entry->lemma);
    return 2;
  }

  //Scan the inflection paradigm
  unichar tmp[DIC_LINE_SIZE];
  pos++;  //Omit the comma
  l = u_scan_until_char(tmp,&(line[pos]),DIC_LINE_SIZE-1,"+:)\\",1);
  pos += l;
  if (!l) {
    error("Inflection paradigm inexistent in line: ");
    u_fprints(line,stderr);
    error(" !\n");
    MU_delete_lemma(entry->lemma);
    return 2;
  }
  entry->lemma->paradigm = (char*) malloc((u_strlen(tmp)+1) * sizeof(char));
  if (!entry->lemma->paradigm)  { 
    fatal_error("Not enough memory in function DLC_line2entry\n");
  }
  for (int c=0; c<=u_strlen(tmp); c++) //Convert to char and copy
    entry->lemma->paradigm[c] = (char) tmp[c];

  //Determine the class (e.g. noun)
  l_class_T* cl;
  cl = DLC_class_para(tmp);
  if (!cl) {    
    error("Impossible to deduce the compound's inflection class (noun, adj, etc.): ");
    u_fprints(line,stderr);
    error(" !\n");
    MU_delete_lemma(entry->lemma);
    return 2;
  }
  entry->lemma->cl = cl;

  //Scan the semantic codes
  l = DLC_scan_codes(entry->codes,&(line[pos]));
  pos += l;

  //Scan the comment
  l = DLC_scan_comment(&(entry->comment),&(line[pos]));
  pos += l;
  
  if (line[pos]) {
    error("Bad format in DELAC line: ");
    u_fprints(line,stderr);
    error(" !\n");
    MU_delete_lemma(entry->lemma);  //delete lemma
    for (int c=0; entry->codes[c]; c++)  //delete codes
      free(entry->codes[c]);  
    free(entry->comment);  //delete comment
    return 2;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Scans a single unit from a DELAC entry. 'line' is non terminated by a newline.
// Initially, 'u' has its space allocated but is empty.
// Returns the length of the scanned sequence, -1 if a format error occurred, -2 if a memory allocation problem occured. 
int DLC_scan_unit(SU_id_T* u,unichar* line) {
  int l;  //length of the scanned sequence
  int pos;  //index of the next caracter to be scanned
  unichar tmp[DIC_LINE_SIZE];
  
  pos = 0;
  //Scan a unit
  l = SU_get_unit(tmp,line,DIC_LINE_SIZE-1,alph,0);  //The single word module determines what is a word and what is a separator, etc.
  if (l<=0) {
    return -1;
  }
  u->form=u_strdup(tmp);
  pos += l;

  //If no lemma indication
  if (line[pos] != (unichar) '(') {
    u->lemma = NULL;
    u->feat = NULL;
  }

  //Scan the unit's description contained between '(' and ')'
  else {
    pos++;  //Omit the '(' 
    //Scan the lemma if any
    u->lemma = (SU_lemma_T*) malloc(sizeof(SU_lemma_T));
    if (!u->lemma) { 
      fatal_error("Not enough memory in function DLC_scan_unit\n");
    }
    l = SU_get_unit(tmp,&(line[pos]),DIC_LINE_SIZE-1,alph,0);  //The single word module determines what is a word and what is a separator, etc.
    if (l < 0) {
      free(u->form);
      SU_delete_lemma(u->lemma);
      return l;
    }
    u->lemma->unit=u_strdup(tmp);
    pos += l;
    
    //Scan the lemma's inflection paradigm
    if (line[pos] != (unichar) '.') {
      error("Dot missing after a unit's lemma: ");
      error(line);
      error(" !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos ++;  //Omit the dot
    unichar u_para[DIC_LINE_SIZE];
    l = u_scan_until_char(u_para,&(line[pos]),DIC_LINE_SIZE-1,"+:\\",1);
    if (!l) {    
      error("Unit's inflection paradigm non existent in DELAC line: ");
      error(line);
      error(" !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;     
    }
    u->lemma->paradigm = (char*) malloc((u_strlen(u_para)+1) * sizeof(char));
    if (! u->lemma->paradigm) {
      fatal_error("Not enough memory in function DLC_scan_unit\n");
    }
    for (int c=0; c<=u_strlen(u_para); c++)
      u->lemma->paradigm[c] = (char)u_para[c];

    //Determine the lemma's inflection class (noun, adj, etc.)
    l_class_T* cl;
    cl = DLC_class_para(u_para);
    if (!cl) {    
      error("Impossible to deduce the unit's inflection class (noun, adj, etc.): ");
      error(line);
      error(" !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    u->lemma->cl = cl;
    pos += l;

    //Scan the unit's inflection features
    unichar tmp[DIC_LINE_SIZE];
    if (line[pos] != (unichar) ':') {
      error("Colon missing after a unit's lemma: ");
      error(line);
      error(" !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos ++;  //Omit the colon
    l = u_scan_until_char(tmp,&(line[pos]),DIC_LINE_SIZE-1,")",1);
    if (l<=0) {
      error("Inflection features missing after ':' for a unit: ");
      error(line);
      error(" !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos += l;
    if (line[pos] != (unichar) ')') {
      error("')' missing after a unit's inflection features: ");
      error(line);
      error(" !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
    pos++; //Omit the ')'
    u->feat = d_get_feat_str(tmp);
    if (! u->feat) {
      error("Incorrect inflection features in a unit: ");
      error(line);
      error(" !\n");
      free(u->form);
      SU_delete_lemma(u->lemma);
      return -1;
    }
  }
  return pos;
}

/////////////////////////////////////////////////////////////////////////////////
// Scans semantic codes (e.g. "+Hum+z1") from a DELAC entry. 'line' is non terminated by a newline.
// The function allocates space for codes scanned. It must be liberated by the calling function.
// Returns the length of the scanned sequence, -1 if a format error occured, -2 if a memory allocation problem occured. 
int DLC_scan_codes(unichar* codes[MAX_CODES],unichar* line) {
  int l; //length of the scanned sequence
  int pos; //position of the current character in line
  int c;  //number of codes
  unichar tmp[DIC_LINE_SIZE];

  pos = 0;
  c = 0;
  while (line[pos] == (unichar) '+') {
    pos ++;  //Omit the '+'
    l = u_scan_until_char(tmp,&(line[pos]),DIC_LINE_SIZE-1,":/\\+",1);
    if (l) {
      codes[c]=u_strdup(tmp);
      pos += l;
      c++;
    }
  }
  codes[c] = NULL; 
  return pos;
}

/////////////////////////////////////////////////////////////////////////////////
// Scans comment (e.g. "/electricity") from a DELAC entry. 'line' is non terminated by a newline.
// The function allocates space for comment scanned. It must be liberated by the calling function.
// Returns the length of the scanned sequence, -1 if a format error occured, -2 if a memory allocation problem occured. 
int DLC_scan_comment(unichar** comment,unichar* line) {
  int l;  //length of the scanned sequence
  unichar tmp[DIC_LINE_SIZE];

  l = 0;
  if (line[0] == '/') {
    line++;  //Omit the '/'
    l = u_scan_until_char(tmp,line,DIC_LINE_SIZE-1,"",1);
    if (l) {
      *comment=u_strdup(tmp);
    }
  }
  else
    *comment = NULL;
  return l+1;  //Length od the comment plus '/'
}
/**************************************************************************************/
/* Deduces the morphological class (e.g. noun) from the inflection paradigm (e.g. "N41").*/
/* If the string beginning the inflection paradigm is not in the list of class        */
/* equivalences, returns NULL.                                                        */
l_class_T* DLC_class_para(unichar* para) {
  unichar cl[MAX_CLASS_NAME];  //buffer for the class string
  int l;   //length of a scanned sequence
  l = u_scan_until_char(cl,para,MAX_CLASS_NAME-1,"0123456789 _;-:/\\+",1);
  if (l) 
    return d_get_class_str(cl);
  else 
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////
// Prints a DELAC entry into a DELAC file..
// If entry void or entry's lemma void returns 1, 0 otherwise.
int DLC_print_entry(DLC_entry_T* entry) {
  unichar line[DIC_LINE_SIZE];
  u_strcpy_char(line,"");
  
  if (!entry || !entry->lemma) 
    return 1;  

  //Print  units
  for (int u=0; u<entry->lemma->no_units; u++) 
    DLC_print_unit(entry->lemma->units[u]);

  //Print paradigm
  u_strcat_char(line,",");
  u_strcat_char(line,entry->lemma->paradigm);
  
  //Concat codes
  for (int c=0; entry->codes[c]; c++) {
    u_strcat_char(line,"+");
    u_strcat(line,entry->codes[c]);
  }

  //Concat comment
  if (entry->comment) {
    u_strcat_char(line,"/");
    u_strcat(line,entry->comment);
  }

  u_strcat_char(line,"\n");

  //Print line
  u_prints(line);
  
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Prints single unit into a DELAC file.
// If 'unit' void returns 1, if memory allocation problem returns -1, 0 otherwise.
int DLC_print_unit(SU_id_T* unit) {
  unichar unit_str[DIC_LINE_SIZE];
  u_strcpy_char(unit_str,"");
  if (!unit)
    return 1;
  
  u_strcat(unit_str,unit->form);
  if (unit->lemma) {
    u_strcat_char(unit_str,"(");
    u_strcat(unit_str,unit->lemma->unit);
    u_strcat_char(unit_str,".");
    u_strcat_char(unit_str,unit->lemma->paradigm);
    if (unit->feat) {
      u_strcat_char(unit_str,":");
      unichar* tmp;
      tmp = d_get_str_feat(unit->feat);
      if (!tmp)
	return -1;
      u_strcat(unit_str,tmp);
      free(tmp);
    }
    u_strcat_char(unit_str,")");      
  }
  u_prints(unit_str);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Puts an inflected multi-word form 'f' corresponding to the DELAC entry 'dlc_entry' into the DELACF format ('entry').
// The resulting enntry may takes up to 'max' characters.
// 'entry' almready has its space allocated.
// Returns 1 on error, 0 otherwise.
int DLC_format_form(unichar* entry, int max, MU_f_T f, DLC_entry_T* dlc_entry) {
  int l;  //length of the entry

  //Inflected form
  l = u_strlen(f.form);
  if (l >= max)
    return 1;
  u_strcpy(entry, f.form);

  //Comma
  l++; 
  if (l >= max)
    return 1;
  u_strcat_char(entry,",");

  //Lemma
  int u;  //index of the current unit in the lemma of the MW form
  for(u=0; u<dlc_entry->lemma->no_units; u++)
    l = l + u_strlen(dlc_entry->lemma->units[u]->form);
  if (l >= max)
    return 1;
  for(u=0; u<dlc_entry->lemma->no_units; u++)
    u_strcat(entry,dlc_entry->lemma->units[u]->form);

  //Full stop
  l++; 
  if (l >= max)
    return 1;
  u_strcat_char(entry,".");

  //Inflection paradigm
  l = l + strlen(dlc_entry->lemma->paradigm);
  if (l >= max)
    return 1;
  u_strcat_char(entry,dlc_entry->lemma->paradigm);

  //Semantic codes
  int c;  //index of the current semantic code
  for (c=0; dlc_entry->codes[c]; c++)
    l = l + u_strlen(dlc_entry->codes[c]) + 1;
  if (l >= max)
    return 1;
  for (c=0; dlc_entry->codes[c]; c++) {
    u_strcat_char(entry,"+");
    u_strcat(entry,dlc_entry->codes[c]);
  }

  //Inflection features
  unichar* feat;  //sequence of single-letter inflection features, e.g. 'sIf'
  if (f.features && f.features->no_cats > 0) {
    feat = d_get_str_feat(f.features);
    l = l + u_strlen(feat) + 1;  //Place for a ':' and all features
    if (l >= max)
      return 1;
    u_strcat_char(entry,":");
    u_strcat(entry,feat);
    free(feat);
  }

  //Comment
  if (dlc_entry->comment && u_strlen(dlc_entry->comment)) {
    l = l + u_strlen(dlc_entry->comment);//Place for a '/' and the comment
    if (l >= max)
      return 1;
    u_strcat_char(entry,"/");
    u_strcat(entry,dlc_entry->comment);    
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Liberates the memory allocated for a DELAC entry.
void DLC_delete_entry(DLC_entry_T* entry) {
  if (!entry)
    return;
  MU_delete_lemma(entry->lemma);
  for (int c=0; entry->codes[c]; c++)  //delete codes
    free(entry->codes[c]);  
  free(entry->comment);  //delete comment
  free(entry);
}

