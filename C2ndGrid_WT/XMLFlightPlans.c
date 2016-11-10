#include < windows.h >
#include <fcntl.h>
#include "unzip.h"
#include "tailor.h"
#include <gd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <math.h>
#include <commctrl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <libxml/xmlIO.h>
#include <libxml/uri.h>
#include "vo.h"
#include "vo_extern.h"
#include "SSI.h"
#include "ExecUtil.h"
#include "vo_db.h"
#include "TexFont.h"
#include "shapefil.h"
#include "Tracks.h"
#include "nexrad.h"
#include "mgl.h"
#include "resource.h"
//#include "gfx.h"
#include "srfc.h"
#include "fnmatch.h"
#include "InstanceData.h"



#pragma comment ( lib, "GLU32.lib" )
//#pragma comment ( lib, "GLUT32.lib" )
#pragma comment ( lib, "OPENGL32.lib" )


extern VO *GetDelayArpts( struct InstanceData* id );

// retrieve xml file using xmllib from nadder


static void
print_element_names(xmlNode * a_node, xmlDocPtr XMLDocPtr)
{
    xmlNode *cur = NULL;
    char *str;

    for (cur = a_node; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur->name);
            str = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
        }

        print_element_names(cur->children, XMLDocPtr);
    }
}


ProcessFltRow(xmlDocPtr XMLDocPtr, xmlNodePtr cur)
{
  const xmlChar *str;
  char route[512];

  
  while (cur != NULL) {
    str = cur->name;
    str = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
    if ( !strcmp(cur->name, "fltroute") ){
      if ( str ){
        strncpy_s(route, sizeof(route), str, _TRUNCATE );
      }
    }
    cur = cur->next;
  }

}





