#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>
#include <string>
#include <vector>
#include <hash_map>
#include "InstanceData.h"
#include "Curl.h"
#include "json.h"

using namespace std;
using namespace stdext;

extern "C"
{
	int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
	void DrawGLTextXY(struct InstanceData* id, double x, double y, char *mystr, GLfloat *color, GLuint fontbase, int bRotate);
	void RotateView(struct InstanceData* id, double xMax, double xMin, double yMax, double yMin );

	extern GLfloat trans[3];
	extern GLfloat darkpurple[3];
	extern GLfloat larry_red[3];
	extern GLfloat white[3];
}

class Fix
{
public:
	Fix(){};
	Fix(string strName):m_strName(strName){ }

	string Name( ) const { return m_strName; }
	void Name(string strName ) { m_strName = strName; }

	string FixName( ) const { return m_strFix; }
	void FixName(string strFix ) { m_strFix = strFix; }

	double Latitude( ) const { return m_dLatitude; }
	void Latitude(double dLatitude ) { m_dLatitude = dLatitude; }

	double Longitude( ) const { return m_dLongitude; }
	void Longitude(double dLongitude ) { m_dLongitude = dLongitude; }

	string Type( ) const { return m_strType; }
	void Type(string strType ) { m_strType = strType; }

private:
	string m_strName;
	string m_strFix;
	string m_strType;
	double m_dLatitude;
	double m_dLongitude;
};

class Turn
{
public:
	typedef vector<Fix*> V_FIXES;
	typedef vector<Fix*>::iterator V_FIXES_ITER;

	Turn(){}
	Turn(string strName):m_strName(strName){ }
	Turn(const Turn* pTurn):
		m_strName(pTurn->m_strName),
		m_strID(pTurn->m_strID),
		m_vPath(pTurn->m_vPath)
	{ 
	}

	string Name( ) const { return m_strName; }
	void Name(string strName ) { m_strName = strName; }

	string ID( ) const { return m_strID; }
	void ID(string strID ) { m_strID = strID; }

	void Add(Fix* pFix){ m_vPath.push_back(pFix); }
	
	virtual void Draw(struct InstanceData* id, int label = 1, GLfloat* color = (GLfloat*)larry_red, GLuint fontbase = 16){
		// Draw Labels First
		if(label)
		{
			for ( V_FIXES_ITER i = m_vPath.begin(); i != m_vPath.end(); i++){
				float lat = (float) (*i)->Latitude();
				float lng = (float) (*i)->Longitude();
				if((*i)->Type() != "AIRPORT")
					DrawGLTextXY(id, lng, lat, (char*)(*i)->Name().c_str(), color, fontbase, TRUE);
			}		
		}

		if( m_vPath.size() > 1 ){
			// Setup and Draw Routes
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
			glDisable(GL_BLEND);		

			glColor3fv(color);
			glBegin(GL_LINE_STRIP);
			for ( V_FIXES_ITER i = m_vPath.begin(); i != m_vPath.end(); i++){
				float lat = (float) (*i)->Latitude();
				float lng = (float) (*i)->Longitude();
				glVertex3f( lng, lat, 0.0f );
			}
			glEnd();

			glFlush ();
			glPopMatrix();

			glColor3fv(white);
		}
	}

private:
	string m_strName;
	string m_strID;
	V_FIXES m_vPath;
};

class Route: public Turn
{
public:
	typedef hash_map<string, Turn*> HM_ALTS;
	typedef hash_map<string, Turn*>::iterator HM_ALTS_ITER;

	Route(){}
	Route(string strName):Turn(strName){ }

	void Add(Turn* pAlt){ m_hmAlts[pAlt->ID()] = pAlt; }
	Turn* Get(string strAltID){
		HM_ALTS_ITER i = m_hmAlts.find(strAltID);
		if(i != m_hmAlts.end())
			return i->second;
		return NULL;
	}

	virtual void Draw( struct InstanceData* id ){
		Turn::Draw(id);
		for(HM_ALTS_ITER i = m_hmAlts.begin(); i != m_hmAlts.end(); i++){
			i->second->Draw(id);
		}
	}
	virtual void Draw(struct InstanceData* id, string alt, int label, GLfloat* color, GLuint fontbase){
		Turn::Draw(id, label, color, fontbase);
		if(!alt.empty() && m_hmAlts.find(alt) != m_hmAlts.end())
			m_hmAlts[alt]->Draw(id, label, color, fontbase);
	}

private:
	HM_ALTS m_hmAlts;
};

