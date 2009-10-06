 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
  *
  */

#ifndef _ACTIVITY_LOGGER_PLUG_CALLBACK_INCLUDED
#define _ACTIVITY_LOGGER_PLUG_CALLBACK_INCLUDED 1


#include "AbstractCallbackFuncModifier.h"
#include "ActivityLogger.h"
#include "Af_stdio.h"
#include "UnitexTool.h"

#ifdef __cplusplus
extern "C" {
#endif




typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_before_af_fopen)(const char* name,const char* MODE,void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_after_af_fopen)(const char* name,const char* MODE,ABSTRACTFILE*,void* privateLoggerPtr);

typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_before_af_fclose)(ABSTRACTFILE*,void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_after_af_fclose)(ABSTRACTFILE*,int,void* privateLoggerPtr);

typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_before_af_rename)(const char* name1,const char* name2,void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_after_af_rename)(const char* name1,const char* name2,int,void* privateLoggerPtr);

typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_before_af_copy)(const char* name1,const char* name2,void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_after_af_copy)(const char* name1,const char* name2,int,void* privateLoggerPtr);

typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_before_af_remove)(const char* name,void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_after_af_remove)(const char* name,int,void* privateLoggerPtr);


typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_before_calling_tool)(mainFunc*,int argc,char* argv[],void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_after_calling_tool)(mainFunc*,int argc,char* argv[],int,void* privateLoggerPtr);


/* two optional (can be just NULL) callbacks to initialize and uninitialize the logger */
typedef int (ABSTRACT_CALLBACK_UNITEX* t_fnc_Init_Logger)(void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_Uninit_Logger)(void* privateLoggerPtr);

typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_LogOutWrite)(const void*Buf, size_t size,void* privateLoggerPtr);
typedef void (ABSTRACT_CALLBACK_UNITEX *t_fnc_LogErrWrite)(const void*Buf, size_t size,void* privateLoggerPtr);

typedef struct
{
    unsigned int size_struct;

    t_fnc_Init_Logger fnc_Init_Logger;
    t_fnc_Uninit_Logger fnc_Uninit_Logger;

    t_fnc_before_af_fopen fnc_before_af_fopen;
    t_fnc_after_af_fopen  fnc_after_af_fopen;

    t_fnc_before_af_fclose fnc_before_af_fclose;
    t_fnc_after_af_fclose fnc_after_af_fclose;

    t_fnc_before_af_rename fnc_before_af_rename;
    t_fnc_after_af_rename fnc_after_af_rename;

    t_fnc_before_af_copy fnc_before_af_copy;
    t_fnc_after_af_copy fnc_after_af_copy;

    t_fnc_before_af_remove fnc_before_af_remove;
    t_fnc_after_af_remove fnc_after_af_remove;

    t_fnc_before_calling_tool fnc_before_calling_tool;
    t_fnc_after_calling_tool fnc_after_calling_tool;

    t_fnc_LogOutWrite fnc_LogOutWrite;
    t_fnc_LogErrWrite fnc_LogErrWrite;
} t_logger_func_array;

/* these functions respectively add and remove filespaces.
  you can add several filespaces with the same func_array callback set, but with different privateSpacePtr
  privateSpacePtr is the parameters which can be set as the last parameter of each callback */
UNITEX_FUNC int UNITEX_CALL AddLogger(const t_logger_func_array* func_array,void* privateLoggerPtr);
UNITEX_FUNC int UNITEX_CALL RemoveLogger(const t_logger_func_array* func_array,void* privateLoggerPtr);

/* just return the number of AbstractFileSpace Installed */
UNITEX_FUNC int UNITEX_CALL GetNbLoggerInstalled();

/**********************************************************************/





#ifdef __cplusplus
}
#endif

#endif