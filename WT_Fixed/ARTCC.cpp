#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlversion.h>
#include <algorithm>
#include <string>
#include <vector>
#include "InstanceData.h"
#include "Curl.h"
#include "json.h"

using namespace std;
using namespace stdext;

static bool g_bARTCCLoaded = false;
extern "C"
{
	int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
	void DrawGLTextXY(struct InstanceData* id, double x, double y, char *mystr, GLfloat *color, GLuint fontbase, int bRotate);
	void RotateView(struct InstanceData* id, double xMax, double xMin, double yMax, double yMin);

	extern GLfloat trans[3];
	extern GLfloat darkpurple[3];
	extern GLfloat white[3];
}

// Coordinate Class (holds x,y location for Lat/Lng)
class Coordinate
{
public:
	Coordinate(double dLatitude = 0.0, double dLongitude = 0.0)
		:m_dLatitude(dLatitude), m_dLongitude(dLongitude){}

	Coordinate(const Coordinate& rCopy)
		:m_dLatitude(rCopy.m_dLatitude), m_dLongitude(rCopy.m_dLongitude){}

	Coordinate& operator=(const Coordinate& rCopy){
		if(this != &rCopy){
			m_dLatitude = rCopy.m_dLatitude;
			m_dLongitude = rCopy.m_dLongitude;
		}
		return *this;
	}

	double Latitude() const{ return m_dLatitude; }
	void Latitude(double dLatitude) { m_dLatitude = dLatitude; }

	double Longitude() const{ return m_dLongitude; }
	void Longitude(double dLongitude) { m_dLongitude = dLongitude; }

private:
	double m_dLatitude;
	double m_dLongitude;
};

typedef vector<Coordinate> VREGION;
typedef VREGION::iterator VREGION_ITER;

// ARTCC Class Holds all info for an artcc
class ARTCC
{
public:
	ARTCC(const char* szName):
	  m_strFullName(szName){
		  string::size_type s = m_strFullName.find("(");
		  string::size_type e = m_strFullName.find(")");
		  m_strShortName = m_strFullName.substr(s + 1, e - s - 1);
	  }

	Coordinate Center( ) const { return m_cCenter; }
	void Center(const Coordinate& rCenter){ m_cCenter = rCenter; }

	string FullName( ) const { return m_strFullName; };
	string ShortName( ) const { return m_strShortName; };
	
	void Add(unsigned int nRegion, const Coordinate& rLocation){ 
		if(m_vRegions.size() < (nRegion + 1)){
			VREGION vRegion;
			m_vRegions.push_back(vRegion);
		}
		m_vRegions[nRegion].push_back(rLocation);
	}

	void Draw( struct InstanceData* id ){
		// Draw Label First
		if(id->m_sCurLayout.m_nARTCCLabel)
			DrawGLTextXY(id, m_cCenter.Longitude(), m_cCenter.Latitude(), (char*)m_strFullName.c_str(), (GLfloat*)&id->m_sCurLayout.m_glfARTCCColor, id->m_arFonts[id->m_sCurLayout.m_nARTCCFontIndex].m_glListBase, TRUE);

		// Setup and Draw ARTCC
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(id->m_sCurLayout.m_dWorldXmin, id->m_sCurLayout.m_dWorldXmax, id->m_sCurLayout.m_dWorldYmin, id->m_sCurLayout.m_dWorldYmax, 0.0f, 1.0f);
		
		RotateView(id, id->m_sCurLayout.m_dWorldXmax, id->m_sCurLayout.m_dWorldXmin, 
			id->m_sCurLayout.m_dWorldYmax, id->m_sCurLayout.m_dWorldYmin);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(trans[0], trans[1], trans[2] );
		glLineWidth(1);
		glDisable(GL_DEPTH_TEST);
		glDisable( GL_BLEND );		

		glColor3fv((GLfloat*)id->m_sCurLayout.m_glfARTCCColor);
		glBegin(GL_LINE_LOOP);
		for ( vector<VREGION>::iterator i = m_vRegions.begin(); i != m_vRegions.end(); i++){
			for ( VREGION_ITER j = (*i).begin(); j != (*i).end(); j++){
				float lat = (float) (*j).Latitude();
				float lng = (float) (*j).Longitude();
				glVertex3f( lng, lat, 0.0f );
			}
		}
		glEnd();

		glFlush ();
		glPopMatrix();

		glColor3fv(white);
	}

private:
	string m_strFullName;
	string m_strShortName;
	Coordinate m_cCenter;
	vector<VREGION> m_vRegions;
};

vector<ARTCC*> g_vARTCCs;

bool Compare(ARTCC* p1, ARTCC* p2){
	return (p1->ShortName() < p2->ShortName());
}