class Airport
{
public:
	typedef hash_map<string, Route*> HM_RTS;
	typedef hash_map<string, Route*>::iterator HM_RTS_ITER;

	Airport(){}
	Airport(string strName):m_strName(strName){ }
	
	string Name( ) const { return m_strName; }
	void Name(string strName ) { m_strName = strName; }

	string ID( ) const { return m_strID; }
	void ID(string strID ) { m_strID = strID; }

	void Add(Route* pRte){ m_hmRts[pRte->ID()] = pRte; }
	Route* Get(string strRteID){
		HM_RTS_ITER i = m_hmRts.find(strRteID);
		if(i != m_hmRts.end())
			return i->second;
		return NULL;
	}

	void Draw(struct InstanceData* id){
		for(HM_RTS_ITER i = m_hmRts.begin(); i != m_hmRts.end(); i++){
			i->second->Draw(id);
		}
	}

	void Draw(struct InstanceData* id, string rte, string alt, int label, GLfloat* color, GLuint fontbase){
		if(!rte.empty() && m_hmRts.find(rte) != m_hmRts.end())
			m_hmRts[rte]->Draw(id, alt, label, color, fontbase);
	}

private:
	string m_strName;
	string m_strID;
	HM_RTS m_hmRts;
};

class State
{
public:
	typedef hash_map<string, Airport*> HM_ARPTS;
	typedef hash_map<string, Airport*>::iterator HM_ARPTS_ITER;

	State(){}
	State(string strName):m_strName(strName){ }
	
	string Name( ) const { return m_strName; }
	void Name(string strName ) { m_strName = strName; }

	string ID( ) const { return m_strID; }
	void ID(string strID) { m_strID = strID; }

	void Add(Airport* pArpt){ m_hmArpts[pArpt->ID()] = pArpt; }
	Airport* Get(string strArptID){
		HM_ARPTS_ITER i = m_hmArpts.find(strArptID);
		if(i != m_hmArpts.end())
			return i->second;
		return NULL;
	}

	void Draw(struct InstanceData* id){
		for(HM_ARPTS_ITER i = m_hmArpts.begin(); i != m_hmArpts.end(); i++){
			i->second->Draw(id);
		}
	}
	
	void Draw(struct InstanceData* id, string aprt, string rte, string alt, int label, GLfloat* color, GLuint fontbase){
		if(!aprt.empty())
			m_hmArpts[aprt]->Draw(id, rte, alt, label, color, fontbase);
	}


private:
	string m_strName;
	string m_strID;
	HM_ARPTS m_hmArpts;
};

#undef json_object_object_foreachC_next
#define json_object_object_foreachC_next(obj,iter) \
	iter.entry = iter.entry->next; \
	iter.entry ? (iter.key = (char*)iter.entry->k, iter.val = (struct json_object*)iter.entry->v, iter.entry) : 0;

typedef hash_map<string, State*> HM_STATES;
typedef hash_map<string, State*>::iterator HM_STATES_ITER;

static bool g_bDPSLoaded = false;
static bool g_bSTARSLoaded = false;
static string g_strSidJSON;
static string g_strStarJSON;
HM_STATES g_hmSids;
HM_STATES g_hmStars;

void ProcessAirport(struct json_object* jobj, State* pState, bool bDeps);
void ProcessTurn(struct json_object* jobj, Route* pRoute, bool bDeps);
void ProcessPath(struct json_object* jobj, Turn* pTurn, bool bDeps);
void ProcessRoute(struct json_object* jobj, Airport* pAirport, bool bDeps);
void ProcessState(struct json_object* jobj, HM_STATES& hmStates, bool bDeps = true);


void ProcessAirport(struct json_object* jobj, State* pState, bool bDeps)
{
	struct json_object_iter jiter;
	Airport* pAirport = new Airport;

	// Extract data from each state object, array of airports
	json_object_object_foreachC(jobj, jiter){
		if(!strcmp(jiter.key, "AIRPORT")){
			pAirport->Name(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "ID")){
			pAirport->ID(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "ROUTES")){
			int l = json_object_array_length(jiter.val);
			for( int i = 0; i < l; i++){
				ProcessRoute(json_object_array_get_idx(jiter.val, i), pAirport, bDeps);
			}
		}
	}
	pState->Add(pAirport);
}