void ProcessFAAXML( struct InstanceData* id, char *xmlstr, int xmlsize )
{
  struct row_index *row_index;
  xmlDocPtr		XMLDocPtr;				// Company Travelers list xml document pointer
  xmlNodePtr cur, curchild, delaychild, groundchild, adchild;
  xmlDocPtr xmlParseMemory();
  const xmlChar *str;
  const char *updatetimestr, *dtdstr, *delaynamestr, *delaytypestr, *airportcode, 
    *reasonstr, *Arrival_Departure, *MinStr, *MaxStr, *TrendStr, *End_Time, *Arrival_Departure_Type,
    *AvgStr, *CTL_Element, *FCA_Start_DateTime, *FCA_End_DateTime, *AFP_StartTime, *AFP_EndTime, *Line,
    *Lat, *Long, *Floor, *Ceiling, *Reopen, *Start;
  char tmpstr[ TMPBUF_SIZE ];
  int len, i, delay_val;
  char *statstr, *mergestr, *curstr;
  const DWORD cdwThreadId = GetCurrentThreadId();
  extern int xmlKeepBlanksDefault();

  // move to another file and use the xml library nadder found
 
  WaitForSingleObject( hFAAMutex, INFINITE );


  if ( !DelayArptsVO ){
     if( !(DelayArptsVO = GetDelayArpts(id)) ){
		 vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to process FAAXML, DelayArptsVO is NULL, GetDelayAirports Failed!\n", cdwThreadId);
		 return;
	 }
  }
 
  
  

  /* COMPAT: Do not generate nodes for formatting spaces */
  LIBXML_TEST_VERSION
  xmlKeepBlanksDefault(0);
  
  // do a sanity check on the xml to make sure it is xml and not a server error
  // xmllib will crash if it is not xml
  if ( !strstr( xmlstr, "AIRPORT_STATUS_INFORMATION" )){
    goto releasemutex;
  }

  if ( ! (XMLDocPtr = xmlParseMemory ( xmlstr, xmlsize ) ) )
  {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ERROR: xmlParseMemory() failed\n", cdwThreadId );
    goto releasemutex;
  }
  

  /* clear out old strings */
  for (i = 0; i < DelayArptsVO->count; i++ ){
    if ( (statstr = VV(DelayArptsVO, i, "StatusStr", char *))){
      free( statstr );
      VV(DelayArptsVO, i, "StatusStr", char *) = NULL;
    }
    VV(DelayArptsVO, i, "colorcode", int ) = FAA_GREEN;
  }

  /*
  * Check the document is of the right kind
  */
  
  cur = xmlDocGetRootElement(XMLDocPtr);
  if (cur == NULL) {
    xmlFreeDoc(XMLDocPtr);
    goto releasemutex;
  }

  // print_element_names(cur, XMLDocPtr);

  if (strcmp(cur->name, "AIRPORT_STATUS_INFORMATION" ) ) {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) document of the wrong type, root node != AIRPORT_STATUS_INFORMATION", cdwThreadId);
    xmlFreeDoc(XMLDocPtr);
    goto releasemutex;
  }

  /*
  * Now, walk the tree.
  */
  /* First level  */
  cur = cur->xmlChildrenNode;
  while ( cur && xmlIsBlankNode ( cur ) )
  {
    str = cur->name;
    str = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
    cur = cur -> next;
  }
	Reopen = "";
	airportcode = "";
	Start = "";
	reasonstr = "";
  for (; cur; cur = cur->next){
    if ( !strcmp(cur->name, "Update_Time")) {
      str = cur->name;
      updatetimestr = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
    } else if ( !strcmp(cur->name, "Dtd_File")) {
      str = cur->name;
      dtdstr = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
    } else if ( !strcmp(cur->name, "Delay_type")) {
      str = cur->name;
      delaytypestr = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
      for (curchild = cur->xmlChildrenNode; curchild; curchild = curchild->next ){
        str = curchild->name;
        if ( !strcmp( curchild->name, "Ground_Delay_List" ) ){
          for (delaychild = curchild->xmlChildrenNode; delaychild; delaychild = delaychild->next ){
            
            str = delaychild->name;
            str = xmlNodeListGetString( XMLDocPtr, delaychild->children, 1 );
            if ( !strcmp(delaychild->name, "Ground_Delay")) {
              for (groundchild = delaychild->xmlChildrenNode; groundchild; groundchild = groundchild->next ){
                str = groundchild->name;
                str = xmlNodeListGetString( XMLDocPtr, groundchild->children, 1 );
                if ( !strcmp( groundchild->name, "ARPT" ) ){
                  airportcode = str;
                } else if ( !strcmp( groundchild->name, "Reason" ) ){// Reason, Arrival_Departure, Arrival_Departure
                  reasonstr = str;
                } else if ( !strcmp( groundchild->name, "Avg" ) ){
                  AvgStr = str;
                } else if ( !strcmp( groundchild->name, "Max" ) ){
                  MaxStr = str;
                }
              }
              // write into delay vo
              sprintf_s(tmpstr, TMPBUF_SIZE, "Due to %s,\ndeparture traffic destined to %s is\n"
                "subject to a ground delay program\n"
                "Delays average %s",
                reasonstr, airportcode, AvgStr );
              len = strlen( tmpstr ) + 1;
              statstr = malloc( len );
              strcpy_s( statstr, len, tmpstr );
              
              delay_val = atoi( AvgStr );
              if ( strstr( AvgStr, "hour" ) ){
                delay_val *= 60;
              }

              if ( (row_index = (struct row_index *) vo_search(DelayArptsVO, "code", airportcode, NULL )) ){
                if ( (curstr = VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) ) &&
                  strlen( curstr ) ){
                  len = strlen( curstr ) + strlen( statstr ) + 3;
                  mergestr = malloc( len );  // add in room for additional 2 newlines and '\0'
                  strcpy_s(mergestr, len, curstr );
                  strcat_s(mergestr, len, "\n\n" );
                  strcat_s(mergestr, len, statstr);
                  free( curstr );
                  free( statstr );
                  statstr = mergestr;
                }
                VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) = statstr;
				statstr = NULL;
                if ( delay_val <= 45 ){
                  VV(DelayArptsVO, row_index->rownum, "colorcode", int ) = FAA_YELLOW;
                } else {
                  VV(DelayArptsVO, row_index->rownum, "colorcode", int ) = FAA_RED;
                }
              } else {
                vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) no index for airport '%s'\n", cdwThreadId, airportcode );
              }
	  		  if (statstr) free(statstr);
			  statstr = NULL;
            }
          }
        } else if ( !strcmp( curchild->name, "Airport_Closure_List") ){
					
          for (delaychild = curchild->xmlChildrenNode; delaychild; delaychild = delaychild->next ){
            
            str = delaychild->name;
            str = xmlNodeListGetString( XMLDocPtr, delaychild->children, 1 );
					
            if ( !strcmp(delaychild->name, "Airport")) {
              for (groundchild = delaychild->xmlChildrenNode; groundchild; groundchild = groundchild->next ){
                str = groundchild->name;
                str = xmlNodeListGetString( XMLDocPtr, groundchild->children, 1 );
                if ( !strcmp( groundchild->name, "ARPT" ) ){
                  airportcode = str;
                } else if ( !strcmp( groundchild->name, "Reason" ) ){// Reason, Arrival_Departure, Arrival_Departure
                  reasonstr = str;
                } else if ( !strcmp( groundchild->name, "Start" ) ){
                  Start = str;  
                } else if ( !strcmp( groundchild->name, "Reopen" ) || !strcmp( groundchild->name, "Repen" ) ){
									// test for mis-spelled version "Repen" since old versions of Insight had that bug
									// server PulseTrack.fcg was modified to misspell the word soo that old version of Insight won't crash
									// test for both spellings so that eventually we can change the server process to spell it correctly
                  Reopen = str;  
                }
              }
              sprintf_s(tmpstr, TMPBUF_SIZE, "Due to %s, %s is Closed\n"
                "It is expected to reopen at %s",                 
                reasonstr, airportcode, Reopen);
              len = strlen( tmpstr ) + 1;
              statstr = malloc( len );
              strcpy_s( statstr, len, tmpstr );
              
              if ( (row_index = (struct row_index *) vo_search(DelayArptsVO, "code", airportcode, NULL )) ){
                if ( (curstr = VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) ) &&
                  strlen( curstr ) ){
                  len = strlen( curstr ) + strlen( statstr ) + 3;
                  mergestr = malloc( len );  // add in room for additional 2 newlines and '\0'
                  strcpy_s(mergestr, len, curstr );
                  strcat_s(mergestr, len, "\n\n" );
                  strcat_s(mergestr, len, statstr);
                  free( curstr );
                  free( statstr );
                  statstr = mergestr;
                }
                VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) = statstr;
                VV(DelayArptsVO, row_index->rownum, "colorcode", int ) = FAA_RED;
				statstr = NULL;
              } else {
                vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) no index for airport '%s'\n", cdwThreadId, airportcode );
              }
			  if (statstr) free(statstr);
			  statstr = NULL;
            }
          }
        } else if ( !strcmp( curchild->name, "Ground_Stop_List") ){
          for (delaychild = curchild->xmlChildrenNode; delaychild; delaychild = delaychild->next ){           
            str = delaychild->name;
            str = xmlNodeListGetString( XMLDocPtr, delaychild->children, 1 );
            if ( !strcmp(delaychild->name, "Program")) {
              for (groundchild = delaychild->xmlChildrenNode; groundchild; groundchild = groundchild->next ){
                str = groundchild->name;
                str = xmlNodeListGetString( XMLDocPtr, groundchild->children, 1 );
                if ( !strcmp( groundchild->name, "ARPT" ) ){
                  airportcode = str;
                } else if ( !strcmp( groundchild->name, "Reason" ) ){// Reason, Arrival_Departure, Arrival_Departure
                  reasonstr = str;
                } else if ( !strcmp( groundchild->name, "End_Time" ) ){
                  End_Time = str;  
                }
                
              }
               // write into delay vo
              sprintf_s(tmpstr, TMPBUF_SIZE, "Due to %s, there is a Traffic\n"
                "Management (Ground Stop) Program\n"
                "In effect for traffic arriving %s.\nUntil %s",
                reasonstr, airportcode, End_Time );
              len = strlen( tmpstr ) + 1;
              statstr = malloc( len );
              strcpy_s( statstr, len, tmpstr );
              
              if ( (row_index = (struct row_index *) vo_search(DelayArptsVO, "code", airportcode, NULL )) ){
                if ( (curstr = VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) ) &&
                  strlen( curstr ) ){
                  len = strlen( curstr ) + strlen( statstr ) + 3;
                  mergestr = malloc( len );  // add in room for additional 2 newlines and '\0'
                  strcpy_s(mergestr, len, curstr );
                  strcat_s(mergestr, len, "\n\n" );
                  strcat_s(mergestr, len, statstr);
                  free( curstr );
                  free( statstr );
                  statstr = mergestr; 
                }
                VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) = statstr;
                VV(DelayArptsVO, row_index->rownum, "colorcode", int ) = FAA_ORANGE;
				statstr = NULL;
              } else {
                vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) no index for airport '%s'\n", cdwThreadId, airportcode );
              }
			  if (statstr) free(statstr);
			  statstr = NULL;
            }
          }
        } else if ( !strcmp( curchild->name, "Airspace_Flow_List") ){
          for (delaychild = curchild->xmlChildrenNode; delaychild; delaychild = delaychild->next ){
            
            str = delaychild->name;
            str = xmlNodeListGetString( XMLDocPtr, delaychild->children, 1 );
            if ( !strcmp(delaychild->name, "Airspace_Flow")) {
              for (groundchild = delaychild->xmlChildrenNode; groundchild; groundchild = groundchild->next ){
                str = groundchild->name;
                str = xmlNodeListGetString( XMLDocPtr, groundchild->children, 1 );
                if ( !strcmp( groundchild->name, "CTL_Element" ) ){
                  CTL_Element = str;
                } else if ( !strcmp( groundchild->name, "Reason" ) ){// Reason, Arrival_Departure, Arrival_Departure
                  reasonstr = str;
                } else if ( !strcmp( groundchild->name, "FCA_Start_DateTime") ){
                  FCA_Start_DateTime = str;
                } else if ( !strcmp( groundchild->name, "FCA_Start_DateTime") ){
                  FCA_End_DateTime = str;
                } else if ( !strcmp( groundchild->name, "AFP_StartTime") ){
                  AFP_StartTime = str;
                } else if ( !strcmp( groundchild->name, "AFP_EndTime") ){
                  AFP_EndTime = str;
                } else if ( !strcmp( groundchild->name, "Line") ){
                  Line = str;
                  for (adchild = groundchild->xmlChildrenNode; adchild; adchild = adchild->next ){
                    str = adchild->name;
                    str = xmlNodeListGetString( XMLDocPtr, adchild->children, 1 );
                    if ( !strcmp( adchild->name, "Point" ) ){
                      Lat = xmlGetProp(adchild, "Lat");
                      Long = xmlGetProp(adchild, "Long");
                      
                    }
                  }
                  
                } else if ( !strcmp( groundchild->name, "Avg") ){
                  AvgStr = str;
                } else if ( !strcmp( groundchild->name, "Floor") ){
                  Floor = str;
                } else if ( !strcmp( groundchild->name, "Ceiling") ){
                  Ceiling = str;
                }
              }
            }
          }
        } else if ( !strcmp( curchild->name, "Arrival_Departure_Delay_List") ){
          for (delaychild = curchild->xmlChildrenNode; delaychild; delaychild = delaychild->next ){
            
            str = delaychild->name;
            str = xmlNodeListGetString( XMLDocPtr, delaychild->children, 1 );
            if ( !strcmp(delaychild->name, "Delay")) {
              for (groundchild = delaychild->xmlChildrenNode; groundchild; groundchild = groundchild->next ){
                str = groundchild->name;
                str = xmlNodeListGetString( XMLDocPtr, groundchild->children, 1 );
                if ( !strcmp( groundchild->name, "ARPT" ) ){
                  airportcode = str;
                } else if ( !strcmp( groundchild->name, "Reason" ) ){// Reason, Arrival_Departure, Arrival_Departure
                  reasonstr = str;
                } else if ( !strcmp( groundchild->name, "Arrival_Departure" ) ){
                  Arrival_Departure = str;
                  Arrival_Departure_Type = xmlGetProp(groundchild, "Type");
                  for (adchild = groundchild->xmlChildrenNode; adchild; adchild = adchild->next ){
                    str = adchild->name;
                    str = xmlNodeListGetString( XMLDocPtr, adchild->children, 1 );
                    if ( !strcmp( adchild->name, "Min" ) ){
                      MinStr = str;
                    } else if ( !strcmp( adchild->name, "Max" ) ){
                      MaxStr = str;
                    } else if ( !strcmp( adchild->name, "Trend" ) ){
                      TrendStr = str;
                    }
                  }
                  // write into delay vo
                  sprintf_s(tmpstr, TMPBUF_SIZE,
                    "%s is experiencing %s delays\n"
                    "due to %s. Delays\n"
                    "vary between %s\n"
                    "and %s and are %s.",
                    airportcode, Arrival_Departure_Type, reasonstr,
                                    MinStr, MaxStr, TrendStr );
                  len = strlen( tmpstr ) + 1;
                  statstr = malloc( len );
                  strcpy_s( statstr, len, tmpstr );

                  delay_val = atoi( MaxStr );
                  if ( strstr( MaxStr, "hour" ) ){
                    delay_val *= 60;
                  }
                  
                  if ( (row_index = (struct row_index *) vo_search(DelayArptsVO, "code", airportcode, NULL )) ){
                    if ( (curstr = VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) ) &&
                      strlen( curstr ) ){
                      len = strlen( curstr ) + strlen( statstr ) + 3;
                      mergestr = malloc( len );  // add in room for additional 2 newlines and '\0'
                      strcpy_s(mergestr, len, curstr );
                      strcat_s(mergestr, len, "\n\n" );
                      strcat_s(mergestr, len, statstr);
                      free( curstr );
                      free( statstr );
                      statstr = mergestr;
                    }
                    VV(DelayArptsVO, row_index->rownum, "StatusStr", char *) = statstr;
					statstr = NULL;
                    if ( delay_val <= 45 ){
                      VV(DelayArptsVO, row_index->rownum, "colorcode", int ) = FAA_YELLOW;
                    } else {
                      VV(DelayArptsVO, row_index->rownum, "colorcode", int ) = FAA_RED;
                    }
                  } else {
                    vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) no index for airport '%s'\n", cdwThreadId, airportcode );
                  }
				  if (statstr) free(statstr);
				  statstr = NULL;
                }
              }
            }
          }
        } else if ( !strcmp( curchild->name, "Name" ) ){
          delaynamestr = xmlNodeListGetString( XMLDocPtr, curchild->children, 1 );  
        } 
        
      }
    }
  }
  xmlFreeDoc(XMLDocPtr);