extern "C" {

void DrawARTCCs( struct InstanceData* id )
{
	for(vector<ARTCC*>::iterator i = g_vARTCCs.begin(); i != g_vARTCCs.end(); i++){
		if(id->m_sCurLayout.m_pstrARTCC && strstr(id->m_sCurLayout.m_pstrARTCC, (*i)->ShortName().c_str()))
			(*i)->Draw(id);
	}
}

const char* ARTCCtoJSON( struct InstanceData* id )
{
	static char* strReturn = NULL;
	if(!strReturn && g_bARTCCLoaded){		
		struct json_object* jobj = json_object_new_object();
		struct json_object* jarr = json_object_new_array();
		sort(g_vARTCCs.begin(), g_vARTCCs.end(), Compare);
		for(vector<ARTCC*>::iterator i = g_vARTCCs.begin(); i != g_vARTCCs.end(); i++){
			json_object_array_add(jarr, json_object_new_string((*i)->ShortName().c_str()));
		}
		json_object_object_add(jobj, "_class", json_object_new_string("ARTCC"));
		json_object_object_add(jobj, "ARTCC", jarr);

		if(json_object_array_length(jarr)){
			const char* json_string = json_object_to_json_string(jobj);
			int nLen = strlen(json_string);
			strReturn = new char[nLen+1];
			strcpy_s(strReturn, nLen+1, json_string);
		}
		json_object_put(jobj);
	}
	return strReturn;
}
}

xmlNodePtr FindChildNode(const char* name, xmlNodePtr children)
{
	xmlNodePtr ret = NULL;
	xmlNodePtr cur = children;
	while(cur && !ret){
		if(!strcmp((const char*)cur->name, name)){
			ret = cur;
		}
		cur = cur->next;
	}
	return ret;
}

void ReleaseARTCCs( struct InstanceData* id ){
	for(vector<ARTCC*>::iterator i = g_vARTCCs.begin(); i != g_vARTCCs.end(); i++){
		ARTCC* p = (*i);
		delete p;
	}
	g_vARTCCs.clear();
	g_bARTCCLoaded = false;
}

void LoadARTCCs( struct InstanceData* id )
{
	char strFilePath[MAX_PATH_SZ] = {0};

	// Check if we have loaded the ARTCC's yet
	if(g_bARTCCLoaded || g_vARTCCs.size())
		return;

	sprintf_s(strFilePath, MAX_PATH_SZ, "%s\\%s", id->m_strMapDir, "FAA_ARTCC.kml");

	if ( GetLocalFile(id, id->m_strMapDir, "FAA_ARTCC.kml", id->m_strHttpMapPath, SM_HTTPS)) {

		xmlXPathContextPtr pContext = NULL;
		xmlNodePtr pNode = NULL, pChild = NULL;
		xmlXPathObjectPtr pObject = NULL;
		xmlDocPtr pDoc = xmlParseFile(strFilePath);

		xmlKeepBlanksDefault(0);

		if(pDoc){
			// Walk Nodes to get Needed Information: (Placemark's -  name and coordinates)
			pContext = xmlXPathNewContext(pDoc);
			if(pContext){
				pObject = xmlXPathEval((const xmlChar*)"//Placemark", pContext); 
				//pObject = xmlXPathEval((const xmlChar*)"//Placemark/MultiGeometry/Point", pContext); 
				if(pObject && pObject->type == XPATH_NODESET){
					pNode = *pObject->nodesetval->nodeTab;
					while(pNode){
						ARTCC* pARTCC = NULL;
						pChild = FindChildNode("name", pNode->children);
						if(pChild){
							xmlChar* xbuf = xmlNodeListGetString(pDoc, pChild->children, 1);
							pARTCC = new ARTCC((char*) xbuf);
						}
						pChild = FindChildNode("MultiGeometry", pNode->children);
						if(pChild){
							pChild = FindChildNode("Point", pChild->children);
							if(pChild){
								pChild = FindChildNode("coordinates", pChild->children);
								if(pChild){
									char* context = NULL;
									char* elem = NULL;
									Coordinate Location;
									xmlChar* xbuf = xmlNodeListGetString(pDoc, pChild->children, 1);
									elem = strtok_s((char*)xbuf, ",", &context);
									while(elem){
										if(Location.Longitude() == 0.0)
											Location.Longitude(atof(elem));
										else if(Location.Latitude() == 0.0)
											Location.Latitude(atof(elem));

										elem = strtok_s(NULL, ",", &context);
									}
									if(pARTCC) pARTCC->Center(Location);
								}
							}
						}
						pChild = FindChildNode("MultiGeometry", pNode->children);
						if(pChild){
							int cnt = 0;
							pChild = FindChildNode("LineString", pChild->children);
							while(pChild){
								xmlNodePtr pChild2 = FindChildNode("coordinates", pChild->children);
								if(pChild2){									
									char* context = NULL;
									char* elem = NULL;
									xmlChar* xbuf = xmlNodeListGetString(pDoc, pChild2->children, 1);
									elem = strtok_s((char*)xbuf + 1, " ", &context);
									while(elem){
										char* context2 = NULL;
										char* elem2 = NULL;
										Coordinate Location;
										elem2 = strtok_s(elem, ",", &context2);
										while(elem2){
											if(Location.Longitude() == 0.0)
												Location.Longitude(atof(elem2));
											else if(Location.Latitude() == 0.0)
												Location.Latitude(atof(elem2));
											elem2 = strtok_s(NULL, ",", &context2);
										}
										if(pARTCC) pARTCC->Add(cnt, Location);
										elem = strtok_s(NULL, " ", &context);										
									}
									cnt++;
								}
								pChild = pChild->next;								
							}
						}
						if(pARTCC){ 
							g_vARTCCs.push_back(pARTCC);
						}
						pNode = pNode->next;
					}
					xmlXPathFreeObject(pObject);
				}
				xmlXPathFreeContext(pContext);
			}
			xmlFreeDoc(pDoc);
			g_bARTCCLoaded = true;			
		}
	}
}