void ProcessTurn(struct json_object* jobj, Route* pRoute, bool bDeps)
{
	struct json_object_iter jiter;
	Turn* pTurn = NULL;
	if(bDeps)
		pTurn = new Turn(pRoute);
	else
		pTurn = new Turn( );

	// Extract data from each state object, array of airports
	json_object_object_foreachC(jobj, jiter){
		if(!strcmp(jiter.key, "NAME")){
			pTurn->Name(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "ID")){
			pTurn->ID(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "PATH")){
			int l = json_object_array_length(jiter.val);
			for( int i = 0; i < l; i++){
				ProcessPath(json_object_array_get_idx(jiter.val, i), pTurn, bDeps);
			}
		}
	}
	pRoute->Add(pTurn);
}

void ProcessPath(struct json_object* jobj, Turn* pTurn, bool bDeps)
{
	struct json_object_iter jiter;
	Fix* pFix = new Fix();

	// Extract data from each state object, array of airports
	json_object_object_foreachC(jobj, jiter){
		if(!strcmp(jiter.key, "FIX")){
			pFix->FixName(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "LAT_DEC")){
			pFix->Latitude(json_object_get_double(jiter.val));
		}else if(!strcmp(jiter.key, "LNG_DEC")){
			pFix->Longitude(json_object_get_double(jiter.val));
		}else if(!strcmp(jiter.key, "NAME")){
			pFix->Name(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "TYPE")){
			pFix->Type(json_object_get_string(jiter.val));
		}
	}
	pTurn->Add(pFix);
}

void ProcessRoute(struct json_object* jobj, Airport* pAirport, bool bDeps)
{
	struct json_object_iter jiter;
	Route* pRoute = new Route();

	// Extract data from each state object, array of airports
	json_object_object_foreachC(jobj, jiter){
		if(!strcmp(jiter.key, "NAME")){
			pRoute->Name(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "ID")){
			pRoute->ID(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "PATH")){
			int l = json_object_array_length(jiter.val);
			for( int i = 0; i < l; i++){
				ProcessPath(json_object_array_get_idx(jiter.val, i), pRoute, bDeps);
			}
		}else if(!strcmp(jiter.key, "TURNS")){
			int l = json_object_array_length(jiter.val);
			for( int i = 0; i < l; i++){
				ProcessTurn(json_object_array_get_idx(jiter.val, i), pRoute, bDeps);
			}
		}
	}
	pAirport->Add(pRoute);
}

void ProcessState(struct json_object* jobj, HM_STATES& hmStates, bool bDeps)
{
	struct json_object_iter jiter;
	State* pState = new State();

	// Extract data from each state object, array of airports
	json_object_object_foreachC(jobj, jiter){		
		if(!strcmp(jiter.key, "AIRPORTS")){
			int l = json_object_array_length(jiter.val);
			for( int i = 0; i < l; i++){			
				ProcessAirport(json_object_array_get_idx(jiter.val, i), pState, bDeps);
			}
		}else if(!strcmp(jiter.key, "ID")){
			// Read State Abbreviation
			pState->ID(json_object_get_string(jiter.val));
		}else if(!strcmp(jiter.key, "STATE")){
			// Read State Name
			pState->Name(json_object_get_string(jiter.val));
		}
	}
	hmStates[pState->ID()] = pState;
}

void LoadDPs( struct InstanceData* id )
{
	char strFilePath[MAX_PATH_SZ] = {0};
	const char* strFile = "dp.json";
	//const char* strFile = "dpq.json";

	// Check if we have loaded the ARTCC's yet
	if(g_bDPSLoaded)
		return;

	sprintf_s(strFilePath, MAX_PATH_SZ, "%s\\%s", id->m_strMapDir, strFile);

	if ( GetLocalFile(id, id->m_strMapDir, strFile, id->m_strHttpMapPath, SM_HTTPS)) {
		struct json_object* jobj = json_object_from_file(strFilePath);
		if(jobj && -1 != ((int)jobj)){
			g_strSidJSON = _strdup(json_object_to_json_string(jobj));

			// Data is an array of state objects
			if( json_object_is_type(jobj,json_type_array) ){
				int l = json_object_array_length(jobj);
				// Iterate over all states
				for( int i = 0; i < l; i++){
					ProcessState(json_object_array_get_idx(jobj, i), g_hmSids);
				}
			}

			json_object_put(jobj);
			g_bDPSLoaded = true;
		}
	}
}