releasemutex:
  ReleaseMutex( hFAAMutex);
  return;

}





void ProcessXMLFltPlans( struct InstanceData* id, char *xmlstr, int xmlsize )
{
  xmlDocPtr		XMLDocPtr;				// Company Travelers list xml document pointer
  xmlNodePtr cur, curchild;
  xmlDocPtr xmlParseMemory();
  const xmlChar *str;
  const DWORD cdwThreadId = GetCurrentThreadId();

  // move to another file and use the xml library nadder found
 
  
  /* COMPAT: Do not generate nodes for formatting spaces */
  LIBXML_TEST_VERSION
  xmlKeepBlanksDefault(0);
  
  
  if ( ! (XMLDocPtr = xmlParseMemory ( xmlstr, xmlsize ) ) )
  {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ERROR: xmlParseMemory() failed\n", cdwThreadId );
    return;
  }
  
  
  /*
  * Check the document is of the right kind
  */
  
  cur = xmlDocGetRootElement(XMLDocPtr);
  if (cur == NULL) {
    xmlFreeDoc(XMLDocPtr);
    return;
  }

  // print_element_names(cur, XMLDocPtr);

  if (strcmp(cur->name, "Root" ) ) {
    fprintf(stderr,"document of the wrong type, root node != Helping");
    xmlFreeDoc(XMLDocPtr);
    return;
  }

  /*
  * Now, walk the tree.
  */
  /* First level  */
  cur = cur->xmlChildrenNode;
  while ( cur && xmlIsBlankNode ( cur ) )
  {
    str = cur->name;
    str = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
    cur = cur -> next;
  }
  
  for (; cur; cur = cur->next){
    if ( strncmp(cur->name, "VO", 2)) {
      str = cur->name;
      str = xmlNodeListGetString( XMLDocPtr, cur->children, 1 );
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) document of the wrong type, was '%s', VOXXXX expected",
        cdwThreadId, cur->name);  
      return;
    }
    /* Second level is a list of Job, but be laxist */
    for (curchild = cur->xmlChildrenNode; curchild; curchild = curchild->next ){
   
      str = curchild->name;
      str = xmlNodeListGetString( XMLDocPtr, curchild->children, 1 );
      
      ProcessFltRow(XMLDocPtr, curchild->xmlChildrenNode);
    }
  }


    
  return; 
}