void LoadSTARs( struct InstanceData* id )
{
	char strFilePath[MAX_PATH_SZ] = {0};
	const char* strFile = "stars.json";

	// Check if we have loaded the Stars yet
	if(g_bSTARSLoaded)
		return;

	sprintf_s(strFilePath, MAX_PATH_SZ, "%s\\%s", id->m_strMapDir, strFile);

	if ( GetLocalFile(id, id->m_strMapDir, strFile, id->m_strHttpMapPath, SM_HTTPS)) {
		struct json_object* jobj = json_object_from_file(strFilePath);
		if(jobj && -1 != ((int)jobj)){
			g_strStarJSON = _strdup(json_object_to_json_string(jobj));

			// Data is an array of state objects
			if( json_object_is_type(jobj,json_type_array) ){
				int l = json_object_array_length(jobj);
				// Iterate over all states
				for( int i = 0; i < l; i++){
					ProcessState(json_object_array_get_idx(jobj, i), g_hmStars, false);
				}
			}

			json_object_put(jobj);
			g_bSTARSLoaded = true;
		}
	}
}


extern "C"
{

void DrawSids( struct InstanceData* id )
{
	if(!g_bDPSLoaded)
		return;
	if(id->m_sCurLayout.m_pstrSIDS){
		char *sidtok;
		char *ctx1;
		char* pSIDS = _strdup(id->m_sCurLayout.m_pstrSIDS);
		
		sidtok = strtok_s ( pSIDS, ",", &ctx1 );
		while(sidtok){
			string st, ap, rt, al;
			char* idtok;
			char* ctx2;
			idtok = strtok_s ( sidtok, "-", &ctx2 );
			while(idtok){
				if(st.empty()) 
					st = idtok;
				else if(ap.empty())
					ap = st + string("-") + string(idtok);
				else if(rt.empty())
					rt = ap + string("-") + string(idtok);
				else if(al.empty())
					al = rt + string("-") + string(idtok);
				idtok = strtok_s ( NULL, "-", &ctx2 );
			}
			
			g_hmSids[st]->Draw(id, ap, rt, al, id->m_sCurLayout.m_nSIDSLabel, (GLfloat*)id->m_sCurLayout.m_glfSIDSColor, id->m_arFonts[id->m_sCurLayout.m_nSIDSFontIndex].m_glListBase);

			sidtok = strtok_s ( NULL, ",", &ctx1 );
		}
		free(pSIDS);
	}
}

void DrawStars( struct InstanceData* id )
{
	if(!g_bSTARSLoaded)
		return;
	if(id->m_sCurLayout.m_pstrSTARS){
		char *startok;
		char *ctx1;
		char* pSTARS = _strdup(id->m_sCurLayout.m_pstrSTARS);
		
		startok = strtok_s ( pSTARS, ",", &ctx1 );
		while(startok){
			string st, ap, rt, al;
			char* idtok;
			char* ctx2;
			idtok = strtok_s ( startok, "-", &ctx2 );
			while(idtok){
				if(st.empty()) 
					st = idtok;
				else if(ap.empty())
					ap = st + string("-") + string(idtok);
				else if(rt.empty())
					rt = ap + string("-") + string(idtok);
				else if(al.empty())
					al = rt + string("-") + string(idtok);
				idtok = strtok_s ( NULL, "-", &ctx2 );
			}
			
			g_hmStars[st]->Draw(id, ap, rt, al, id->m_sCurLayout.m_nSTARSLabel, (GLfloat*)id->m_sCurLayout.m_glfSTARSColor, id->m_arFonts[id->m_sCurLayout.m_nSTARSFontIndex].m_glListBase);

			startok = strtok_s ( NULL, ",", &ctx1 );
		}
		free(pSTARS);
	}
}


const char* SIDtoJSON( struct InstanceData* id )
{
	return g_strSidJSON.c_str();
}

const char* STARtoJSON( struct InstanceData* id )
{
	return g_strStarJSON.c_str();
}

}
