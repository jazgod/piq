#include "gd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <errno.h>	/*declares extern errno*/
#include <math.h>
//#include <SDL/SDL.h>
//#include <SDL/SDL_syswm.h>
#include <GL/gl.h>
//#include <GL/glut.h>
#include <assert.h>
#include "vo.h"
#include "vo_extern.h"
#include "SSI.h"
#include "ExecUtil.h"
#include "vo_db.h"
#include "mgl.h"
#include "InstanceData.h"
#include "dbg_mem.h"
#include "strptime.h"
#include "Curl.h"
#include "bitmapfont.h"
#include "srfc.h"




/*   TASKS for mgl ---

*/

extern double rint(double dval);


extern MGLGRID *AlertsGrid;
extern VO *HistAlertsVO;


extern int ProcessKeydown(struct InstanceData* id, WPARAM wParam, int keystate);
extern void DrawRasterString(struct InstanceData* id, int x, int y, GLfloat *fgcolor, char *textstr);
//extern int GridListCallback(struct InstanceData* id, MGLGRID *Grid_List, int b, int m, int x, int y);
//extern void CrGridListVO(MGLGRID *GridList);
extern GLfloat trans[3];
extern HDC hDC_ETAGridWin;

extern void DrawGLGUIText(struct InstanceData* id, int xpixel, int ypixel, char *str, GLfloat *color);
extern void CreateEMailGrid(struct InstanceData* id, MGLGRID *parentgrid);
extern void CreateSubjectGrid(struct InstanceData* id, MGLGRID *parentgrid);
#define PANEL_TEXT_XBORDER 20
#define PANEL_TEXT_YBORDER 5
#define PANEL_ITEM_XBORDER 30
#define PANEL_ITEM_YBORDER 3

#define MENU_ITEM_HEIGHT 20;
#define MENU_ITEM_WIDTH 100;

#define MENU_ITEM_MARGIN 3

panellist *G_panellist;

//extern GLfloat white[3];
extern GLfloat black[3];
extern GLfloat red[3];
extern GLfloat blue[3];
extern GLfloat grey[3];


const GLfloat gridgrey0[3] = { 0.2f, 0.2f, 0.2f };
const GLfloat gridgrey1[3] = { 0.3f, 0.3f, 0.3f };

const GLfloat gridgreyelev[3] = { 0.25f, 0.25f, 0.25f };


const GLfloat color504e4f[3] = { (GLfloat)(0x50 / 255.0f), (GLfloat)(0x4e / 255.0f), (GLfloat)(0x4f / 255.0) };  // light grey for bar
const GLfloat color454545[3] = { (GLfloat)(0x45 / 255.0f), (GLfloat)(0x45 / 255.0f), (GLfloat)(0x45 / 255.0) };  // dark grey for bar
const GLfloat colorfc4740[3] = { (GLfloat)(0xfc / 255.0f), (GLfloat)(0x47 / 255.0f), (GLfloat)(0x40 / 255.0) };  // redish for tex
const GLfloat color868686[3] = { (GLfloat)(0x86 / 255.0f), (GLfloat)(0x86 / 255.0f), (GLfloat)(0x86 / 255.0) };  // light grey for text
const GLfloat colorf8cf53[3] = { (GLfloat)(0xf8 / 255.0f), (GLfloat)(0xcf / 255.0f), (GLfloat)(0x53 / 255.0) };  // yellowish for text
const GLfloat color353535[3] = { (GLfloat)(0x35 / 255.0f), (GLfloat)(0x35 / 255.0f), (GLfloat)(0x35 / 255.0) }; // separator color
const GLfloat color686868[3] = { (GLfloat)(0x68 / 255.0f), (GLfloat)(0x68 / 255.0f), (GLfloat)(0x68 / 255.0) }; // separator color
const GLfloat colorD6B859[3] = { (GLfloat)(0xD6 / 255.0f), (GLfloat)(0xB8 / 255.0f), (GLfloat)(0x59 / 255.0) }; // SubNavGold selected color

#ifdef USEOLDPNG
static int UseDisplayMode = 1;  // if set, then use older graphics calls that are not as precise
#else
static int UseDisplayMode = 0;
#endif


const GLfloat colorblue[3] = { 0.0f, 0.0f, 1.0f };
const GLfloat colorbrightblue[3] = { (GLfloat)(82 / 255.0f), (GLfloat)(158 / 255.0f), (GLfloat)(253 / 255.0) };  // 529EFD blue for text


// these widgets dynamically move if user resizes window
widgets *TopBarW, *PassurLogoW, *AirportTextW, *arpt0W, *arpt1W, *arpt2W, *WebTrackerTextW, *ILightW, *ReplayButW, *PITMButW, *ILightsTextW, *LeftEdgeW;
widgets *AirportText2W;
widgets *AirportText3W;

widgets *SeparatorW, *RightEdgeW, *AppButtonBufW, *OverlaysButtonBufW, *FiltersButtonBufW, *LayoutsButtonBufW, *SettingsButtonBufW, *AirportsButtonBufW;
widgets *XLightW, *XLightsTextW, *BLightW, *BLightsTextW;

widgets *testappW;  // temporary


// widgets for subnav Overlays
#define N_SUBNAVS 7
widgets *SubNavStartWidgets[N_SUBNAVS];
widgets *SubNavLastWidgets[N_SUBNAVS];

widgets *OverlaysBarW, *ApplicationsBarW, *FiltersBarW, *LayoutsBarW, *SettingsBarW, *AirportsBarW;
widgets *SubNavSeparatorTopW, *SubNavSeparatorBotW, *SubNavSeparatorDarkW, *PassurButW, *ASDIButW, *ASDEXButW, *ASDSBButW, *AirwaysButW, *WxButW;

widgets *SubNavSeparatorTopW_A, *SubNavSeparatorBotW_A, *SubNavSeparatorDarkW_A;
widgets *SubNavSeparatorTopW_O, *SubNavSeparatorBotW_O, *SubNavSeparatorDarkW_O;
widgets *SubNavSeparatorTopW_S, *SubNavSeparatorBotW_S, *SubNavSeparatorDarkW_S;
widgets *SubNavSeparatorTopW_Ai, *SubNavSeparatorBotW_Ai, *SubNavSeparatorDarkW_Ai;

widgets *PassurButW, *ASDIButW, *ASDEXButW, *ASDSBButW, *AirwaysButW, *WxButW;
widgets	*FindFlightTopW, *FindAirportTopW, *AirlineLookupW, *GateConflictsW, *RegionOfInterestW;
widgets *AlertsW, *ColorsW, *TagsW, *WorldMapW, *ARTCC_W, *SIDS_W, *STARS_W, *LocalTimeW, *LKP_W, *AllSurfaceW, *VehiclesW, *VehicleLabelW, *RecordDurationW;
widgets *PASSURAirportW;

widgets *FlightPlanW, *AircraftTrailsW, *TrailTimesW, *AircraftRingsW, *RangeRingsW, *AirportStatusW, *RunwaysW, *TerminalFixesW, *HighFixesW, *LowFixesW;


// right side bar widgets
widgets *SideBarW, *SideSeparatorW, *BlkSeparatorW, *SideBarArrowW, *USAMapW, *FindFlightW, *DisableFiltersW, *LastSideBarW, *HelpW;
widgets *ZoomSliderW, *LeftSliderEdgeW, *RightSliderEdgeW, *SliderBarW, *SliderBarBGW, *SliderArrowW;

//gui *oglgui;
MGLGRID *ActiveTextGrid;

//extern MGLGRID *DataTypeMenuGrid;  // menu for user to select what type of data object he wants, i.e., FDO, ROI,, etc
//extern MGLGRID *Grid_Report;
//extern MGLGRID *Grid_List;



// x spacing between network lights
#define NET_LIGHTS_X_SPACE 21


// integer colors that are rgb values stored in an integer format to match integer value stored in vo row_index[].fg. bg
int iOrange = 244 << 16 | 146 << 8 << 31;

int GAlertsID;  // give a unique id to alerts, not sure yet if this will stay


///////////////////////////////////////////////////////////   new regex


//#include <stdio.h>
//#include <ctype.h>
//#include <string.h>

///  #include "slre.h"

#define MAX_BRANCHES 100
#define MAX_BRACKETS 100
#define FAIL_IF(condition, error_code) if (condition) return (error_code)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof((ar)[0]))
#endif

#ifdef SLRE_DEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif



struct bracket_pair {
	const char *ptr;  /* Points to the first char after '(' in regex  */
	int len;          /* Length of the text between '(' and ')'       */
	int branches;     /* Index in the branches array for this pair    */
	int num_branches; /* Number of '|' in this bracket pair           */
};

struct branch {
	int bracket_index;    /* index for 'struct bracket_pair brackets' */
	/* array defined below                      */
	const char *schlong;  /* points to the '|' character in the regex */
};

struct regex_info {
	/*
	* Describes all bracket pairs in the regular expression.
	* First entry is always present, and grabs the whole regex.
	*/
	struct bracket_pair brackets[MAX_BRACKETS];
	int num_brackets;

	/*
	* Describes alternations ('|' operators) in the regular expression.
	* Each branch falls into a specific branch pair.
	*/
	struct branch branches[MAX_BRANCHES];
	int num_branches;

	/* Array of captures provided by the user */
	struct slre_cap *caps;
	int num_caps;

	/* E.g. SLRE_IGNORE_CASE. See enum below */
	int flags;
};

static int is_metacharacter(const unsigned char *s) {
	static const char *metacharacters = "^$().[]*+?|\\Ssdbfnrtv";
	return strchr(metacharacters, *s) != NULL;
}

static int op_len(const char *re) {
	return re[0] == '\\' && re[1] == 'x' ? 4 : re[0] == '\\' ? 2 : 1;
}

static int set_len(const char *re, int re_len) {
	int len = 0;

	while (len < re_len && re[len] != ']') {
		len += op_len(re + len);
	}

	return len <= re_len ? len + 1 : -1;
}

static int get_op_len(const char *re, int re_len) {
	return re[0] == '[' ? set_len(re + 1, re_len - 1) + 1 : op_len(re);
}

static int is_quantifier(const char *re) {
	return re[0] == '*' || re[0] == '+' || re[0] == '?';
}

static int toi(int x) {
	return isdigit(x) ? x - '0' : x - 'W';
}

static int hextoi(const unsigned char *s) {
	return (toi(tolower(s[0])) << 4) | toi(tolower(s[1]));
}

static int match_op(const unsigned char *re, const unsigned char *s,
struct regex_info *info) {
	int result = 0;
	switch (*re) {
	case '\\':
		/* Metacharacters */
		switch (re[1]) {
		case 'S': FAIL_IF(isspace(*s), SLRE_NO_MATCH); result++; break;
		case 's': FAIL_IF(!isspace(*s), SLRE_NO_MATCH); result++; break;
		case 'd': FAIL_IF(!isdigit(*s), SLRE_NO_MATCH); result++; break;
		case 'b': FAIL_IF(*s != '\b', SLRE_NO_MATCH); result++; break;
		case 'f': FAIL_IF(*s != '\f', SLRE_NO_MATCH); result++; break;
		case 'n': FAIL_IF(*s != '\n', SLRE_NO_MATCH); result++; break;
		case 'r': FAIL_IF(*s != '\r', SLRE_NO_MATCH); result++; break;
		case 't': FAIL_IF(*s != '\t', SLRE_NO_MATCH); result++; break;
		case 'v': FAIL_IF(*s != '\v', SLRE_NO_MATCH); result++; break;

		case 'x':
			/* Match byte, \xHH where HH is hexadecimal byte representaion */
			FAIL_IF(hextoi(re + 2) != *s, SLRE_NO_MATCH);
			result++;
			break;

		default:
			/* Valid metacharacter check is done in bar() */
			FAIL_IF(re[1] != s[0], SLRE_NO_MATCH);
			result++;
			break;
		}
		break;

	case '|': FAIL_IF(1, SLRE_INTERNAL_ERROR); break;
	case '$': FAIL_IF(1, SLRE_NO_MATCH); break;
	case '.': result++; break;

	default:
		if (info->flags & SLRE_IGNORE_CASE) {
			FAIL_IF(tolower(*re) != tolower(*s), SLRE_NO_MATCH);
		}
		else {
			FAIL_IF(*re != *s, SLRE_NO_MATCH);
		}
		result++;
		break;
	}

	return result;
}

static int match_set(const char *re, int re_len, const char *s,
struct regex_info *info) {
	int len = 0, result = -1, invert = re[0] == '^';

	if (invert) re++, re_len--;

	while (len <= re_len && re[len] != ']' && result <= 0) {
		/* Support character range */
		if (re[len] != '-' && re[len + 1] == '-' && re[len + 2] != ']' &&
			re[len + 2] != '\0') {
			result = info->flags &  SLRE_IGNORE_CASE ?
				tolower(*s) >= tolower(re[len]) && tolower(*s) <= tolower(re[len + 2]) :
				*s >= re[len] && *s <= re[len + 2];
			len += 3;
		}
		else {
			result = match_op((unsigned char *)re + len, (unsigned char *)s, info);
			len += op_len(re + len);
		}
	}
	return (!invert && result > 0) || (invert && result <= 0) ? 1 : -1;
}

static int doh(const char *s, int s_len, struct regex_info *info, int bi);

static int bar(const char *re, int re_len, const char *s, int s_len,
struct regex_info *info, int bi) {
	/* i is offset in re, j is offset in s, bi is brackets index */
	int i, j, n, step;

	for (i = j = 0; i < re_len && j <= s_len; i += step) {

		/* Handle quantifiers. Get the length of the chunk. */
		step = re[i] == '(' ? info->brackets[bi + 1].len + 2 :
			get_op_len(re + i, re_len - i);

		DBG(("%s [%.*s] [%.*s] re_len=%d step=%d i=%d j=%d\n", __func__,
			re_len - i, re + i, s_len - j, s + j, re_len, step, i, j));

		FAIL_IF(is_quantifier(&re[i]), SLRE_UNEXPECTED_QUANTIFIER);
		FAIL_IF(step <= 0, SLRE_INVALID_CHARACTER_SET);

		if (i + step < re_len && is_quantifier(re + i + step)) {
			DBG(("QUANTIFIER: [%.*s]%c [%.*s]\n", step, re + i,
				re[i + step], s_len - j, s + j));
			if (re[i + step] == '?') {
				int result = bar(re + i, step, s + j, s_len - j, info, bi);
				j += result > 0 ? result : 0;
				i++;
			}
			else if (re[i + step] == '+' || re[i + step] == '*') {
				int j2 = j, nj = j, n1, n2 = -1, ni, non_greedy = 0;

				/* Points to the regexp code after the quantifier */
				ni = i + step + 1;
				if (ni < re_len && re[ni] == '?') {
					non_greedy = 1;
					ni++;
				}

				do {
					if ((n1 = bar(re + i, step, s + j2, s_len - j2, info, bi)) > 0) {
						j2 += n1;
					}
					if (re[i + step] == '+' && n1 < 0) break;

					if (ni >= re_len) {
						/* After quantifier, there is nothing */
						nj = j2;
					}
					else if ((n2 = bar(re + ni, re_len - ni, s + j2,
						s_len - j2, info, bi)) >= 0) {
						/* Regex after quantifier matched */
						nj = j2 + n2;
					}
					if (nj > j && non_greedy) break;
				} while (n1 > 0);

				/*
				* Even if we found one or more pattern, this branch will be executed,
				* changing the next captures.
				*/
				if (n1 < 0 && n2 < 0 && re[i + step] == '*' &&
					(n2 = bar(re + ni, re_len - ni, s + j, s_len - j, info, bi)) > 0) {
					nj = j + n2;
				}

				DBG(("STAR/PLUS END: %d %d %d %d %d\n", j, nj, re_len - ni, n1, n2));
				FAIL_IF(re[i + step] == '+' && nj == j, SLRE_NO_MATCH);

				/* If while loop body above was not executed for the * quantifier,  */
				/* make sure the rest of the regex matches                          */
				FAIL_IF(nj == j && ni < re_len && n2 < 0, SLRE_NO_MATCH);

				/* Returning here cause we've matched the rest of RE already */
				return nj;
			}
			continue;
		}

		if (re[i] == '[') {
			n = match_set(re + i + 1, re_len - (i + 2), s + j, info);
			DBG(("SET %.*s [%.*s] -> %d\n", step, re + i, s_len - j, s + j, n));
			FAIL_IF(n <= 0, SLRE_NO_MATCH);
			j += n;
		}
		else if (re[i] == '(') {
			n = SLRE_NO_MATCH;
			bi++;
			FAIL_IF(bi >= info->num_brackets, SLRE_INTERNAL_ERROR);
			DBG(("CAPTURING [%.*s] [%.*s] [%s]\n",
				step, re + i, s_len - j, s + j, re + i + step));

			if (re_len - (i + step) <= 0) {
				/* Nothing follows brackets */
				n = doh(s + j, s_len - j, info, bi);
			}
			else {
				int j2;
				for (j2 = 0; j2 <= s_len - j; j2++) {
					if ((n = doh(s + j, s_len - (j + j2), info, bi)) >= 0 &&
						bar(re + i + step, re_len - (i + step),
						s + j + n, s_len - (j + n), info, bi) >= 0) break;
				}
			}

			DBG(("CAPTURED [%.*s] [%.*s]:%d\n", step, re + i, s_len - j, s + j, n));
			FAIL_IF(n < 0, n);
			if (info->caps != NULL && n > 0) {
				info->caps[bi - 1].ptr = s + j;
				info->caps[bi - 1].len = n;
			}
			j += n;
		}
		else if (re[i] == '^') {
			FAIL_IF(j != 0, SLRE_NO_MATCH);
		}
		else if (re[i] == '$') {
			FAIL_IF(j != s_len, SLRE_NO_MATCH);
		}
		else {
			FAIL_IF(j >= s_len, SLRE_NO_MATCH);
			n = match_op((unsigned char *)(re + i), (unsigned char *)(s + j), info);
			FAIL_IF(n <= 0, n);
			j += n;
		}
	}

	return j;
}

/* Process branch points */
static int doh(const char *s, int s_len, struct regex_info *info, int bi) {
	const struct bracket_pair *b = &info->brackets[bi];
	int i = 0, len, result;
	const char *p;

	do {
		p = i == 0 ? b->ptr : info->branches[b->branches + i - 1].schlong + 1;
		len = b->num_branches == 0 ? b->len :
			i == b->num_branches ? (int)(b->ptr + b->len - p) :
			(int)(info->branches[b->branches + i].schlong - p);
		DBG(("%s %d %d [%.*s] [%.*s]\n", __func__, bi, i, len, p, s_len, s));
		result = bar(p, len, s, s_len, info, bi);
		DBG(("%s <- %d\n", __func__, result));
	} while (result <= 0 && i++ < b->num_branches);  /* At least 1 iteration */

	return result;
}

static int baz(const char *s, int s_len, struct regex_info *info) {
	int i, result = -1, is_anchored = info->brackets[0].ptr[0] == '^';

	for (i = 0; i <= s_len; i++) {
		result = doh(s + i, s_len - i, info, 0);
		if (result >= 0) {
			result += i;
			break;
		}
		if (is_anchored) break;
	}

	return result;
}

static void setup_branch_points(struct regex_info *info) {
	int i, j;
	struct branch tmp;

	/* First, sort branches. Must be stable, no qsort. Use bubble algo. */
	for (i = 0; i < info->num_branches; i++) {
		for (j = i + 1; j < info->num_branches; j++) {
			if (info->branches[i].bracket_index > info->branches[j].bracket_index) {
				tmp = info->branches[i];
				info->branches[i] = info->branches[j];
				info->branches[j] = tmp;
			}
		}
	}

	/*
	* For each bracket, set their branch points. This way, for every bracket
	* (i.e. every chunk of regex) we know all branch points before matching.
	*/
	for (i = j = 0; i < info->num_brackets; i++) {
		info->brackets[i].num_branches = 0;
		info->brackets[i].branches = j;
		while (j < info->num_branches && info->branches[j].bracket_index == i) {
			info->brackets[i].num_branches++;
			j++;
		}
	}
}

static int foo(const char *re, int re_len, const char *s, int s_len,
struct regex_info *info) {
	int i, step, depth = 0;

	/* First bracket captures everything */
	info->brackets[0].ptr = re;
	info->brackets[0].len = re_len;
	info->num_brackets = 1;

	/* Make a single pass over regex string, memorize brackets and branches */
	for (i = 0; i < re_len; i += step) {
		step = get_op_len(re + i, re_len - i);

		if (re[i] == '|') {
			FAIL_IF(info->num_branches >= (int)ARRAY_SIZE(info->branches),
				SLRE_TOO_MANY_BRANCHES);
			info->branches[info->num_branches].bracket_index =
				info->brackets[info->num_brackets - 1].len == -1 ?
				info->num_brackets - 1 : depth;
			info->branches[info->num_branches].schlong = &re[i];
			info->num_branches++;
		}
		else if (re[i] == '\\') {
			FAIL_IF(i >= re_len - 1, SLRE_INVALID_METACHARACTER);
			if (re[i + 1] == 'x') {
				/* Hex digit specification must follow */
				FAIL_IF(re[i + 1] == 'x' && i >= re_len - 3,
					SLRE_INVALID_METACHARACTER);
				FAIL_IF(re[i + 1] == 'x' && !(isxdigit(re[i + 2]) &&
					isxdigit(re[i + 3])), SLRE_INVALID_METACHARACTER);
			}
			else {
				FAIL_IF(!is_metacharacter((unsigned char *)re + i + 1),
					SLRE_INVALID_METACHARACTER);
			}
		}
		else if (re[i] == '(') {
			FAIL_IF(info->num_brackets >= (int)ARRAY_SIZE(info->brackets),
				SLRE_TOO_MANY_BRACKETS);
			depth++;  /* Order is important here. Depth increments first. */
			info->brackets[info->num_brackets].ptr = re + i + 1;
			info->brackets[info->num_brackets].len = -1;
			info->num_brackets++;
			FAIL_IF(info->num_caps > 0 && info->num_brackets - 1 > info->num_caps,
				SLRE_CAPS_ARRAY_TOO_SMALL);
		}
		else if (re[i] == ')') {
			int ind = info->brackets[info->num_brackets - 1].len == -1 ?
				info->num_brackets - 1 : depth;
			info->brackets[ind].len = (int)(&re[i] - info->brackets[ind].ptr);
			DBG(("SETTING BRACKET %d [%.*s]\n",
				ind, info->brackets[ind].len, info->brackets[ind].ptr));
			depth--;
			FAIL_IF(depth < 0, SLRE_UNBALANCED_BRACKETS);
			FAIL_IF(i > 0 && re[i - 1] == '(', SLRE_NO_MATCH);
		}
	}

	FAIL_IF(depth != 0, SLRE_UNBALANCED_BRACKETS);
	setup_branch_points(info);

	return baz(s, s_len, info);
}


/*
examples using slre_match


all DAL or AAL flights:
AAL|DAL  

AAL flights between 2000-2999 only
AAL2[0-9][0-9][0-9]

AAL flights between 3000-3499 only
AAL3[0-4][0-9][0-9]

All all flights and ENY 3000-3499 only
AAL|ENY3[0-4][0-9][0-9]


AAL flights between 2000-2999 only
AAL2[0-9][0-9][0-9]

AAL flights between 3000-3499 only
AAL3[0-4][0-9][0-9]

AAL flights between 0-99 and ENY 3000-3499 only   $ means end of buffer
AAL[1-9]$|AAL[0-9][0-9]$|ENY3[0-4][0-9][0-9]


supported syntax:

(?i)    Must be at the beginning of the regex. Makes match case-insensitive
^       Match beginning of a buffer
$       Match end of a buffer
()      Grouping and substring capturing
\s      Match whitespace
\S      Match non-whitespace
\d      Match decimal digit
\n      Match new line character
\r      Match line feed character
\f      Match form feed character
\v      Match vertical tab character
\t      Match horizontal tab character
\b      Match backspace character
+       Match one or more times (greedy)
+?      Match one or more times (non-greedy)
*       Match zero or more times (greedy)
*?      Match zero or more times (non-greedy)
?       Match zero or once (non-greedy)
x|y     Match x or y (alternation operator)
\meta   Match one of the meta character: ^$().[]*+?|\
\xHH    Match byte with hex value 0xHH, e.g. \x4a
[...]   Match any character from set. Ranges like [a-z] are supported
[^...]  Match any character but ones from set


*/
int slre_match(const char *regexp, const char *s, int s_len,
struct slre_cap *caps, int num_caps, int flags) {
	struct regex_info info;

	/* Initialize info structure */
	info.flags = flags;
	info.num_brackets = info.num_branches = 0;
	info.num_caps = num_caps;
	info.caps = caps;

	DBG(("========================> [%s] [%.*s]\n", regexp, s_len, s));
	return foo(regexp, (int)strlen(regexp), s, s_len, &info);
}







////////////////////////////////////////////////////////////////

// regex stuff 
/*
* These routines are BSD regex(3)/ed(1) compatible regular-expression
* routines written by Ozan S. Yigit, Computer Science, York University.
* Parts of the code that are not needed by Prospero have been removed,
* but most of the accompanying information has been left intact.
* This file is to be included on those operating systems that do not
* support re_comp and re_exec.
*/

/*
* regex - Regular expression pattern matching
*         and replacement
*
* by:  Ozan S. Yigit (oz@nexus.yorku.ca)
*	Dept. of Computing Services
*      York University
*
* These routines are the PUBLIC DOMAIN equivalents
* of regex routines as found in 4.nBSD UN*X, with minor
* extensions.
*
* Modification history:
*
* $Log: regex.c,v $
* Revision 1.4  1996/04/25  20:06:29  blob
* Added const's so that it will compile on OSs that have const in the prototype in
* unistd.h
*
* Revision 1.3  1996/04/11  06:52:27  blob
* *** empty log message ***
*
* Revision 1.2  1996/04/11  06:51:34  blob
* Cleaned up warnings...
*
* Revision 1.1.1.2  1996/03/14  22:06:31  blob
* Try 1000000...
*
* Revision 1.1.1.1  1996/03/13  20:34:40  blob
* Initial Socks5 Beta import.
*
* Revision 1.1  1991/11/20  02:32:13  brendan
* entered into RCS
*
* Revision 1.1  1991/11/20  02:32:13  brendan
* entered into RCS
*
* Revision 1.3  89/04/01  14:18:09  oz
* Change all references to a dfa: this is actually an nfa.
*
* Revision 1.2  88/08/28  15:36:04  oz
* Use a complement bitmap to represent NCL.
* This removes the need to have seperate
* code in the pmatch case block - it is
* just CCL code now.
*
* Use the actual CCL code in the CLO
* section of pmatch. No need for a recursive
* pmatch call.
*
* Use a bitmap table to set char bits in an
* 8-bit chunk.
*
* Routines:
*      re_comp:        compile a regular expression into
*                      a NFA.
*
*			char *re_comp(s)
*			char *s;
*
*      re_exec:        execute the NFA to match a pattern.
*
*			int re_exec(s)
*			char *s;
*
* Regular Expressions:
*
*      [1]     char    matches itself, unless it is a special
*                      character (metachar): . \ [ ] * + ^ $
*
*      [2]     .       matches any character.
*
*      [3]     \       matches the character following it, except
*			when followed by a left or right round bracket,
*			a digit 1 to 9 or a left or right angle bracket.
*			(see [7], [8] and [9])
*			It is used as an escape character for all
*			other meta-characters, and itself. When used
*			in a set ([4]), it is treated as an ordinary
*			character.
*
*      [4]     [set]   matches one of the characters in the set.
*                      If the first character in the set is "^",
*                      it matches a character NOT in the set, i.e.
*			complements the set. A shorthand S-E is
*			used to specify a set of characters S upto
*			E, inclusive. The special characters "]" and
*			"-" have no special meaning if they appear
*			as the first chars in the set.
*                      examples:        match:
*
*                              [a-z]    any lowercase alpha
*
*                              [^]-]    any char except ] and -
*
*                              [^A-Z]   any char except uppercase
*                                       alpha
*
*                              [a-zA-Z] any alpha
*
*      [5]     *       any regular expression form [1] to [4], followed by
*                      closure char (*) matches zero or more matches of
*                      that form.
*
*      [6]     +       same as [5], except it matches one or more.
*
*      [7]             a regular expression in the form [1] to [10], enclosed
*                      as \(form\) matches what form matches. The enclosure
*                      creates a set of tags, used for [8] and for
*                      pattern substution. The tagged forms are numbered
*			starting from 1.
*
*      [8]             a \ followed by a digit 1 to 9 matches whatever a
*                      previously tagged regular expression ([7]) matched.
*
*	[9]	\<	a regular expression starting with a \< construct
*		\>	and/or ending with a \> construct, restricts the
*			pattern matching to the beginning of a word, and/or
*			the end of a word. A word is defined to be a character
*			string beginning and/or ending with the characters
*			A-Z a-z 0-9 and _. It must also be preceded and/or
*			followed by any character outside those mentioned.
*
*      [10]            a composite regular expression xy where x and y
*                      are in the form [1] to [10] matches the longest
*                      match of x followed by a match for y.
*
*      [11]	^	a regular expression starting with a ^ character
*		$	and/or ending with a $ character, restricts the
*                      pattern matching to the beginning of the line,
*                      or the end of line. [anchors] Elsewhere in the
*			pattern, ^ and $ are treated as ordinary characters.
*
*
* Acknowledgements:
*
*	HCR's Hugh Redelmeier has been most helpful in various
*	stages of development. He convinced me to include BOW
*	and EOW constructs, originally invented by Rob Pike at
*	the University of Toronto.
*
* References:
*              Software tools			Kernighan & Plauger
*              Software tools in Pascal        Kernighan & Plauger
*              Grep [rsx-11 C dist]            David Conroy
*		ed - text editor		Un*x Programmer's Manual
*		Advanced editing on Un*x	B. W. Kernighan
*		regexp routines			Henry Spencer
*
* Notes:
*
*	This implementation uses a bit-set representation for character
*	classes for speed and compactness. Each character is represented
*	by one bit in a 128-bit block. Thus, CCL always takes a
*	constant 16 bytes in the internal nfa, and re_exec does a single
*	bit comparison to locate the character in the set.
*
* Examples:
*
*	pattern:	foo*.*
*	compile:	CHR f CHR o CLO CHR o END CLO ANY END END
*	matches:	fo foo fooo foobar fobar foxx ...
*
*	pattern:	fo[ob]a[rz]
*	compile:	CHR f CHR o CCL bitset CHR a CCL bitset END
*	matches:	fobar fooar fobaz fooaz
*
*	pattern:	foo\\+
*	compile:	CHR f CHR o CHR o CHR \ CLO CHR \ END END
*	matches:	foo\ foo\\ foo\\\  ...
*
*	pattern:	\(foo\)[1-3]\1	(same as foo[1-3]foo)
*	compile:	BOT 1 CHR f CHR o CHR o EOT 1 CCL bitset REF 1 END
*	matches:	foo1foo foo2foo foo3foo
*
*	pattern:	\(fo.*\)-\1
*	compile:	BOT 1 CHR f CHR o CLO ANY END EOT 1 CHR - REF 1 END
*	matches:	foo-foo fo-fo fob-fob foobar-foobar ...
*
*/

#define MAXNFA  1024
#define MAXTAG  10

#define OKP     1
#define NOP     0

#define CHR     1
#define ANY     2
#define CCL     3
#define BOL     4
#define EOL     5
#define BOT     6
#define EOT     7
#define BOW	8
#define EOW	9
#define REF     10
#define CLO     11

#define END     0

/*
* The following defines are not meant
* to be changeable. They are for readability
* only.
*
*/
#define MAXCHR	128
#define CHRBIT	8
#define BITBLK	MAXCHR/CHRBIT
#define BLKIND	0170
#define BITIND	07

#define ASCIIB	0177

typedef /*unsigned*/ char CHAR;

static int  tagstk[MAXTAG];             /* subpat tag stack..*/
static CHAR nfa[MAXNFA];		/* automaton..       */
static int  sta = NOP;               	/* status of lastpat */

static CHAR bittab[BITBLK];		/* bit table for CCL */
/* pre-set bits...   */
static CHAR bitarr[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

static int internal_error;

static void
chset(c)
register CHAR c;
{
	bittab[((c)& BLKIND) >> 3] |= bitarr[(c)& BITIND];
}

#define badpat(x)	return (*nfa = END, x)
#define store(x)	*mp++ = x

char *
re_comp(pat)
const char *pat;
{
	register const char *p;               /* pattern pointer   */
	register CHAR *mp = nfa;        /* nfa pointer       */
	register CHAR *lp;              /* saved pointer..   */
	register CHAR *sp = nfa;        /* another one..     */

	register int tagi = 0;          /* tag stack index   */
	register int tagc = 1;          /* actual tag count  */

	register int n;
	register CHAR mask;		/* xor mask -CCL/NCL */
	int c1, c2;

	if (!pat || !*pat)
		if (sta)
			return 0;
		else
			badpat("No previous regular expression");
	sta = NOP;

	for (p = pat; *p; p++) {
		lp = mp;
		switch (*p) {

		case '.':               /* match any char..  */
			store(ANY);
			break;

		case '^':               /* match beginning.. */
			if (p == pat)
				store(BOL);
			else {
				store(CHR);
				store(*p);
			}
			break;

		case '$':               /* match endofline.. */
			if (!*(p + 1))
				store(EOL);
			else {
				store(CHR);
				store(*p);
			}
			break;

		case '[':               /* match char class..*/
			store(CCL);

			if (*++p == '^') {
				mask = 0377;
				p++;
			}
			else
				mask = 0;

			if (*p == '-')		/* real dash */
				chset(*p++);
			if (*p == ']')		/* real brac */
				chset(*p++);
			while (*p && *p != ']') {
				if (*p == '-' && *(p + 1) && *(p + 1) != ']') {
					p++;
					c1 = *(p - 2) + 1;
					c2 = *p++;
					while (c1 <= c2)
						chset(c1++);
				}
#ifdef EXTEND
				else if (*p == '\\' && *(p + 1)) {
					p++;
					chset(*p++);
				}
#endif
				else
					chset(*p++);
			}
			if (!*p)
				badpat("Missing ]");

			for (n = 0; n < BITBLK; bittab[n++] = (char)0)
				store(mask ^ bittab[n]);

			break;

		case '*':               /* match 0 or more.. */
		case '+':               /* match 1 or more.. */
			if (p == pat)
				badpat("Empty closure");
			lp = sp;		/* previous opcode */
			if (*lp == CLO)		/* equivalence..   */
				break;
			switch (*lp) {

			case BOL:
			case BOT:
			case EOT:
			case BOW:
			case EOW:
			case REF:
				badpat("Illegal closure");
			default:
				break;
			}

			if (*p == '+')
				for (sp = mp; lp < sp; lp++)
					store(*lp);

			store(END);
			store(END);
			sp = mp;
			while (--mp > lp)
				*mp = mp[-1];
			store(CLO);
			mp = sp;
			break;

		case '\\':              /* tags, backrefs .. */
			switch (*++p) {

			case '(':
				if (tagc < MAXTAG) {
					tagstk[++tagi] = tagc;
					store(BOT);
					store(tagc++);
				}
				else
					badpat("Too many \\(\\) pairs");
				break;
			case ')':
				if (*sp == BOT)
					badpat("Null pattern inside \\(\\)");
				if (tagi > 0) {
					store(EOT);
					store(tagstk[tagi--]);
				}
				else
					badpat("Unmatched \\)");
				break;
			case '<':
				store(BOW);
				break;
			case '>':
				if (*sp == BOW)
					badpat("Null pattern inside \\<\\>");
				store(EOW);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				n = *p - '0';
				if (tagi > 0 && tagstk[tagi] == n)
					badpat("Cyclical reference");
				if (tagc > n) {
					store(REF);
					store(n);
				}
				else
					badpat("Undetermined reference");
				break;
#ifdef EXTEND
			case 'b':
				store(CHR);
				store('\b');
				break;
			case 'n':
				store(CHR);
				store('\n');
				break;
			case 'f':
				store(CHR);
				store('\f');
				break;
			case 'r':
				store(CHR);
				store('\r');
				break;
			case 't':
				store(CHR);
				store('\t');
				break;
#endif
			default:
				store(CHR);
				store(*p);
			}
			break;

		default:               /* an ordinary char  */
			store(CHR);
			store(*p);
			break;
		}
		sp = lp;
	}
	if (tagi > 0)
		badpat("Unmatched \\(");
	store(END);
	sta = OKP;
	return 0;
}


static const char *bol;
static const char *bopat[MAXTAG];
static const char *eopat[MAXTAG];
// orig does not compile:  static const char *pmatch P((const char *, CHAR *));
static const char *pmatch(const char *, CHAR *);

/*
* re_exec:
* 	execute nfa to find a match.
*
*	special cases: (nfa[0])
*		BOL
*			Match only once, starting from the
*			beginning.
*		CHR
*			First locate the character without
*			calling pmatch, and if found, call
*			pmatch for the remaining string.
*		END
*			re_comp failed, poor luser did not
*			check for it. Fail fast.
*
*	If a match is found, bopat[0] and eopat[0] are set
*	to the beginning and the end of the matched fragment,
*	respectively.
*
*/

int
re_exec(lp)
register const char *lp;
{
	register char c;
	register const char *ep = 0;
	register CHAR *ap = nfa;

	bol = lp;

	bopat[0] = 0;
	bopat[1] = 0;
	bopat[2] = 0;
	bopat[3] = 0;
	bopat[4] = 0;
	bopat[5] = 0;
	bopat[6] = 0;
	bopat[7] = 0;
	bopat[8] = 0;
	bopat[9] = 0;

	switch (*ap) {

	case BOL:			/* anchored: match from BOL only */
		ep = pmatch(lp, ap);
		break;
	case CHR:			/* ordinary char: locate it fast */
		c = *(ap + 1);
		while (*lp && *lp != c)
			lp++;
		if (!*lp)		/* if EOS, fail, else fall thru. */
			return 0;
	default:			/* regular matching all the way. */
		while (*lp) {
			if ((ep = pmatch(lp, ap)))
				break;
			lp++;
		}
		break;
	case END:			/* munged automaton. fail always */
		return 0;
	}
	if (!ep)
		return 0;

	if (internal_error)
		return -1;

	bopat[0] = lp;
	eopat[0] = ep;
	return 1;
}

/*
* pmatch:
*	internal routine for the hard part
*
* 	This code is mostly snarfed from an early
* 	grep written by David Conroy. The backref and
* 	tag stuff, and various other mods are by oZ.
*
*	special cases: (nfa[n], nfa[n+1])
*		CLO ANY
*			We KNOW ".*" will match ANYTHING
*			upto the end of line. Thus, go to
*			the end of line straight, without
*			calling pmatch recursively. As in
*			the other closure cases, the remaining
*			pattern must be matched by moving
*			backwards on the string recursively,
*			to find a match for xy (x is ".*" and
*			y is the remaining pattern) where
*			the match satisfies the LONGEST match
*			for x followed by a match for y.
*		CLO CHR
*			We can again scan the string forward
*			for the single char without recursion,
*			and at the point of failure, we execute
*			the remaining nfa recursively, as
*			described above.
*
*	At the end of a successful match, bopat[n] and eopat[n]
*	are set to the beginning and end of subpatterns matched
*	by tagged expressions (n = 1 to 9).
*
*/

/*
* character classification table for word boundary
* operators BOW and EOW. the reason for not using
* ctype macros is that we can let the user add into
* our own table. see re_modw. This table is not in
* the bitset form, since we may wish to extend it
* in the future for other character classifications.
*
*	TRUE for 0-9 A-Z a-z _
*/
static char chrtyp[MAXCHR] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 1, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0, 0, 0, 0, 0
};

#define inascii(x)	(0177&(x))
#define iswordc(x) 	chrtyp[inascii(x)]
#define isinset(x,y) 	((x)[((y)&BLKIND)>>3] & bitarr[(y)&BITIND])

/*
* skip values for CLO XXX to skip past the closure
*
*/

#define ANYSKIP	2 	/* [CLO] ANY END ...	     */
#define CHRSKIP	3	/* [CLO] CHR chr END ...     */
#define CCLSKIP 18	/* [CLO] CCL 16bytes END ... */

static const char *
pmatch(lp, ap)
register const char *lp;
register CHAR *ap;
{
	register int op, c, n;
	register const char *e;		/* extra pointer for CLO */
	register const char *bp;	/* beginning of subpat.. */
	register const char *ep;	/* ending of subpat..	 */
	const char *are;		/* to save the line ptr. */

	while ((op = *ap++) != END)
		switch (op) {

		case CHR:
			if (*lp++ != *ap++)
				return 0;
			break;
		case ANY:
			if (!*lp++)
				return 0;
			break;
		case CCL:
			c = *lp++;
			if (!isinset(ap, c))
				return 0;
			ap += BITBLK;
			break;
		case BOL:
			if (lp != bol)
				return 0;
			break;
		case EOL:
			if (*lp)
				return 0;
			break;
		case BOT:
			bopat[(int)(*ap++)] = lp;
			break;
		case EOT:
			eopat[(int)(*ap++)] = lp;
			break;
		case BOW:
			if ((lp != bol && iswordc(lp[-1])) || !iswordc(*lp))
				return 0;
			break;
		case EOW:
			if (lp == bol || !iswordc(lp[-1]) || iswordc(*lp))
				return 0;
			break;
		case REF:
			n = *ap++;
			bp = bopat[n];
			ep = eopat[n];
			while (bp < ep)
				if (*bp++ != *lp++)
					return 0;
			break;
		case CLO:
			are = lp;
			switch (*ap) {

			case ANY:
				while (*lp)
					lp++;
				n = ANYSKIP;
				break;
			case CHR:
				c = *(ap + 1);
				while (*lp && c == *lp)
					lp++;
				n = CHRSKIP;
				break;
			case CCL:
				while ((c = *lp) && isinset(ap + 1, c))
					lp++;
				n = CCLSKIP;
				break;
			default:
				internal_error++;
				return 0;
			}

			ap += n;

			while (lp >= are) {
				if ((e = pmatch(lp, ap)))
					return e;
				--lp;
			}
			return 0;
		default:
			internal_error++;
			return 0;
	}

	return lp;
}

///// regex stuff 

int InRectangle(int checkx, int checky, MRECT *rect)
{
	assert(rect);

	if (checkx >= rect->x1 && checkx <= rect->x2 && checky >= rect->y1 && checky <= rect->y2){
		return(TRUE);
	}
	return(FALSE);
}

// this routine needed since MS does not support rint()

double my_rint(double a)
{
	const double two_to_52 = 4.5035996273704960e+15;
	double fa = fabs(a);
	double r = two_to_52 + fa;
	if (fa >= two_to_52) {
		r = a;
	}
	else {
		r = r - two_to_52;
		r = _copysign(r, a);
	}
	return r;
}

VO *MGLFindLabel(VO *vo, char *colname)
{
	VO *vcol;
	int i;


	if (!vo || !colname || !vo->vcols){
		return(NULL);
	}
	for (i = 0; i < vo->vcols->count; i++){
		vcol = V_ROW_PTR(vo->vcols, i);
		if (vcol && (vcol->magic_cookie == VO_MAGIC_NUM) &&
			strcasecmp(colname, vcol->label) == 0)
		{
			return(vcol);
		}
	}

	return(NULL);  /* not found */
}



VO *MGLFindCol(VO *vo, char *colname)
{
	VO *vcol;
	int i;


	if (!vo || !colname || !vo->vcols){
		return(NULL);
	}
	for (i = 0; i < vo->vcols->count; i++){
		vcol = V_ROW_PTR(vo->vcols, i);
		if (vcol && (vcol->magic_cookie == VO_MAGIC_NUM) &&
			strcasecmp(colname, vcol->name) == 0)
		{
			return(vcol);
		}
	}

	return(NULL);  /* not found */
}




MGLGridTexQuad(GLuint texture, int x1, int y1, int x2, int y2, GLfloat *c)
{
	GLboolean bEnable;

	assert(c);

	glGetBooleanv(GL_TEXTURE_2D, &bEnable);
	if (!bEnable)
		glEnable(GL_TEXTURE_2D);

	glColor3fv(c);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(x1, y2);
	glTexCoord2i(1, 0);
	glVertex2i(x2, y2);
	glTexCoord2i(1, 1);
	glVertex2i(x2, y1);
	glTexCoord2i(0, 1);
	glVertex2i(x1, y1);
	glEnd();

	if (!bEnable)
		glDisable(GL_TEXTURE_2D);
}

MGLGridInitRows(MGLGRID *grid)
{
	int i;
	int curypos = 0;
	int rowheight;


	assert(grid);


	for (i = 0; i < MGL_GRID_MAX_ROWS; i++){
		if (i == 0){
			curypos = grid->gridrect.y1 + grid->winheight -
				grid->gridrows[i].height;
		}
		grid->gridrows[i].texture = 0;
		grid->gridrows[i].fg = NULL;  // do not define as default so precedence is cell,row,grid in that order
		grid->gridrows[i].bg = NULL;  // do not define as default so precedence is cell,row,grid in that order
		grid->gridrows[i].width = MGL_DEFAULT;
		grid->gridrows[i].height = MGL_DEFAULT;
		grid->gridrows[i].rowrect.x1 = grid->gridrect.x1;

		// subtract height for this row from the current position
		if (grid->gridrows[i].height == MGL_DEFAULT){
			rowheight = MGL_GRID_DEF_ROW_HEIGHT;
		}
		else {
			rowheight = grid->gridrows[i].height;
		}
		grid->gridrows[i].rowrect.y1 = curypos - rowheight;
		curypos = grid->gridrows[i].rowrect.y1; //move cursor down to this row
	}

}


int MGLGridYMin(MGLGRID *grid)
{
	assert(grid);

	return(grid->gridrows[grid->nrows - 1].rowrect.y1);

}

MGLGRID *CreateWarning(struct InstanceData* id)
{

	//pops up "SAVED" when save is pressed

	widgets *w, *lastw, *panel_widget;



	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* WarningGrid = (MGLGRID*)id->WarningGrid;
	
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "WarningDisplay");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Alerts that are active
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "WarningDisplay");
	w->gregion.x1 = DataAppGrid->gridrect.x2;
	w->gregion.y1 = DataAppGrid->gridrect.y2 - 20;
	w->gregion.x2 = 300;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "WarningDisplay", 1, 2, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	WarningGrid = w->grid;



	w->grid->gridrows[0].fg = (GLfloat*)white;
	w->grid->gridrows[0].bg = (GLfloat*)color454545;



	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	MGLGridText(id, WarningGrid, 0, 0, "Warning: Cannot show more than 15 columns");

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Warning: Cannot show more than 15 columns at a time", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Warning", _TRUNCATE);


	id->flag_check = 2;//to turn the grid on/off
	id->m_poglgui = oglgui;
	id->DataAppGrid = DataAppGrid;
	id->WarningGrid = WarningGrid;
	return(WarningGrid);
}



int MGLGridYMax(MGLGRID *grid)
{
	int rowheight;

	assert(grid);

	if (grid->gridrows[0].height == MGL_DEFAULT){
		rowheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		rowheight = grid->gridrows[0].height;
	}

	return(grid->gridrows[0].rowrect.y1 + rowheight);

}



int MGLGridXMax(MGLGRID *grid)
{
	int i, maxx, vocol, x, width;
	int bestcol = -1;

	assert(grid);

	// find x location of last visible column
	for (i = 0, maxx = -99999; i < grid->ncols; i++){

		// ignore invisible rows, and check all rows since display order may be different than col order
		vocol = grid->DisplayCol[i];
		if (vocol < 0){
			// invisible row
			continue;
		}
		x = grid->gridcells[0][vocol].cellrect.x1;
		if (x > maxx){
			maxx = x;
			bestcol = vocol;
		}

	}

	if (bestcol >= 0){
		if (grid->gridcells[0][bestcol].width != MGL_DEFAULT){
			width = grid->gridcells[0][bestcol].width;
		}
		else if (grid->gridcols[bestcol].width != MGL_DEFAULT){
			width = grid->gridcols[bestcol].width;
		}
		else {
			width = MGL_GRID_DEF_COL_WIDTH;
		}

		x = grid->gridcells[0][bestcol].cellrect.x1 + width;
	}

	return(x);
}




int MGLGridXMin(MGLGRID *grid)
{
	assert(grid);

	return(grid->gridrect.x1);
}


void MGLGridInitScroll(MGLGRID *grid)
{
	int rowheight;

	assert(grid);

	// first position is at top, below top arrow

	grid->elev_rect.y1 = grid->gridrows[1].rowrect.y1;

	if (grid->gridrows[0].height == MGL_DEFAULT){
		rowheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		rowheight = grid->gridrows[0].height;
	}
	grid->elev_rect.y2 = grid->elev_rect.y1 + rowheight;

}


MGLGridInitCells(MGLGRID *grid)
{
	int r, c, rowheight;
	int curx;

	assert(grid);

	for (r = 0; r < grid->nrows; r++){
		curx = grid->gridrect.x1 + grid->marginx;
		if (grid->gridrows[r].height == MGL_DEFAULT){
			rowheight = MGL_GRID_DEF_ROW_HEIGHT;
		}
		else {
			rowheight = grid->gridrows[r].height;
		}

		for (c = 0; c < grid->ncols; c++){
			// all y values are the same for this row 
			grid->gridcells[r][c].cellrect.y1 = grid->gridrows[r].rowrect.y1;

			grid->gridcells[r][c].cellrect.x1 = curx;
			grid->gridcells[r][c].width = MGL_DEFAULT;
			grid->gridcells[r][c].height = MGL_DEFAULT;
			grid->gridcells[r][c].cellrect.y2 = grid->gridcells[r][c].cellrect.y1 + rowheight;
			grid->gridcells[r][c].cellrect.x2 = curx + grid->gridcells[r][c].width;

			grid->gridcells[r][c].fg = NULL;  // do not define as default so precedence is cell,row,grid in that order

			// move cursor to next grid cell x position
			if (grid->gridcells[r][c].width != MGL_DEFAULT){
				curx += grid->gridcells[r][c].width;
			}
			else if (grid->gridcols[c].width != MGL_DEFAULT){
				curx += grid->gridcols[c].width;
			}
			else {
				curx += MGL_GRID_DEF_COL_WIDTH;
			}
		}
	}

}


int MGLWhichRowCol(MGLGRID *grid, int x, int y, int *rownum, int *colnum)
{
	int c, r;

	// given an x, y pixel location find rownum and colnum

	if (!grid) return(FAIL);
	// find row number
	if (grid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - grid->gridrect.y1) / grid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - grid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	if (r < 0 || r >= grid->nrows){
		return(FAIL);
	}
	// reverse the order
	r = grid->nrows - r - 1;
	*rownum = r;

	for (c = 0; c < grid->ncols; c++){
		// all y values are the same for this row 
		// is mouse in this cell ??
		if (InRectangle(x, y, &(grid->gridcells[r][c].cellrect))){
			*colnum = c;
			return(SUCCEED);
		}
	}
	return(FAIL);
}

//mtm--adjusts the position of all the grids --- build grids, dataappselector, dataappcreater, secondtabgrid, titlegrid, buildsummary grid
void Move2AdjustGrids(struct InstanceData* id)
{


	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	MGLGridMove(id, DataAppCreater, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1 - (3 * DataAppGrid->gridrows[0].height));
	if (SecondTabGrid)
	{
		if (SecondTabGrid->childgrid)
		{
			MGLGridMove(id, SecondTabGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (SecondTabGrid->nrows* (DataAppGrid->gridrows[0].height)));
			if (SecondTabGrid->parent_w->visible == 1)
			{
				MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (SecondTabGrid->nrows* (DataAppGrid->gridrows[0].height)) - (SecondTabGrid->childgrid->BuildSummaryGrid->nrows* (DataAppGrid->gridrows[0].height)));
				if (SecondTabGrid->childgrid->ActionsGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->ParametersGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->AlertsColorGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.x2 + DataAppGrid->gridrows[0].height, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.y2 - (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.x2 + DataAppGrid->gridrows[0].height, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->gridrect.y1 - (2 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->SortAvailableGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->RowColumn)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->ActionsGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->ParametersGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->AlertsColorGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.x2 + DataAppGrid->gridrows[0].height, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.y2 - (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.x2 + DataAppGrid->gridrows[0].height, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->gridrect.y1 - (2 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->SortAvailableGrid)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				}
				if (SecondTabGrid->childgrid->RowColumn)
				{
					MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));
				}
			}
			else if (SecondTabGrid->parent_w->visible == 0)
			{
				MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - ((SecondTabGrid->childgrid->BuildSummaryGrid->nrows + 1) * (DataAppGrid->gridrows[0].height)));
				if (SecondTabGrid->childgrid->TitleGrid)
					MGLGridMove(id, SecondTabGrid->childgrid->TitleGrid, DataAppGrid->gridrect.x2 - 1, SecondTabGrid->childgrid->BuildSummaryGrid->gridrect.y2);
			}
		}
	}

}

int CalcStrPixelWidth(struct InstanceData* id, char *linestr, MGLGRID *grid)
{
	SIZE size;
	int len, adjsize;
	int r;
	// char errorbuf[ TMPBUF_SIZE ];

	assert(linestr);

	if ((len = strlen(linestr))){
		if (GetTextExtentPoint32(wglGetCurrentDC(), linestr, len, &size) == 0){

			for (r = 0; r < grid->nrows; r++){

				grid->gridrows[r].height = size.cy + 4;

			}
			//id->m_gluCustomBase;
			// function failed
			// sprintf_s(errorbuf, TMPBUF_SIZE, "error is %s", GetLastErrorStr());
		}
		else {
			// MS sucks, so size is inaccurate
			//  too big return( size.cx + (strlen( linestr ) * 2 ));
			// add some more spaces if there is a W in the string
			adjsize = size.cx;
			if (strchr(linestr, 'W')){
				adjsize += 3;
			}
			if (strchr(linestr, 'M')){
				adjsize += 3;
			}
			if (strchr(linestr, 'O')){
				adjsize += 3;
			}				

			for (r = 0; r < grid->nrows; r++){

				grid->gridrows[r].height = size.cy + 4;

			}
			
			Move2AdjustGrids(id); //mtm--adjusts the position of all the grids --- build grids, dataappselector, dataappcreater, secondtabgrid, titlegrid, buildsummary grid
			return(adjsize);
		}
	}
	return(0);

}

int CalcStrPixelWidth_fixedlenght(char *linestr)
{
	SIZE size;
	int len, adjsize;
	// char errorbuf[ TMPBUF_SIZE ];

	assert(linestr);

	len = strlen(linestr);
	size.cx = len * 7;
	adjsize = size.cx;
	if (strchr(linestr, 'W')){
		adjsize += 3;
				}
	if (strchr(linestr, 'M')){
		adjsize += 3;
				}
	if (strchr(linestr, 'O')){
		adjsize += 3;
				}
	return(adjsize);

}




VO *vo_dupnew(VO *src_vo)
{
	VO *newvo, *src_cols, *src_vcol;
	int i;

	assert(src_vo);

	newvo = (VO *)vo_create(0, NULL);
	vo_set(newvo, V_NAME, src_vo->name, NULL);


	src_cols = src_vo->vcols;

	for (i = 0; src_cols && i < src_cols->count; i++){
		src_vcol = V_ROW_PTR(src_cols, i);
		VOPropAdd(newvo, src_vcol->name, src_vcol->dbtype, src_vcol->size, VO_NO_ROW);
	}

	vo_rm_rows(newvo, 0, newvo->count);

	return(newvo);

}



void MGLFitRect2Screen(struct InstanceData *id, MGLGRID *grid)
{
	int absdiff;

	assert(id);
	assert(grid);

	if ((strcmp(grid->name, "DataAppSelectorGrid") == 0) || (strcmp(grid->name, "DataAppCreaterGrid") == 0) || (strcmp(grid->name, "TitleGridGrid") == 0) || (strcmp(grid->name, "SecondTabGrid") == 0) ||  (strcmp(grid->name, "BuildSummaryGrid") == 0))
	{
	}

	else
	{
		// adjust gridrect.x1 and y1 if needed
		if (grid->title_rect.x1 < 0){
			absdiff = -grid->title_rect.x1;
			grid->gridrect.x1 += absdiff;
		}
		if (grid->title_rect.x2 >= id->m_nWidth){
			absdiff = grid->title_rect.x2 - id->m_nWidth;
			grid->gridrect.x1 -= absdiff; // move x1 , not x2 since MGLGridMove will take care of the rest
		}
		if (grid->title_rect.y1 < 0){
			absdiff = -grid->title_rect.y1;
			grid->gridrect.y1 += absdiff;
		}
		if (grid->title_rect.y2 >= id->m_nHeight){
			absdiff = grid->title_rect.y2 - id->m_nHeight;
			grid->gridrect.y1 -= absdiff;  // move y1 , not y2 since MGLGridMove will take care of the rest			
		}
	}

}




void MGLSetTitle(MGLGRID *grid, char *appname)
{
	widgets *w;

	assert(appname);

	if (!grid || !grid->title_widgets) return;

	// look through all title widgets to find the title name widget "TitleWidget"
	for (w = grid->title_widgets; w; w = w->np){
		if (strstr(w->name, "TitleWidget")){
			strncpy_s(w->textbuf,64, appname, _TRUNCATE);
			return;
		}
	}
	
}





void DrawGLGUITextBase(struct InstanceData* id, int xpixel, int ypixel, char *str, GLuint fontbase, GLfloat *color)
{
	static int pflag = 0;

	assert(id);
	assert(str);

	glPushMatrix();
	glDisable(GL_DITHER);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glShadeModel(GL_FLAT);

	glLoadIdentity();

	glColor3f(1.0, 1.0, 1.0); /* white line */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* glortho (left, right, bottom , top, near, far ) */
	/* glOrtho(0.0f, w, 0.0f, h, 0.0f, 1.0f); */
	glOrtho(0, id->m_nWidth, 0, id->m_nHeight, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(trans[0], trans[1], trans[2]);
	glLineWidth(1);

	glColor3fv(color);
	glRasterPos2i(xpixel, ypixel);

	glPrintGUITextBase(id, fontbase, str);

	glFlush();
	glPopMatrix();
	glColor3fv(white);

}

void DrawGLGUIText(struct InstanceData* id, int xpixel, int ypixel, char *str, GLfloat *color)
{
	static int pflag = 0;

	assert(id);
	assert(str);

	glPushMatrix();
	glDisable(GL_DITHER);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glShadeModel(GL_FLAT);

	glLoadIdentity();

	glColor3f(1.0, 1.0, 1.0); /* white line */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* glortho (left, right, bottom , top, near, far ) */
	/* glOrtho(0.0f, w, 0.0f, h, 0.0f, 1.0f); */
	glOrtho(0, id->m_nWidth, 0, id->m_nHeight, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(trans[0], trans[1], trans[2]);
	glLineWidth(1);

	glColor3fv(color);
	glRasterPos2i(xpixel, ypixel);

	glPrintGUIText(id, str);

	glFlush();
	glPopMatrix();
	glColor3fv(white);

}


void DrawCachedPNGTest(struct InstanceData* id, char *imagename, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, int alpha_mode, int blendflag, int ChangeColorAlpha, int filtertype,
	GLfloat angle)
{
	// differs from DrawCachedPNG because it allows any size PNG image and any size texture area
	int map_square_pixels = 0;
	int width, height;
	// int imgxsize, imgysize;
	GLubyte *mapimage;
	MAPLL *png = NULL;
	GLfloat params[4];
	char tmpstr[TMPBUF_SIZE];
	FILE *in;
	gdImagePtr im;
	char pngname[128];
	const DWORD cdwThreadId = GetCurrentThreadId();
	extern int GetLocalFile(struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
	extern void PNGRect2Texture(struct InstanceData* id, GLubyte *Gmapimage, gdImagePtr im, int alpha_mode);
	extern void ImgChangeColorAlpha(GLubyte *mapimage, int ncols, int nrows, int r, int g, int b, int alpha,
		int bg_red, int bg_green, int bg_blue);
	extern GLfloat MapIntensity;

	assert(id);
	assert(imagename);

	// this routine will load in a png image, create a texture, and draw it on the screen. 
	//  This routine will cache the texture so it will not load into memory on subsequent calls

	// see if we have this file loaded into a texture already
	for (png = id->m_pPngLL; png; png = png->np){
		if (!strcmp(png->mapfilename, imagename)){
			// already have this processed
			break;
		}
	}

	if (!png){
		// load in the PNG
		if (!GetLocalFile(id, id->m_strImageDir, imagename, id->m_strHttpImagePath, SM_HTTPS))
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to find local texture(%s).", cdwThreadId, imagename);
			return;
		}
		_snprintf_s(pngname, _countof(pngname), _TRUNCATE, "%s%s", id->m_strImageDir, imagename);

		if (fopen_s(&in, pngname, "rb")){
			sprintf_s(tmpstr, TMPBUF_SIZE, "Could not open file %s", imagename);
			return;
		}
		im = gdImageCreateFromPng(in);
		fclose(in);

		width = im->sx;
		height = im->sy;

		mapimage = (GLubyte *)calloc(1, width * height * 4);

		// copy the gdimage into the opengl image format (mapimage)
		PNGRect2Texture(id, mapimage, im, alpha_mode);

		if (ChangeColorAlpha){
			// Added to Change make white "see thru"
			ImgChangeColorAlpha(mapimage, width, height, 255, 255, 255, 0, 255, 255, 255);
		}

		gdImageDestroy(im);

		if (!id->m_pPngLL){
			id->m_pPngLL = calloc(1, sizeof(MAPLL));
			id->m_pPngCurp = id->m_pPngLL;
		}
		else {
			(id->m_pPngCurp)->np = calloc(1, sizeof(MAPLL));
			id->m_pPngCurp = (id->m_pPngCurp)->np;
		}

		strncpy_s((id->m_pPngCurp)->mapfilename, NAMESIZE, imagename, _TRUNCATE);
		//(id->m_pPngCurp)->x1 = (GLfloat) ploc->x1;
		//(id->m_pPngCurp)->y1 = (GLfloat) ploc->y1;
		//(id->m_pPngCurp)->x2 = (GLfloat) ploc->x2;
		//(id->m_pPngCurp)->y2 = (GLfloat) ploc->y2;

		glGenTextures(1, &(id->m_pPngCurp)->maptexture);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Generated %s Texture: %d.\n", cdwThreadId, (id->m_pPngCurp)->mapfilename, (id->m_pPngCurp)->maptexture);
		png = id->m_pPngCurp;

		glBindTexture(GL_TEXTURE_2D, png->maptexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, mapimage);

		/* Linear Filtering produces a smoothed image, more accurate and realistic for many uses */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtertype); // filter type is GL_NEAREST or GL_LINEAR
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtertype);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


		free(mapimage);
		mapimage = NULL;

	}


	/* reset the color to white so textures will appear */
	glColor3f(1.0, 1.0, 1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* glortho (left, right, bottom , top, near, far ) */
	/* glOrtho(0.0f, w, 0.0f, h, 0.0f, 1.0f); */
	glOrtho(id->m_sCurLayout.m_dWorldXmin, id->m_sCurLayout.m_dWorldXmax, id->m_sCurLayout.m_dWorldYmin, id->m_sCurLayout.m_dWorldYmax,
		0.0f, 1.0f);
	//glOrtho((GLfloat) 0.0f, (GLfloat)id->m_nWidth,
	//(GLfloat) 0.0f, (GLfloat) id->m_nHeight, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, png->maptexture);

	/* solid with alpha masking */
	//  fw  glEnable(GL_DEPTH_TEST);


	// glDepthFunc(GL_LEQUAL); // must be Less than or Equal since ocean has depth now for terrain
	// glDepthFunc(GL_EQUAL);

	glPushMatrix();
	glLoadIdentity();


	glTranslatef((GLfloat)x1, (GLfloat)y1, (GLfloat)trans[2]);

	params[0] = 1.0f;
	params[1] = 1.0f;
	params[2] = 1.0f;
	params[3] = 1.0f;

	glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, params);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glEnable(GL_TEXTURE_2D);
	glColor4f(MapIntensity, MapIntensity, MapIntensity, 1.0f);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f((GLfloat) 0.0f, (GLfloat)y2 - y1, 0.0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f((GLfloat) 0.0f, (GLfloat) 0.0, 0.0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f((GLfloat)x2 - x1, (GLfloat) 0.0, 0.0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f((GLfloat)x2 - x1, (GLfloat)y2 - y1, 0.0);

	glEnd();

	glFlush();

	glPopMatrix();

	glDisable(GL_TEXTURE_ENV);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// reset to standard blend func
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}






MGLGridInitCols(MGLGRID *grid)
{
	int i;

	assert(grid);

	for (i = 0; i < MGL_GRID_MAX_COLS; i++){
		grid->gridcols[i].fg = NULL;  // do not define as default so precedence is cell,row,grid in that order
		grid->gridcols[i].bg = NULL;  // do not define as default so precedence is cell,row,grid in that order
		grid->gridcols[i].width = MGL_DEFAULT;
		grid->gridcols[i].height = MGL_DEFAULT;
	}

}

MGLGRID *MGLGridCreate(struct InstanceData* id, MGLGRID *parentgrid, char *name, int nrows, int ncols, int nxpos, int nypos)
{
	MGLGRID *newgrid;
	char tmpstr[256];
	int i;
	// int width;

	assert(id);

	if (nrows > MGL_GRID_MAX_ROWS || ncols > MGL_GRID_MAX_COLS){
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Grid sizes exceed maximum, nrows=%d ncols=%d\n", GetCurrentThreadId(), nrows, ncols);
		return(NULL);
	}

	newgrid = (MGLGRID *)vo_malloc(sizeof(MGLGRID));
	if (name){
		strncpy_s(newgrid->name, MGL_GRID_NAME_MAX, name, _TRUNCATE);
	}

	newgrid->nrows = nrows;
	newgrid->ncols = ncols;
	newgrid->parentgrid = parentgrid;

	/* set default values for features */
	newgrid->gridrect.x1 = nxpos;
	if (nypos < 0){
		sprintf_s(tmpstr, _countof(tmpstr), "bad y pos\n");
		OutputDebugString(tmpstr);
	}
	else {
		newgrid->gridrect.y1 = nypos;
	}
	newgrid->height = nrows * MGL_GRID_DEF_ROW_HEIGHT;
	newgrid->width = ncols * MGL_GRID_DEF_COL_WIDTH;
	newgrid->gridrect.y2 = newgrid->gridrect.y1 + newgrid->height;
	newgrid->gridrect.x2 = newgrid->gridrect.x1 + newgrid->width;

	newgrid->bg = NULL;
	newgrid->fg = NULL;
	newgrid->marginx = MGL_GRID_MARGIN_X;
	newgrid->marginy = MGL_GRID_MARGIN_Y;

	/* set default row values */
	MGLGridInitRows(newgrid);
	MGLGridInitCols(newgrid);
	MGLGridInitCells(newgrid);
	//	MGLGridInitScroll(newgrid);


	// test
	//width = CalcStrPixelWidth("WWWWW");
	//width = CalcStrPixelWidth("11111");
	//width = CalcStrPixelWidth("W");
	//width = CalcStrPixelWidth("L");

#ifdef OLDWAY
	newgrid->RowVO = MGLCreateRowVO();
	newgrid->ColVO = MGLCreateColVO();
#endif

	for (i = 0; i < newgrid->ncols; i++){
		newgrid->DisplayCol[i] = i; // default location is same of col number order
	}

	return(newgrid);
}



void MGLGridResizeCol(struct InstanceData *id, MGLGRID *grid, int c, int maxwidth)
{
	char tmpstr[256];
	

	grid->gridcols[c].width = maxwidth + grid->marginx;
	sprintf_s(tmpstr, _countof(tmpstr), "MGLGridResizeCol c=%d new width=%d\n", c, maxwidth);
	OutputDebugString(tmpstr);
	MGLGridMove(id, grid, grid->gridrect.x1, grid->gridrect.y1);


}



int MGLGridMove(struct InstanceData *id, MGLGRID *grid, int x1, int y1)
{
	int i, vocol;
	int curypos = 0;
	int rowheight;
	char tmpstr[256];

	int r, c;
	int curx;


	//if (grid->move_flag == 20)
	//	goto next;
	//if (grid->move_flag == 30)
	//	goto next;

	assert(id);
	assert(grid);

	// Move Main grid
	grid->gridrect.x1 = x1;
	grid->gridrect.y1 = y1;


	// force title row to be within our visible window
	MGLFitRect2Screen(id, grid);
	

	grid->gridrect.y2 = grid->gridrect.y1 + grid->height;
	grid->gridrect.x2 = grid->gridrect.x1 + grid->width;


	// Move Grid Rows
	for (i = 0; i < grid->nrows; i++){
		if (i == 0){
			if (grid->gridrows[i].height != MGL_DEFAULT){
				curypos = grid->gridrect.y2;
			}
			else {
				curypos = grid->gridrect.y2;
			}
		}
		grid->gridrows[i].rowrect.x1 = grid->gridrect.x1;

		// subtract height for this row from the current position
		if (grid->gridrows[i].height == MGL_DEFAULT){
			rowheight = MGL_GRID_DEF_ROW_HEIGHT;
		}
		else {
			rowheight = grid->gridrows[i].height;
		}
		if (curypos - rowheight < 0){
			sprintf_s(tmpstr, _countof(tmpstr), "bad y pos\n");
			OutputDebugString(tmpstr);
			grid->gridrows[i].rowrect.y1 = curypos - rowheight;
		}
		else {
			grid->gridrows[i].rowrect.y1 = curypos - rowheight;
		}
		curypos = grid->gridrows[i].rowrect.y1; //move cursor down to this row	
	}

	// Move Grid Cols
	// Nothing to do here

	// Move Grid Cells	
	for (r = 0; r < grid->nrows; r++){
		curx = grid->gridrect.x1 + grid->marginx;
		for (c = 0; c < grid->ncols; c++){

			// these values should be in display order
			vocol = grid->DisplayCol[c];
			if (vocol < 0){
				// invisible row
				continue;
			}

			// all y values are the same for this row 
			grid->gridcells[r][vocol].cellrect.y1 = grid->gridrows[r].rowrect.y1;
			grid->gridcells[r][vocol].cellrect.x1 = curx;

			// move cursor to next grid cell x position
			if (grid->gridcells[r][vocol].width != MGL_DEFAULT){
				curx += grid->gridcells[r][vocol].width;
			}
			else if (grid->gridcols[vocol].width != MGL_DEFAULT){
				curx += grid->gridcols[vocol].width;
			}
			else {
				curx += MGL_GRID_DEF_COL_WIDTH;
			}

			if (grid->gridrows[r].height == MGL_DEFAULT){
				rowheight = MGL_GRID_DEF_ROW_HEIGHT;
			}
			else {
				rowheight = grid->gridrows[i].height;
			}
			grid->gridcells[r][vocol].cellrect.y2 = grid->gridcells[r][c].cellrect.y1 + rowheight;
			grid->gridcells[r][vocol].cellrect.x2 = curx;

			// this has already been added to gridcols width curx += grid->marginx;  see MGLGridResizeCol
		}
	}
	grid->gridrect.x2 = curx;
	grid->width = grid->gridrect.x2 - grid->gridrect.x1;

	// set the min or max grid coordinates depending upon our state
	if (grid->IsMinimized){
		//grid->gridrect.x2 = 50;
		grid->gridrectmin = grid->gridrect;

	}
	else {
		grid->gridrectmax = grid->gridrect;
	}


	return(TRUE);
}

int MGLGridText(struct InstanceData* id, MGLGRID *grid, int row, int col, char *text)
{
	int len = 0;

	assert(id);

	if (grid && row >= 0 && row < grid->nrows &&
		col >= 0 && col < grid->ncols){
		/* free old string, if one */
		if (grid->gridcells[row][col].val && strlen(grid->gridcells[row][col].val)){
			s_free(grid->gridcells[row][col].val);
			grid->gridcells[row][col].val = NULL; // make sure
		}
		if (text && ((len = strlen(text)) < 100) && len > 0){
			grid->gridcells[row][col].val = str_falloc(text);
		}
		else {
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Text size too large, len=%d\n", GetCurrentThreadId(), len);
		}
	}
	return(TRUE);
}




int IsRowFiltered(MGLGRID *grid, int r)
{
	return(FALSE);

}



int MGLGridDrawRows(MGLGRID *grid)
{
	int r, rowheight, vorow;
	int x1, y1, x2, y2, bgint, bgred, bggreen, bgblue;
	GLfloat *bg;
	GLfloat bgcolor[3];
	VO *vo;
	

	assert(grid);


	for (r = 0; r < grid->nrows; r++){

		/* draw backgound of entire row */

		if (IsRowFiltered(grid, r)){
			// row was filtered out
		}
		
		x1 = grid->gridrows[r].rowrect.x1;
		y1 = grid->gridrows[r].rowrect.y1;

		

		if (grid->gridrows[r].height == MGL_DEFAULT){
			rowheight = MGL_GRID_DEF_ROW_HEIGHT;
		}
		else {
			rowheight = grid->gridrows[r].height;
		}

		x2 = x1 + grid->width;
		y2 = y1 + rowheight;

		if (grid->gridrows[r].texture)
		{
			MGLGridTexQuad(grid->gridrows[r].texture, x1, y1, x2, y2, grid->gridrows[r].bg);
		}
		else
		{
			if (grid->gridrows[r].bg_callback){
				bg = (GLfloat *)grid->gridrows[r].bg_callback(grid, r);
			}
			else {
				bg = grid->gridrows[r].bg;
			}
			if (r > 0){
				// see if row bg was set by grid row values
				vorow = r - 1 + grid->iVscrollPos;
				if (grid->filteredvo && grid->filteredvo->count){
					vo = grid->filteredvo;
				}
				else if (grid->vo  && grid->vo->count){
					vo = grid->vo;
				}
				else { vo = NULL; }
				if (vo && vorow < vo->count){
					if ((bgint = vo->row_index[vorow].bg) >= 0){
						// convert integer value to rgb
						bgred = (bgint & 0xff0000) >> 16;
						bggreen = (bgint & 0xff00) >> 8;
						bgblue = (bgint & 0xff);
						bgcolor[0] = (GLfloat)bgred / 255.0f;
						bgcolor[1] = (GLfloat)bggreen / 255.0f;
						bgcolor[2] = (GLfloat)bgblue / 255.0f;
						bg = bgcolor;
					}
				}
			}

			glColor3fv(bg);
			glRecti(x1, y1, x2, y2);
		}

		if ((strcmp(grid->name, "TitleGridGrid") == 0) || (strcmp(grid->name, "BuildSummaryGrid") == 0) || (strcmp(grid->name, "SecondTabGrid") == 0))
		{
			glLineWidth(0.5);
			glColor3f(0.6274509f, 0.6274509f, 0.6274509f);
			glBegin(GL_LINES);
			glVertex2f((GLfloat)x1, (GLfloat)y1);
			glVertex2f((GLfloat)x1, (GLfloat)y2);
			glEnd();
		}

		if (r == grid->nrows - 1)
		{
			if ((strcmp(grid->name, "SecondTabGrid") == 0))
			{
				glLineWidth(0.5);
				glColor3f(0.6274509f, 0.6274509f, 0.6274509f);
				glBegin(GL_LINES);
				glVertex2f((GLfloat)(x1 + 15), (GLfloat)y1);
				glVertex2f((GLfloat)(x2 - 15), (GLfloat)y1);
				glEnd();
			}
		}
	}



	return(TRUE);
}

int	MGLGridDrawColumns(MGLGRID *grid)
{
	return(TRUE);
}


///added//
void MGLInitAlertsColorsVO(MGLGRID *topgrid)
{

	assert(topgrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!topgrid->AlertsColorGrid->vo){
		CrAlertsColorVO(topgrid);
	}
	else {
		// already created the SortAvailableGrid->vo 
	}

	// clear out the current settings
	vo_rm_rows(topgrid->AlertsColorGrid->vo, 0, topgrid->AlertsColorGrid->vo->count);
	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Foreground color", _TRUNCATE);
	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Background color", _TRUNCATE);
	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);	
	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Icon Color", _TRUNCATE);
	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Filter", _TRUNCATE);
	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "E-Mail", _TRUNCATE);
	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Show flights", _TRUNCATE);
	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
	


}
//mtm
void CrRowColumnVO(MGLGRID *parentgrid)
{

	VO *RowColumnVO;

	assert(parentgrid);

	RowColumnVO = (VO *)vo_create(0, NULL);
	vo_set(RowColumnVO, V_NAME, "RowColumnVO", NULL);
	VOPropAdd(RowColumnVO, "Column", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(RowColumnVO, "Settings", NTBSTRINGBIND, 16, VO_NO_ROW);


	// BUG??  vo_set(RowColumnVO, 2, "Column", NULL, NULL);

	parentgrid->RowColumn->vo = RowColumnVO;

}

//mtm : CreateVO for EMail Grid
void CrEMailVO(MGLGRID *EMailGrid)
{

	VO *EMailVO;

	assert(EMailGrid);

	EMailVO = (VO *)vo_create(0, NULL);
	vo_set(EMailVO, V_NAME, "EMailVO", NULL);
	VOPropAdd(EMailVO, "To", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(EMailVO, "EMail Notifications", NTBSTRINGBIND, 64, VO_NO_ROW);
	EMailGrid->vo = EMailVO;

}
//mtm : CreateVO for Subject Grid - Email
void CrSubjectVO(MGLGRID *SubjectGrid)
{

	VO *SubjectVO;

	assert(SubjectGrid);

	SubjectVO = (VO *)vo_create(0, NULL);
	vo_set(SubjectVO, V_NAME, "SubjectVO", NULL);
	VOPropAdd(SubjectVO, "To", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(SubjectVO, "Subject Line", NTBSTRINGBIND, 64, VO_NO_ROW);
	SubjectGrid->vo = SubjectVO;

}
//mtm : VO for EMail: EMailID//
void FillEMailVO(struct InstanceData* id, MGLGRID *EMailGrid)
{

	assert(id);
	assert(EMailGrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!EMailGrid->vo){
		CrEMailVO(EMailGrid);
	}
	else {
		// already created the DataAppGrid->vo
		return;
	}

	vo_alloc_rows(EMailGrid->vo, 1);
	strncpy_s(VP(EMailGrid->vo, EMailGrid->vo->count - 1, "To", char), 64, "To :", _TRUNCATE);
	strncpy_s(VP(EMailGrid->vo, EMailGrid->vo->count - 1, "EMail Notifications", char), 64, "Enter EMailID", _TRUNCATE);

}
//mtm : VO for Subject: EMail//
void FillSubjectVO(struct InstanceData* id, MGLGRID *SubjectGrid)
{

	assert(id);
	assert(SubjectGrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!SubjectGrid->vo){
		CrSubjectVO(SubjectGrid);
	}
	else {
		// already created the DataAppGrid->vo
		return;
	}

	vo_alloc_rows(SubjectGrid->vo, 1);
	strncpy_s(VP(SubjectGrid->vo, SubjectGrid->vo->count - 1, "Subject", char), 64, "Subject :", _TRUNCATE);	
	


}
///added//
void MGLInitGridofRowsColumnsVO(MGLGRID *topgrid)
{
	
	assert(topgrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!topgrid->RowColumn->vo){
		CrRowColumnVO(topgrid);
	}
	else {
		// already created the SortAvailableGrid->vo 
	}

	// clear out the current settings
	vo_rm_rows(topgrid->RowColumn->vo, 0, topgrid->RowColumn->vo->count);
	vo_alloc_rows(topgrid->RowColumn->vo, 1);
	strncpy_s(VP(topgrid->RowColumn->vo, topgrid->RowColumn->vo->count - 1, "Rows", char), 64, "Number of Rows", _TRUNCATE);
	vo_alloc_rows(topgrid->RowColumn->vo, 1);
	strncpy_s(VP(topgrid->RowColumn->vo, topgrid->RowColumn->vo->count - 1, "Column", char), 64, "Number of Columns", _TRUNCATE);
	vo_alloc_rows(topgrid->RowColumn->vo, 1);

}



void CalcColInternalWidths(struct InstanceData* id, MGLGRID *grid)
{
	int r, c, mincolwidth, maxcolwidth, width;
	char *colname, *valstr;
	// calculate width needed for each column

	assert(id);
	assert(grid);

	for (c = 0; c < grid->ncols; c++){
		mincolwidth = 9999;
		maxcolwidth = 0;
		colname = grid->ColOrderNames[c];

		for (r = 0; r < grid->nrows; r++){
			if (r == 0){
				// title row
				if (strlen((valstr = grid->ColLabels[c]))){
				}
				else {
					valstr = colname;
				}
			}
			else if (grid->gridcells[r][c].val && strlen(grid->gridcells[r][c].val)){
				valstr = grid->gridcells[r][c].val;
			}
			else {
				valstr = " ";
			}

			//width = CalcStrPixelWidth_fixedlenght(valstr);
			width = CalcStrPixelWidth(id,valstr,grid);
			if (width > maxcolwidth){
				maxcolwidth = width;
			}
			if (width < mincolwidth){
				mincolwidth = width;
			}
		}

		//if (maxcolwidth > grid->gridcols[c].width - grid->marginx){
		//	MGLGridResizeCol(id, grid, c, maxcolwidth);
		//}
		//if (maxcolwidth < grid->gridcols[c].width - grid->marginx){
			MGLGridResizeCol(id, grid, c, maxcolwidth);
		//}

	}

}

//mtm

void CalcColWidths_condition(struct InstanceData* id, MGLGRID *grid)
{
	int r, c, mincolwidth, maxcolwidth, width;
	VO *vo, *vcol;
	int vorow, vocol; // used to convert visible rows and cols to vo struct
	char tmpstr[1024];
	char *colname, *valstr = NULL;
	static int testspeedflag = 0;
	static int testtimes = 0;
	struct timeb start_time, done_time;

	assert(id);
	assert(grid);

	
	//Setting void pointers to MGLGRID struct	
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	
	// calculate width needed for each column
		if (testspeedflag){
			return;  // see if this routine is causing issues
		}
		if (testtimes) ftime(&start_time);


		// we need to process all rows, even invisible ones, in order to keep the column sizes from changing
		//    whenever we scroll. So process entire vo, not just visible rows, but only do this when data changes
		if (grid->filteredvo){
			vo = grid->filteredvo;
		}
		else if (grid->vo){
			vo = grid->vo;
		}
		else {
			vo = NULL;
		}
		if (!vo || !vo->count){
		//	if (strcmp(grid->name, "SecondTabGrid") == 0)
		//	{
			//}
			//else
			//{
				CalcColInternalWidths(id, grid);
				return;
			//}
		}



		for (c = 0; c < grid->ncols; c++){
			mincolwidth = 9999;
			maxcolwidth = 0;


			vocol = grid->DisplayCol[c];
			if (vocol < 0){
				// invisible row
				/*if (SecondTabGrid->childgrid)
				if (SecondTabGrid->childgrid->SortAvailableGrid)
				VV(SecondTabGrid->childgrid->SortAvailableGrid->vo,c, "Visible", int) = FALSE;*/
				continue;

			}
			colname = grid->ColOrderNames[vocol];

			for (r = 0; r < vo->count + 1; r++){

				//  visible only does not work for ( r = 0; r < grid->nrows; r++ ){

				///  OLD WAY LOOKS AT ALL ROWS for ( r = 0; r < vo->count + 1; r++ ){
				if (r == 0){
					// title row
					if (strlen((valstr = grid->ColLabels[vocol]))){
						//id->OuterGLPrint(valstr);
					}
					else {
						// id->OuterGLPrint(colname);
						valstr = colname;
					}
				}
				else {
					vorow = r - 1; // title row, so subtract one from r
					if (vorow < vo->count && vorow >= 0 && vocol < vo->vcols->count){
						if (strlen(colname)){
							vcol = vcol_find(vo, colname);
						}
						else {
							vcol = V_ROW_PTR(vo->vcols, vocol);
						}
						if (!vcol){
							vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "bad null vcol for colname '%s'\n", colname);
						}
						else {
							valstr = get_vcol_valstr(vcol, V_ROW_PTR(vo, vorow), TRUE);
							if (valstr && grid->gridcols[vocol].val_callback){
								valstr = (char *)grid->gridcols[vocol].val_callback(id,valstr);
							}
						}
					}
				}
				if (valstr){
					//width = CalcStrPixelWidth_fixedlenght(valstr);
					width = CalcStrPixelWidth(id,valstr, grid);
					if (width > maxcolwidth){
						maxcolwidth = width;
					}
					if (width < mincolwidth){
						mincolwidth = width;
					}
				}
			}

			
				if (strcmp((grid->name), "DataAppSelectorGrid") == 0)
				{
					MGLGridResizeCol(id, DataAppGrid, vocol, maxcolwidth);
					MGLGridResizeCol(id, DataAppCreater, vocol, maxcolwidth);
				}
		
			//To keep the Grids same size:Buildsummary,Titlegrid,SecondTabGrid
				if (strcmp((grid->name), "BuildSummaryGrid") == 0)
				{
					MGLGridResizeCol(id, grid, vocol, maxcolwidth);
					if (SecondTabGrid->childgrid->TitleGrid->parent_w->visible == 1)
					{
						MGLGridResizeCol(id, SecondTabGrid->childgrid->TitleGrid, vocol, maxcolwidth);
					}
					if (SecondTabGrid->childgrid->TitleGrid->parent_w->visible == 0)
					{
						MGLGridResizeCol(id, SecondTabGrid, vocol, maxcolwidth);
					}

				}
		
		}

		if (testtimes){
			ftime(&done_time);
			sprintf_s(tmpstr, 256, "%s grid %s nrows=%d diff=%g\n", __FUNCTION__, grid->name, vo->count, uts_timediff(&start_time, &done_time));
			OutputDebugString(tmpstr);
		}

		id->DataAppGrid = DataAppGrid;
		id->DataAppCreater = DataAppCreater;
}

void CalcColWidths_condition2(struct InstanceData* id, MGLGRID *grid)
{
	int r, c, mincolwidth, maxcolwidth, width;
	VO *vo, *vcol;
	int vorow, vocol; // used to convert visible rows and cols to vo struct
	char tmpstr[1024];
	char *colname, *valstr = NULL;
	static int testspeedflag = 0;
	static int testtimes = 0;
	struct timeb start_time, done_time;

	assert(id);
	assert(grid);

	// calculate width needed for each column
	if (strcmp("TitleGridGrid", grid->name) == 0)
		return;

	if (testspeedflag){
		return;  // see if this routine is causing issues
	}
	if (testtimes) ftime(&start_time);


	// we need to process all rows, even invisible ones, in order to keep the column sizes from changing
	//    whenever we scroll. So process entire vo, not just visible rows, but only do this when data changes
	if (grid->filteredvo){
		vo = grid->filteredvo;
	}
	else if (grid->vo){
		vo = grid->vo;
	}
	else {
		vo = NULL;
	}
	

	for (c = 0; c < grid->ncols; c++){
		mincolwidth = 9999;
		maxcolwidth = 0;


		vocol = grid->DisplayCol[c];
		if (vocol < 0){
			// invisible row
			/*if (SecondTabGrid->childgrid)
			if (SecondTabGrid->childgrid->SortAvailableGrid)
			VV(SecondTabGrid->childgrid->SortAvailableGrid->vo,c, "Visible", int) = FALSE;*/
			continue;

		}
		colname = grid->ColOrderNames[vocol];

		for (r = 0; r < vo->count + 1; r++){

			//  visible only does not work for ( r = 0; r < grid->nrows; r++ ){

			///  OLD WAY LOOKS AT ALL ROWS for ( r = 0; r < vo->count + 1; r++ ){
			if (r == 0){
				// title row
				if (strlen((valstr = grid->ColLabels[vocol]))){
					//id->OuterGLPrint(valstr);
				}
				else {
					// id->OuterGLPrint(colname);
					valstr = colname;
				}
			}
			else {
				vorow = r - 1; // title row, so subtract one from r
				if (vorow < vo->count && vorow >= 0 && vocol < vo->vcols->count){
					if (strlen(colname)){
						vcol = vcol_find(vo, colname);
					}
					else {
						vcol = V_ROW_PTR(vo->vcols, vocol);
					}
					if (!vcol){
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "bad null vcol for colname '%s'\n", colname);
					}
					else {
						valstr = get_vcol_valstr(vcol, V_ROW_PTR(vo, vorow), TRUE);
						if (valstr && grid->gridcols[vocol].val_callback){
							valstr = (char *)grid->gridcols[vocol].val_callback(id,valstr);
						}
					}
				}
			}
			if (valstr){
				//	width = CalcStrPixelWidth_fixedlenght(valstr);
				width = CalcStrPixelWidth(id, valstr, grid);
			
			}
		}

		

	}

	if (testtimes){
		ftime(&done_time);
		sprintf_s(tmpstr, 256, "%s grid %s nrows=%d diff=%g\n", __FUNCTION__, grid->name, vo->count, uts_timediff(&start_time, &done_time));
		OutputDebugString(tmpstr);
	}


}

void CalcColWidths(struct InstanceData* id, MGLGRID *grid)
{
	int r, c, mincolwidth, maxcolwidth, width;
	VO *vo, *vcol;
	int vorow, vocol; // used to convert visible rows and cols to vo struct
	char tmpstr[1024];
	char *colname, *valstr = NULL;
	static int testspeedflag = 0;
	static int testtimes = 0;
	struct timeb start_time, done_time;

	assert(id);
	assert(grid);

	// calculate width needed for each column

	if (testspeedflag){
		return;  // see if this routine is causing issues
	}
	if (testtimes) ftime(&start_time);	


	// we need to process all rows, even invisible ones, in order to keep the column sizes from changing
	//    whenever we scroll. So process entire vo, not just visible rows, but only do this when data changes
	if (grid->filteredvo){
		vo = grid->filteredvo;
	}
	else if (grid->vo){
		vo = grid->vo;
	}
	else {
		vo = NULL;
	}
	if (!vo || !vo->count){		
		CalcColInternalWidths(id, grid);
		return;
	}



	for (c = 0; c < grid->ncols; c++){
		mincolwidth = 9999;
		maxcolwidth = 0;


		vocol = grid->DisplayCol[c];
		if (vocol < 0){
			// invisible row
			/*if (SecondTabGrid->childgrid)
				if (SecondTabGrid->childgrid->SortAvailableGrid)
				VV(SecondTabGrid->childgrid->SortAvailableGrid->vo,c, "Visible", int) = FALSE;*/
			continue;

		}
		colname = grid->ColOrderNames[vocol];

		for (r = 0; r < vo->count+1; r++){

			//  visible only does not work for ( r = 0; r < grid->nrows; r++ ){

			///  OLD WAY LOOKS AT ALL ROWS for ( r = 0; r < vo->count + 1; r++ ){
			if (r == 0){
				// title row
				if (strlen((valstr = grid->ColLabels[vocol]))){
					//id->OuterGLPrint(valstr);
				}
				else {
					// id->OuterGLPrint(colname);
					valstr = colname;
				}
			}
			else {
				vorow = r - 1; // title row, so subtract one from r
				if (vorow < vo->count && vorow >= 0 && vocol < vo->vcols->count){
					if (strlen(colname)){
						vcol = vcol_find(vo, colname);
					}
					else {
						vcol = V_ROW_PTR(vo->vcols, vocol);
					}
					if (!vcol){
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "bad null vcol for colname '%s'\n", colname);
					}
					else {
						valstr = get_vcol_valstr(vcol, V_ROW_PTR(vo, vorow), TRUE);
						if (valstr && grid->gridcols[vocol].val_callback){
							valstr = (char *)grid->gridcols[vocol].val_callback(id,valstr);
						}
					}
				}
			}
			if (valstr){
			//	width = CalcStrPixelWidth_fixedlenght(valstr);
				width = CalcStrPixelWidth(id,valstr, grid);
				if (width > maxcolwidth){
					maxcolwidth = width;
				}
				if (width < mincolwidth){
					mincolwidth = width;
				}
			}
		}

		if (maxcolwidth > grid->gridcols[vocol].width - grid->marginx){
			MGLGridResizeCol(id, grid, vocol, maxcolwidth);					
	    }
		if (maxcolwidth < grid->gridcols[vocol].width - grid->marginx){
			MGLGridResizeCol(id, grid, vocol, maxcolwidth);					
		}

	}

	if (testtimes){
		ftime(&done_time);
		sprintf_s(tmpstr, 256, "%s grid %s nrows=%d diff=%g\n", __FUNCTION__, grid->name, vo->count, uts_timediff(&start_time, &done_time));
		OutputDebugString(tmpstr);
	}


}



SetPNGMode(struct InstanceData* id)
{

	assert(id);
	glPushMatrix();
	glLoadIdentity();


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* glortho (left, right, bottom , top, near, far ) */
	/* glOrtho(0.0f, w, 0.0f, h, 0.0f, 1.0f); */
	glOrtho(0, id->m_nWidth, 0, id->m_nHeight, 0.0f, 1.0f);
	//glOrtho(id->m_sCurLayout.m_dWorldXmin, id->m_sCurLayout.m_dWorldXmax, id->m_sCurLayout.m_dWorldYmin, id->m_sCurLayout.m_dWorldYmax,
	// 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(trans[0], trans[1], trans[2]);
	/* glTranslatef(0.0, 0.0, 0.0 ); */
	glLineWidth(1);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);



	glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}




int MGLCalcCol(MGLGRID *grid, int x)
{
	int c, x1, x2, width;

	assert(grid);

	// brute force method to find which cell the user selected
	for (c = 0; c < grid->ncols; c++){

		if (grid->gridcells[0][c].width != MGL_DEFAULT){
			width = grid->gridcells[0][c].width;
		}
		else if (grid->gridcols[c].width != MGL_DEFAULT){
			width = grid->gridcols[c].width;
		}
		else {
			width = MGL_GRID_DEF_COL_WIDTH;
		}


		x1 = grid->gridcells[0][c].cellrect.x1;
		x2 = x1 + width;
		if (x >= x1 && x <= x2){
			return(c);
		}
	}

	// should not get here
	return(0);

}


void MGLGridDrawColImage(struct InstanceData* id, MGLGRID *grid, int vocol, int r, int c)
{
	double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode,int rotate);
	int shrink = 4, cellwidth, cellheight;
	MRECT image_rect;

	assert(id);
	assert(grid);

	// draw the named png file into the grid cell location

	SetPNGMode(id);

	if (grid->gridcells[r][vocol].height == MGL_DEFAULT){
		cellheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		cellheight = grid->gridrows[r].height;
	}
	if (grid->gridcells[r][vocol].width == MGL_DEFAULT){
		cellwidth = grid->gridcols[vocol].width;
	}
	else {
		cellwidth = grid->gridcells[r][vocol].width;
	}
	image_rect.x1 = grid->gridcells[r][vocol].cellrect.x1;
	image_rect.y1 = grid->gridcells[r][vocol].cellrect.y1;
	image_rect.x2 = image_rect.x1 + cellwidth;
	image_rect.y2 = image_rect.y1 + cellheight;

	// center image in middle of column
	shrink = (cellwidth - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.x1 += shrink;
	image_rect.x2 -= shrink;


	shrink = (cellheight - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.y1 += shrink;
	image_rect.y2 -= shrink;


	y1 = (((double)image_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)image_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)image_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)image_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

	DisplayCachedPNG(id, grid->gridcols[vocol].pngfilename, grid->gridcols[vocol].texturesize,
		(float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1), GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_BLACK,0);

	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);

}

////mtm to change png for grid actions
//void MGLGridDrawColImage(struct InstanceData* id, MGLGRID *grid, int vocol, int r, int c)
//{
//	double x1, x2, y1, y2;
//	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
//		GLfloat YSize, GLenum dfactor, int alpha_mode);
//	int shrink = 4, cellwidth, cellheight;
//	MRECT image_rect;
//
//	assert(id);
//	assert(grid);
//
//	// draw the named png file into the grid cell location
//
//	SetPNGMode(id);
//
//	if (grid->gridcells[r][vocol].height == MGL_DEFAULT){
//		cellheight = MGL_GRID_DEF_ROW_HEIGHT;
//	}
//	else {
//		cellheight = grid->gridrows[r].height;
//	}
//	if (grid->gridcells[r][vocol].width == MGL_DEFAULT){
//		cellwidth = grid->gridcols[vocol].width;
//	}
//	else {
//		cellwidth = grid->gridcells[r][vocol].width;
//	}
//	image_rect.x1 = grid->gridcells[r][vocol].cellrect.x1;
//	image_rect.y1 = grid->gridcells[r][vocol].cellrect.y1;
//	image_rect.x2 = image_rect.x1 + cellwidth;
//	image_rect.y2 = image_rect.y1 + cellheight;
//
//	// center image in middle of column
//	shrink = (cellwidth - grid->gridcols[vocol].img_displaysize) / 2;
//	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
//	image_rect.x1 += shrink;
//	image_rect.x2 -= shrink;
//
//
//	shrink = (cellheight - grid->gridcols[vocol].img_displaysize) / 2;
//	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
//	image_rect.y1 += shrink;
//	image_rect.y2 -= shrink;
//
//
//	y1 = (((double)image_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
//		+ id->m_sCurLayout.m_dWorldYmin;
//	// draw above scrollbar , if one
//	x1 = (((double)image_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
//		+ id->m_sCurLayout.m_dWorldXmin;
//	y2 = (((double)image_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
//		+ id->m_sCurLayout.m_dWorldYmin;
//	x2 = (((double)image_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
//		+ id->m_sCurLayout.m_dWorldXmin;
//
//	DisplayCachedPNG(id, grid->gridcols[vocol].pngfilename, grid->gridcols[vocol].texturesize,
//		(float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1), GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_BLACK);
//
//	glFlush();
//	glPopMatrix();  // pop out push from SetPNGMode
//	glColor3fv(white);
//
//}


//mtm ** to change the checkbox_yes.png and checkbox_no.png accordinlgy for show column
void MGLGridDrawColRowImage_show(struct InstanceData* id, MGLGRID *grid, int vocol, int r, int c)
{

	//	char *AppName;
	int row_number[100], j, active_number[100], detail_number[100];;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	double x1, x2, y1, y2;


	int shrink = 4, cellwidth, cellheight;
	MRECT image_rect;

	assert(id);
	assert(grid);

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	

	for (j = 0; j < 100; j++)
		detail_number[j] = 200;
	//to mark the "detail column" that has been selected
	for (j = 0; j < 100; j++)
	{
		if (DataAppGrid->row_checkbox[j] == 20)
		{
			if (j < DataAppGrid->iVscrollPos)
			{
			}
			else
				detail_number[j - DataAppGrid->iVscrollPos] = j - DataAppGrid->iVscrollPos;

		}
	}

	if (vocol == 8)
	{
		if (r == (detail_number[r - 1] + 1)){
			strncpy_s(DataAppGrid->gridcols[8].pngfilename, sizeof(DataAppGrid->gridcols[8].pngfilename), "zoom-arrow-combo.png", _TRUNCATE);

		}
		else
			strncpy_s(DataAppGrid->gridcols[8].pngfilename, sizeof(DataAppGrid->gridcols[8].pngfilename), "zoom-arrow-off.png", _TRUNCATE);
	}

	for (j = 0; j < 100; j++)
		row_number[j] = 200;
	//to mark the row that has been selected of "show column"
	for (j = 0; j < 100; j++)
	{
		if (DataAppGrid->show_checkbox[j] == 20)
		{
			if (j < DataAppGrid->iVscrollPos)
			{
			}
			else
				row_number[j - DataAppGrid->iVscrollPos] = j - DataAppGrid->iVscrollPos;

		}
	}

	if (vocol == 6)
	{
		if (r == (row_number[r - 1] + 1)){
			strncpy_s(DataAppGrid->gridcols[6].pngfilename, sizeof(DataAppGrid->gridcols[6].pngfilename), "circle-on.png", _TRUNCATE);

		}
		else
			strncpy_s(DataAppGrid->gridcols[6].pngfilename, sizeof(DataAppGrid->gridcols[6].pngfilename), "show-circle-off.png", _TRUNCATE);
	}

	//to mark the row that has been selected of "active column"
	for (j = 0; j < 100; j++)
	{
		if (DataAppGrid->active_checkbox[j] == 20)
		{
			if (j < DataAppGrid->iVscrollPos)
			{
			}
			else
				active_number[j - DataAppGrid->iVscrollPos] = j - DataAppGrid->iVscrollPos;

		}
	}

	if (vocol == 5)
	{
		if (r == (active_number[r - 1] + 1)){
			strncpy_s(DataAppGrid->gridcols[5].pngfilename, sizeof(DataAppGrid->gridcols[5].pngfilename), "circle-on.png", _TRUNCATE);

		}
		else
			strncpy_s(DataAppGrid->gridcols[5].pngfilename, sizeof(DataAppGrid->gridcols[5].pngfilename), "show-circle-off.png", _TRUNCATE);
	}

	//Actions control grid
	if (strcmp(grid->name, "AlertsColorGrid") == 0){
		
			if (grid->row_checkbox[r - 1] == 1)
			{
				strncpy_s(grid->gridcols[2].pngfilename, sizeof(grid->gridcols[2].pngfilename), "zoom-arrow-combo.png", _TRUNCATE);
			}
			else if (grid->row_checkbox[r - 1] == 0)
				strncpy_s(grid->gridcols[2].pngfilename, sizeof(grid->gridcols[2].pngfilename), "zoom-arrow-off.png", _TRUNCATE);
	
	}

	//Actions control grid
	if (strcmp(grid->name, "AlertsColorGrid") == 0){

		if (grid->show_checkbox[r-1] == 1)
		{
			strncpy_s(grid->gridcols[1].pngfilename, sizeof(grid->gridcols[1].pngfilename), "circle-on.png", _TRUNCATE);
		}
		else if (grid->show_checkbox[r-1] == 0)
			strncpy_s(grid->gridcols[1].pngfilename, sizeof(grid->gridcols[1].pngfilename), "show-circle-off.png", _TRUNCATE);

	}



	SetPNGMode(id);

	if (grid->gridcells[r][vocol].height == MGL_DEFAULT){
		cellheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		cellheight = grid->gridrows[r].height;
	}
	if (grid->gridcells[r][vocol].width == MGL_DEFAULT){
		cellwidth = grid->gridcols[vocol].width;
	}
	else {
		cellwidth = grid->gridcells[r][vocol].width;
	}
	image_rect.x1 = grid->gridcells[r][vocol].cellrect.x1;
	image_rect.y1 = grid->gridcells[r][vocol].cellrect.y1;
	image_rect.x2 = image_rect.x1 + cellwidth;
	image_rect.y2 = image_rect.y1 + cellheight;

	// center image in middle of column
	shrink = (cellwidth - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.x1 += shrink;
	image_rect.x2 -= shrink;


	shrink = (cellheight - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.y1 += shrink;
	image_rect.y2 -= shrink;


	y1 = (((double)image_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)image_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)image_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)image_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

	DisplayCachedPNG(id, grid->gridcols[vocol].pngfilename, grid->gridcols[vocol].texturesize,
		(float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1), GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_BLACK,0);

	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);
	id->DataAppGrid = DataAppGrid;

}



//mtm for flight icons
void MGLGridDrawFlightIcons(struct InstanceData* id, MGLGRID *grid, int vocol, int r, int c)
{


		extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	double x1, x2, y1, y2;


	int shrink = 4, cellwidth, cellheight;
	MRECT image_rect;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	assert(id);
	assert(grid);
	if (vocol == 3){
		if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
		{
			if (SecondTabGrid->childgrid->icon_set == 1)
			{
				if (r == 1){


					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 0)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "0.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 1)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "1.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 2)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "2.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 3)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "3.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 4)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "4.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 5)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "5.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 6)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "6.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 7)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "7.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 8)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "8.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 9)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "9.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 10)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "10.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 11)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "11.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 12)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "12.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 13)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "13.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 14)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "14.png", _TRUNCATE);
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->icon_color == 15)
						strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "15.png", _TRUNCATE);
				}
				else
					strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "new.png", _TRUNCATE);
			}
			else
				strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "new.png", _TRUNCATE);


		}
		else
			strncpy_s(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename, sizeof(SecondTabGrid->childgrid->BuildSummaryGrid->gridcols[3].pngfilename), "new.png", _TRUNCATE);
	}
	SetPNGMode(id);


	if (grid->gridcells[r][vocol].height == MGL_DEFAULT){
		cellheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		cellheight = grid->gridrows[r].height;
	}
	if (grid->gridcells[r][vocol].width == MGL_DEFAULT){
		cellwidth = grid->gridcols[vocol].width;
	}
	else {
		cellwidth = grid->gridcells[r][vocol].width;
	}
	image_rect.x1 = grid->gridcells[r][vocol].cellrect.x1;
	image_rect.y1 = grid->gridcells[r][vocol].cellrect.y1;
	image_rect.x2 = image_rect.x1 + cellwidth;
	image_rect.y2 = image_rect.y1 + cellheight;

	// center image in middle of column
	shrink = (cellwidth - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.x1 += shrink;
	image_rect.x2 -= shrink;


	shrink = (cellheight - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.y1 += shrink;
	image_rect.y2 -= shrink;


	y1 = (((double)image_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)image_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)image_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)image_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;


	DisplayCachedPNG(id, grid->gridcols[3].pngfilename, grid->gridcols[3].texturesize,
		(float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1), GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_BLACK, 0);

	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);



}
//mtm ** to change the checkbox_yes.png and checkbox_no.png accordinlgy for build column
void MGLGridDrawColRowImage(struct InstanceData* id, MGLGRID *grid, int vocol, int r, int c)
{

	
	
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode,int rotate);
	double x1, x2, y1, y2;


	int shrink = 4, cellwidth, cellheight;
	MRECT image_rect;

	assert(id);
	assert(grid);

	

	SetPNGMode(id);

	if (grid->gridcells[r][vocol].height == MGL_DEFAULT){
		cellheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		cellheight = grid->gridrows[r].height;
	}
	if (grid->gridcells[r][vocol].width == MGL_DEFAULT){
		cellwidth = grid->gridcols[vocol].width;
	}
	else {
		cellwidth = grid->gridcells[r][vocol].width;
	}
	image_rect.x1 = grid->gridcells[r][vocol].cellrect.x1;
	image_rect.y1 = grid->gridcells[r][vocol].cellrect.y1;
	image_rect.x2 = image_rect.x1 + cellwidth;
	image_rect.y2 = image_rect.y1 + cellheight;

	// center image in middle of column
	shrink = (cellwidth - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.x1 += shrink;
	image_rect.x2 -= shrink;


	shrink = (cellheight - grid->gridcols[vocol].img_displaysize) / 2;
	if (shrink < 2) shrink = 2; // minimal margin of 2 pixels
	image_rect.y1 += shrink;
	image_rect.y2 -= shrink;


	y1 = (((double)image_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)image_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)image_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)image_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

	DisplayCachedPNG(id, grid->gridcols[vocol].pngfilename, grid->gridcols[vocol].texturesize,
		(float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1), GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_BLACK,0);

	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);




}




void MGLGridDrawCellsText(struct InstanceData* id, MGLGRID *grid, int vocol, int r, int c)
{
	int x1, y1;
	VO *vo, *vcol;
	int vorow; // used to convert visible rows and cols to vo struct
	char *colname, *valstr, *display_fmt;
	int fgred, fggreen, fgblue, fgint;
	GLfloat fgcolor[3];
	GLfloat *fg;


	assert(id);
	assert(grid);

	if (grid->filteredvo){
		vo = grid->filteredvo;
	}
	else if (grid->vo){
		vo = grid->vo;
	}
	else {
		vo = NULL;
	}
	if (vo){
		// use data from VO object

		colname = grid->ColOrderNames[vocol];

		if (grid->gridrows[r].fg_callback){
			// row has a callback for fg color
			fg = (GLfloat *)grid->gridrows[r].fg_callback(grid, r);
			glColor3fv(fg);
		}
		else if (grid->gridcells[r][vocol].fg){
			glColor3fv(grid->gridcells[r][vocol].fg);
		}
		else if (grid->gridrows[r].fg){
			glColor3fv(grid->gridrows[r].fg);
		}
		else if (grid->fg){
			glColor3fv(grid->fg);
		}

		if (r > 0){
			// check row fg color actions
			vorow = r - 1 + grid->iVscrollPos;
			if (grid->filteredvo){
				vo = grid->filteredvo;
			}
			else {
				vo = grid->vo;
			}
			if (vo && vo->count && vorow < vo->count){
				if ((fgint = vo->row_index[vorow].fg) >= 0){
					// convert integer value to rgb
					fgred = (fgint & 0xff0000) >> 16;
					fggreen = (fgint & 0xff00) >> 8;
					fgblue = (fgint & 0xff);
					fgcolor[0] = (GLfloat)fgred / 255.0f;
					fgcolor[1] = (GLfloat)fggreen / 255.0f;
					fgcolor[2] = (GLfloat)fgblue / 255.0f;
					glColor3fv(fgcolor);
				}
			}
		}

		x1 = grid->gridcells[r][vocol].cellrect.x1; // + grid->marginx;  x margin already added in 
		y1 = grid->gridcells[r][vocol].cellrect.y1 + grid->marginy;
		glRasterPos2i(x1, y1);

		if (r == 0){
			// title row
			if (strlen((valstr = grid->ColLabels[vocol]))){
				//id->OuterGLPrint(valstr);
				glPrintCustom(id, id->m_gluCustomBase, valstr);
			}
			else {
			//	id->OuterGLPrint(colname);
				glPrintCustom(id, id->m_gluCustomBase, colname);
			}
		}
		else {

			vorow = r - 1 + grid->iVscrollPos;

			if (vorow < vo->count && vorow >= 0 && vocol < vo->vcols->count){
				// vcol = V_ROW_PTR( vo->vcols, c );
				// grid->
				if (strlen(colname)){
					vcol = vcol_find(vo, colname);
				}
				else {
					vcol = V_ROW_PTR(vo->vcols, vocol);
				}
				if (!vcol){
					vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "bad null vcol for colname '%s', vo '%s'\n", colname, vo->name);
				}
				else {
					display_fmt = grid->gridcols[vocol].display_fmt;
					if (strlen(display_fmt)){
						if (!vcol_find(vcol, "fmtstr")){
							VOPropertyAdd(vcol, "fmtstr", NTBSTRINGBIND, 32);
							strcpy_s(VP(vcol, 0, "fmtstr", char), 32, display_fmt);
						}
					}
					valstr = get_vcol_valstr(vcol, V_ROW_PTR(vo, vorow), TRUE);
					if (grid->gridcols[vocol].val_callback){
						valstr = (char *)grid->gridcols[vocol].val_callback(id,valstr);
					}
					//id->OuterGLPrint("%s", valstr);
					glPrintCustom(id, id->m_gluCustomBase, valstr);
				}
			}
		}
	}
	else {
		// use data stored in grid structure
		if (grid->gridcells[r][c].val){
			if (grid->gridcells[r][c].fg){
				glColor3fv(grid->gridcells[r][c].fg);
			}
			else if (grid->gridrows[r].fg){
				glColor3fv(grid->gridrows[r].fg);
			}
			else {
				glColor3fv(grid->fg);
			}
			x1 = grid->gridcells[r][c].cellrect.x1; // + grid->marginx;
			y1 = grid->gridcells[r][c].cellrect.y1; // + grid->marginy;
			glRasterPos2i(x1, y1);
			if (strlen(grid->gridcells[r][c].val)){
				id->OuterGLPrint(grid->gridcells[r][c].val);
			}
		}
	}

}


int	MGLGridDrawCells(struct InstanceData* id, MGLGRID *grid)
{
	int x1, y1, x2, y2, r, c, cellheight, cellwidth, tt;
	int test;
	int vocol; // used to convert visible rows and cols to vo struct
	//	int me;
	assert(id);
	assert(grid);

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	if (SecondTabGrid)
	{
		if (SecondTabGrid->childgrid)
		{
			if (SecondTabGrid->childgrid->SortAvailableGrid)
			{
				if (strcmp(grid->name, "AvailableGrid") == 0)
					tt = 0;
			}
		}
	}

	for (r = 0; r < grid->nrows; r++){
		for (c = 0; c < grid->ncols; c++){
			/* draw backgound of entire cell, if different
			than default values */

			vocol = grid->DisplayCol[c];  // map from display column number to vo value
			if (vocol < 0){
				// invisible col
				continue;
			}

			if (grid->gridcells[r][c].bg){
				x1 = grid->gridcells[r][vocol].cellrect.x1;
				y1 = grid->gridcells[r][vocol].cellrect.y1;

				if (grid->gridcells[r][vocol].height == MGL_DEFAULT){
					cellheight = MGL_GRID_DEF_ROW_HEIGHT;
				}
				else {
					cellheight = grid->gridrows[r].height;
				}
				if (grid->gridcells[r][vocol].width == MGL_DEFAULT){
					cellwidth = grid->gridcols[vocol].width;
				}
				else {
					cellwidth = grid->gridcells[r][vocol].width;
				}
				x2 = x1 + cellwidth;
				y2 = y1 + cellheight;

				glColor3fv(grid->gridcells[r][vocol].bg);
				glRecti(x1, y1, x2, y2);
			}
			/* draw text, if any */
			if (SecondTabGrid){
				if (SecondTabGrid->childgrid){
					if (strcmp(grid->name, SecondTabGrid->childgrid->BuildSummaryGrid->name) == 0)
					{
						if (strlen(grid->gridcols[vocol].pngfilename) && r > 0)
						{
							MGLGridDrawFlightIcons(id, grid, vocol, r, c);
						}
					}
				}
			}
			if (strlen(grid->gridcols[vocol].pngfilename) && r > 0){
				if (strcmp(grid->name, "DataAppSelectorGrid") == 0)
				{
					test = 100;
				}
				MGLGridDrawColRowImage_show(id, grid, vocol, r, c);
				if (DataAppGrid->callback == 20){//added mtm to bring about the checkbox changes for each row **checkbox_yes.png/checkbox_no.png
					MGLGridDrawColRowImage(id, grid, vocol, r, c);

				}

				else
					MGLGridDrawColImage(id, grid, vocol, r, c);
				SetGridGLMode(id);
				glTranslatef(trans[0], trans[1], trans[2]);
			}
			else {
				MGLGridDrawCellsText(id, grid, vocol, r, c);
			}


		}
	}
	id->DataAppGrid = DataAppGrid;
	return(TRUE);
}



int MGLGridRowColorBG(MGLGRID *grid, int row, GLfloat *bg)
{

	grid->gridrows[row].bg = bg;
	return(TRUE);
}



MoveElevator(widgets *w, int x, int y)
{
	double DRowsInGrid;
	VO *vo;

	assert(w);

	if (w->grid->filteredvo){
		vo = w->grid->filteredvo;
	}
	else if (w->grid->vo){
		vo = w->grid->vo;
	}
	else {
		vo = NULL;
	}
	if (vo){
		DRowsInGrid = (double)vo->count;
	}
	else {
		DRowsInGrid = (double)w->grid->nrows;
	}

	w->grid->iVscrollPos = (int)my_rint(((double)(w->grid->scrolltop_rect.y1 - y)) /
		(double)(w->grid->scrolltop_rect.y1 - w->grid->scrollbot_rect.y2) * DRowsInGrid);
	if (w->grid->iVscrollPos < 0)
		w->grid->iVscrollPos = 0;
	if (w->grid->iVscrollPos >= DRowsInGrid - w->grid->nrows - 1)
		w->grid->iVscrollPos = (int)DRowsInGrid - (w->grid->nrows - 1);

}



void MGLTitleDrawMinMaxButtons(struct InstanceData* id, MGLGRID *grid)
{
	char pngfilename[64];
	// double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode,int rotate);
	extern void DrawCachedPNG2(struct InstanceData* id, char *imagename, MRECT *ploc, int alpha_mode, int blendflag, int ChangeColorAlpha, int filtertype, GLfloat angle);

	assert(id);
	assert(grid);

	SetPNGMode(id);
	sprintf_s(pngfilename, _countof(pngfilename), "min-window-icon.png");

	if (grid->AllowScrollbar && grid->NeedsScrollbar && !grid->IsMinimized){
		grid->minimize_rect.x1 = grid->xclose_rect.x1 - 32;
	}
	else {
		grid->minimize_rect.x1 = grid->xclose_rect.x1 - 32;
	}


	grid->minimize_rect.y1 = grid->title_rect.y1 + 3;
	grid->minimize_rect.x2 = grid->minimize_rect.x1 + 12; // image is 11x11
	grid->minimize_rect.y2 = grid->minimize_rect.y1 + 12;

	/*
	y1 = ( ( (double)  grid->minimize_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
	+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = ( ( (double) grid->minimize_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
	+ id->m_sCurLayout.m_dWorldXmin;
	y2 = ( ( (double) grid->minimize_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
	+ id->m_sCurLayout.m_dWorldYmin;
	x2 =  ( ( (double) grid->minimize_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
	+ id->m_sCurLayout.m_dWorldXmin;
	*/

	DrawCachedPNG2(id, pngfilename, &grid->minimize_rect, ALPHA_MODE_BLACK, 1, 0, GL_LINEAR, 0.0f);

	// minimize next to maximize
	sprintf_s(pngfilename, _countof(pngfilename), "max-window-icon.png");

	grid->maximize_rect.x1 = grid->minimize_rect.x2 + 5;
	grid->maximize_rect.y1 = grid->title_rect.y1 + 3;
	grid->maximize_rect.x2 = grid->maximize_rect.x1 + 12; // image is 8 x 2
	grid->maximize_rect.y2 = grid->maximize_rect.y1 + 9;

	/*
	y1 = ( ( (double)  grid->maximize_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
	+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = ( ( (double) grid->maximize_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
	+ id->m_sCurLayout.m_dWorldXmin;
	y2 = ( ( (double) grid->maximize_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
	+ id->m_sCurLayout.m_dWorldYmin;
	x2 =  ( ( (double) grid->maximize_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
	+ id->m_sCurLayout.m_dWorldXmin;
	*/

	DrawCachedPNG2(id, pngfilename, &grid->maximize_rect, ALPHA_MODE_BLACK, 1, 0, GL_LINEAR, 0.0f);



	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);
}


///mtm -edit button
void MGLTitleDrawEdit(struct InstanceData* id, MGLGRID *grid)
{
	char pngfilename[64];
	// double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode,int rotate);
	extern void DrawCachedPNG2(struct InstanceData* id, char *imagename, MRECT *ploc, int alpha_mode, int blendflag, int ChangeColorAlpha, int filtertype, GLfloat angle);

	assert(id);
	assert(grid);

	SetPNGMode(id);
	sprintf_s(pngfilename, _countof(pngfilename), "edit_new.png");

	
	grid->rules.x1 = grid->xclose_rect.x1 - 200;
	grid->rules.y1 = grid->title_rect.y1 + 3;
	grid->rules.x2 = grid->rules.x1 + 30; // image is 11x11
	grid->rules.y2 = grid->rules.y1 + 12;



	DrawCachedPNG2(id, pngfilename, &grid->rules, ALPHA_MODE_BLACK, 1, 0, GL_LINEAR, 0.0f);



	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);
}

///< button
void MGLTitleDrawCalPrevNextButtons(struct InstanceData* id, MGLGRID *grid)
{
	char pngfilename[64];
	// double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	extern void DrawCachedPNG2(struct InstanceData* id, char *imagename, MRECT *ploc, int alpha_mode, int blendflag, int ChangeColorAlpha, int filtertype, GLfloat angle);

	assert(id);
	assert(grid);

	SetPNGMode(id);
	sprintf_s(pngfilename, _countof(pngfilename), "prev-arrow.png");

	//this is for the prev arrow '<'
	grid->cal_prev.x1 = grid->xclose_rect.x1 - 83;
	grid->cal_prev.y1 = grid->title_rect.y1 + 3;
	grid->cal_prev.x2 = grid->cal_prev.x1 + 12; // image is 11x11
	grid->cal_prev.y2 = grid->cal_prev.y1 + 12;

	DrawCachedPNG2(id, pngfilename, &grid->cal_prev, ALPHA_MODE_BLACK, 1, 0, GL_LINEAR, 0.0f);

	// next arrow after '<'
	sprintf_s(pngfilename, _countof(pngfilename), "next-arrow.png");

	grid->cal_next.x1 = grid->cal_prev.x2 + 5;
	grid->cal_next.y1 = grid->title_rect.y1 + 3;
	grid->cal_next.x2 = grid->cal_next.x1 + 12; // image is 11 x 11
	grid->cal_next.y2 = grid->cal_next.y1 + 12;


	DrawCachedPNG2(id, pngfilename, &grid->cal_next, ALPHA_MODE_BLACK, 1, 0, GL_LINEAR, 0.0f);

	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);
}
//mtm--when horizontal scrollbar is to the right of the grid
void MGLTitleDrawX_scrollbarright(struct InstanceData* id, MGLGRID *grid)
{
	char pngfilename[64];
	double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	int shrink = 4;

	assert(id);
	assert(grid);

	SetPNGMode(id);
	// sprintf_s(pngfilename, _countof(pngfilename), "redx32.png" );
	sprintf_s(pngfilename, _countof(pngfilename), "window-X.png");

	if (grid->AllowScrollbar && grid->NeedsScrollbar && !grid->IsMinimized){
		grid->xclose_rect.x1 = MGLGridXMax(grid) - grid->GScrollWidth;
	}
	else {
		grid->xclose_rect.x1 = MGLGridXMax(grid) - grid->GScrollWidth;
	}


	grid->xclose_rect.y1 = grid->title_rect.y1;
	grid->xclose_rect.x2 = MGLGridXMax(grid);
	grid->xclose_rect.y2 = grid->title_rect.y2;

	// shrink x by a little on all four points
	grid->xclose_rect.x1 += shrink;
	grid->xclose_rect.x2 -= shrink;
	grid->xclose_rect.y1 += shrink;
	grid->xclose_rect.y2 -= shrink;

	// drop y values a little
	grid->xclose_rect.y1 -= 1;
	grid->xclose_rect.y2 -= 1;


	y1 = (((double)grid->xclose_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)grid->xclose_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->xclose_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->xclose_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

	DisplayCachedPNG(id, pngfilename, 11, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_BLACK, 0);

	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);
}



void MGLTitleDrawX(struct InstanceData* id, MGLGRID *grid)
{
	char pngfilename[64];
	double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	int shrink = 4;

	assert(id);
	assert(grid);

	SetPNGMode(id);
	// sprintf_s(pngfilename, _countof(pngfilename), "redx32.png" );
	sprintf_s(pngfilename, _countof(pngfilename), "window-X.png");

	if (grid->AllowScrollbar && grid->NeedsScrollbar && !grid->IsMinimized){
		grid->xclose_rect.x1 = grid->scrollbar_rect.x1;
	}
	else {
		grid->xclose_rect.x1 = MGLGridXMax(grid) - grid->GScrollWidth;
	}


	grid->xclose_rect.y1 = grid->title_rect.y1;
	grid->xclose_rect.x2 = grid->title_rect.x2;
	grid->xclose_rect.y2 = grid->title_rect.y2;

	// shrink x by a little on all four points
	grid->xclose_rect.x1 += shrink;
	grid->xclose_rect.x2 -= shrink;
	grid->xclose_rect.y1 += shrink;
	grid->xclose_rect.y2 -= shrink;

	// drop y values a little
	grid->xclose_rect.y1 -= 1;
	grid->xclose_rect.y2 -= 1;


	y1 = (((double)grid->xclose_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)grid->xclose_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->xclose_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->xclose_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

	DisplayCachedPNG(id, pngfilename, 11, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_BLACK,0);

	glFlush();
	glPopMatrix();  // pop out push from SetPNGMode
	glColor3fv(white);
}





void MGLDrawButton(struct InstanceData* id, MGLGRID *grid, widgets *w)
{
	extern void DrawCachedPNG(struct InstanceData* id, char *imagename, int gmap_texture_pixels, MRECT *wrld, MRECT *ploc);
	double x1, y1, x2, y2;

	assert(id);
	assert(grid);

	x1 = grid->title_rect.x1 + w->gregion.x1;
	y1 = grid->title_rect.y1 + w->gregion.y1;
	x2 = grid->title_rect.x1 + w->gregion.x2;
	y2 = grid->title_rect.y1 + w->gregion.y2;


	y1 = (((double)y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;


	DisplayCachedPNG(id, w->active_image, w->texsize, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE,0);

}




void MGLDrawTextBaseButton(struct InstanceData* id, widgets *w)
{
	int nlines, len;
	char *membuf, *eolstr;
	char linebuf[256];

	assert(id);
	assert(w);

	SetVerdanaFontIndex(id, w->fontindex, 0);


	nlines = 0;
	membuf = w->textbuf;



	// there may be multiple lines, so we must process each line
	do {
		if ((eolstr = strchr(membuf, '\n'))){
			len = eolstr - membuf;
			if (len > 255){
				break;  // some problem with the data
			}
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = eolstr;
			membuf++; // skip over eol char
		}
		else {
			len = strlen(membuf);
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = NULL;
		}
		nlines++;
		// does not work here since we did not set hdc, so use prev values CalcTextSize( linebuf, &size );
		//  if ( size.cx > maxwidth ) maxwidth = size.cx;
		if (w->IsActive){
			if (!strcmp(w->name, "AirportText3W")){
#ifdef _FREE_TYPE_TEST
				PrintFreeType(id, w->fontindex, (float)w->gregion.x1, (float)(w->gregion.y2 - (w->textheight * nlines )), (GLfloat *) colorD6B859, linebuf ); // SubNavGold
#endif
			}
			else{
				DrawGLGUITextBase(id, w->gregion.x1, w->gregion.y2 - (w->textheight * nlines), linebuf, w->fontbase, (GLfloat *)colorD6B859); // SubNavGold
			}
		}
		else {
			DrawGLGUITextBase(id, w->gregion.x1, w->gregion.y2 - (w->textheight * nlines), linebuf, w->fontbase, (GLfloat *)white);
		}
	} while (membuf && *membuf != '\0');

}



void MGLDrawTextButton(struct InstanceData* id, widgets *w)
{
	int nlines, len;
	char *membuf, *eolstr;
	char linebuf[256];

	assert(id);
	assert(w);

	if (!strcmp(w->name, "AirportText2W") || !strcmp(w->name, "AirportText3W")){
		MGLDrawTextBaseButton(id, w);
		return;
	}

	// not needed since opengl just needs base image number unless we are measuring sizes of text ------ SetGUIFontIndex( id, w->fontindex, 0 );
	if (!w->fontbase){
		SetGUIFontIndex(id, w->fontindex, 0);
	}



	nlines = 0;
	membuf = w->textbuf;

	// there may be multiple lines, so we must process each line
	// first time through , just calc lineheight
#ifdef oldway
	do {
		if ((eolstr = strchr(membuf, '\n'))){
			len = eolstr - membuf;
			if (len > 255){
				break;  // some problem with the data
			}
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = eolstr;
			membuf++; // skip over eol char
		}
		else {
			len = strlen(membuf);
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = NULL;
		}
		// does not work here since we did not set hdc, so use prev values CalcTextSize( linebuf, &size );
		//  if ( size.cx > maxwidth ) maxwidth = size.cx;
		nlines++;
	} while (membuf && *membuf != '\0');
#endif
	// lineheight = w->gregion.y2 - ((w->gregion.y2 - w->gregion.y1) / nlines;

	do {
		if ((eolstr = strchr(membuf, '\n'))){
			len = eolstr - membuf;
			if (len > 255){
				break;  // some problem with the data
			}
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = eolstr;
			membuf++; // skip over eol char
		}
		else {
			len = strlen(membuf);
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = NULL;
		}
		nlines++;
		// does not work here since we did not set hdc, so use prev values CalcTextSize( linebuf, &size );
		//  if ( size.cx > maxwidth ) maxwidth = size.cx;
		if (w->IsActive){
			DrawGLGUITextBase(id, w->gregion.x1, w->gregion.y2 - (w->textheight * nlines), linebuf, w->fontbase, (GLfloat *)colorD6B859); // SubNavGold
		}
		else {
			DrawGLGUITextBase(id, w->gregion.x1, w->gregion.y2 - (w->textheight * nlines), linebuf, w->fontbase, (GLfloat *)white);
		}
	} while (membuf && *membuf != '\0');

}





void MGLDrawPanelImage(struct InstanceData* id, widgets *w)
{
	double x1, y1, x2, y2;
	char *imgname;

	assert(id);
	assert(w);

	if (!strcmp(w->name, "OverlaysBarW")){
		// test
		x1 = 0.0;
	}
	if (w->type == WGT_BUTTON && w->IsActive){
		imgname = w->active_image;
	}
	else if (w->type == WGT_BUTTON){
		imgname = w->inactive_image;
	}
	else {
		imgname = w->active_image;
	}

	if (UseDisplayMode){
		// for incompatible opengl drivers -- not sure if we need to support this or if it is only a problem with VMWare users
		SetPNGMode(id);
		y1 = (((double)w->gregion.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin)) + id->m_sCurLayout.m_dWorldYmin;
		x1 = (((double)w->gregion.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin)) + id->m_sCurLayout.m_dWorldXmin;
		y2 = (((double)w->gregion.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin)) + id->m_sCurLayout.m_dWorldYmin;
		x2 = (((double)w->gregion.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin)) + id->m_sCurLayout.m_dWorldXmin;

		DisplayCachedPNG(id, imgname, 64, (float)x1, (float)x2, (float)y1, (float)y2,
			(float)(x2 - x1), (float)(y2 - y1), GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE,0);

		glFlush();
		glPopMatrix();  // pop out push from SetPNGMode
		glColor3fv(white);
	}
	else {
		SetGridGLMode(id);
		DrawCachedPNG2(id, imgname, &w->gregion, w->img_alpha_mode, w->img_blend, 0, w->filtertype, w->ImgAngle);
	}

}



void MGLDrawTitleButton(struct InstanceData* id, MGLGRID *grid, widgets *w)
{
	extern void DrawCachedPNG(struct InstanceData* id, char *imagename, int gmap_texture_pixels, MRECT *wrld, MRECT *ploc);
	double x1, y1, x2, y2;

	assert(id);
	assert(grid);
	assert(w);

	x1 = grid->title_rect.x1 + w->gregion.x1;
	y1 = grid->title_rect.y1 + w->gregion.y1;
	x2 = grid->title_rect.x1 + w->gregion.x2;
	y2 = grid->title_rect.y1 + w->gregion.y2;


	y1 = (((double)y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	// draw above scrollbar , if one
	x1 = (((double)x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;


	DisplayCachedPNG(id, w->active_image, w->texsize, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE,0);

}


void MGLDrawText(struct InstanceData* id, widgets *w)
{

	assert(id);
	assert(w);

	// coordinates are in pixels, not world
	SetGridGLMode(id);
	glTranslatef(trans[0], trans[1], trans[2]);

	if (w->wgt_text_cb){
		(w->wgt_text_cb)(id, w);
	}

	glColor3fv(yellow);
	glRasterPos2i(w->gregion.x1, w->gregion.y1);

	//id->OuterGLPrint(w->textbuf);
	glPrintCustom(id, id->m_gluCustomBase, w->textbuf);
}




void MGLTitleDrawText(struct InstanceData* id, MGLGRID *grid, widgets *w)
{
	int x1, y1, x2, y2;

	assert(id);
	assert(grid);
	assert(w);

	// coordinates are in pixels, not world
	SetGridGLMode(id);
	glTranslatef(trans[0], trans[1], trans[2]);


	if (w->wgt_text_cb){
		(w->wgt_text_cb)(id, w);
	}

	x1 = grid->title_rect.x1 + w->gregion.x1;
	y1 = grid->title_rect.y1 + w->gregion.y1;
	x2 = x1 + w->gregion.x2;
	y2 = y1 + w->gregion.y2;

	glColor3fv(Text_yellow);
	glRasterPos2i(x1, y1);

	//id->OuterGLPrint(w->textbuf);
	glPrintCustom(id, id->m_gluCustomBase, w->textbuf);

}



int MGLGridDrawTitle(struct InstanceData* id, MGLGRID *grid)
{

	widgets *w;

	assert(id);
	assert(grid);

	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;

	grid->title_rect.x1 = MGLGridXMin(grid);
	grid->title_rect.y1 = MGLGridYMax(grid);
	if (grid->AllowScrollbar && grid->NeedsScrollbar && !grid->IsMinimized){
		if (strcmp(grid->name, "DataAppSelectorGrid") == 0)
		{
			grid->title_rect.x2 = MGLGridXMax(grid);
		}
		else
			grid->title_rect.x2 = MGLGridXMax(grid) + grid->GScrollWidth;
	}
	else {
		grid->title_rect.x2 = MGLGridXMax(grid);
	}
	grid->title_rect.y2 = MGLGridYMax(grid) + DataAppGrid->gridrows[0].height;

	// draw background
	glColor3fv(Blue_tab);//  grid->gridcells[r][c].bg );
	glRecti(grid->title_rect.x1, grid->title_rect.y1, grid->title_rect.x2, grid->title_rect.y2);

	if ((strcmp(grid->name, "TitleGridGrid") == 0) || (strcmp(grid->name, "BuildSummaryGrid") == 0) || (strcmp(grid->name, "SecondTabGrid") == 0))
	{
		glLineWidth(0.5);
		glColor3f(0.6274509f, 0.6274509f, 0.6274509f);
		glBegin(GL_LINES);
		glVertex2f((GLfloat)grid->title_rect.x1, (GLfloat)grid->title_rect.y1);
		glVertex2f((GLfloat)grid->title_rect.x1, (GLfloat)grid->title_rect.y2);
		glEnd();
	}


	if (strcmp(grid->name, "CalendarGrid") == 0)
	{

		MGLTitleDrawCalPrevNextButtons(id, grid);

	}

	if ((!strcmp(grid->name, "DataAppSelectorGrid") == 0) && (!strcmp(grid->name, "SecondTabGrid") == 0) && (!strcmp(grid->name, "ActionsGrid") == 0) && (!strcmp(grid->name, "ParametersGrid") == 0) && (!strcmp(grid->name, "AvailableGrid") == 0) && (!strcmp(grid->name, "AlertsColorGrid") == 0) && (!strcmp(grid->name, "RowColumn") == 0) && (!strcmp(grid->name, "BuildSummaryGrid") == 0) && (!strcmp(grid->name, "TitleGridGrid") == 0) && (!strcmp(grid->name, "ConfirmDisplay") == 0) && (!strcmp(grid->name, "CriteriaConfirmDisplay") == 0))
	{
		MGLTitleDrawMinMaxButtons(id, grid);
		MGLTitleDrawX(id, grid);

		if (grid->edit){
			MGLTitleDrawEdit(id, grid);
		}
	}
	if ((strcmp(grid->name, "SecondTabGrid") == 0) || (strcmp(grid->name, "TitleGridGrid") == 0))
	
		{
			MGLTitleDrawX(id, grid);
		}

		if (strcmp(grid->name, "TitleGridGrid") == 0)
		{
			MGLTitleDrawEdit(id, grid);
		}
		if (strcmp(grid->name, "DataAppSelectorGrid") == 0)
		{
			MGLTitleDrawMinMaxButtons(id, grid);
			MGLTitleDrawX_scrollbarright(id, grid); //mtm--The scrollbar is to the right of the grid. Few changes related to the position of the 'x'
		}


		for (w = grid->title_widgets; w; w = w->np){
			switch (w->type)
			{
			case WGT_BUTTON:
				MGLDrawTitleButton(id, grid, w);
				break;
			case WGT_TEXT:
				MGLTitleDrawText(id, grid, w);
				break;
			default:
				break;
			}
		}
	
	return(TRUE);
}




int MGLGridDrawScroll(struct InstanceData* id, MGLGRID *grid)

{
	int rowheight, ypixels, ysize, ypos;
	char pngfilename[256];
	double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	VO *vo;

	assert(id);
	assert(grid);

	// scrollbar for grid is composed of 4 rectangles, background panel, elevator , and up and down arrows rects for arrow png's

	if (grid->vo && grid->filteredvo){
		vo = grid->filteredvo;
	}
	else if (grid->vo){
		vo = grid->vo;
	}
	else {
		vo = NULL;
	}
	if (vo && grid->nrows > vo->count){
		// no need for a scrollbar
		grid->NeedsScrollbar = FALSE;
		return(FALSE);
	}
	else {
		grid->NeedsScrollbar = TRUE;
	}


	grid->scrollbar_rect.x1 = MGLGridXMax(grid);
	grid->scrollbar_rect.y1 = MGLGridYMin(grid);
	grid->scrollbar_rect.x2 = grid->scrollbar_rect.x1 + grid->GScrollWidth;
	grid->scrollbar_rect.y2 = MGLGridYMax(grid);

	// draw background
	glColor3fv(grey);//  grid->gridcells[r][c].bg );
	glRecti(grid->scrollbar_rect.x1, grid->scrollbar_rect.y1, grid->scrollbar_rect.x2, grid->scrollbar_rect.y2);

	// top arrow
	grid->scrolltop_rect.x1 = MGLGridXMax(grid);
	grid->scrolltop_rect.x2 = grid->scrolltop_rect.x1 + grid->GScrollWidth;
	grid->scrolltop_rect.y1 = grid->gridrows[0].rowrect.y1;
	if (grid->gridrows[0].height == MGL_DEFAULT){
		rowheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		rowheight = grid->gridrows[0].height;
	}
	grid->scrolltop_rect.y2 = grid->scrolltop_rect.y1 + rowheight;

	if (grid->gridrows[0].texture)
	{
		MGLGridTexQuad(grid->gridrows[0].texture, grid->scrolltop_rect.x1, grid->scrolltop_rect.y1, grid->scrolltop_rect.x2, grid->scrolltop_rect.y2,
			grid->gridrows[0].bg);
	}
	else
	{
		glColor3fv(grey);
		// glRecti(grid->scrolltop_rect.x1, grid->scrolltop_rect.y1, grid->scrolltop_rect.x2, grid->scrolltop_rect.y2 );		
	}


	// bottom arrow area
	grid->scrollbot_rect.x1 = grid->scrolltop_rect.x1;
	grid->scrollbot_rect.x2 = grid->scrolltop_rect.x2;

	grid->scrollbot_rect.y1 = grid->gridrows[grid->nrows - 1].rowrect.y1;
	grid->scrollbot_rect.y2 = grid->scrollbot_rect.y1 + rowheight;

	glColor3fv(grey);
	// glRecti(grid->scrollbot_rect.x1, grid->scrollbot_rect.y1, grid->scrollbot_rect.x2, grid->scrollbot_rect.y2 );

	// elevator box
	// y values already set in Init routine, or scrollbar move routines

	// position elevator based on scroll position and size

	if (vo && vo->count){
		ypixels = grid->scrolltop_rect.y1 - grid->scrollbot_rect.y2;
		ysize = (int)((double)(grid->nrows - 1) / vo->count * (double)ypixels); // subtract 2 for title and bot arrow areas
		if (ysize < MIN_ELEVATOR_HEIGHT) ysize = MIN_ELEVATOR_HEIGHT;

		ypos = (int)((double)grid->iVscrollPos / vo->count * ypixels);
		ypos = grid->scrolltop_rect.y1 - ypos;

		grid->elev_rect.y2 = ypos;
		grid->elev_rect.y1 = ypos - ysize;

		if (grid->elev_rect.y2 > grid->scrolltop_rect.y1){
			grid->elev_rect.y2 = grid->scrolltop_rect.y1;
			grid->elev_rect.y1 = grid->elev_rect.y2 - ysize;
		}
	}
	grid->elev_rect.x1 = grid->scrollbar_rect.x1;
	grid->elev_rect.x2 = grid->scrollbar_rect.x2;
	glColor3fv(gridgreyelev);

	glRecti(grid->elev_rect.x1, grid->elev_rect.y1, grid->elev_rect.x2, grid->elev_rect.y2);

	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "brownsquare.png");


	y1 = (((double)grid->scrollbot_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrollbot_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrollbot_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrollbot_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

#ifdef OLDWAY
	y1 = grid->scrollbot_rect.y1;
	x1 = grid->scrollbot_rect.x1;
	y2 = grid->scrollbot_rect.y2;
	x2 = grid->scrollbot_rect.x2;
#endif



	SetPNGMode(id);


	//  glColor3fv(darkpurple);

	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "uparrow.png");


	y1 = (((double)grid->scrolltop_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrolltop_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrolltop_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrolltop_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	DisplayCachedPNG(id, pngfilename, 32, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE,0);


	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "downarrow.png");
	y1 = (((double)grid->scrollbot_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrollbot_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrollbot_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrollbot_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	DisplayCachedPNG(id, pngfilename, 32, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE,0);


	glFlush();
	glPopMatrix();

	glColor3fv(white);


	return(TRUE);
}

//mtm
int MGLGridDrawScroll_condition(struct InstanceData* id, MGLGRID *grid)

{
	
	int rowheight, ypixels, ysize, ypos;
	char pngfilename[256];
	double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	VO *vo;

	assert(id);
	assert(grid);

	// scrollbar for grid is composed of 4 rectangles, background panel, elevator , and up and down arrows rects for arrow png's

	if (grid->vo && grid->filteredvo){
		vo = grid->filteredvo;
	}
	else if (grid->vo){
		vo = grid->vo;
	}
	else {
		vo = NULL;
	}
	if (vo && grid->nrows > vo->count){
		// no need for a scrollbar
		grid->NeedsScrollbar = FALSE;
		return(FALSE);
	}
	else {
		grid->NeedsScrollbar = TRUE;
	}


	grid->scrollbar_rect.x1 = MGLGridXMin(grid) - grid->GScrollWidth;
	grid->scrollbar_rect.y1 = MGLGridYMin(grid);
	grid->scrollbar_rect.x2 = grid->scrollbar_rect.x1 + grid->GScrollWidth;
	grid->scrollbar_rect.y2 = MGLGridYMax(grid);

	// draw background
	glColor3fv(grey);//  grid->gridcells[r][c].bg );
	glRecti(grid->scrollbar_rect.x1, grid->scrollbar_rect.y1, grid->scrollbar_rect.x2, grid->scrollbar_rect.y2);

	// top arrow
	grid->scrolltop_rect.x1 = MGLGridXMin(grid) - grid->GScrollWidth;
	grid->scrolltop_rect.x2 = grid->scrolltop_rect.x1 + grid->GScrollWidth;
	grid->scrolltop_rect.y1 = grid->gridrows[0].rowrect.y1;
	if (grid->gridrows[0].height == MGL_DEFAULT){
		rowheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		rowheight = grid->gridrows[0].height;
	}
	grid->scrolltop_rect.y2 = grid->scrolltop_rect.y1 + rowheight;

	if (grid->gridrows[0].texture)
	{
		MGLGridTexQuad(grid->gridrows[0].texture, grid->scrolltop_rect.x1, grid->scrolltop_rect.y1, grid->scrolltop_rect.x2, grid->scrolltop_rect.y2,
			grid->gridrows[0].bg);
	}
	else
	{
		glColor3fv(grey);
		// glRecti(grid->scrolltop_rect.x1, grid->scrolltop_rect.y1, grid->scrolltop_rect.x2, grid->scrolltop_rect.y2 );		
	}


	// bottom arrow area
	grid->scrollbot_rect.x1 = grid->scrolltop_rect.x1;
	grid->scrollbot_rect.x2 = grid->scrolltop_rect.x2;

	grid->scrollbot_rect.y1 = grid->gridrows[grid->nrows - 1].rowrect.y1;
	grid->scrollbot_rect.y2 = grid->scrollbot_rect.y1 + rowheight;

	glColor3fv(grey);
	// glRecti(grid->scrollbot_rect.x1, grid->scrollbot_rect.y1, grid->scrollbot_rect.x2, grid->scrollbot_rect.y2 );

	// elevator box
	// y values already set in Init routine, or scrollbar move routines

	// position elevator based on scroll position and size

	if (vo && vo->count){
		ypixels = grid->scrolltop_rect.y1 - grid->scrollbot_rect.y2;
		ysize = (int)((double)(grid->nrows - 1) / vo->count * (double)ypixels); // subtract 2 for title and bot arrow areas
		if (ysize < MIN_ELEVATOR_HEIGHT) ysize = MIN_ELEVATOR_HEIGHT;

		ypos = (int)((double)grid->iVscrollPos / vo->count * ypixels);
		ypos = grid->scrolltop_rect.y1 - ypos;

		grid->elev_rect.y2 = ypos;
		grid->elev_rect.y1 = ypos - ysize;

		if (grid->elev_rect.y2 > grid->scrolltop_rect.y1){
			grid->elev_rect.y2 = grid->scrolltop_rect.y1;
			grid->elev_rect.y1 = grid->elev_rect.y2 - ysize;
		}
	}
	grid->elev_rect.x1 = grid->scrollbar_rect.x1;
	grid->elev_rect.x2 = grid->scrollbar_rect.x2;
	glColor3fv(gridgreyelev);

	glRecti(grid->elev_rect.x1, grid->elev_rect.y1, grid->elev_rect.x2, grid->elev_rect.y2);

	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "brownsquare.png");


	y1 = (((double)grid->scrollbot_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrollbot_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrollbot_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrollbot_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

#ifdef OLDWAY
	y1 = grid->scrollbot_rect.y1;
	x1 = grid->scrollbot_rect.x1;
	y2 = grid->scrollbot_rect.y2;
	x2 = grid->scrollbot_rect.x2;
#endif



	SetPNGMode(id);


	//  glColor3fv(darkpurple);

	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "uparrow.png");


	y1 = (((double)grid->scrolltop_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrolltop_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrolltop_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrolltop_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	DisplayCachedPNG(id, pngfilename, 32, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE,0);


	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "downarrow.png");
	y1 = (((double)grid->scrollbot_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrollbot_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrollbot_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrollbot_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	DisplayCachedPNG(id, pngfilename, 32, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE,0);


	glFlush();
	glPopMatrix();

	glColor3fv(white);


	return(TRUE);
}


int MGLGridDrawScroll_horizontal(struct InstanceData* id, MGLGRID *grid)

{
	int rowheight, ypixels, ysize, ypos;
	char pngfilename[256];
	double x1, x2, y1, y2;
	extern int DisplayCachedPNG(struct InstanceData* id, char *filename, int texturesize, GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2, GLfloat XSize,
		GLfloat YSize, GLenum dfactor, int alpha_mode, int rotate);
	VO *vo;

	assert(id);
	assert(grid);

	// scrollbar for grid is composed of 4 rectangles, background panel, elevator , and up and down arrows rects for arrow png's

	if (grid->vo && grid->filteredvo){
		vo = grid->filteredvo;
	}
	else if (grid->vo){
		vo = grid->vo;
	}
	else {
		vo = NULL;
	}
	if (vo && grid->nrows > vo->count){
		// no need for a scrollbar
		grid->NeedsScrollbar = FALSE;
		return(FALSE);
	}
	else {
		grid->NeedsScrollbar = TRUE;
	}


	grid->scrollbar_rect.x1 = grid->gridrect.x2;
	grid->scrollbar_rect.y1 = MGLGridYMax(grid);
	grid->scrollbar_rect.x2 = grid->gridrect.x2 - MGL_GRID_DEF_ROW_HEIGHT;
	grid->scrollbar_rect.y2 = MGLGridYMax(grid) + grid->GScrollWidth;

	// draw background
	glColor3fv(grey);//  grid->gridcells[r][c].bg );
	glRecti(grid->scrollbar_rect.x1, grid->scrollbar_rect.y1, grid->scrollbar_rect.x2, grid->scrollbar_rect.y2);

	// top arrow
	grid->scrolltop_rect.x1 = grid->gridrect.x1;
	grid->scrolltop_rect.x2 = grid->gridrect.x1 + grid->GScrollWidth;
	grid->scrolltop_rect.y1 = grid->gridrows[grid->nrows - 1].rowrect.y1 - MGL_GRID_DEF_ROW_HEIGHT;
	if (grid->gridrows[0].height == MGL_DEFAULT){
		rowheight = MGL_GRID_DEF_ROW_HEIGHT;
	}
	else {
		rowheight = grid->gridrows[0].height;
	}
	grid->scrolltop_rect.y2 = grid->scrolltop_rect.y1 + rowheight;

	if (grid->gridrows[0].texture)
	{
		MGLGridTexQuad(grid->gridrows[0].texture, grid->scrolltop_rect.x1, grid->scrolltop_rect.y1, grid->scrolltop_rect.x2, grid->scrolltop_rect.y2,
			grid->gridrows[0].bg);
	}
	else
	{
		glColor3fv(grey);
		// glRecti(grid->scrolltop_rect.x1, grid->scrolltop_rect.y1, grid->scrolltop_rect.x2, grid->scrolltop_rect.y2 );		
	}


	// bottom arrow area
	grid->scrollbot_rect.x1 = grid->gridrect.x2 - grid->GScrollWidth;
	grid->scrollbot_rect.x2 = grid->gridrect.x2;

	grid->scrollbot_rect.y1 = grid->gridrows[grid->nrows - 1].rowrect.y1 - MGL_GRID_DEF_ROW_HEIGHT;
	grid->scrollbot_rect.y2 = grid->scrollbot_rect.y1 + rowheight;

	glColor3fv(grey);
	// glRecti(grid->scrollbot_rect.x1, grid->scrollbot_rect.y1, grid->scrollbot_rect.x2, grid->scrollbot_rect.y2 );

	// elevator box
	// y values already set in Init routine, or scrollbar move routines

	// position elevator based on scroll position and size

	if (vo && vo->count){
		ypixels = grid->scrolltop_rect.y1 - grid->scrollbot_rect.y2;
		ysize = (int)((double)(grid->nrows - 1) / vo->count * (double)ypixels); // subtract 2 for title and bot arrow areas
		if (ysize < MIN_ELEVATOR_HEIGHT) ysize = MIN_ELEVATOR_HEIGHT;

		ypos = (int)((double)grid->iVscrollPos / vo->count * ypixels);
		ypos = grid->scrolltop_rect.y1 - ypos;

		grid->elev_rect.y2 = ypos;
		grid->elev_rect.y1 = ypos - ysize;

		if (grid->elev_rect.y2 > grid->scrolltop_rect.y1){
			grid->elev_rect.y2 = grid->scrolltop_rect.y1;
			grid->elev_rect.y1 = grid->elev_rect.y2 - ysize;
		}
	}
	grid->elev_rect.x1 = grid->scrollbar_rect.x1;
	grid->elev_rect.x2 = grid->scrollbar_rect.x2;
	glColor3fv(gridgreyelev);

	glRecti(grid->elev_rect.x1, grid->elev_rect.y1, grid->elev_rect.x2, grid->elev_rect.y2);

	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "brownsquare.png");


	y1 = (((double)grid->scrollbot_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrollbot_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrollbot_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrollbot_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;

#ifdef OLDWAY
	y1 = grid->scrollbot_rect.y1;
	x1 = grid->scrollbot_rect.x1;
	y2 = grid->scrollbot_rect.y2;
	x2 = grid->scrollbot_rect.x2;
#endif



	SetPNGMode(id);


	//  glColor3fv(darkpurple);

	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "Left_arr.png");


	y1 = (((double)grid->scrolltop_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrolltop_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrolltop_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrolltop_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	DisplayCachedPNG(id, pngfilename, 32, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE, 0);


	// draw up and down elevators 
	sprintf_s(pngfilename, _countof(pngfilename), "Right_arr.png");
	y1 = (((double)grid->scrollbot_rect.y1 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x1 = (((double)grid->scrollbot_rect.x1 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	y2 = (((double)grid->scrollbot_rect.y2 / id->m_nHeight) * (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin))
		+ id->m_sCurLayout.m_dWorldYmin;
	x2 = (((double)grid->scrollbot_rect.x2 / id->m_nWidth) * (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin))
		+ id->m_sCurLayout.m_dWorldXmin;
	DisplayCachedPNG(id, pngfilename, 32, (float)x1, (float)x2, (float)y1, (float)y2, (float)(x2 - x1), (float)(y2 - y1),
		GL_ONE_MINUS_SRC_ALPHA, ALPHA_MODE_NONE, 0);


	glFlush();
	glPopMatrix();

	glColor3fv(white);


	return(TRUE);
}


void SetGridGLMode(struct InstanceData* id)
{
	assert(id);

	glPushMatrix();
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f); /* white line */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, id->m_nWidth, 0, id->m_nHeight, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

char setnames(MGLGRID *grid, char *name, char labelname[64])
{
	//char labelname[24];
	if (grid->roi_fdo == 2)
	{
		if (strcmp("Flight ID", name) == 0)
			strcpy_s(labelname, 24, "FltNum");

		else if (strcmp("Origin", name) == 0)
			strcpy_s(labelname, 24, "Origin");

		else if (strcmp("Destination", name) == 0)
			strcpy_s(labelname, 24, "Destin");

		else if (strcmp("A/C Type (FAA)", name) == 0)
			strcpy_s(labelname, 24, "actype");

		else if (strcmp("ETOn (PASSUR)", name) == 0)
			strcpy_s(labelname, 24, "eta");

		else if (strcmp("ATOn (PASSUR)", name) == 0)
			strcpy_s(labelname, 24, "ata");

		else if (strcmp("STD", name) == 0)
			strcpy_s(labelname, 24, "std");

		else if (strcmp("ETD", name) == 0)
			strcpy_s(labelname, 24, "etd");

		else if (strcmp("ATOff (PASSUR)", name) == 0)
			strcpy_s(labelname, 24, "atd");

		else if (strcmp("STA", name) == 0)
			strcpy_s(labelname, 24, "sta");

		else if (strcmp("Speed", name) == 0)
			strcpy_s(labelname, 24, "speed");

		else if (strcmp("N Miles", name) == 0)
			strcpy_s(labelname, 24, "DestDis");


		else if (strcmp("Beacon", name) == 0)
			strcpy_s(labelname, 24, "beacon");

		else if (strcmp("Altitude", name) == 0)
			strcpy_s(labelname, 24, "altitude");

		else if (strcmp("ATD (Carrier)", name) == 0)
			strcpy_s(labelname, 24, "outtime");

		else if (strcmp("ATOff (Carrier)", name) == 0)
			strcpy_s(labelname, 24, "offtime");

		else if (strcmp("ATOn (Carrier)", name) == 0)
			strcpy_s(labelname, 24, "ontime");

		else if (strcmp("ATA (Carrier)", name) == 0)
			strcpy_s(labelname, 24, "intime");

	//	else if (strcmp("Gate", name) == 0)
	//		strcpy_s(labelname, 24, "gate");

	//	else if (strcmp("D ETA", name) == 0)
	//		strcpy_s(labelname, 24, "bestarrgmtts");

	//	else if (strcmp("D STA", name) == 0)
	//		strcpy_s(labelname, 24, "d_sta");

		else if (strcmp("A/C Type (Carrier)", name) == 0)
			strcpy_s(labelname, 24, "d_actype");


		//else if (strcmp("Drag Off", name) == 0)
		//	strcpy_s(labelname, 24, "drag_off");

		else if (strcmp("Pax Closeout", name) == 0)
			strcpy_s(labelname, 24, "psgr_clseout");

		else if (strcmp("Dep Gate", name) == 0)
			strcpy_s(labelname, 24, "intdprtgate");

	//	else if (strcmp("PrevPubDepGate", name) == 0)
		//	strcpy_s(labelname, 24, "prevpubdprtgate");

	//	else if (strcmp("prevpubarrgate", name) == 0)
	//		strcpy_s(labelname, 24, "prevpubarrgate");

	//	else if (strcmp("previntdprtgate", name) == 0)
	//		strcpy_s(labelname, 24, "previntdprtgate");

	//	else if (strcmp("previntarrgate", name) == 0)
	//		strcpy_s(labelname, 24, "previntarrgate");

	//	else if (strcmp("defaultdprtgateind", name) == 0)
	//		strcpy_s(labelname, 24, "defaultdprtgateind");

		else if (strcmp("Arr Gate", name) == 0)
			strcpy_s(labelname, 24, "int_arr_gate");	

		else if (strcmp("Tbfm_Offtime", name) == 0)
			strcpy_s(labelname, 24, "tbfm_offtime");
		else if (strcmp("Tbfm_Ontime", name) == 0)
			strcpy_s(labelname, 24, "tbfm_ontime");
		else if (strcmp("Tbfm_Freezetime", name) == 0)
			strcpy_s(labelname, 24, "tbfm_freezetime");
		else if (strcmp("Cargo_Door_Clsd", name) == 0)
			strcpy_s(labelname, 24, "cargo_door_clsd");
		else if (strcmp("Cargo_Door_Open", name) == 0)
			strcpy_s(labelname, 24, "cargo_door_open");
		else if (strcmp("Pax_Door_Clsd", name) == 0)
			strcpy_s(labelname, 24, "pax_door_clsd");
		else if (strcmp("Pax_Door_Open", name) == 0)
			strcpy_s(labelname, 24, "pax_door_open");
		else if (strcmp("Bridge_On_Time", name) == 0)
			strcpy_s(labelname, 24, "bridge_on_time");
		else if (strcmp("Crew_Out_Time", name) == 0)
			strcpy_s(labelname, 24, "crew_out_time");
		else if (strcmp("Brake_Released", name) == 0)
			strcpy_s(labelname, 24, "brake_released");
		else if (strcmp("Brake_Set", name) == 0)
			strcpy_s(labelname, 24, "brake_set");
		else if (strcmp("Dep_Gate", name) == 0)
			strcpy_s(labelname, 24, "dep_gate");
		else if (strcmp("Arr_Gate", name) == 0)
			strcpy_s(labelname, 24, "arr_gate");
		else if (strcmp("Acars_Init", name) == 0)
			strcpy_s(labelname, 24, "acars_init");

		else if (strcmp("TBFM-msgTime", name) == 0)
			strcpy_s(labelname, 24, "first_msg");
		else if (strcmp("TBFM-old_fid", name) == 0)
			strcpy_s(labelname, 24, "old_fid");
		else if (strcmp("TBFM-A/C Flt Pln Status", name) == 0)
			strcpy_s(labelname, 24, "fps");
		else if (strcmp("TBFM-A/C Current Status", name) == 0)
			strcpy_s(labelname, 24, "acs");
		else if (strcmp("TBFM-STA Freeze Flag", name) == 0)
			strcpy_s(labelname, 24,"sfzflag");
		else if (strcmp("TBFM-Rwy Freeze Flag", name) == 0)
			strcpy_s(labelname, 24, "rfzflag");
		else if (strcmp("TBFM-rwy_freeze", name) == 0)
			strcpy_s(labelname, 24,"rwy_freeze" );
		else if (strcmp("TBFM-A/C Type", name) == 0)
			strcpy_s(labelname, 24,"tbfm_actype");
		else if (strcmp("TBFM-TRACON Asgn Rwy", name) == 0)
			strcpy_s(labelname, 24, "tracon_rwy");
		else if (strcmp("TBFM-Arr Runway", name) == 0)
			strcpy_s(labelname, 24,"arr_rwy");
		else if (strcmp("TBFM-EDCT Status", name) == 0)
			strcpy_s(labelname, 24,"edc_status");
		else if (strcmp("TBFM-EDCT", name) == 0)
			strcpy_s(labelname, 24,"edc_time");
		else if (strcmp("TBFM-ATOff", name) == 0)
			strcpy_s(labelname, 24, "tbfm_etd");
		else if (strcmp("TBFM-STA at Rwy", name) == 0)
			strcpy_s(labelname, 24,"sta_at_rwy");
		else if (strcmp("TBFM-ETA at Rwy", name) == 0)
			strcpy_s(labelname, 24,"eta_at_rwy");
		else if (strcmp("Assigned Gate Status", name) == 0)
			strcpy_s(labelname, 24, "Assigned Gate Status");

		//10/12/2016
		else if (strcmp("TBFM Time", name) == 0)
			strcpy_s(labelname, 24, "estdprtctrlgmtts");
		else if (strcmp("TBFM Status", name) == 0)
			strcpy_s(labelname, 24, "estdprtctrlgmttssrc");
		else if (strcmp("Boarding Started", name) == 0)
			strcpy_s(labelname, 24, "brdngstrtdgmtts");
		else if (strcmp("Pax Door Closed", name) == 0)
			strcpy_s(labelname, 24, "paxdrclsgmtts");
		else if (strcmp("Ready to Depart", name) == 0)
			strcpy_s(labelname, 24, "readytodprtgmtts");
		else if (strcmp("onlinetsgmt", name) == 0)
			strcpy_s(labelname, 24, "onlinetsgmt");
		else if (strcmp("draggateid", name) == 0)
			strcpy_s(labelname, 24, "draggateid");
		else if (strcmp("Flt_at_Gate", name) == 0)
			strcpy_s(labelname, 24, "Flt_at_Gate");
		else if (strcmp("Gt_Exp_Clr_Time", name) == 0)
			strcpy_s(labelname, 24, "Gt_Exp_Clr_Time");
		
		else
			strcpy_s(labelname, 24, "");
		return labelname[64];
	}
	else if (grid->roi_fdo == 1)
	{
		if (strcmp("Region", name) == 0)
			strcpy_s(labelname, 24, "Region");

		else if (strcmp("Flight", name) == 0)
			strcpy_s(labelname, 24, "flightid");

		else if (strcmp("Start", name) == 0)
			strcpy_s(labelname, 24, "starttime");

		else if (strcmp("End", name) == 0)
			strcpy_s(labelname, 24, "endtime");

		else if (strcmp("Elapsed", name) == 0)
			strcpy_s(labelname, 24, "elapsed");

		else if (strcmp("In", name) == 0)
			strcpy_s(labelname, 24, "Active");

		else if (strcmp("A/D", name) == 0)
			strcpy_s(labelname, 24, "arrdep");

		else if (strcmp("fix", name) == 0)
			strcpy_s(labelname, 24, "fix");
		else
			strcpy_s(labelname, 24, "");
		return labelname[64];

	}

	else if (grid->roi_fdo == 3)
	{
		if (strcmp("seqnum", name) == 0)
			strcpy_s(labelname, 24, "seqnum");

		else if (strcmp("appname", name) == 0)
			strcpy_s(labelname, 24, "appname");

		else if (strcmp("username", name) == 0)
			strcpy_s(labelname, 24, "username");

		else if (strcmp("alertname", name) == 0)
			strcpy_s(labelname, 24, "alertname");

		else if (strcmp("priority", name) == 0)
			strcpy_s(labelname, 24, "priority");

		else if (strcmp("starttime", name) == 0)
			strcpy_s(labelname, 24, "starttime");

		else if (strcmp("endtime", name) == 0)
			strcpy_s(labelname, 24, "endtime");

		else if (strcmp("comments", name) == 0)
			strcpy_s(labelname, 24, "comments");

		else if (strcmp("alertsource", name) == 0)
			strcpy_s(labelname, 24, "alertsource");
		else
			strcpy_s(labelname, 24, "");
		return labelname[64];

	}
	return labelname[64];

}

void MGLSetColNamesFromVO(MGLGRID *grid)
{
	VO *vo, *vcol;
	int c;

	if (!grid || !grid->vo){
		return;
	}

	vo = grid->vo;

	for (c = 0; c < vo->vcols->count && c < MGL_GRID_MAX_COLS; c++){
		vcol = V_ROW_PTR(vo->vcols, c);
		strncpy_s(grid->ColOrderNames[c], MGL_GRID_LABEL_MAX, vcol->name, _TRUNCATE);

	}


}

void MGLSetlabelNamesFromVO(MGLGRID *grid)
{
	VO *vo, *vcol;
	int c;

	if (!grid || !grid->vo){
		return;
	}

	vo = grid->vo;
	if (grid->roi_fdo == 2 || grid->roi_fdo ==3)
	{
		for (c = 0; c < vo->vcols->count && c < MGL_GRID_MAX_COLS; c++){
			vcol = V_ROW_PTR(vo->vcols, c);

			strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, grid->ColLabels[c], _TRUNCATE);
			//strncpy_s(grid->ColOrderNames[c], MGL_GRID_LABEL_MAX, vcol->label, _TRUNCATE);

		}
	}

	else if (grid->roi_fdo == 1)
	{
		vcol = V_ROW_PTR(vo->vcols, 0);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 1);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "Region", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 2);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "Flight", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 3);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "", _TRUNCATE);


		vcol = V_ROW_PTR(vo->vcols, 4);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "Start", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 5);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "End", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 6);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 7);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "Elapsed", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 8);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "A/D", _TRUNCATE);


		vcol = V_ROW_PTR(vo->vcols, 9);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "In", _TRUNCATE);

		vcol = V_ROW_PTR(vo->vcols, 10);
		strncpy_s(vcol->label, MGL_GRID_LABEL_MAX, "fix", _TRUNCATE);


	}
}

int MGLGridDraw(struct InstanceData* id, MGLGRID *grid)
{
	int x1, y1, x2, y2, nrows;
	char tmpstr[256];
	static int testtimes = 0;
	struct timeb start_time, done_time;
	time_t nowsecs;
	int curypos = 0;
	int rowheight;

	
	assert(id);

	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	if (testtimes) ftime(&start_time);

	if (!grid){
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Grid is null, call MGLGridCreate first.\n", GetCurrentThreadId());
		return (FALSE);
	}

	nowsecs = time(0);

	if (!grid->lastcalccol || nowsecs - grid->lastcalccol > 30 || id->resize == 2){
		if (grid->fixed_width==1)
		{
			CalcColWidths_condition(id, grid);
			grid->lastcalccol = nowsecs;
		}
		else if (grid->fixed_width == 2)
		{
			CalcColWidths_condition2(id, grid);
			if (strcmp("DataAppCreaterGrid",grid->name) == 0)
				MGLGridMove(id, grid, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1 - (2.5 * DataAppGrid->gridrows[0].height));
			grid->lastcalccol = nowsecs;
		}
		else
		{
			CalcColWidths(id, grid);
			grid->lastcalccol = nowsecs;
		}
	}


	/* draw backgound of entire grid */

	/* create a polygon and fill it to match the panel */
	x1 = grid->gridrect.x1;
	y1 = grid->gridrect.y1;
	x2 = x1 + grid->winwidth;
	y2 = y1 + grid->winheight;
	
	if (strcmp(grid->name, "BuildSummaryGrid") && strcmp(grid->name, "SecondTabGrid") && (strcmp(grid->name, "TitleGridGrid") != 0))
	{
		//Checking boundary conditions
		if (grid->gridrect.y2 > id->m_nHeight)
		{
			grid->gridrect.y1 = grid->gridrect.y1 - grid->height;
			grid->gridrect.y2 = grid->gridrect.y1 + grid->height;
			y1 = grid->gridrect.y1;
			y2 = y1 + grid->winheight;


			for (int i = 0; i < MGL_GRID_MAX_ROWS; i++){
				if (i == 0){
					curypos = grid->gridrect.y1 + grid->winheight -
						grid->gridrows[i].height;
				}

				// subtract height for this row from the current position
				if (grid->gridrows[i].height == MGL_DEFAULT){
					rowheight = MGL_GRID_DEF_ROW_HEIGHT;
				}
				else {
					rowheight = grid->gridrows[i].height;
				}
				grid->gridrows[i].rowrect.y1 = curypos - rowheight;
				curypos = grid->gridrows[i].rowrect.y1; //move cursor down to this row
			}
			MGLGridMove(id, grid, x1, y1);
		}
		if (grid->gridrect.x2 > id->m_nWidth)
		{
			grid->gridrect.x1 = grid->gridrect.x1 - grid->width;
			grid->gridrect.x2 = grid->gridrect.x1 + grid->width;
			x1 = grid->gridrect.x1;
			x2 = x1 + grid->winwidth;
			for (int i = 0; i < MGL_GRID_MAX_ROWS; i++){
				if (i == 0){
					curypos = grid->gridrect.y1 + grid->winheight -
						grid->gridrows[i].height;
				}

				// subtract height for this row from the current position
				if (grid->gridrows[i].height == MGL_DEFAULT){
					rowheight = MGL_GRID_DEF_ROW_HEIGHT;
				}
				else {
					rowheight = grid->gridrows[i].height;
				}
				grid->gridrows[i].rowrect.y1 = curypos - rowheight;
				curypos = grid->gridrows[i].rowrect.y1; //move cursor down to this row
			}
			MGLGridMove(id, grid, x1, y1);
		}
	}
	glColor3fv(grid->bg);
	glRecti(x1, y1, x2, y2);


	//SetGridGLMode(id);
	//glTranslatef(trans[0], trans[1], trans[2] );

	// just reset with current columns in case there are no rows
	//  this seems wasteful, if reallyh needed I need to figure out and call only when needed MGLGridMove(id, grid, grid->gridrect.x1, grid->gridrect.y1 );


	if (testtimes){
		if (grid->vo){
			nrows = grid->vo->count;
		}
		else {
			nrows = grid->nrows;
		}
		ftime(&done_time);
		sprintf_s(tmpstr, 256, "griddraw before drawrows %s nrows=%d diff=%g\n", grid->name, nrows, uts_timediff(&start_time, &done_time));
		OutputDebugString(tmpstr);
	}

	if (!grid->IsMinimized){
		MGLGridDrawRows(grid);
		if (testtimes){
			ftime(&done_time);
			sprintf_s(tmpstr, 256, "griddraw after  drawrows %s nrows=%d diff=%g\n", grid->name, nrows, uts_timediff(&start_time, &done_time));
			OutputDebugString(tmpstr);
		}
		MGLGridDrawColumns(grid);
		MGLGridDrawCells(id, grid);
		if (testtimes){
			ftime(&done_time);
			sprintf_s(tmpstr, 256, "griddraw after drawcells drawrows %s nrows=%d diff=%g\n", grid->name, nrows, uts_timediff(&start_time, &done_time));
			OutputDebugString(tmpstr);
		}
	}

	glPopMatrix();

	if (grid->AllowScrollbar && !grid->IsMinimized){
		if (strcmp(grid->name, "DataAppSelectorGrid") == 0)
		{
			if (grid->vo->count > 9)
				MGLGridDrawScroll_condition(id, grid);

		}
		
		else
		{
			MGLGridDrawScroll(id, grid);
			//MGLGridDrawScroll_horizontal(id, grid);
		}
	}
	if (grid->HasTitleBar){
		SetGridGLMode(id);
		MGLGridDrawTitle(id, grid);
		glPopMatrix();
	}

	SetGridGLMode(id);
	// update overall grid sizes after redraw (may have changed due to col or row width changes
	x1 = MGLGridXMin(grid);
	y1 = MGLGridYMin(grid);
	x2 = MGLGridXMax(grid) + grid->GScrollWidth;
	y2 = MGLGridYMax(grid);


	grid->gridrect.x1 = x1;
	if (y1 < 0){
		sprintf_s(tmpstr, _countof(tmpstr), "bad y pos\n");
		OutputDebugString(tmpstr);
	}
	else {
		grid->gridrect.y2 = y2;  // gridrect.y2 is top of grid , so use y2 MGLGridYMax	
	}

	grid->height = y2 - y1;
	grid->width = MGLGridXMax(grid) - x1;
	grid->gridrect.y2 = y2;
	grid->gridrect.y1 = y1;

	if (testtimes){
		if (grid->vo){
			nrows = grid->vo->count;
		}
		else {
			nrows = grid->nrows;
		}
		ftime(&done_time);
		sprintf_s(tmpstr, 256, "griddraw %s nrows=%d diff=%g\n", grid->name, nrows, uts_timediff(&start_time, &done_time));
		OutputDebugString(tmpstr);
	}

	MGLGridMove(id, grid, x1, y1);
	return(TRUE);
}




MGLMenuItem *MGLCreateMenuItem(MGLMenu *menu, char *label)
{
	MGLMenuItem *mi;
	MGLMenuItem *np, *lastnp;

	assert(menu);

	mi = (MGLMenuItem *)vo_malloc(sizeof(MGLMenuItem));
	mi->label = label;
	mi->fg = black;
	mi->bg = grey;

	/* add the item to the menu's list */

	if (!menu->itemlist){
		menu->itemlist = mi;
	}
	else {
		/* find last item in linked list */
		for (np = menu->itemlist; np; np = np->np){
			lastnp = np;
		}
		lastnp->np = mi;
	}

	return(mi);
}




int MGLMenuXOffset(MGLPanel *panel, int *xoffset, int *yoffset)
{
	int panelmax;
	panellist *plist;
	MGLMenu *menu;
	int maxx;

	assert(panel);

	/* calculate the default offset of this menu */

	panelmax = 0;
	for (plist = panel->plist; plist; plist = plist->np){
		switch (plist->itemtype)
		{
		case MGL_MENU:
			menu = (MGLMenu *)plist->panelitem;
			maxx = menu->labelx2;
			if (maxx > panelmax){
				panelmax = maxx;
			}
			break;
		default:
			break;
		}
	}

	*xoffset = panelmax + PANEL_ITEM_XBORDER;
	*yoffset = 0;  /* all menus have same vertical location */

	return(TRUE);
}




MGLMenu *MGLCreateMenu(MGLPanel *panel, char *label, int xoffset, int yoffset,
	void(*callback)(MGLMenu *menu))
{
	MGLMenu *menu = NULL;
	panellist *pitem;
	int label_font_width = 10; /* hardwired since only one fond now, 8 pixels plus 2 margin */
	int label_font_height = 15; /* hardwired since only one fond now, 13 pixels plus 2 margin */

	menu = (MGLMenu *)vo_malloc(sizeof(MGLMenu));
	menu->panel = panel;
	menu->label = label;
	menu->callback = callback;

	if (xoffset == -1){
		MGLMenuXOffset(panel, &xoffset, &yoffset);
	}
	else {
		/* use user supplied offsets */
	}
	menu->xoffset = xoffset;
	menu->yoffset = yoffset;

	menu->fg = black;
	menu->bg = grey;

	xoffset = panel->x + menu->xoffset + PANEL_TEXT_XBORDER;
	yoffset = panel->y + menu->yoffset + PANEL_TEXT_YBORDER;
	menu->labelx1 = panel->x + menu->xoffset + PANEL_TEXT_XBORDER;
	menu->labelx2 = menu->labelx1 + (label_font_width * strlen(label));
	menu->labely1 = panel->y + menu->yoffset + PANEL_TEXT_YBORDER;
	menu->labely2 = menu->labely1 + label_font_height;

	/* add menu to items for this panel */
	pitem = (panellist *)vo_malloc(sizeof(panellist));
	pitem->panelitem = menu;
	pitem->itemtype = MGL_MENU;

	if (!panel->plist){
		/* first item */
		panel->plist = pitem;
	}
	else {
		panel->plist->lastp->np = pitem;
	}

	panel->plist->lastp = pitem;

	return(menu);
}




MGLPanel *MGLCreatePanel(int x, int y, int width, int height,
	GLfloat *fg, GLfloat *bg)
{
	MGLPanel *panel;
	panellist *pitem;

	panel = (MGLPanel *)vo_malloc(sizeof(MGLPanel));

	panel->x = x;
	panel->y = y;
	panel->width = width;
	panel->height = height;
	panel->fg = fg;
	panel->bg = bg;

	/* add to global list of gui panels */

	pitem = (panellist *)vo_malloc(sizeof(panellist));
	pitem->panelitem = panel;
	pitem->itemtype = MGL_PANEL;

	if (!G_panellist){
		/* first item */
		G_panellist = pitem;
	}
	else {
		G_panellist->lastp->np = pitem;
	}

	G_panellist->lastp = pitem;
	return(panel);
}



MGLDrawMenu(struct InstanceData* id, MGLPanel *panel, panellist *plist)
{
	MGLMenu *menu;
	int micount, menuheight, menuwidth, xoffset, yoffset;
	MGLMenuItem *mi;
	int x1, y1, x2, y2;
	int itemx, itemy, basex, basey;
	int menuitemheight, menuitemwidth;

	assert(id);

	menu = (MGLMenu *)plist->panelitem;

	/* draw the menu's label */
	/* DrawTextureString( 20, 20, 12, black, menu->label ); */

	xoffset = panel->x + menu->xoffset + PANEL_TEXT_XBORDER;
	yoffset = panel->y + menu->yoffset + PANEL_TEXT_YBORDER;

	DrawRasterString(id, xoffset, yoffset, menu->fg, menu->label);

	if (menu->IsActive){
		//printf("MGLDrawMenu, drawing active menu %s\n", menu->label );

		/* create window that displays all menu items */
		for (micount = 0, mi = menu->itemlist; mi; mi = mi->np){
			micount++;
		}

		/* create backgound panel that has number of rows == micount */
		menuitemheight = MENU_ITEM_HEIGHT;
		menuitemwidth = MENU_ITEM_WIDTH;
		menuheight = micount * menuitemheight;
		menuwidth = menuitemwidth;

		x1 = xoffset;
		x2 = x1 + menuwidth;
		y1 = yoffset - menuheight;
		y2 = y1 + menuheight;
		glColor3fv(panel->bg);
		glRecti(x1, y1, x2, y2);

		/* draw each item label */
		basex = x1;
		basey = yoffset; /* top */
		for (micount = 0, mi = menu->itemlist; mi; mi = mi->np){
			itemx = basex + MENU_ITEM_MARGIN;
			itemy = basey - ((micount + 1) * menuitemheight) + MENU_ITEM_MARGIN;
			DrawRasterString(id, itemx, itemy, mi->fg, mi->label);
			micount++;
		}
	}
}




void SetZoomBarW(struct InstanceData* id)
{
	double minxworldsize, maxworldsize, barsize, curxsize, barx;

	//  SliderBarW is set based on zoom values
	minxworldsize = 0.5;
	maxworldsize = 75.25;
	barsize = 80.0 + (4 * 2) - 6;  // barsize is length plus edges, minux width of bar

	curxsize = id->m_sCurLayout.m_dWorldXSize;

	curxsize = min(maxworldsize, curxsize);
	barx = (barsize * curxsize) / maxworldsize;


	// convert ratio to location
	barx = min(RightSliderEdgeW->gregion.x2, barx);

	SliderBarW->gregion.x1 = (int)rint(LeftSliderEdgeW->gregion.x1 + barx);
	SliderBarW->gregion.x2 = SliderBarW->gregion.x1 + 6;
	SliderBarW->gregion.y1 = RightSliderEdgeW->gregion.y1 - 6;
	SliderBarW->gregion.y2 = RightSliderEdgeW->gregion.y1 + 13; // tot size of image is 19


}


void MGLDrawSlider(struct InstanceData* id, widgets *w)
{

	if (!w){
		return;
	}


	SetZoomBarW(id);  // reset based on current map xworldsize

	glColor3fv(w->bg);
	glRecti(w->gregion.x1, w->gregion.y1, w->gregion.x2, w->gregion.y2);

	// draw the slider bar where it should be based on it's value


}


void MGLDrawPanel(struct InstanceData* id, widgets *w)
{

	assert(id);

	if (!w){
		return;
	}

	SetGridGLMode(id);
	glColor3fv(w->bg);
	glRecti(w->gregion.x1, w->gregion.y1, w->gregion.x2, w->gregion.y2);
}


void MGLDrawPanel_new(struct InstanceData* id, widgets *w)
{

	assert(id);

	if (!w){
		return;
	}

	SetGridGLMode(id);
	glColor3fv(w->bg);
	glRecti(w->gregion.x1, w->gregion.y1, w->gregion.x2, w->gregion.y2);
}


MGLDrawPanelItem(struct InstanceData* id, MGLPanel *panel, panellist *plist)
{

	assert(id);
	assert(plist);

	switch (plist->itemtype)
	{
	case MGL_MENU:
		MGLDrawMenu(id, panel, plist);
		break;
	default:
		break;
	}

}





MGLDraw(struct InstanceData* id)
{
	MGLPanel *panel;
	panellist *pitem, *plist;
	int x1, y1, x2, y2;

	assert(id);

	/* draw the gui */

	/* draw all window panels */
	for (pitem = G_panellist; pitem; pitem = pitem->np){

		if (pitem->itemtype != MGL_PANEL){
			/* error */
			break;
		}

		panel = (MGLPanel *)pitem->panelitem;

		/* create a polygon and fill it to match the panel */

		x1 = panel->x;
		y1 = panel->y;
		x2 = x1 + panel->width;
		y2 = y1 + panel->height;

		glColor3fv(panel->bg);
		glRecti(x1, y1, x2, y2);

		/* draw this panel's items */

		for (plist = panel->plist; plist; plist = plist->np){
			MGLDrawPanelItem(id, panel, plist);
		}
	}


	/* reset the color to white so textures will appear */
	glColor3f(1.0, 1.0, 1.0);

}


MGLMotionMenu(MGLPanel *panel, panellist *plist, int x, int y)
{
	MGLMenu *menu;
	int micount, menuheight, menuwidth, xoffset, yoffset;
	MGLMenuItem *mi;
	int x1, y1, x2, y2;
	int itemx, itemy, basex, basey;
	int menuitemheight, menuitemwidth, found;

	assert(panel);

	menu = (MGLMenu *)plist->panelitem;

	xoffset = panel->x + menu->xoffset + PANEL_TEXT_XBORDER;
	yoffset = panel->y + menu->yoffset + PANEL_TEXT_YBORDER;

	if (menu->IsActive){
		/* create window that displays all menu items */
		for (micount = 0, mi = menu->itemlist; mi; mi = mi->np){
			micount++;
			mi->fg = black;
			mi->IsActive = FALSE;
		}

		/* create backgound panel that has number of rows == micount */
		menuitemheight = MENU_ITEM_HEIGHT;
		menuitemwidth = MENU_ITEM_WIDTH;
		menuheight = micount * menuitemheight;
		menuwidth = menuitemwidth;

		x1 = xoffset;
		x2 = x1 + menuwidth;
		y1 = yoffset - menuheight;
		y2 = y1 + menuheight;

		basex = x1;
		basey = yoffset; /* top */
		found = FALSE;
		for (micount = 0, mi = menu->itemlist; mi; mi = mi->np){
			itemx = basex + MENU_ITEM_MARGIN;
			itemy = basey - ((micount + 1) * menuitemheight) + MENU_ITEM_MARGIN;

			if (x >= itemx && x <= itemx + menuwidth &&
				y >= itemy && y <= itemy + menuitemheight){
				//printf("MGLMotionMenu, INRANGE of item %s\n",
				//    mi->label);
				/* motion is over a menuitem */
				mi->IsActive = TRUE;
				mi->fg = red;
				found = TRUE;
			}
			else {
				mi->IsActive = FALSE;
				mi->fg = black;
			}
		}

		if (found == TRUE){
			return(TRUE);
		}
	}

	/* see if motion is over menu label */
	/* see if the panel label was selected and TOGGLE menu visibility if so */
	if (x >= menu->labelx1 && x <= menu->labelx2 && y >= menu->labely1 &&
		y <= menu->labely2){
		menu->IsActive = TRUE;
		menu->fg = red;
		// printf("MGLMotionMenu, INRANGE of menu %s\n", menu->label );
		return(TRUE);  /* item was in range of this item */
	}
	menu->IsActive = FALSE;
	menu->fg = black;
	return(FALSE);
}


MGLTestMenu(MGLPanel *panel, panellist *plist, int b, int m, int x, int y)
{
	MGLMenu *menu;
	int micount, menuheight, menuwidth, xoffset, yoffset;
	MGLMenuItem *mi;
	int x1, y1, x2, y2;
	int itemx, itemy, basex, basey;
	int menuitemheight, menuitemwidth, found;

	assert(panel);

	menu = (MGLMenu *)plist->panelitem;

	xoffset = panel->x + menu->xoffset + PANEL_TEXT_XBORDER;
	yoffset = panel->y + menu->yoffset + PANEL_TEXT_YBORDER;

	if (menu->IsActive){
		/* create window that displays all menu items */
		for (micount = 0, mi = menu->itemlist; mi; mi = mi->np){
			micount++;
			mi->fg = black;
			mi->IsActive = FALSE;
		}

		/* create backgound panel that has number of rows == micount */
		menuitemheight = MENU_ITEM_HEIGHT;
		menuitemwidth = MENU_ITEM_WIDTH;
		menuheight = micount * menuitemheight;
		menuwidth = menuitemwidth;
		x1 = xoffset;
		x2 = x1 + menuwidth;
		y1 = yoffset - menuheight;
		y2 = y1 + menuheight;

		basex = x1;
		basey = yoffset; /* top */
		found = FALSE;
		for (micount = 0, mi = menu->itemlist; mi; mi = mi->np){
			itemx = basex + MENU_ITEM_MARGIN;
			itemy = basey - ((micount + 1) * menuitemheight) + MENU_ITEM_MARGIN;

			/* only select on mouse down events */
			if (m == 1){
				/* mouse down event */
				if (x >= itemx && x <= itemx + menuwidth &&
					y >= itemy && y <= itemy + menuitemheight){
					//printf("MGLTestMenu, MOUSE DOWN INRANGE of item %s\n",
					// mi->label);
					found = TRUE;
					/* call the callback routine */
					(*menu->callback)(menu);
					/* deactivate the item since we took action */
					mi->IsActive = FALSE;
					mi->fg = black;
					/* deactivate the menu also */
					menu->IsActive = FALSE;
					menu->fg = black;
				}
				else {
					mi->IsActive = FALSE;
					mi->fg = black;
				}
			}
		}
		if (found == TRUE){
			return(TRUE);
		}
	}

	/* see if the panel label was selected and TOGGLE menu visibility if so */
	if (x >= menu->labelx1 && x <= menu->labelx2 && y >= menu->labely1 &&
		y <= menu->labely2){
		/* only toggle on mouse down events */
		if (m == 1){
			/* mouse down event */
			if (menu->IsActive == TRUE){
				menu->IsActive = FALSE;
				menu->fg = black;
			}
			else {
				//printf("MGLTestMenu, Activate menu %s\n",
				//     menu->label );
				menu->IsActive = TRUE;
				mi->fg = red;
			}
		}
		return(TRUE);  /* item was in range of this item */
	}
	return(FALSE);
}



MGLMouseoverPanelItem(MGLPanel *panel, panellist *plist, int x, int y)
{

	assert(panel);

	switch (plist->itemtype)
	{
	case MGL_MENU:
		if (MGLMotionMenu(panel, plist, x, y)){
			return(TRUE);
		}
		break;
	default:
		break;
	}

	return(FALSE);
}




MGLTestPanelItem(MGLPanel *panel, panellist *plist, int b, int m, int x, int y)
{

	assert(panel);

	switch (plist->itemtype)
	{
	case MGL_MENU:
		if (MGLTestMenu(panel, plist, b, m, x, y)){
			return(TRUE);
		}
		break;
	default:
		break;
	}

	return(FALSE);
}



int MGLIsGuiMotionEvent(struct InstanceData* id, int x, int y)
{
	panellist *pitem;
	MGLPanel *panel;
	int x1, y1, x2, y2;
	panellist *plist;
	widgets *w;
	char tmpstr[1024];

	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	
	/* activate any menu labels or menu items the mouse is over */
	printf("MGLIsGuiMotionEvent, x=%d y=%d\n", x, y);

	if (y < 10 || y> id->m_nHeight - 10 || x < 10 || x > id->m_nWidth-20)
	{
		return FALSE;
	}
		
	
	if (!oglgui && !G_panellist) return(FALSE);

	if (oglgui){
		for (w = oglgui->widgets; w; w = w->np){
			// draw the image 
			if (!w->visible)
				continue;

			if (w->grid) {
				// see if we are in scrollbar
				if (InRectangle(x, y, &(w->grid->gridrect)) || InRectangle(x, y, &(w->grid->scrollbar_rect))){
					if (InRectangle(x, y, &(w->grid->scrollbar_rect))){
						// in scrollbar

						//  need this or the elevator will not move all the way   if ( InRectangle( x, y, &(w->grid->scrolltop_rect )) ||  InRectangle( x, y, &(w->grid->scrollbot_rect )) ){
						// ignore top and bottom arrow boxes 
						if (w->grid->nBox == TRUE && abs(w->grid->mouse_rect.y1 - y) > 7){// ignore small mouse movements
							// move elevator to scroll position
							_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMotionEvent, in grid scrollbar x=%d y=%d\n", x, y);
							OutputDebugString(tmpstr);
							MoveElevator(w, x, y);
						}
					}

					// are we dragging the grid???
					if (InRectangle(x, y, &(w->grid->gridrect))){
						// do not return, process drag below
					}
					else {
						return(TRUE);
					}
				}

				// see if we are dragging the title bar
				// see if we are still dragging
				if (w->grid->MoveBox == TRUE) {
					//     ????? && ( abs( w->grid->mouse_rect.y1 - y) > 7 || abs( w->grid->mouse_rect.x1 - x) > 7 ) ){// ignore small mouse movements
					// move grid
					_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMotionEvent, dragging move grid dx=%d dy=%d x=%d y=%d mrx=%d mry=%d\n",
						x - w->grid->mouse_rect.x1, y - w->grid->mouse_rect.y1, x, y, w->grid->mouse_rect.x1, w->grid->mouse_rect.y1);
					OutputDebugString(tmpstr);
					// old MGLGridMove(w->grid, w->grid->gridrect.x1 + ( x - w->grid->mouse_rect.x1) , w->grid->gridrect.y1 + (y - w->grid->mouse_rect.y1) );
					if ((strcmp((w->grid->name), "ActionsGrid") == 0) || (strcmp((w->grid->name), "ParametersGrid") == 0) || (strcmp((w->grid->name), "GridofActions") == 0) || (strcmp((w->grid->name), "AvailableGrid") == 0) || (strcmp((w->grid->name), "RowColumn") == 0) || (strcmp((w->grid->name), "BuildSummaryGrid") == 0) || (strcmp((w->grid->name), "SecondTabGrid") == 0) || (strcmp((w->grid->name), "TitleGridGrid") == 0))
					{
					}
					else if (strcmp((w->grid->name), "DataAppSelectorGrid") == 0)
					{	
						
						MGLGridMove(id, DataAppGrid, w->grid->drag_start_rect.x1 + (x - w->grid->mouse_rect.x1), w->grid->drag_start_rect.y1 + (y - w->grid->mouse_rect.y1));
						MGLGridMove(id, DataAppCreater, w->grid->drag_start_rect.x1 + (x - w->grid->mouse_rect.x1), w->grid->drag_start_rect.y1 + (y - w->grid->mouse_rect.y1) - (3 * DataAppCreater->gridrows[0].height));
						if (SecondTabGrid)
						{
							MGLGridMove(id, SecondTabGrid, DataAppGrid->gridrect.x2 - 1, w->grid->drag_start_rect.y1 + (y - w->grid->mouse_rect.y1) - ((SecondTabGrid->nrows - 1) * DataAppGrid->gridrows[0].height) + ((DataAppGrid->nrows - 1) * DataAppGrid->gridrows[0].height));
							if (SecondTabGrid->childgrid)
							{
								if (SecondTabGrid->childgrid->TitleGrid->parent_w->visible == 1)
								{
									MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (SecondTabGrid->childgrid->BuildSummaryGrid->nrows * DataAppGrid->gridrows[0].height) - DataAppGrid->gridrows[0].height);
									MGLGridMove(id, SecondTabGrid->childgrid->TitleGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - DataAppGrid->gridrows[0].height);
								}

								else if (SecondTabGrid->childgrid->TitleGrid->parent_w->visible == 0)
								{
									MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, SecondTabGrid->gridrect.y1 - SecondTabGrid->childgrid->BuildSummaryGrid->nrows * DataAppGrid->gridrows[0].height);
								}
							}
						}

						if (SecondTabGrid->childgrid)
						{
							if (SecondTabGrid->childgrid->ActionsGrid)
							{
								MGLGridMove(id, SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
							}
							if (SecondTabGrid->childgrid->ParametersGrid)
							{
								MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
							}
							if (SecondTabGrid->childgrid->AlertsColorGrid)
							{
								MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
								if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
									MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
								if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
									MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
								if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
									MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
								if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
									MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.x2 + DataAppGrid->gridrows[0].height, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.y2 - (6 * DataAppGrid->gridrows[0].height));
								if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
									MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.x2 + DataAppGrid->gridrows[0].height, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->gridrect.y1 - (2 * DataAppGrid->gridrows[0].height));
							}
							if (SecondTabGrid->childgrid->SortAvailableGrid)
							{
								MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
							}
							if (SecondTabGrid->childgrid->RowColumn)
							{
								MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));
							}
						}
					}


					else

						MGLGridMove(id, w->grid, w->grid->drag_start_rect.x1 + (x - w->grid->mouse_rect.x1), w->grid->drag_start_rect.y1 + (y - w->grid->mouse_rect.y1));



				}

			}
			else if (w->type == WGT_SLIDER){
				if (InRectangle(x, y, &w->gregion)){
					_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMotionEvent, in WGT_SLIDER x=%d y=%d\n", x, y);
					OutputDebugString(tmpstr);
					// activate callback
					if (w->OnClickCB){
						(w->OnClickCB)(id, w, x, y);
					}
					return(TRUE);
				}
			}


		}
	}


	/* see if the value is in range of any gui panels */
	for (pitem = G_panellist; pitem; pitem = pitem->np){

		if (pitem->itemtype != MGL_PANEL){
			/* error */
			break;
		}

		panel = (MGLPanel *)pitem->panelitem;
		/* check individual panel items first before testing entire panel */
		for (plist = panel->plist; plist; plist = plist->np){
			if (MGLMouseoverPanelItem(panel, plist, x, y)){
				return(TRUE);
			}
		}

		x1 = panel->x;
		y1 = panel->y;
		x2 = x1 + panel->width;
		y2 = y1 + panel->height;

		if (x >= x1 && x <= x2 && y >= y1 && y <= y2){
			// printf("MGLIsGuiMotionEvent, INRANGE of panel, x=%d y=%d\n", x, y );
			return(TRUE);
		}
	}

	id->m_poglgui = oglgui;

	id->DataAppGrid = DataAppGrid;
	id->DataAppCreater = DataAppCreater;
	return(FALSE);
}

/*
* return the day of the week for particualr date
* (the divisions are integer divisions that truncate the result)
* Sun = 0, Mon = 1, etc.
*/
int get_week_day(int day, int month, int year) {
	int a, m, y;
	a = (14 - month) / 12;
	y = year - a;
	m = month + 12 * a - 2;
	return ((day + y + y / 4 - y / 100 + y / 400 + ((31 * m) / 12)) % 7); // GREGORIAN CALENDAR
}
/*
* retun the month and year based on the user click
* If user clicked '<' return the previous month
* If user clicked '>' return next month
* upper and lower bounds for calendar established
*/
int get_month_year(char click, int *mptr, int *yptr, int miny, int maxy, struct InstanceData* id){
	if (click == '<'){
		/*for months b/w Jan-Dec*/
		if ((*mptr >0) && (*mptr <= 12))
			*mptr -= 1;
		else{
			/*transition month from Jan-Dec*/
			if (((*yptr - 1) >= miny) && ((*yptr - 1) <= maxy)){
				*yptr -= 1;
				*mptr = 11;
			}
			else{
				//printf("'<' Out of Bounds\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):'<' Out of Bounds\n", get_month_year);
				return(-1);
			}
		}
	}
	else{
		/*for months b/w Feb-Dec*/
		if (*mptr >= 0 && *mptr <11)
			*mptr += 1;
		else{
			/*Transition month from Dec-Jan*/
			if (((*yptr + 1) >= miny) && ((*yptr + 1) <= maxy)){
				*yptr += 1;
				*mptr = 0;
			}
			else{
				//printf("'>' Out of Bounds\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):'>' Out of Bounds\n", get_month_year);
				return(-1);
			}
		}
	}
	return(0);
}
/*display_calendar(start,symbol,id,topw);*/
display_calendar(int start, int click, struct InstanceData* id, MGLGRID *CalendarGrid){
	int i = 0,j=0;
	char j_c[5];
	time_t t;
	struct tm *t_date;
	static int year = 0, month = 0;
	int date = 0;
	char date_str[10];
	char year_str[512];
	int curr_year, curr_month, curr_day;//current values to be displayed
	int max_years = 0, min_years = 0;
	
	int startingDay = 0;     /*starting of the month*/
	char *names[] = { "January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December" };
	int months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int daysInMonth = 0;
	int weekOfMonth = 1;
	int dayOfWeek = 0;
	char dayOfWeek_str[10];
	/*obtain current date for display*/
	time(&t);
	t_date = localtime(&t);
	curr_year = t_date->tm_year + 1900;
	curr_month = t_date->tm_mon;
	curr_day = t_date->tm_mday;
	/*set max and min limits for the calendar*/
	max_years = curr_year + 5;
	min_years = curr_year - 5;
	/*clear the vo matrix*/
	for (i = 0; i < 9; i++){
		for (j = 0; j < 7; j++){
			memset(j_c, 0, sizeof(j_c));
			_snprintf(j_c, 1, "%d", j);
			memset(VP(CalendarGrid->vo, i, j_c, char), 0, sizeof(VP(CalendarGrid->vo, i, j_c, char)));
		}
	}

	/*the program has been init*/
	if (start == 1){
		year = curr_year;
		month = curr_month;
		start = 0;
	}

	else{
		/*if click made by the user*/
		if (click == '<' || click == '>'){
			if ((get_month_year(click, &month, &year, min_years, max_years,id)) < 0){
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):Out of range values accessed,quitting\n ", display_calendar);
				//printf("Error:Out of range values accessed,quitting\n");
				//exit(1);
			}
		}
		/*check if click=='s',else input invalid*/
        else{
                if(click!='s'){
					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):Invalid Input\n", display_calendar);
                    //printf("Invalid Input\n");
                    //exit(1);
                }
            }
        }

	/******************* PRINT CALENDAR***************************************/
	
	/*obtain starting day of the month*/
	startingDay = get_week_day(1, month + 1, year);

	/*check for leap year*/
	if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
		months[1] = 29;
	else
		months[1] = 28;

	/*display ----monthyear---- */
	daysInMonth = months[month];  /* set # of days */
	memset(year_str, 0, sizeof(year_str));
   _snprintf(year_str,256, "     %s  %d ",names[month],year);
    MGLSetTitle(CalendarGrid, year_str);
	//strcpy(VP(CalendarGrid->vo, 0, "0", char), year_str);

	/*display dates for the month*/
	for (dayOfWeek = 0; dayOfWeek < startingDay; ++dayOfWeek){
		_snprintf(dayOfWeek_str, 10, "%d", dayOfWeek);
		strcpy(VP(CalendarGrid->vo, weekOfMonth, dayOfWeek_str, char), "  ");
	}
	for(date = 1; date <= daysInMonth; ++date) {
	
		_snprintf(dayOfWeek_str, 10, "%d", dayOfWeek);
		_snprintf(date_str, 10, "%5d", date);

		strcpy(VP(CalendarGrid->vo, weekOfMonth, dayOfWeek_str, char), date_str);

		if (++dayOfWeek > 6) {
			dayOfWeek = 0;
			weekOfMonth++;
		}
	} // for date
} 

//mtm-- Check if a 'Maxmize' is hit the corresponding grid/grids close and do the corresponding actions if the 'Maximize' button is hit
int MaxClick(struct InstanceData* id, widgets *topw, int b, int m, int x, int y, MGLGRID* DataAppGrid, MGLGRID* DataAppCreater, MGLGRID* SecondTabGrid)
{
	//check if the maximize button is hit -- if yes do the corresponding actions and return TRUE, else return FALSE
	if (InRectangle(x, y, &(topw->grid->maximize_rect))){
		// maximize the grid
		if (DataAppGrid->parent_w->maintab_visible == 1)
			DataAppGrid->parent_w->visible = 1;
		else if (SecondTabGrid->parent_w->maintab_visible == 1){
			SecondTabGrid->parent_w->visible = 1;
			if (SecondTabGrid->childgrid){
				if (SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory == 1)
					SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = TRUE;
				if (SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory == 1)
					SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = TRUE;
				if (SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory == 1)
					SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = TRUE;
				if (SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory == 1)
					SecondTabGrid->childgrid->RowColumn->parent_w->visible = TRUE;
				if (SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory == 1)
					SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = TRUE;
			}
		}
		topw->grid->IsMinimized = 0;
		topw->grid->gridrect = topw->grid->gridrectmax; // go to last stored location of maximzed grid
		MGLGridMove(id, topw->grid, topw->grid->gridrect.x1, topw->grid->gridrect.y1);

		if ((strcmp(topw->grid->name, "DataAppSelectorGrid")) == 0)
		{
			DataAppGrid->IsMinimized = 0;
			DataAppCreater->parent_w->visible = 1;

			MGLGridMove(id, DataAppGrid, topw->grid->gridrect.x1, topw->grid->gridrect.y1 - (DataAppGrid->nrows * DataAppGrid->gridrows[0].height));
			MGLGridMove(id, DataAppCreater, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1 - (3 * DataAppGrid->gridrows[0].height));

			if (SecondTabGrid)
			{
				if (SecondTabGrid->childgrid)
				{
					if (SecondTabGrid->childgrid->to_open_build == 1)
					{
						MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (SecondTabGrid->childgrid->BuildSummaryGrid->nrows * DataAppGrid->gridrows[0].height));
						SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = TRUE;
						MGLGridMove(id, SecondTabGrid->childgrid->TitleGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - DataAppGrid->gridrows[0].height);
						SecondTabGrid->childgrid->TitleGrid->parent_w->visible = TRUE;
						MGLGridMove(id, SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					}


					else if (SecondTabGrid->childgrid->to_open_details == 1)
					{
						SecondTabGrid->parent_w->visible = 1;
						SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = TRUE;
						MGLGridMove(id, SecondTabGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (SecondTabGrid->nrows* (DataAppGrid->gridrows[0].height)));
						MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, (SecondTabGrid->gridrect.y1 - ((SecondTabGrid->nrows - 4) * DataAppGrid->gridrows[0].height)));
						MGLGridMove(id, SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
						MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));

						if (SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory == TRUE)
							SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = 1;

						if (SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory == TRUE)
							SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = 1;

						if (SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory == TRUE)
							SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = 1;

						if (SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory == TRUE)
							SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = 1;

						if (SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory == TRUE)
							SecondTabGrid->childgrid->RowColumn->parent_w->visible = 1;


						if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
							if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible_memory == 1)
								SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 1;
						if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
							if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible_memory == 1)
								SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 1;
						if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
							if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible_memory == 1)
								SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 1;
						if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible_memory == 1)
							SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = 1;
						if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible_memory == 1)
							SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 1;

						if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
						if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
							MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));

					}
				}
			}

		}
		return(TRUE);
	}

	return(FALSE);
}

//mtm-- Check if a 'Minimize' is hit the corresponding grid/grids close and do the corresponding actions if the 'Minimize' button is hit
int MinClick(struct InstanceData* id, widgets *topw, int b, int m, int x, int y, MGLGRID* DataAppGrid, MGLGRID* DataAppCreater, MGLGRID* SecondTabGrid)
{
	//check if the minimize button is hit -- if yes do the corresponding actions and return TRUE, else return FALSE
	if (InRectangle(x, y, &(topw->grid->minimize_rect))){
		// minimize the grid by setting visrows = 1???
		if (SecondTabGrid)
		{
			if (DataAppGrid->parent_w->visible == 1)
				DataAppGrid->parent_w->maintab_visible = 1;
			else
				DataAppGrid->parent_w->maintab_visible = 0;

			if (SecondTabGrid->parent_w->visible == 1)
				SecondTabGrid->parent_w->maintab_visible = 1;
			else
				SecondTabGrid->parent_w->maintab_visible = 0;

		}
		topw->grid->IsMinimized = 1;
		if ((strcmp(topw->grid->name, "DataAppSelectorGrid")) == 0)
		{
			DataAppCreater->parent_w->visible = 0;
			DataAppGrid->IsMinimized = 1;

			if (SecondTabGrid)
			{
				if (SecondTabGrid->childgrid)
				{

					if (SecondTabGrid->childgrid->BuildSummaryGrid)
					{
						if (SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible == 1 && SecondTabGrid->parent_w->visible == 0)
						{
							SecondTabGrid->childgrid->TitleGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->to_open_build = 1;
						}


						else if (SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible == 1 && SecondTabGrid->parent_w->visible == 1)
						{
							SecondTabGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->to_open_details = 1;

							SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = 0;
							SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = 0;
							SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = 0;
							SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = 0;
							SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = 0;

							if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible_memory = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible_memory = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible_memory = 0;

							if (SecondTabGrid->childgrid->ActionsGrid->parent_w->visible == 1)
								SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = 1;
							if (SecondTabGrid->childgrid->ParametersGrid->parent_w->visible == 1)
								SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = 1;
							if (SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible == 1)
								SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = 1;
							if (SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible == 1)
								SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = 1;
							if (SecondTabGrid->childgrid->RowColumn->parent_w->visible == 1)
								SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = 1;

							if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
								if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible == 1)
									SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible_memory = 1;
							if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
								if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible == 1)
									SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible_memory = 1;
							if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
								if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible == 1)
									SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible_memory = 1;

							SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->RowColumn->parent_w->visible = 0;

							if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;

						}
					}
				}
			}

		}


		else
		{
			if (DataAppGrid->parent_w->visible == FALSE)
				DataAppGrid->parent_w->visible = FALSE;
			else
				DataAppGrid->parent_w->visible = TRUE;

			if (SecondTabGrid->parent_w->visible == FALSE)
				SecondTabGrid->parent_w->visible = FALSE;
			else
				SecondTabGrid->parent_w->visible = TRUE;
		}
		topw->grid->gridrect = topw->grid->gridrectmin; // go to last stored location of minimized grid
		MGLGridMove(id, topw->grid, topw->grid->gridrect.x1, topw->grid->gridrect.y1);

		return(TRUE);

	}

	return(FALSE);
}

//mtm-- Check if  a 'X' is hit the corresponding grid/grids close -- and do the corresponding actions if 'X' is clicked
int XClick(struct InstanceData* id, widgets *topw, int b, int m, int x, int y, MGLGRID* DataAppGrid, MGLGRID* DataAppCreater, MGLGRID* SecondTabGrid)
{


	MGLGRID *SourceGrid;
	char *AppName;

	/// check if  X close was selected
	if (InRectangle(x, y, &(topw->grid->xclose_rect))){
		// close this widget

		if (strcmp(topw->grid->name, "DataAppSelectorGrid") == 0){
			topw->visible = FALSE;
			DataAppCreater->parent_w->visible = FALSE;
			DataAppGrid->parent_w->visible = FALSE;
			if (ActiveTextGrid)
			{
				if (strcmp(ActiveTextGrid->name, "DataAppCreaterGrid") == 0)
				{
					ActiveTextGrid = NULL;
					AppName = VP(DataAppCreater->vo, 0, "AppName", char);
					if (strcmp(AppName, "  Add New"))
					{
						strncpy_s(VP(DataAppCreater->vo, 0, "AppName", char), 64, "  Add New", _TRUNCATE);

					}
				}

				else if (strcmp(ActiveTextGrid->name, "ActionsGrid") == 0)
				{
					strncpy_s(VP(ActiveTextGrid->vo, ActiveTextGrid->vo->count - 1, "Name", char), 64, "New", _TRUNCATE);
					ActiveTextGrid->gridcells[ActiveTextGrid->vo->count][0].bg = (GLfloat*)grey_Tab_bg;
					ActiveTextGrid = NULL;

				}

				else if (strcmp(ActiveTextGrid->name, "RowColumn") == 0)
				{
					AppName = VP(ActiveTextGrid->vo, 0, "Settings", char);
					if (strcmp(AppName, "") == 0)
					{
						strncpy_s(VP(ActiveTextGrid->vo, 0, "Settings", char), 16, "", _TRUNCATE);

					}
					ActiveTextGrid->gridcells[ActiveTextGrid->vo->count][0].bg = (GLfloat*)grey_Tab_bg;
					ActiveTextGrid = NULL;

				}

				else if (strcmp(ActiveTextGrid->name, "ParametersGrid") == 0)
				{
					for (int m = 0; m < ActiveTextGrid->vo->count; m++)
					{
						if (ActiveTextGrid->vo->row_index[m].bg != -1)
						{
							ActiveTextGrid->vo->row_index[m].bg = -1;
							ActiveTextGrid->gridcells[m + 1][3].bg = (GLfloat*)SubGrid_grey2;
							if (ActiveTextGrid->parentgrid->OperatorGrid->parent_w->visible == 1)
								ActiveTextGrid->parentgrid->OperatorGrid->parent_w->visible = 0;
							ActiveTextGrid = NULL;
							break;
						}
					}
				}
			}

		}
		else if (strcmp(topw->grid->name, "FilterGrid") == 0)
		{
			topw->grid->parent_w->visible = 0;
		}

		else if (strcmp(topw->grid->name, "TitleGridGrid") == 0){
			topw->visible = FALSE;
			DataAppGrid->callback = 0;
			SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = FALSE;
			SecondTabGrid->childgrid->TitleGrid->parent_w->visible = FALSE;
			SecondTabGrid->childgrid->to_open_build = 0;
			for (int xx = 0; xx < 100; xx++){
				AppName = VP(DataAppGrid->vo, xx, "AppName", char);
				if (AppName == NULL)
					goto next;
				if (strcmp(AppName, SecondTabGrid->childgrid->name) == 0){
					DataAppGrid->row_checkbox[xx] = 0;
					strncpy_s(DataAppGrid->gridcols[8].pngfilename, sizeof(DataAppGrid->gridcols[8].pngfilename), "zoom-arrow-off.png", _TRUNCATE);
					goto next;
				}
			}
		}

		else if (strcmp(topw->grid->name, "SecondTabGrid") == 0)
		{
			topw->visible = FALSE;
			SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = FALSE;
			SecondTabGrid->childgrid->to_open_details = 0;
			for (int xx = 0; xx < 100; xx++){
				AppName = VP(DataAppGrid->vo, xx, "AppName", char);
				if (AppName == NULL)
					goto next;
				if (strcmp(AppName, SecondTabGrid->childgrid->name) == 0){
					DataAppGrid->row_checkbox[xx] = 0;
					goto next;
				}
			}
		}
		else if (strcmp(topw->grid->name, "CalendarGrid") == 0)
		{
			topw->grid->parent_w->visible = 0;
		}
		//topw->checkbox_show
		//to check and uncheck when we close the ROIgrid and FDOGrid
		else
		{
			//	DataAppGrid->del = 1;
			topw->visible = FALSE;
			for (int xx = 0; xx < 100; xx++){
				AppName = VP(DataAppGrid->vo, xx, "AppName", char);
				if (AppName == NULL)
					goto next;
				if (strcmp(AppName, topw->grid->name) == 0){
					DataAppGrid->show_checkbox[xx] = 0;
					goto next;
				}

			}
		}


	next:		if (topw->grid && (!strcmp(topw->grid->name, "DataAppSelectorGrid")))
	{
		topw->grid->parent_w->visible = FALSE;

	}

				// if this is one of Alerts or Parameters of Colors grid, then close all of them
				else if (topw->grid && (!strcmp(topw->grid->name, "ActionsGrid")))
				{
					// take down all actions grids
					SourceGrid = topw->grid->parentgrid;
					SourceGrid->ActionsGrid->parent_w->visible = FALSE;
					//	SourceGrid->ParametersGrid->parent_w->visible = FALSE;
				}

				else if (topw->grid && (!strcmp(topw->grid->name, "ParametersGrid")))
				{
					SourceGrid = topw->grid->parentgrid;
					SourceGrid->ParametersGrid->parent_w->visible = FALSE;
					//	SourceGrid->ActionsGrid->parent_w->visible = FALSE;

				}



				else if (topw->grid && (!strcmp(topw->grid->name, "AlertsColorGrid")))
				{
					SourceGrid = topw->grid;
					topw->grid->parent_w->visible = FALSE;
					/*if (SourceGrid->AlertsColorGrid)
					{
					SourceGrid->AlertsColorGrid->parent_w->visible = 0;
					}*/
				}


				else if (topw->grid && (!strcmp(topw->grid->name, "colormatrixforeground")))
				{
					SourceGrid = topw->grid->parentgrid;
					SourceGrid->flag1 = 0;
				}

				else if (topw->grid && (!strcmp(topw->grid->name, "colormatrixbackground")))
				{
					SourceGrid = topw->grid->parentgrid;
					SourceGrid->flag2 = 0;
				}

				else if (topw->grid && (!strcmp(topw->grid->name, "ROIGrid")))
				{
					if (topw->grid->GridSettingsMenu){
						topw->grid->GridSettingsMenu->parent_w->visible = 0;
					}
					SourceGrid = topw->grid;
					SourceGrid->parent_w->visible = FALSE;
					DataAppGrid->vo->row_index[0].fg = MGLColorHex2Int("Black");//To revert color back in the dataapp grid


				}

				else if (topw->grid && (!strcmp(topw->grid->name, "FDOGrid")))
				{

					if (topw->grid->GridSettingsMenu){ //mtm-closes the grid with rules and layout if open
						topw->grid->GridSettingsMenu->parent_w->visible = 0;
					}
					DataAppGrid->vo->row_index[2].fg = MGLColorHex2Int("Black");//To revert color back in the dataapp grid
				}
				else if (topw->grid && (!strcmp(topw->grid->name, "SySAlertsGrid")))
				{

					if (topw->grid->GridSettingsMenu){ //mtm-closes the grid with rules and layout if open
						topw->grid->GridSettingsMenu->parent_w->visible = 0;
					}
					DataAppGrid->vo->row_index[2].fg = MGLColorHex2Int("Black");//To revert color back in the dataapp grid
				}
				else if (topw->grid && (!strcmp(topw->grid->name, "GridofActions"))){
					topw->grid->parentgrid->AlertsColorGrid->parent_w->visible = FALSE;
				}
				else if (topw->grid && (!strcmp(topw->grid->name, "AlertsGrid")))
				{
					if (topw->grid->GridSettingsMenu){
						topw->grid->GridSettingsMenu->parent_w->visible = 0;
					}
					DataAppGrid->vo->row_index[1].fg = MGLColorHex2Int("Black");//To revert color back in the dataapp grid
				}

				else if (topw->grid && (!strcmp(topw->grid->name, "TarmacGrid")))
				{
					if (topw->grid->GridSettingsMenu){
						topw->grid->GridSettingsMenu->parent_w->visible = 0;
					}
					DataAppGrid->vo->row_index[3].fg = MGLColorHex2Int("Black");//To revert color back in the dataapp grid
				}

				///mtm-to close and revert the color back to default foreground color in dataapp for "New" grid-ROI grid///
				else if (topw->grid->check_newgrid == 1)
				{
					if (topw->grid->GridSettingsMenu){
						topw->grid->GridSettingsMenu->parent_w->visible = 0;
					}
					//int nrows = topw->grid->nrows;
					DataAppGrid->vo->row_index[topw->grid->rownumbindataapp].fg = MGLColorHex2Int("Black");
				}

				///to close and revert the color back to default foreground color in dataapp for "New" grid- FDO grid///
				else if (topw->grid->check_newgrid == 2)
				{
					if (topw->grid->GridSettingsMenu){
						topw->grid->GridSettingsMenu->parent_w->visible = 0;
					}
					DataAppGrid->vo->row_index[topw->grid->rownumbindataapp].fg = MGLColorHex2Int("Black");

				}

				else if (topw->grid && (!strcmp(topw->grid->name, "SecondTabGrid")))
				{
					SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->RowColumn->parent_w->visible = FALSE;

					if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
					{
						SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
					}
					if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
					{
						SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
					}
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
					{
						SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
					}
					if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
						SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
					if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
						SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
				}


				return(TRUE);
	}
	return(FALSE);
	//check x close ends//
}

int MGLProcessTitleClick(struct InstanceData* id, widgets *topw, int b, int m, int x, int y)
{

	char tmpstr[1024];
	int XButton, MinButton, MaxButton;

	//The below variables are for Calendar
	int start = 0;
	char symbol = 's';

	assert(id);
	assert(topw);


	//Setting void pointers to MGLGRID struct	
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;


	DataAppGrid->del = 0;


	// check if Red X close was selected
	XButton = XClick(id, topw, b, m, x, y, DataAppGrid, DataAppCreater, SecondTabGrid);
	if (XButton)
		return TRUE;

	MinButton = MinClick(id, topw, b, m, x, y, DataAppGrid, DataAppCreater, SecondTabGrid);
	if (MinButton)
		return TRUE;

	MaxButton = MaxClick(id, topw, b, m, x, y, DataAppGrid, DataAppCreater, SecondTabGrid);
	if (MaxButton)
		return TRUE;





	///mtm-to open Build grids

	if (InRectangle(x, y, &(topw->grid->rules))){
		GridMenuCallback_rules(id, topw->grid, b, m, x, y);
		id->resize = 2;
		MGLDrawGUI(id);
	}

	//check if previous icon on calendar was clicked
	if (strcmp(topw->grid->name, "CalendarGrid") == 0){

		if (InRectangle(x, y, &(topw->grid->cal_prev))){
			symbol = '<';
			start = 0;
		}
		else if (InRectangle(x, y, &(topw->grid->cal_next))){
			symbol = '>';
			start = 0;
		}
		else{
			symbol = 's';
			start = 0;
		}
		display_calendar(start, symbol, id, topw->grid);

	}

	if (m == 1){
		/* mouse down event */
		// mouse was not over a title widget, so assume we are dragging the window
		// scrollbar drag events must start inside of the elevator box for scrollbar
		/// use MoveBox  topw->grid->nBox = TRUE;
		topw->grid->MoveBox = TRUE;
		// assume we are starting a drag
		topw->grid->mouse_rect.x1 = x;
		topw->grid->mouse_rect.y1 = y;

		topw->grid->drag_start_rect.x1 = topw->grid->gridrect.x1;
		topw->grid->drag_start_rect.y1 = topw->grid->gridrect.y1;

		_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLProcessTitleClick, MOUSE DOWN MOUSE_RECT x=%d y=%d\n", x, y);
		OutputDebugString(tmpstr);
	}
	else if (topw->grid->MoveBox == TRUE){
		/* just ended a drag operation */

		//printf("MGLProcessTitleClick, ended a drag operation x=%d y=%d\n", x, y );
		// MoveElevator( w, x, y );

		_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLProcessTitleClick, MOUSE UP, Move Drag ended x=%d y=%d\n", x, y);
		OutputDebugString(tmpstr);
		topw->grid->MoveBox = FALSE;

	}
	else if (topw->grid->MoveBox == FALSE){
		// mouse is up
	}
	id->DataAppGrid = DataAppGrid;
	id->DataAppCreater = DataAppCreater;

	return(FALSE);

}




int MGLProcessLClick(struct InstanceData* id, widgets *w, int b, int m, int x, int y)
{
	int r, c;

	assert(id);
	assert(w);

	// we must find a fast way to search for cells with click callbacks
	// search via x,y or go through linked list of cells with callbacks?
	//   linked list will be  faster up to a certain point

	if (!w->grid){
		return(FALSE);
	}
	// if all cells have callbacks, then fast x,y search is needed


	if (w->grid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - w->grid->gridrect.y1) / w->grid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - w->grid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	     
	if (r < 0 || r >= w->grid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = w->grid->nrows - r - 1;
	if (w->grid->gridrows[r].row_callback){
		// get parent grid and top level grid 
		if (w->grid->gridrows[r].row_callback(id, w->grid, b, m, x, y) == TRUE){
			// the row callback handled this click
			return(TRUE);
		}
		else {
			// returned false , so check for individual cell callbacks , if found
		}
	}


	if (w->grid->gridrows[r].HasCellCallbacks){
		//  flag set if this row of cells has callbacks , this will allow skipping rows w/o callbacks
		for (c = 0; c < w->grid->ncols; c++){
			// all y values are the same for this row 
			// is mouse in this cell ??
			if (InRectangle(x, y, &(w->grid->gridcells[r][c].cellrect))){
				if (w->grid->gridcells[r][c].cell_callback){
					return(w->grid->gridcells[r][c].cell_callback(id, b, m, x, y));
				}
			}

		}
		// no callbacks found, error?
		return(FALSE);

	}


	return(FALSE); // no callbacks found

}




MGLGRID *MGLActiveGridText(struct InstanceData* id)
{
	widgets *w;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	if (oglgui) {
		for (w = oglgui->widgets; w; w = w->np){
			if (w->grid && w->TextActive)
			{
				return(w->grid);
			}
		}
	}

	id->m_poglgui = oglgui;
	return(NULL);

}



void MGLSetInActiveGridText(struct InstanceData* id)
{
	widgets *w;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	// find the widget that is active and inactivate it
	for (w = oglgui->widgets; w; w = w->np){
		if (w->grid && w->TextActive)
		{
			w->TextActive = FALSE;
			ActiveTextGrid = NULL;
		}
	}
	id->m_poglgui = oglgui;
}



char *MGLFindAlertsOperator(MGLGRID *ParametersGrid, int vorow)
{
	char *opstr;

	assert(ParametersGrid);

	// find out which operator the user selected
	opstr = VP(ParametersGrid->vo, vorow, "Operator", char);
	if (!strcmp(opstr, "Equal to")){
		return("EQ");
	}
	else if (!strcmp(opstr, "Not Equal to")){
		return("NE");
	}
	else if (!strcmp(opstr, "Greater than")){
		return("GT");
	}
	else if (!strcmp(opstr, "Less than")){
		return("LT");
	}
	else if (!strcmp(opstr, "Less than or equal to")){
		return("LE");
	}
	else if (!strcmp(opstr, "Greater than or equal to")){
		return("GE");
	}
	else if (!strcmp(opstr, "Between")){
		return("BT");
	}
	return(NULL);

}


int MGLMatch_lt(MGLGRID *SourceGrid, int action, int f, char *cmp_value, char *valstr)
{
	int cmpi, cmps, res;
	double cmpd, cmpds;
	int match = TRUE;

	assert(SourceGrid);

	switch (SourceGrid->Actions[action].gridconfig->filters[f].field_type)
	{
	case INTBIND:
		cmpi = atoi(cmp_value);
		cmps = atoi(valstr);
		if (cmps < cmpi){
			return(TRUE);
		}
		return(FALSE);
	case NTBSTRINGBIND:
		if ((res = strcmp(valstr, cmp_value)) < 0){
			return(TRUE); // LT
		}
		else {
			return(FALSE);
		}
		break;
	case DATETIMEBIND:
		break;
	case FLT8BIND:
		cmpd = atof(cmp_value);
		cmpds = atof(valstr);
		if (cmpds < cmpd){
			return(TRUE);
		}
		return(FALSE);
		break;
	default:
		break;
	}
	return(match);

}


int MGLMatch_le(MGLGRID *SourceGrid, int action, int f, char *cmp_value, char *valstr)
{
	int cmpi, cmps, res;
	double cmpd, cmpds;
	int match = TRUE;

	assert(SourceGrid);

	switch (SourceGrid->Actions[action].gridconfig->filters[f].field_type)
	{
	case INTBIND:
		cmpi = atoi(cmp_value);
		cmps = atoi(valstr);
		if (cmps <= cmpi){
			return(TRUE);
		}
		return(FALSE);
	case NTBSTRINGBIND:
		if ((res = strcmp(valstr, cmp_value)) <= 0){
			return(TRUE); // LE
		}
		else {
			return(FALSE);
		}
		break;
	case DATETIMEBIND:
		break;
	case FLT8BIND:
		cmpd = atof(cmp_value);
		cmpds = atof(valstr);
		if (cmpds <= cmpd){
			return(TRUE);
		}
		return(FALSE);
		break;
	default:
		break;
	}
	return(match);

}
int MGLMatch_gt(MGLGRID *SourceGrid, int action, int f, char *cmp_value, char *valstr)
{
	int cmpi, cmps, res;
	double cmpd, cmpds;
	int match = TRUE;

	assert(SourceGrid);

	switch (SourceGrid->Actions[action].gridconfig->filters[f].field_type)
	{
	case INTBIND:
		cmpi = atoi(cmp_value);
		cmps = atoi(valstr);
		if (cmps > cmpi){
			return(TRUE);
		}
		return(FALSE);
	case NTBSTRINGBIND:
		if ((res = strcmp(valstr, cmp_value)) > 0){
			return(TRUE); // GT
		}
		else {
			return(FALSE);
		}
		break;
	case DATETIMEBIND:
		break;
	case FLT8BIND:
		cmpd = atof(cmp_value);
		cmpds = atof(valstr);
		if (cmpds > cmpd){
			return(TRUE);
		}
		return(FALSE);
		break;
	default:
		break;
	}
	return(match);

}





int MGLMatch_ge(MGLGRID *SourceGrid, int action, int f, char *cmp_value, char *valstr)
{
	int cmpi, cmps, res;
	double cmpd, cmpds;
	int match = TRUE;

	assert(SourceGrid);

	switch (SourceGrid->Actions[action].gridconfig->filters[f].field_type)
	{
	case INTBIND:
		cmpi = atoi(cmp_value);
		cmps = atoi(valstr);
		if (cmps >= cmpi){
			return(TRUE);
		}
		return(FALSE);
	case NTBSTRINGBIND:
		if ((res = strcmp(valstr, cmp_value)) >= 0){
			return(TRUE); // GE
		}
		else {
			return(FALSE);
		}
		return(FALSE);
	case DATETIMEBIND:
		break;
	case FLT8BIND:
		cmpd = atof(cmp_value);
		cmpds = atof(valstr);
		if (cmpds >= cmpd){
			return(TRUE);
		}
		return(FALSE);
		break;
	default:
		break;
	}
	return(match);

}



int MGLMatch_eq(MGLGRID *SourceGrid, int action, int f, char *cmp_value, char *valstr)
{
	int cmpi, cmps;
	int match = TRUE;
	const char *request = " GET /index.html HTTP/1.0\r\n\r\n";
	struct slre_cap caps[4];



	assert(SourceGrid);

	switch (SourceGrid->Actions[action].gridconfig->filters[f].field_type)
	{
	case 0:
		if (SourceGrid->roi_fdo == 1)
		{
			if (strcmp(cmp_value, "Y") == 0)
				cmpi = 1;
			else if (strcmp(cmp_value, "N") == 0)
				cmpi = 0;
			cmps = atoi(valstr);
		}
		else if (SourceGrid->roi_fdo == 2)
		{
			cmpi = atoi(cmp_value);
			cmps = atoi(valstr);

		}
		else if (SourceGrid->roi_fdo == 3)
		{
			cmpi = atoi(cmp_value);
			cmps = atoi(valstr);

		}
		if (cmps == cmpi){
			return(TRUE);
		}
		else {
			return(FALSE);
		}
		break;

	case INTBIND:
		if (SourceGrid->roi_fdo == 1)
		{
			if (strcmp(cmp_value, "Y") == 0)
				cmpi = 1;
			else if (strcmp(cmp_value, "N") == 0)
				cmpi = 0;
			cmps = atoi(valstr);
			cmpi = atoi(cmp_value);
		}
		else if (SourceGrid->roi_fdo == 2)
		{
			cmpi = atoi(cmp_value);
			cmps = atoi(valstr);

		}
		else if (SourceGrid->roi_fdo == 3)
		{
			cmpi = atoi(cmp_value);
			cmps = atoi(valstr);

		}
		if (cmps == cmpi){
			return(TRUE);
		}
		else {
			return(FALSE);
		}
		break;
	case NTBSTRINGBIND:
		// use regular expression to match values
		if (slre_match(cmp_value, valstr, strlen(valstr), caps, 4, 0) > 0) {
			// matches
			match = TRUE;
			//printf("Method: [%.*s], URI: [%.*s]\n",
			//	caps[0].len, caps[0].ptr,
			//	caps[1].len, caps[1].ptr);
		}
		else if(strcmp(cmp_value, valstr) == 0)
		{
			match = TRUE;
		}
		else {
			match = FALSE;
			printf("Error parsing [%s]\n", valstr);
		}


#ifdef  OLD_WAY_RE_COMP
		if (re_comp(cmp_value)){
			// some error
		}
		else if (re_exec(valstr) == 1){
			// matches
			match = TRUE;
		}
		else {
			if (!strlen(cmp_value) && !strlen(valstr)){
				// they match
				match = TRUE;
			}
			else {
				match = FALSE;
			}
		}
#endif
		break;
	case DATETIMEBIND:
		break;
	case FLT8BIND:
		cmps = (int)atof(valstr);
		cmpi = (int)atof(cmp_value);
		if (cmps == cmpi)
			match = TRUE;
		else
			match = FALSE;
		break;
	default:
		break;
	}
	return(match);

}


int MGLMatch_ne(MGLGRID *SourceGrid, int action, int f, char *cmp_value, char *valstr)
{

	return(!MGLMatch_eq(SourceGrid, action, f, cmp_value, valstr));
}





int MGLMatch(MGLGRID *SourceGrid, int action, char *op, int f, char *cmp_value, char *valstr)
{
	assert(SourceGrid);

	if (!strcmp(op, "EQ")){
		return(MGLMatch_eq(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "GE")){
		return(MGLMatch_ge(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "GT")){
		return(MGLMatch_gt(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "LE")){
		return(MGLMatch_le(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "LT")){
		return(MGLMatch_lt(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "NE")){
		return(MGLMatch_ne(SourceGrid, action, f, cmp_value, valstr));
	}

	return(FALSE);

}



int MGLMatch_all(MGLGRID *SourceGrid, int action, char *op, int f, char *cmp_value, char *valstr, char *colname)
{
	// modified from MGLMatch_all_mtm, skipping colname checks, not sure why those are there

	if (!strcmp(op, "EQ")){
		return(MGLMatch_eq(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "GE")){
		return(MGLMatch_ge(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "GT")){
		return(MGLMatch_gt(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "LE")){
		return(MGLMatch_le(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "LT")){
		return(MGLMatch_lt(SourceGrid, action, f, cmp_value, valstr));
	}
	else if (!strcmp(op, "NE")){
		return(MGLMatch_ne(SourceGrid, action, f, cmp_value, valstr));
	}

	return(FALSE);

}


///mtm -- Written for new "Actions" parent grid///
//
void CreateParametersGrid(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);


	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ActionsPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// Create Actions grid

	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "AlertsColorWidget");
	w->gregion.x1 = 600;
	w->gregion.y1 = 280;
	w->gregion.x2 = 1450;
	w->gregion.y2 = 150;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "AlertsColorGrid", 3, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	//parentgrid->AlertsColorGrid = w->grid;
	parentgrid->GridActions = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Color", _TRUNCATE);
	//strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Settings", _TRUNCATE);



	/*strncpy_s(w->grid->gridcols[1].pngfilename, sizeof(w->grid->gridcols[1].pngfilename), "down-reorder-arrow.png", _TRUNCATE);
	w->grid->gridcols[1].texturesize = 16;
	w->grid->gridcols[1].img_displaysize = 10;*/





	// set row callbacks
	for (i = 0; i < parentgrid->GridActions->nrows; i++){
		parentgrid->GridActions->gridrows[i].row_callback = DropDownColor;
	}

	// set fg and bg colors for cells



	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->GridActions->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->GridActions->marginx;
	w->gregion.y1 = parentgrid->GridActions->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->GridActions->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Grid Actions", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->GridActions;

	id->m_poglgui = oglgui;
}


//added mtm//
void CreateBackgroundPanel(struct InstanceData* id, MGLGRID *parentgrid)
{
	widgets *w, *lastw, *panel_widget;
	int r;

	assert(id);
	assert(parentgrid);

	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "BackgroundPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Actions
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "BackgroundPanel");
	w->gregion.x1 = 420;
	w->gregion.y1 = id->m_nHeight - 220;
	w->gregion.x2 = 800;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "BackgroundPanel", 13, 4, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->BackgroundPanel = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}

		w->grid->gridrows[r].bg = (GLfloat*)color868686;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color868686;
	}


	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}


	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "   Criteria ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, " Parameters ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "   Layout   ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Grid Actions", _TRUNCATE);
	//strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Visible rows/columns", _TRUNCATE);

	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->BackgroundPanel->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "BackgroundPanel");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->BackgroundPanel->marginx;
	w->gregion.y1 = parentgrid->BackgroundPanel->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->BackgroundPanel->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Grid Builder", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->BackgroundPanel;
	id->m_poglgui = oglgui;

}

void IconsGrid(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	widgets *w, *lastw, *panel_widget;
	int r;

	assert(id);


	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "IconGrid");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// Create Actions grid

	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "IconGrid");
	w->gregion.x1 = 1070;
	w->gregion.y1 = 280;
	w->gregion.x2 = 1450;
	w->gregion.y2 = 580;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "IconGrid", 6, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	//parentgrid->AlertsColorGrid = w->grid;
	parentgrid->IconGrid = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Selections", _TRUNCATE);


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->IconGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->IconGrid->marginx;
	w->gregion.y1 = parentgrid->IconGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->IconGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Icon", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->IconGrid;
	id->m_poglgui = oglgui;

}

void DatabookGrid(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	widgets *w, *lastw, *panel_widget;
	int r;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "DatabookGrid");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// Create Actions grid

	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "DatabookGrid");
	w->gregion.x1 = 1070;
	w->gregion.y1 = 280;
	w->gregion.x2 = 1450;
	w->gregion.y2 = 580;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "DatabookGrid", 6, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	//parentgrid->AlertsColorGrid = w->grid;
	parentgrid->Databookgrid = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Selections", _TRUNCATE);


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->Databookgrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->Databookgrid->marginx;
	w->gregion.y1 = parentgrid->Databookgrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->Databookgrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Databook", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->Databookgrid;
	id->m_poglgui = oglgui;


}

void NotifyGrid(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	widgets *w, *lastw, *panel_widget;
	int r;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "NotifyGrid");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// Create Actions grid

	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "NotifyGrid");
	w->gregion.x1 = 1070;
	w->gregion.y1 = 280;
	w->gregion.x2 = 1450;
	w->gregion.y2 = 580;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "NotifyGrid", 6, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	//parentgrid->AlertsColorGrid = w->grid;
	parentgrid->Notifygrid = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Selections", _TRUNCATE);


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->Notifygrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->Notifygrid->marginx;
	w->gregion.y1 = parentgrid->Notifygrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->Notifygrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Notify", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->Notifygrid;
	id->m_poglgui = oglgui;

}

void InitParametersinBuildSummary(struct InstanceData* id,MGLGRID *grid)
{
	int t,line_count,f,buffer_count;
	GridConfig *gridconfig;
	char *op, *cmp_value, *field_name;
	char full[512];
	line_count = -1;

	for (t = 0; t < grid->nActions; t++)
	{
		gridconfig = grid->Actions[t].gridconfig;
		buffer_count = 0;
		for (f = 0; gridconfig && f < gridconfig->nFilters; f++){

			op = gridconfig->filters[f].op;
			cmp_value = gridconfig->filters[f].cmp_value;
			field_name = gridconfig->filters[f].field_name;
			strcpy_s(full, 512, field_name);
			strcat_s(full, 512, " ");
			strcat_s(full, 512, op);
			strcat_s(full, 512, " ");
			strcat_s(full, 512, cmp_value);

			if (grid->BuildSummaryGrid->vo->count <= (line_count+1))
				vo_alloc_rows(grid->BuildSummaryGrid->vo, 1);
			line_count++;
			buffer_count++;
			strncpy_s(VP(grid->BuildSummaryGrid->vo, line_count, "Parameters", char), 512, full, _TRUNCATE);
			
		}
		if (buffer_count == 0)
			line_count++;
	}
}

void InitCriteriainBuildSummary(struct InstanceData* id, MGLGRID *grid)
{
	int  line_count,buffer_count;
	GridConfig *gridconfig;
	char *op;	
	line_count = 0;
	buffer_count=0;
	for (int t = 0; t < grid->nActions; t++)
	{
		gridconfig = grid->Actions[t].gridconfig;
		if (line_count>=grid->BuildSummaryGrid->vo->count)
			vo_alloc_rows(grid->BuildSummaryGrid->vo, 1);
		strncpy_s(VP(grid->BuildSummaryGrid->vo, line_count, "Criteria", char), 64, grid->Actions[t].ActionName, _TRUNCATE);
		buffer_count = line_count;
		                                                                                                    
		for (int f = 0; gridconfig && f < gridconfig->nFilters; f++)
		{
			op = gridconfig->filters[f].op;
			if (strcmp(op, "") != 0)
			{
				line_count++;				

			}
		}
		if (buffer_count == line_count)
			line_count++;
	}

}
////mtm
/////Build confirm grid
MGLGRID *CreateCriteriaConfirmGrid(struct InstanceData* id)
{
	//pops up "SAVED" when save is pressed

	widgets *w, *lastw, *panel_widget;
	int r;


	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* CriteriaConfirmGrid = (MGLGRID*)id->CriteriaConfirmGrid;
	
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "CriteriaConfirmDisplay");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Alerts that are active
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "CriteriaConfirmDisplay");
	w->gregion.x1 = DataAppGrid->gridrect.x1+500;
	w->gregion.y1 = DataAppGrid->gridrect.y1 + 35;
	w->gregion.x2 = 500;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "CriteriaConfirmDisplay", 3, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	CriteriaConfirmGrid = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}

		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;




	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	MGLGridText(id, CriteriaConfirmGrid, 0, 0, "Do you want to delete the criteria?");
	MGLGridText(id, CriteriaConfirmGrid, 1, 0, "Yes");
	MGLGridText(id, CriteriaConfirmGrid, 2, 0, "No");
	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Do you want to delete the criteria?", _TRUNCATE);

	////

	//w = (widgets *)vo_malloc(sizeof(widgets));
	//CriteriaConfirmGrid->title_widgets = w;

	//strcpy_s(w->name, sizeof(w->name), "ConfirmGrid");
	//// give location of widget relative to title_rect
	//w->gregion.x1 = CriteriaConfirmGrid->marginx;
	//w->gregion.y1 = CriteriaConfirmGrid->marginy;
	//w->gregion.x2 = w->gregion.x1 + 500;
	//w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - CriteriaConfirmGrid->marginy;
	//w->type = WGT_TEXT;
	//w->wgt_mouse_cb = title_view_cb;
	//strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Do you want to delete", _TRUNCATE);
	////w->visible = 1;
	//w->grid = CriteriaConfirmGrid;

	// add new widget
	/*if (lastw){
		lastw->np = w;
		lastw = w;
	}*/
	for (r = 0; r <CriteriaConfirmGrid->nrows; r++){
		CriteriaConfirmGrid->gridrows[r].row_callback = CriteriaConfirmCallback;
	}

	id->m_poglgui = oglgui;
	id->DataAppGrid = DataAppGrid;
	id->CriteriaConfirmGrid = CriteriaConfirmGrid;
	return(CriteriaConfirmGrid);


}
//mtm 
void color_actions(struct InstanceData* id)
{
	char *cptr1, *cptr2;
	int  red_ival, green_ival, blue_ival, fg_ival, bg_ival;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	cptr1 = SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_fg;
	cptr2 = SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_bg;
	if (strcmp(cptr1, "") != 0)
	{

		red_ival = HexStrtod(&cptr1[0], 2);
		green_ival = HexStrtod(&cptr1[2], 2);
		blue_ival = HexStrtod(&cptr1[4], 2);
		fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;

		SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[0].fg = fg_ival;
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[0] = 1;
	}
	else
	{
		red_ival = 0;
		green_ival = 0;
		blue_ival = 0;
		fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;

		SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[0].fg = fg_ival;
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[0] = 0;
	}

	if (strcmp(cptr2, "") != 0)
	{

		red_ival = HexStrtod(&cptr2[0], 2);
		green_ival = HexStrtod(&cptr2[2], 2);
		blue_ival = HexStrtod(&cptr2[4], 2);
		bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;

		SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[1].bg = bg_ival;
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[1] = 1;
	}
	else
	{
		red_ival = 192;
		green_ival = 192;
		blue_ival = 192;
		bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;

		SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[1].bg = bg_ival;
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[1] = 0;
	}
	SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
	SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
	SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
	SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[3] = 0;
//	SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
	if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
		SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 0;
	if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
		SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 0;
	if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
		SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
	/*if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
		SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = 0;
	if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
		SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 0;*/
}
//mtm: E-Mail callback function
int EMailCallback(struct InstanceData* id, MGLGRID *EMailGrid, int b, int m, int x, int y)
{
	// Toggle the selected action On or OFF
	// allow user to add new actions
	int r, vorow, c;
	char *AppName;
	//	MGLGRID *grid;


	assert(id);

	if (!EMailGrid) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (EMailGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - EMailGrid->gridrect.y1) / EMailGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - EMailGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}


	// toggle the highlight on or off
	if (r < 0 || r >= EMailGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = EMailGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + EMailGrid->iVscrollPos;
	// see which column was selected
	c = MGLCalcCol(EMailGrid, x);

 	if (c == 1 && vorow >= 0 && vorow < EMailGrid->vo->count){

		// user wants to create a new app, so set the text input box and allow him to enter the name of the app
		// popup white text box in place of the "New"
		if (vorow >= 0 && vorow < EMailGrid->vo->count){
			// clear out current string
			EMailGrid->gridcells[r][c].bg = (GLfloat*)white;
			EMailGrid->gridcells[r][c].fg = (GLfloat*)black;
			EMailGrid->textrow = vorow;
			EMailGrid->parent_w->TextActive = TRUE;
			ActiveTextGrid = EMailGrid;
			strcpy_s(EMailGrid->textcolname, sizeof(EMailGrid->textcolname), "EMail Notifications");		
			AppName = VP(EMailGrid->vo, vorow, "EMail Notifications", char);
			if (strcmp(AppName, "Enter EMailID") == 0)
				strcpy_s(VP(EMailGrid->vo, vorow, "EMail Notifications", char), 64, "");


		}

	}
	return(TRUE);
}


//mtm: Subject callback
int SubjectCallback(struct InstanceData* id, MGLGRID *SubjectGrid, int b, int m, int x, int y)
{
	// Toggle the selected action On or OFF
	// allow user to add new actions
	int r, vorow, c;
	char *AppName;
	//	MGLGRID *grid;


	assert(id);

	if (!SubjectGrid) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (SubjectGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - SubjectGrid->gridrect.y1) / SubjectGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - SubjectGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}


	// toggle the highlight on or off
	if (r < 0 || r >= SubjectGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = SubjectGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + SubjectGrid->iVscrollPos;
	// see which column was selected
	c = MGLCalcCol(SubjectGrid, x);

	if (c == 1 && vorow >= 0 && vorow < SubjectGrid->vo->count){

		// user wants to create a new app, so set the text input box and allow him to enter the name of the app
		// popup white text box in place of the "New"
		if (vorow >= 0 && vorow < SubjectGrid->vo->count){
			// clear out current string
			SubjectGrid->gridcells[r][c].bg = (GLfloat*)white;
			SubjectGrid->gridcells[r][c].fg = (GLfloat*)black;
			SubjectGrid->textrow = vorow;
			SubjectGrid->parent_w->TextActive = TRUE;
			ActiveTextGrid = SubjectGrid;
			strcpy_s(SubjectGrid->textcolname, sizeof(SubjectGrid->textcolname), "Subject Line");
			AppName = VP(SubjectGrid->vo, vorow, "Subject Line", char);
			if (strcmp(AppName, "Enter Subject") == 0)
				strcpy_s(VP(SubjectGrid->vo, vorow, "Subject Line", char), 64, "");


		}

	}
	return(TRUE);
}

int ActionsCallback(struct InstanceData* id, MGLGRID *ActionsGrid, int b, int m, int x, int y)
{
	// Toggle the selected action On or OFF
	// allow user to add new actions
	int r, vorow, OnOffColnum, c, t;
	
	char *curstr;
	VO *vcol;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* CriteriaConfirmGrid = (MGLGRID*)id->CriteriaConfirmGrid;

	
	
	
	assert(id);
	assert(ActionsGrid);

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	if (ActionsGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - ActionsGrid->gridrect.y1) / ActionsGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - ActionsGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= ActionsGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = ActionsGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}

	vorow = r - 1 + ActionsGrid->iVscrollPos;

	// see which column was selected
	c = MGLCalcCol(ActionsGrid, x);

	// see if we selected the del column
	vcol = vcol_find(ActionsGrid->vo, "Del");
	if (vcol) {
		// Check to see if the column selected is the delete column
		if (c == vcol->colnum) {
			curstr = VP(ActionsGrid->vo, vorow, "Action", char);
			//
			// If no action name setup then probably clicked on empty entry
			//
			if (curstr == NULL) {
				return (FALSE);
			}

			if (!strcmp(curstr, "New")){
				// don't delete if new
			}
			else if (!strcmp(curstr, ""))
			{
			}
			else {
				if (!CriteriaConfirmGrid)
				{
					for (t = 0; t < ActionsGrid->parentgrid->nActions; t++)
						ActionsGrid->criteria_confirmdelete[t] = 0;
					ActionsGrid->criteria_confirmdelete[vorow] = 1;
					CriteriaConfirmGrid = CreateCriteriaConfirmGrid(id);
					id->CriteriaConfirmGrid = CriteriaConfirmGrid;
					CriteriaConfirmGrid->parent_w->visible = 1;
				}
				else if (CriteriaConfirmGrid)
				{
					for (t = 0; t < ActionsGrid->parentgrid->nActions; t++)
						ActionsGrid->criteria_confirmdelete[t] = 0;
					ActionsGrid->criteria_confirmdelete[vorow] = 1;					
					CriteriaConfirmGrid->parent_w->visible = 1;
				}				
						
			}
		}
	}

	// set the active input row in the grid
	// toggle the on/off for this row

	vcol = vcol_find(ActionsGrid->vo, "OnOff");
	OnOffColnum = vcol->colnum;

	if (c == 0){	

		if (vorow >= 0 && vorow < ActionsGrid->vo->count){
			curstr = VP(ActionsGrid->vo, vorow, "Action", char);
			if (!strcmp(curstr, "New")){
				// clear out current string
				curstr[0] = '\0';
				ActionsGrid->gridcells[vorow + 1][c].bg = (GLfloat*)white;
				ActionsGrid->gridcells[vorow + 1][c].fg = (GLfloat*)black;
				ActionsGrid->textrow = vorow;
				ActionsGrid->parent_w->TextActive = TRUE;
				ActiveTextGrid = ActionsGrid;
				strcpy_s(ActionsGrid->textcolname, sizeof(ActionsGrid->textcolname), "Name");
				MGLFillParameters(id, ActionsGrid->parentgrid->ParametersGrid);
				InitParametersinBuildSummary(id, ActionsGrid->parentgrid);
				InitCriteriainBuildSummary(id, ActionsGrid->parentgrid);
			//	id->click = vorow;
			}
			else if (ActionsGrid->gridcells[vorow + 1][c].bg == (GLfloat*)white)
			{
				ActionsGrid->textrow = vorow;
				ActionsGrid->parent_w->TextActive = TRUE;
				ActiveTextGrid = ActionsGrid;
				MGLFillParameters(id, ActionsGrid->parentgrid->ParametersGrid);
				InitParametersinBuildSummary(id, ActionsGrid->parentgrid);
				InitCriteriainBuildSummary(id, ActionsGrid->parentgrid);
				
			}
			else {
				MGLClearParametersGrid(ActionsGrid->parentgrid); // clear out current settings from action parameters
				ActionsGrid->parentgrid->CurActionNum = vorow;
				MGLFillParametersGrid(ActionsGrid, ActionsGrid->parentgrid->CurActionNum,id);
				CalcColWidths(id, ActionsGrid->parentgrid->ParametersGrid);
				for (x = 0; x < ActionsGrid->vo->count; x++)
				{
					ActionsGrid->vo->row_index[x].fg = MGLColorHex2Int("000000");
				}
				ActionsGrid->vo->row_index[vorow].fg = MGLColorHex2Int("529EFD");
				MGLFillParameters(id, ActionsGrid->parentgrid->ParametersGrid);
				InitParametersinBuildSummary(id, ActionsGrid->parentgrid);
				InitCriteriainBuildSummary(id, ActionsGrid->parentgrid);					
				color_actions(id);
				if (ActionsGrid->parentgrid->Actions[ActionsGrid->parentgrid->CurActionNum].filter_active == 1)
					SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 1;
				else if (ActionsGrid->parentgrid->Actions[ActionsGrid->parentgrid->CurActionNum].filter_active == 0)
					SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 0;

			}
		}
		

	}
	else if (c == OnOffColnum){
		if (vorow >= 0 && vorow < ActionsGrid->vo->count){
			ActionsGrid->parentgrid->iVscrollPos = 0;
			curstr = VP(ActionsGrid->vo, vorow, "OnOff", char);
			if (!strcmp(curstr, "On")){
				// switch to off
				strncpy_s(VP(ActionsGrid->vo, vorow, "OnOff", char), 16, "Off", _TRUNCATE);				
				ActionsGrid->parentgrid->Actions[vorow].IsActive = 0;
			}
			else {
				strncpy_s(VP(ActionsGrid->vo, vorow, "OnOff", char), 16, "On", _TRUNCATE);
				ActionsGrid->parentgrid->Actions[vorow].IsActive = 1;
			}
			SecondTabGrid->updateroi = 1;
		}

		MGLFillParameters(id, ActionsGrid->parentgrid->ParametersGrid);
	}
	//else if (c == 3 && vorow >= 0 && vorow < DataAppGrid->vo->count){
	//	// delete
	//	AppName = VP(DataAppGrid->vo, vorow, "AppName", char);
	//	if (!strcmp(AppName, "New")){
	//		// don't delete if new
	//	}
	//	else {
	//		vo_rm_row(DataAppGrid->vo, vorow);
	//	}
	//}

//	MGLFillParameters(id, ActionsGrid->parentgrid->ParametersGrid);
	// if the user selected a new action, then fill the colors grid with the correct values
	//ActionsGrid->parentgrid->CurActionNum = 0;
	return(TRUE);
}



int redx_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	int nProcessed;

	assert(id);

	// 	return ProcessKeydown( id, VK_UP , 0);
	nProcessed = TRUE;
	return nProcessed;
	// return ProcessKeydown( id, VK_UP , 0);
}


int title_airport_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	int nProcessed;
	// 	return ProcessKeydown( id, VK_UP , 0);
	nProcessed = TRUE;
	return nProcessed;
	// return ProcessKeydown( id, VK_UP , 0);
}


int title_layout_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	int nProcessed;

	assert(id);

	// 	return ProcessKeydown( id, VK_UP , 0);
	nProcessed = TRUE;
	return nProcessed;
	// return ProcessKeydown( id, VK_UP , 0);
}




int title_view_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	int nProcessed;

	assert(id);

	// 	return ProcessKeydown( id, VK_UP , 0);
	nProcessed = TRUE;
	return nProcessed;
}



int up_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	assert(id);

	return ProcessKeydown(id, VK_UP, 0);
}

int down_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	assert(id);

	return ProcessKeydown(id, VK_DOWN, 0);
}

int left_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	assert(id);

	return ProcessKeydown(id, VK_LEFT, 0);
}

int right_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	assert(id);

	return ProcessKeydown(id, VK_RIGHT, 0);
}

int zoom_in_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	assert(id);

	return ProcessKeydown(id, VK_ADD, 0);
}

int zoom_out_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	assert(id);

	return ProcessKeydown(id, VK_SUBTRACT, 0);
}


void CreateActionsGrid(struct InstanceData* id, MGLGRID *parentgrid)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);
	assert(parentgrid);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ActionPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Actions
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ActionsGridWidget");
	w->gregion.x1 = SecondTabGrid->gridrect.x1 + 60;
	w->gregion.y1 = SecondTabGrid->gridrect.y1 + 60;
	w->gregion.x2 = 800;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "ActionsGrid", 6, 3, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->ActionsGrid = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}

		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// w->grid->gridcols[1].val_callback = Cnv2Eq;


	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Action", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "On / Off", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Del", _TRUNCATE);

	// draw the delete red x in the Del column
	// strncpy_s( w->grid->gridcols[5].pngfilename, 32, "redx32.png", _TRUNCATE);
	strncpy_s(w->grid->gridcols[2].pngfilename, sizeof(w->grid->gridcols[2].pngfilename), "alert-red-x.png", _TRUNCATE);
	w->grid->gridcols[2].texturesize = 16;
	w->grid->gridcols[2].img_displaysize = 10;

	// ActionsGrid->gridcells[1][1].bg = (GLfloat*) green;  // green means active default

	// set row callbacks
	for (i = 0; i < parentgrid->ActionsGrid->nrows; i++){
		parentgrid->ActionsGrid->gridrows[i].row_callback = ActionsCallback;
	}



	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->ActionsGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "ViewActionsListTitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->ActionsGrid->marginx;
	w->gregion.y1 = parentgrid->ActionsGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->ActionsGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Criteria", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->ActionsGrid;
	id->m_poglgui = oglgui;
}





void CrActionsVO(MGLGRID *parentgrid)
{

	VO *ActionsVO;

	assert(parentgrid);

	ActionsVO = (VO *)vo_create(0, NULL);
	vo_set(ActionsVO, V_NAME, "ActionsVO", NULL);
	VOPropAdd(ActionsVO, "Name", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(ActionsVO, "OnOff", NTBSTRINGBIND, 16, VO_NO_ROW);
	VOPropAdd(ActionsVO, "Del", INTBIND, -1, VO_NO_ROW);

	parentgrid->ActionsGrid->vo = ActionsVO;

}

void CrABackgroundPanelVO(MGLGRID *parentgrid)
{

	VO *BackgroundPanelVO;

	assert(parentgrid);

	BackgroundPanelVO = (VO *)vo_create(0, NULL);
	parentgrid->BackgroundPanel->vo = BackgroundPanelVO;

}



void FillActionsVO(struct InstanceData* id, MGLGRID *parentgrid)
{
	MGLGRID *ActionsGrid;
	int action;

	assert(id);
	assert(parentgrid);

	ActionsGrid = parentgrid->ActionsGrid;

	// fill a vo with all the unique values in this columns for all rows
	if (!parentgrid->ActionsGrid->vo){
		CrActionsVO(parentgrid);
	}
	else {
		// already created the  ActionsGrid->vo
		return;
	}

	vo_rm_rows(ActionsGrid->vo, 0, ActionsGrid->vo->count);


	if (parentgrid->nActions){
		// fill from Actions values
		for (action = 0; action < parentgrid->nActions; action++){
			vo_alloc_rows(ActionsGrid->vo, 1);
			strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "Name", char), 64, parentgrid->Actions[action].ActionName, _TRUNCATE);
			if (parentgrid->Actions[action].IsActive){
				strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "OnOff", char), 16, "On", _TRUNCATE);
			}
			else {
				strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "OnOff", char), 16, "On", _TRUNCATE);
			}
		}
	}

	// create "NEW" row, that user will click on to add an action
	vo_alloc_rows(ActionsGrid->vo, 1);
	strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "Name", char), 64, "New", _TRUNCATE);
	strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "OnOff", char), 16, "On", _TRUNCATE);


}

//added mtm//

void FillBackgroundPanelVO(struct InstanceData* id, MGLGRID *parentgrid)
{
	MGLGRID *BackgroundPanel;


	assert(id);
	assert(parentgrid);

	BackgroundPanel = parentgrid->BackgroundPanel;

	// fill a vo with all the unique values in this columns for all rows
	if (!parentgrid->BackgroundPanel->vo){
		CrABackgroundPanelVO(parentgrid);
	}
	else {
		// already created the  ActionsGrid->vo
		return;
	}

	vo_rm_rows(BackgroundPanel->vo, 0, BackgroundPanel->vo->count);




}




void MGLGridFree(MGLGRID *FilterGrid)
{


	// free the resources of the grid


}




GLfloat *FilterFGCallback(MGLGRID *grid, int rownum)
{
	int vorow;
	GLfloat *fg;
	MGLGRID *parentgrid;

	assert(grid);

	if (rownum == 0){
		// title row
		fg = (GLfloat*)white;
		return(fg);
	}


	// we want the parent grid
	parentgrid = grid->parentgrid;  // top level grid that contains the data

	vorow = rownum - 1 + parentgrid->FilterGrid->iVscrollPos;

	// figure out which fg color
	if (vorow < 0 || vorow > parentgrid->FilterGrid->vo->count){
		if (rownum == 0){
			fg = (GLfloat*)white;
		}
		else {
			fg = (GLfloat*)white;
		}
		return(fg);
	}
	if (vorow < parentgrid->FilterGrid->vo->count && VV(parentgrid->FilterGrid->vo, vorow, "ValActive", int)){
		// row is active 
		fg = (GLfloat*)white;
		return(fg);
	}
	else {
		fg = (GLfloat*)gridgreyelev;
		return(fg);
	}
}



GLfloat *FilterBGCallback(MGLGRID *grid, int rownum)
{
	int vorow;
	GLfloat *bg;
	MGLGRID *parentgrid;

	assert(grid);

	// we want the parent grid
	parentgrid = grid->parentgrid;  // top level grid that contains the data

	vorow = rownum - 1 + parentgrid->FilterGrid->iVscrollPos;


	// figure out which bg color
	if (rownum == 0)
	{
		bg = (GLfloat*)Blue_title;
		return(bg);
	}

	if (vorow < 0 || !parentgrid->FilterGrid->vo || vorow >= parentgrid->FilterGrid->vo->count){
		bg = (GLfloat*)color868686;
		return(bg);
	}
	if (!parentgrid->FilterGrid->vo || !parentgrid->FilterGrid->vo->count){
		bg = (GLfloat*)red;
		return(bg);
	}
	if (vorow < parentgrid->FilterGrid->vo->count && VV(parentgrid->FilterGrid->vo, vorow, "ValActive", int)){
		// row is active 
		bg = (GLfloat*)gridgreyelev;
		return(bg);
	}
	else {
		bg = (GLfloat*)color868686;
		return(bg);
	}

}





int MGLNextVisCol(MGLGRID *topgrid, int vorow)
{
	char *colname;
	int i;
	char labelname[64];

	assert(topgrid);

	colname = VP(topgrid->SortAvailableGrid->vo, vorow, "Column", char);
	setnames(topgrid, colname, labelname);

	// find out original order number for this column, as user set up in ColOrderNames
	for (i = 0; i < topgrid->ncols; i++){
		if (!strcmp(labelname, topgrid->ColOrderNames[i])){
			return(i);
		}
	}

	return(-1); // test
}

//mtm

int MGLNextVisCol_dataapp(MGLGRID *topgrid, int vorow)
{
	char *colname;
	int i;

	assert(topgrid);

	colname = VP(topgrid->vo, vorow, "Column", char);

	// find out original order number for this column, as user set up in ColOrderNames
	for (i = 0; i < topgrid->ncols; i++){
		if (!strcmp(colname, topgrid->ColOrderNames[i])){
			return(i);
		}
	}

	return(-1); // test
}



VO *CrFilterVO()
{
	VO *FilterVO;

	FilterVO = (VO *)vo_create(0, NULL);
	vo_set(FilterVO, V_NAME, "FilterVO", NULL);
	VOPropAdd(FilterVO, "valstr", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(FilterVO, "ValActive", INTBIND, -1, VO_NO_ROW);  // this row's value is Active and should NOT be filtered out
	vo_set(FilterVO, V_ORDER_COLS, "valstr", NULL, NULL);

	return(FilterVO);

}



void UniqueFilterCB(struct InstanceData* id, MGLGRID *grid, int c, char *colname)
{
	int i, f, matchfound;
	VO *vcol;
	char *valstr;


	assert(id);
	assert(grid);

	// find all unique values in the grid and create a row for each value
	if (!grid->FilterGrid->vo){
		grid->FilterGrid->vo = CrFilterVO();
	}

	vo_rm_rows(grid->FilterGrid->vo, 0, grid->FilterGrid->vo->count);

	if (!grid) return;

	if (grid->vo){

		for (i = 0; i < grid->vo->count; i++){

			if (strlen(grid->ColOrderNames[c])){
				colname = grid->ColOrderNames[c];
				vcol = vcol_find(grid->vo, colname);
			}
			else {
				vcol = V_ROW_PTR(grid->vo->vcols, c);
			}
			if (vcol){
				valstr = get_vcol_valstr(vcol, V_ROW_PTR(grid->vo, i), TRUE);
				if (grid->gridcols[c].val_callback){
					valstr = (char *)grid->gridcols[c].val_callback(id,valstr);
				}
				// see if the value is already in the FilterGrid->vo
				for (f = 0, matchfound = FALSE; f < grid->FilterGrid->vo->count; f++){
					if (!strcmp(valstr, VP(grid->FilterGrid->vo, f, "valstr", char))){
						// already matched
						matchfound = TRUE;
						break;
					}
				}
				if (matchfound == FALSE){
					vo_alloc_rows(grid->FilterGrid->vo, 1);
					strncpy_s(VP(grid->FilterGrid->vo, grid->FilterGrid->vo->count - 1, "valstr", char), 64, valstr, _TRUNCATE);
					VV(grid->FilterGrid->vo, grid->FilterGrid->vo->count - 1, "ValActive", int) = TRUE;
				}
			}
		}
	}
	else {
		// use values stored in grid??? not really needed for non-vo grids ??? may have to implement later
		return;
	}

	//vo_order(grid->FilterGrid->vo);	
	grid->FilterGrid->vo = grid->FilterGrid->vo;
	strncpy_s(grid->FilterGrid->vo->name, sizeof(grid->FilterGrid->vo->name), grid->ColOrderNames[c], _TRUNCATE);


}





char *CnvElapsedSecs2HHMMSS(struct InstanceData* id, char *valstr)
{
	static char timebuf[64];
	time_t secs;
	char *str;

	assert(valstr);

	secs = atol(valstr);

	if (secs > 0){
		str = VOGMTime(timebuf, secs, "%H:%M:%S");
	}
	else {
		return("");
	}

	return(timebuf);
}



char *Cnv2YN(struct InstanceData* id, char *valstr)
{
	int ival;

	ival = atol(valstr);

	if (ival == -1){
		// do not show, since this column did not have a valid filter
		return(" ");
	}
	else if (ival){
		return("Y");
	}
	else {
		return("N");
	}

	return(NULL);
}


char *Cnv2List(struct InstanceData* id, char *valstr)
{

	return("List");
}

//mtm
char *HideOption(char *valstr)
{

	return("      Hide");
}
//mtm
char *ShowOption(char *valstr)
{

	return("      Show");
}

char *Cnv2BT(char *valstr)
{

	return(" <> ");
}


char *Cnv2GE(char *valstr)
{

	return(" >= ");
}


char *Cnv2LE(char *valstr)
{

	return(" <= ");
}


char *Cnv2LT(char *valstr)
{

	return(" < ");
}


char *Cnv2GT(char *valstr)
{

	return(" > ");
}


char *Cnv2NE(char *valstr)
{

	return(" != ");
}



char *Cnv2Eq(char *valstr)
{

	return(" = ");
}


char *Cnv2Plus(struct InstanceData* id, char *valstr)
{

	return(" + ");
}




char *Cnv2Minus(struct InstanceData* id, char *valstr)
{

	return(" - ");
}



char *Cnv2SortPriority(struct InstanceData* id, char *valstr)
{
	int ival;

	ival = atol(valstr);

	if (ival == -1){
		// do not show, since this column did not have a valid filter
		return(" ");
	}
	else if (ival == 0){
		// no priority
		return(" ");
	}
	else  if (ival > 0){
		return(valstr);
	}
	else {
		return(" ");
	}

}



char *Cnv2SortAscDsc(struct InstanceData* id, char *valstr)
{
	int ival;

	assert(valstr);

	ival = atol(valstr);

	if (ival == -1){
		// do not show, since this column did not have a valid filter
		return(" ");
	}
	else if (ival == 0){
		return("Off");
	}
	else  if (ival == 1){
		return("Asc");
	}
	else  if (ival == 2){
		return("Dsc");
	}
	else {
		return(" ");
	}


}



char *Cnv2HideShow(struct InstanceData* id, char *valstr)
{
	int ival;

	assert(valstr);

	ival = atol(valstr);

	//if (strcmp(valstr, "Show") == 0)
	//	ival = 0;

	//if (strcmp(valstr, "Hide") == 0)
	//	ival = 1;

	if (ival == -1){
		// do not show, since this column did not have a valid filter
		return(" ");
	}
	else if (ival){
		return("Show");
	}
	else {
		return("Hide");
	}
}


//mtm//
char *Cnv2HideShow_mtm(char *valstr)
{

	int ival;

	assert(valstr);

	ival = atol(valstr);

	//if (strcmp(valstr, "Show") == 0)
	//	ival = 0;

	//if (strcmp(valstr, "Hide") == 0)
	//	ival = 1;

	if (ival == -1){
		// do not show, since this column did not have a valid filter
		return(" ");
	}
	else if (ival){
		return("Show");
	}
	else {
		return("Hide");
	}
}



int FilterCallback(struct InstanceData* id, MGLGRID *FilterGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, vorow, curval;
	

	assert(id);
	assert(FilterGrid);

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	if (FilterGrid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - FilterGrid->gridrect.y1) / FilterGrid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - FilterGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= FilterGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = FilterGrid->nrows - r - 1;

	if (r == 0){
		// user selected "select All"
		for (vorow = 0; vorow < FilterGrid->vo->count; vorow++){
			VV(FilterGrid->vo, vorow, "ValActive", int) = TRUE;
		}
		for (r = 0; r < FilterGrid->nrows; r++){
			FilterGrid->gridrows[r].fg = (GLfloat*)black;
			FilterGrid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		}
		return(TRUE);
	}

	vorow = r - 1 + FilterGrid->iVscrollPos;

	if (vorow >= 0 && vorow < FilterGrid->vo->count){
		curval = VV(FilterGrid->vo, vorow, "ValActive", int);
		if (curval){
			// turn off
			// reverse the video colors
			VV(FilterGrid->vo, vorow, "ValActive", int) = FALSE;
			//OutputDebugString("Turn off Filter color\n");
			FilterGrid->gridrows[r].fg = (GLfloat*)black;
			FilterGrid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		}
		else {
			VV(FilterGrid->vo, vorow, "ValActive", int) = TRUE;
			//OutputDebugString("Turn ON Filter color\n");
			FilterGrid->gridrows[r].fg = (GLfloat*)black;
			FilterGrid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;

		}
		//strcpy_s(valuesource, 64, VP(FilterGrid->vo, vorow, "valstr", int));
		//strcpy_s(VP(FilterGrid->parentgrid->ParametersGrid->vo, vorow, "Value1", char), 64, valuesource);
	}

	return(TRUE);
}




int filter_submit_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g)
{
	MGLGRID *grid, *parentgrid;
	char *fieldname, *valstr;
	int i, filtercount;
	char tmpbuf[128];

	assert(id);
	assert(w);

	// left off here ......  the user has submitted the filter changes, so use these new values

	//   fill in config structure, field_name, and select_value[2048], comman separated values 

	// which column does this filter pertain to???

	// get data from list from FilterGrid->vo, columns "valstr" and "CalActive"

	grid = w->grid;


	// we want the parent grid
	parentgrid = grid->parentgrid;  // top level grid that contains the data


	// the column name that this FilterGrid->vo refers to is stored in FilterGrid->vo->name 
	fieldname = parentgrid->FilterGrid->vo->name;

	for (i = 0, filtercount = -1; parentgrid && i < parentgrid->Actions[parentgrid->CurActionNum].gridconfig->nFilters; i++){
		if (!strncmp(parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[i].field_name, fieldname, 16)){
			// found the column name for the values in FilterGrid that we will filter with
			filtercount = i;
			break;
		}
	}

	if (filtercount == -1){
		// this is a new filter column to add
		if (parentgrid->Actions[parentgrid->CurActionNum].gridconfig->nFilters < 7){
			(parentgrid->Actions[parentgrid->CurActionNum].gridconfig->nFilters)++;
		}
		filtercount = parentgrid->Actions[parentgrid->CurActionNum].gridconfig->nFilters - 1;
	}

	// now create the select_value buffer with the comma separated values
	if (filtercount >= 0 && filtercount < 7){
		strncpy_s(parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].field_name,
			sizeof(parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].field_name), fieldname, _TRUNCATE);


		parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].select_value[0] = '\0';
		for (i = 0; i < parentgrid->FilterGrid->vo->count; i++){
			if (VV(parentgrid->FilterGrid->vo, i, "ValActive", int) == TRUE){
				valstr = VP(parentgrid->FilterGrid->vo, i, "valstr", char);
				_snprintf_s(tmpbuf, _countof(tmpbuf), _TRUNCATE, "%s,", valstr);
				strncat_s(parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].select_value,
					sizeof(parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].select_value),
					tmpbuf, _TRUNCATE);
			}
		}


		// recalculate the searchvo
		if (parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].searchvo){
			vo_free(parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].searchvo);
		}
		parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].searchvo = NULL;
		parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].searchvo =
			MGLCrSearchVO((char *)&parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].select_value);
		parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].filter_type = FTYPE_LIST;
	}

	strncpy_s(parentgrid->FilterGrid->vo->name, sizeof(parentgrid->FilterGrid->vo->name),
		parentgrid->Actions[parentgrid->CurActionNum].gridconfig->filters[filtercount].field_name, _TRUNCATE);

	// hide the grid
	parentgrid->FilterGrid->parent_w->visible = FALSE;

	return ProcessKeydown(id, VK_UP, 0);
}





MGLGRID *CrFilterGrid(struct InstanceData* id, MGLGRID *parentgrid)
{
	int r, i;
	widgets *w, *lastw;
	MGLGRID *FilterGrid;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create MGLGrid
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "FilterGridWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->gregion.x2 = 200;
	w->gregion.y2 = 570;
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "FilterGrid", 10, 2, w->gregion.x1, w->gregion.y1);

	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust

	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = TRUE;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget


	parentgrid->FilterGrid = w->grid;
	FilterGrid = parentgrid->FilterGrid;

	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)color868686;
	w->grid->fg = (GLfloat*)white;
	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;
	// add new widget
	if (lastw){
		lastw->np = w;
	}

	w->grid->gridcols[1].val_callback = Cnv2HideShow;

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Select All", _TRUNCATE);
	// we also need to fetch initial filter values from user settings DB

	// set row callbacks
	for (i = 0; i < FilterGrid->nrows; i++){
		FilterGrid->gridrows[i].row_callback = FilterCallback;
	}


	// add next widget to linked list
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "FilterTextWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = FilterGrid->marginx;
	w->gregion.y1 = FilterGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 60;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - FilterGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = filter_submit_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Submit", _TRUNCATE);
	w->visible = 1;
	w->grid = FilterGrid;
	FilterGrid->title_widgets = w;

	id->m_poglgui = oglgui;
	return(FilterGrid);

}




MGLGRID *CreateFilterGrid(struct InstanceData* id, MGLGRID *parentgrid)
{
	MGLGRID *FilterGrid = parentgrid->FilterGrid;

	assert(id);

	//  Filter by Regions -- display grid of one column, each row is a region name
	// user clicks on rows to turn them off, or on  , highlighted means on, black bg means off 

	//  or two columns     ON   TWY A
	//                     OFF   TWY Z  -- still a single column,  we put in text of ON or OFF
	//   default is all rows on
	// 


	// we will load in all possible regions for an airport
	if (!FilterGrid || FilterGrid->nrows != id->m_nRegions){
		// need to create the structure for this grid
		if (FilterGrid){
			MGLGridFree(FilterGrid);
		}
		FilterGrid = CrFilterGrid(id, parentgrid);
	}

	if (!FilterGrid) return(NULL);

	return(FilterGrid);
}





void DisabledFilterCB(struct InstanceData* id, MGLGRID *topgrid, int c)
{

	assert(id);

	if (!topgrid) return;

	// find all unique values in the grid and create a row for each value
	if (!topgrid->FilterGrid->vo){
		topgrid->FilterGrid->vo = CrFilterVO();
	}

	vo_rm_rows(topgrid->FilterGrid->vo, 0, topgrid->FilterGrid->vo->count);

	vo_alloc_rows(topgrid->FilterGrid->vo, 1);
	strncpy_s(VP(topgrid->FilterGrid->vo, topgrid->FilterGrid->vo->count - 1, "valstr", char), 64, "Filter Not Available", _TRUNCATE);
	VV(topgrid->FilterGrid->vo, topgrid->FilterGrid->vo->count - 1, "ValActive", int) = -1;

	// make col width wider to display this error message
	// FilterGrid->gridcols[0].width = 100;
	MGLGridResizeCol(id, topgrid->FilterGrid, 0, 200);

	topgrid->FilterGrid->vo = topgrid->FilterGrid->vo;
}



void FillFilterVO(struct InstanceData* id, MGLGRID *topgrid, char *colname)
{
	int colnum, i;

	assert(id);
	assert(topgrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!topgrid->FilterGrid->vo){
		topgrid->FilterGrid->vo = CrFilterVO();
	}

	vo_rm_rows(topgrid->FilterGrid->vo, 0, topgrid->FilterGrid->vo->count);

	for (i = 0; i < topgrid->ncols; i++){
		if (!strcmp(colname, topgrid->ColOrderNames[i])){
			break;
		}
	}

	if (i < topgrid->ncols){
		colnum = i;
	}
	else {
		// bad 
		return;
	}

	if (topgrid->gridcols[colnum].filter_callback){
		topgrid->gridcols[colnum].filter_callback(id, topgrid, colnum, colname);
	}
	else {
		// default, throw up disallowed message
		DisabledFilterCB(id, topgrid, colnum);
	}


}





void MGLGridFilterShow(struct InstanceData* id, MGLGRID *topgrid, char *colname)
{
	MGLGRID *FilterGrid = topgrid->FilterGrid;

	assert(id);

	if (!FilterGrid){
		FilterGrid = CreateFilterGrid(id, topgrid);
	}
	if (FilterGrid){
		FilterGrid->parent_w->visible = TRUE;  // set parent widget to display the grid
		MGLGridMove(id, FilterGrid, topgrid->gridrect.x2 + MGL_GRID_DEF_ROW_HEIGHT, topgrid->gridrect.y1);
	}
	FillFilterVO(id, topgrid, colname);

}



int ParametersCallback(struct InstanceData* id, MGLGRID *ParametersGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, c, vorow, i, colnum, valcolnum, listcolnum, gridrow;
	char *curstr, *colname;
	VO *vcol;
	extern MGLGRID *CreateOperatorMenu(struct InstanceData* id, MGLGRID *parentgrid, int x, int y);

	assert(id);

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	if (ParametersGrid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - ParametersGrid->gridrect.y1) / ParametersGrid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - ParametersGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= ParametersGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = ParametersGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}

	vorow = r - 1 + ParametersGrid->iVscrollPos;

	// set the active input row in the grid
	ParametersGrid->textrow = vorow;
	ParametersGrid->parent_w->TextActive = TRUE;
	ActiveTextGrid = ParametersGrid;

	// clear out current string

	if (vorow >= 0 && vorow < ParametersGrid->vo->count){
		curstr = VP(ParametersGrid->vo, vorow, "Value1", char);
		curstr[0] = '\0';
	}

	// see which column was selected
	c = MGLCalcCol(ParametersGrid, x);

	if (c == 1){
		// user selected the operators grid, so popup the operators menu
		//if (!ParametersGrid->parentgrid->OperatorGrid){
		//	CreateOperatorMenu(id, ParametersGrid->parentgrid, x, y);
		//}
		//else
		if (ParametersGrid->parentgrid->OperatorGrid){
			if (ParametersGrid->parentgrid->OperatorGrid->parent_w->visible == 1)
				ParametersGrid->parentgrid->OperatorGrid->parent_w->visible = 0;
		}
		/*IconsGrid(id,SecondTabGrid->childgrid, 1);
		SecondTabGrid->childgrid->IconGrid->parent_w->visible = TRUE;
		MGLGridMove(id, SecondTabGrid->childgrid->IconGrid, 0, 2000);*/
		CreateOperatorMenu(id, ParametersGrid->parentgrid, x, y);

		if (ParametersGrid->parentgrid->OperatorGrid){
			//SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = 0;
			ParametersGrid->parentgrid->OperatorGrid->parent_w->visible = TRUE;
			MGLGridMove(id, ParametersGrid->parentgrid->OperatorGrid, x, y - 126);
			//SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = 1;
		}
	}

	// value1 column number?
	vcol = vcol_find(ParametersGrid->vo, "Value1");
	valcolnum = vcol->colnum;

	vcol = vcol_find(ParametersGrid->vo, "List");
	listcolnum = vcol->colnum;

	// reset all other colors
	for (i = 1; i < valcolnum; i++){
		ParametersGrid->gridcells[vorow + 1][i].bg = NULL;
	}


	// set Value1 fg and bg
	// turn off all other Value1 cells to default fg and bg
	for (r = 1; r < ParametersGrid->nrows; r++){
		ParametersGrid->gridcells[r][valcolnum].bg = (GLfloat*)NULL;
		ParametersGrid->gridcells[r][valcolnum].fg = (GLfloat*)NULL;
	}

	// turn off any operator values highlighted, but without a value set
	for (r = 0; r < ParametersGrid->vo->count; r++){
		// if an operator is selected in prior row, but there is no value, then reset the operator to default BG
		curstr = VP(ParametersGrid->vo, r, "Value1", char);
		if (!strlen(curstr)){
			// no value in prior row, so set the operator background to default
			// set all operators to blank
			for (colnum = 1; colnum < valcolnum; colnum++){
				ParametersGrid->gridcells[r + 1][colnum].bg = (GLfloat*)NULL;
			}
		}
	}

	// add one to row to skip column row
	if (c > 0 && c < valcolnum){
		// old way , set gridcells color, but won't work if scrolling ParametersGrid->gridcells[vorow + 1][c].bg = (GLfloat*) orange;
		ParametersGrid->vo->row_index[vorow].bg = iOrange;
	}

	if (c == listcolnum){
		// pop up filter grid
		// find out column name that we will match to
		colname = VP(ParametersGrid->vo, vorow, "Column", char);

		// 
		// Prevent I.E. Crash if colname is null
		//
		if (colname) {
			MGLGridFilterShow(id, ParametersGrid->parentgrid, colname);
		}
	}

	if (c > 0 && c < listcolnum){
		// set Value1 to white to look like a text entry space
		gridrow = vorow - ParametersGrid->iVscrollPos + 1;
		// 		vorow = r - 1 + ParametersGrid->iVscrollPos;
		ParametersGrid->gridcells[gridrow][valcolnum].bg = (GLfloat*)white;
		ParametersGrid->gridcells[gridrow][valcolnum].fg = (GLfloat*)black;
	}


	// MGLFillParameters(id, ParametersGrid );
	return(TRUE);
}

void SetColorCol(MGLGRID *AlertsColorGrid)
{
	strcpy_s(AlertsColorGrid->row[0].col[0], 20, "yellow1");
	strcpy_s(AlertsColorGrid->row[0].col[1], 20, "yellow2");
	strcpy_s(AlertsColorGrid->row[0].col[2], 20, "yellow3");
	strcpy_s(AlertsColorGrid->row[0].col[3], 20, "yellow4");
	strcpy_s(AlertsColorGrid->row[0].col[4], 20, "yellow5");

	strcpy_s(AlertsColorGrid->row[1].col[0], 20, "red1");
	strcpy_s(AlertsColorGrid->row[1].col[1], 20, "red2");
	strcpy_s(AlertsColorGrid->row[1].col[2], 20, "red3");
	strcpy_s(AlertsColorGrid->row[1].col[3], 20, "red4");
	strcpy_s(AlertsColorGrid->row[1].col[4], 20, "red5");

	strcpy_s(AlertsColorGrid->row[2].col[0], 20, "violet1");
	strcpy_s(AlertsColorGrid->row[2].col[1], 20, "violet2");
	strcpy_s(AlertsColorGrid->row[2].col[2], 20, "violet3");
	strcpy_s(AlertsColorGrid->row[2].col[3], 20, "violet4");
	strcpy_s(AlertsColorGrid->row[2].col[4], 20, "violet5");

	strcpy_s(AlertsColorGrid->row[3].col[0], 20, "green1");
	strcpy_s(AlertsColorGrid->row[3].col[1], 20, "green2");
	strcpy_s(AlertsColorGrid->row[3].col[2], 20, "green3");
	strcpy_s(AlertsColorGrid->row[3].col[3], 20, "green4");
	strcpy_s(AlertsColorGrid->row[3].col[4], 20, "green5");

	strcpy_s(AlertsColorGrid->row[4].col[0], 20, "blue1");
	strcpy_s(AlertsColorGrid->row[4].col[1], 20, "blue2");
	strcpy_s(AlertsColorGrid->row[4].col[2], 20, "blue3");
	strcpy_s(AlertsColorGrid->row[4].col[3], 20, "blue4");
	strcpy_s(AlertsColorGrid->row[4].col[4], 20, "blue5");

	//	return Color;




}


void MGLSetAlertsColor(struct InstanceData* id, MGLGRID *AlertsColorGrid, char *xcolname, int vorow)
{
	MGLGRID  *bottomgrid;
	int colsize;
	char *cptr;


	VO *AlertsColorVO;

	AlertsColorVO = (VO *)vo_create(0, NULL);
	assert(AlertsColorGrid);

	// set the color values in the actions structure

	bottomgrid = AlertsColorGrid->parentgrid;
	if (vorow == 0) {
		// fg color
		colsize = sizeof(bottomgrid->Actions[bottomgrid->CurActionNum].row_fg);
		cptr = bottomgrid->Actions[bottomgrid->CurActionNum].row_fg;
		//cptr2 = bottomgrid->Actions[bottomgrid->CurActionNum].row_fg;
		if (!strcmp(xcolname, "Yellow1")){
			strncpy_s(cptr, colsize, "FFFF99", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
		}
		else if (!strcmp(xcolname, "Yellow2")){
			strncpy_s(cptr, colsize, "FFFF00", _TRUNCATE);
		}
		else if (!strcmp(xcolname, "Yellow3")){
			strncpy_s(cptr, colsize, "FFCC00", _TRUNCATE);
		}
		else if (!strcmp(xcolname, "Yellow4")){
			strncpy_s(cptr, colsize, "FF9900", _TRUNCATE);
		}
		else if (!strcmp(xcolname, "Yellow5")){
			strncpy_s(cptr, colsize, "996600", _TRUNCATE);
		}
	}



}

void MGLSetActionColor(struct InstanceData* id, MGLGRID *AlertsColorGrid, char *xcolname, int vorow)
{
	MGLGRID *topgrid, *bottomgrid;
	int colsize;
	char *cptr;


	VO *AlertsColorVO;

	AlertsColorVO = (VO *)vo_create(0, NULL);
	assert(AlertsColorGrid);

	// set the color values in the actions structure
	topgrid = AlertsColorGrid->parentgrid->parentgrid;
	bottomgrid = AlertsColorGrid->parentgrid;

	//Check if Foreground is selected or background is selected in Alerts grid and then fix the color selected//

	if (AlertsColorGrid->row_number == 1){

		if (vorow == 0) {
			// bg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_bg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_bg;
			//cptr2 = bottomgrid->Actions[bottomgrid->CurActionNum].row_fg;
			if (!strcmp(xcolname, "yellow1")){
				strncpy_s(cptr, colsize, "FFFF99", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "yellow2")){
				strncpy_s(cptr, colsize, "FFFF00", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "yellow3")){
				strncpy_s(cptr, colsize, "FFCC00", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "yellow4")){
				strncpy_s(cptr, colsize, "FF9900", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "yellow5")){
				strncpy_s(cptr, colsize, "996600", _TRUNCATE);
			}


		}


		if (vorow == 1){
			// bg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_bg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_bg;
			if (!strcmp(xcolname, "red1")){
				strncpy_s(cptr, colsize, "FFDB9D", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "red2")){
				strncpy_s(cptr, colsize, "FFCC66", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "red3")){
				strncpy_s(cptr, colsize, "FF9933", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "red4")){
				strncpy_s(cptr, colsize, "FF794B", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "red5")){
				strncpy_s(cptr, colsize, "FF3300", _TRUNCATE);
			}

		}

		if (vorow == 2) {
			// bg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_bg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_bg;
			if (!strcmp(xcolname, "violet1")){
				strncpy_s(cptr, colsize, "DEFFFF", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "violet2")){
				strncpy_s(cptr, colsize, "FFCCFF", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "violet3")){
				strncpy_s(cptr, colsize, "CCCCFF", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "violet4")){
				strncpy_s(cptr, colsize, "9999FF", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "violet5")){
				strncpy_s(cptr, colsize, "6666CC", _TRUNCATE);
			}

		}
		if (vorow == 3) {
			// bg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_bg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_bg;
			if (!strcmp(xcolname, "green1")){
				strncpy_s(cptr, colsize, "CCFFCC", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "green2")){
				strncpy_s(cptr, colsize, "99FF66", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "green3")){
				strncpy_s(cptr, colsize, "66CC33", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "green4")){
				strncpy_s(cptr, colsize, "009900", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "green5")){
				strncpy_s(cptr, colsize, "006600", _TRUNCATE);
			}

		}
		if (vorow == 4) {
			// bg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_bg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_bg;
			if (!strcmp(xcolname, "blue1")){
				strncpy_s(cptr, colsize, "3E9ADE", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "blue2")){
				strncpy_s(cptr, colsize, "009ADE", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "blue3")){
				strncpy_s(cptr, colsize, "0066CC", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "blue4")){
				strncpy_s(cptr, colsize, "003399", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "blue5")){
				strncpy_s(cptr, colsize, "333366", _TRUNCATE);
			}

		}
	}

	if (AlertsColorGrid->row_number == 0){

		if (vorow == 0) {
			// fg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_fg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_fg;
			//cptr2 = bottomgrid->Actions[bottomgrid->CurActionNum].row_fg;
			if (!strcmp(xcolname, "yellow1")){
				strncpy_s(cptr, colsize, "FFFF99", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "yellow2")){
				strncpy_s(cptr, colsize, "FFFF00", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "yellow3")){
				strncpy_s(cptr, colsize, "FFCC00", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "yellow4")){
				strncpy_s(cptr, colsize, "FF9900", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "yellow5")){
				strncpy_s(cptr, colsize, "996600", _TRUNCATE);
			}


		}


		if (vorow == 1){
			// fg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_fg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_fg;
			if (!strcmp(xcolname, "red1")){
				strncpy_s(cptr, colsize, "FFDB9D", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "red2")){
				strncpy_s(cptr, colsize, "FFCC66", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "red3")){
				strncpy_s(cptr, colsize, "FF9933", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "red4")){
				strncpy_s(cptr, colsize, "FF794B", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "red5")){
				strncpy_s(cptr, colsize, "FF3300", _TRUNCATE);
			}

		}

		if (vorow == 2) {
			// fg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_fg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_fg;
			if (!strcmp(xcolname, "violet1")){
				strncpy_s(cptr, colsize, "DEFFFF", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "violet2")){
				strncpy_s(cptr, colsize, "FFCCFF", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "violet3")){
				strncpy_s(cptr, colsize, "CCCCFF", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "violet4")){
				strncpy_s(cptr, colsize, "9999FF", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "violet5")){
				strncpy_s(cptr, colsize, "6666CC", _TRUNCATE);
			}

		}
		if (vorow == 3) {
			// fg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_fg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_fg;
			if (!strcmp(xcolname, "green1")){
				strncpy_s(cptr, colsize, "CCFFCC", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "green2")){
				strncpy_s(cptr, colsize, "99FF66", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "green3")){
				strncpy_s(cptr, colsize, "66CC33", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "green4")){
				strncpy_s(cptr, colsize, "009900", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "green5")){
				strncpy_s(cptr, colsize, "006600", _TRUNCATE);
			}

		}
		if (vorow == 4) {
			// fg color
			colsize = sizeof(topgrid->Actions[topgrid->CurActionNum].row_fg);
			cptr = topgrid->Actions[topgrid->CurActionNum].row_fg;
			if (!strcmp(xcolname, "blue1")){
				strncpy_s(cptr, colsize, "3E9ADE", _TRUNCATE); // use same red as Larry's text red so it shows up against grey bars
			}
			else if (!strcmp(xcolname, "blue2")){
				strncpy_s(cptr, colsize, "009ADE", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "blue3")){
				strncpy_s(cptr, colsize, "0066CC", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "blue4")){
				strncpy_s(cptr, colsize, "003399", _TRUNCATE);
			}
			else if (!strcmp(xcolname, "blue5")){
				strncpy_s(cptr, colsize, "333366", _TRUNCATE);
			}

		}
	}


}



int MGLColorHex2Int(char *hexcolorstr)
{
	int red_ival, green_ival, blue_ival, fg_ival;

	// convert hex string to ival 

	if (hexcolorstr && strlen(hexcolorstr)){
		// fg is set, format char row_fg[7];  // in rgb hex  RRGGBB, FF00FE 
		// 	pColor[0] = (GLfloat) HexStrtod( &str[1], 2 ) / 255.0f;
		red_ival = HexStrtod(&hexcolorstr[0], 2);
		green_ival = HexStrtod(&hexcolorstr[2], 2);
		blue_ival = HexStrtod(&hexcolorstr[4], 2);

		fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
		return(fg_ival);
	}

	return(0);

}




int AlertsColorCallback_colorpalettefg(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, c, vorow;
	int  red_ival, green_ival, blue_ival, fg_ival, bg_ival;
	char *cptr1, *cptr2;
	char  *xcolname;
	extern int re_exec(const char *lp);
	extern char *re_comp(const char *pat);
	MGLGRID *topgrid = AlertsColorGrid->parentgrid->parentgrid;
	MGLGRID *bottomgrid = topgrid->AlertsColorGrid;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;


	//row_number stores if fg or bg was selected in the Alerts grid
	if (strcmp(AlertsColorGrid->name, "colormatrixbackground") == 0)
		AlertsColorGrid->row_number = 1;
	if (strcmp(AlertsColorGrid->name, "colormatrixforeground") == 0)
		AlertsColorGrid->row_number = 0;



	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	if (AlertsColorGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / AlertsColorGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= AlertsColorGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	vorow = AlertsColorGrid->nrows - r - 1;

	// see which column was selected
	c = MGLCalcCol(AlertsColorGrid, x);

	SetColorCol(AlertsColorGrid);

	xcolname = AlertsColorGrid->row[vorow].col[c];

	if (AlertsColorGrid->row_number == 0){

		MGLSetActionColor(id, AlertsColorGrid, xcolname, vorow);
		//topgrid->Actions[topgrid->CurActionNum].ActionType = 5;
		//	topgrid->Actions[topgrid->CurActionNum].IsActive = 1;

		topgrid->Actions[topgrid->CurActionNum].ActionType |= ACTION_COLOR_FG;
		/*	strcpy_s(topgrid->Actions[topgrid->CurActionNum].ActionName, sizeof(topgrid->Actions[topgrid->CurActionNum].ActionName),
				curstr); */
		//strcpy_s(topgrid->Actions[topgrid->CurActionNum].ActionName, sizeof(topgrid->Actions[topgrid->CurActionNum].ActionName), topgrid->name);

		cptr1 = topgrid->Actions[topgrid->CurActionNum].row_fg;

		red_ival = HexStrtod(&cptr1[0], 2);
		green_ival = HexStrtod(&cptr1[2], 2);
		blue_ival = HexStrtod(&cptr1[4], 2);
		fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;

		SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[0].fg = fg_ival;
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[0] = 1;

		


	}


	else if (AlertsColorGrid->row_number == 1){
		MGLSetActionColor(id, AlertsColorGrid, xcolname, vorow);
		topgrid->Actions[topgrid->CurActionNum].ActionType |= ACTION_COLOR_BG;
		cptr2 = topgrid->Actions[topgrid->CurActionNum].row_bg;


		red_ival = HexStrtod(&cptr2[0], 2);
		green_ival = HexStrtod(&cptr2[2], 2);
		blue_ival = HexStrtod(&cptr2[4], 2);
		bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;


		SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[1].bg = bg_ival;
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[1] = 1;

	}

	//for (int i = 0; i < topgrid->vo->name->count; i++)
	//{
	//	topgrid->vo->row_index[i].fg = MGLColorHex2Int("868686");

	//}





	if (topgrid->UpdateCB){
		topgrid->UpdateCB(id, topgrid);
	}



	return(TRUE);
}

int AlertsColorCallback(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, c, vorow, i, turnon;
	VO *vcol;
	char *colname, *sptr, *curstr, *xcolname;
	extern int re_exec(const char *lp);
	extern char *re_comp(const char *pat);
	MGLGRID *topgrid = AlertsColorGrid->parentgrid;


	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	if (AlertsColorGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / AlertsColorGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= AlertsColorGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = AlertsColorGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + AlertsColorGrid->iVscrollPos;

	// set the active input row in the grid
	//AlertsColorGrid->textrow = vorow;
	//strcpy_s(	AlertsColorGrid->textcolname, sizeof(AlertsColorGrid->textcolname) , "Value1" );


	// see which column was selected
	c = MGLCalcCol(AlertsColorGrid, x);

	if (c < 8 && c > 0){

		//  see which color user selected = colname
		vcol = V_ROW_PTR(AlertsColorGrid->vo->vcols, c);
		xcolname = vcol->name;


		curstr = VP(AlertsColorGrid->vo, vorow, xcolname, char);
		if (!strcmp(curstr, "X")){
			turnon = FALSE;
		}
		else {
			turnon = TRUE;
			if (vorow < 2){
				// a color was set
				// Create a dropdown color box

				//MGLSetActionColor(id, AlertsColorGrid, xcolname, vorow);
			}
		}

		// clear out other selections
		for (i = 1; i < AlertsColorGrid->vo->vcols->count; i++){
			vcol = V_ROW_PTR(AlertsColorGrid->vo->vcols, i);
			colname = vcol->name;
			sptr = VP(AlertsColorGrid->vo, vorow, colname, char);
			sptr[0] = '\0';
		}

		// get the column name for the column the user selected
		if (!turnon){
			// turn off
			sptr = VP(AlertsColorGrid->vo, vorow, xcolname, char);
			sptr[0] = '\0';
			// strncpy_s( VP(AlertsColorGrid->vo, vorow, xcolname, char ), vcol->size, "",  _TRUNCATE );
			switch (vorow)
			{
			case 0:
				topgrid->Actions[topgrid->CurActionNum].ActionType ^= ACTION_COLOR_FG;
				break;
			case 1:
				topgrid->Actions[topgrid->CurActionNum].ActionType ^= ACTION_COLOR_BG;
				break;
			case 2:
				topgrid->Actions[topgrid->CurActionNum].ActionType ^= ACTION_ALERT_LIST;
				break;
			case 3:
				topgrid->Actions[topgrid->CurActionNum].ActionType ^= ACTION_FILTER;
				break;
			default:
				break;
			}
		}
		else {
			// turn on
			strncpy_s(VP(topgrid->AlertsColorGrid->vo, vorow, xcolname, char), vcol->size, "X", _TRUNCATE);
			//    for example , the user can color the bg, fg, and make an action a filter by or'ing the values together
			switch (vorow)
			{
			case 0:
				topgrid->Actions[topgrid->CurActionNum].ActionType |= ACTION_COLOR_FG;
				break;
			case 1:
				topgrid->Actions[topgrid->CurActionNum].ActionType |= ACTION_COLOR_BG;
				break;
			case 2:
				topgrid->Actions[topgrid->CurActionNum].ActionType |= ACTION_ALERT_LIST;
				break;
			case 3:
				topgrid->Actions[topgrid->CurActionNum].ActionType |= ACTION_FILTER;
				break;
			default:
				break;
			}
		}
	}

	if (topgrid->UpdateCB){
		topgrid->UpdateCB(id, topgrid);
	}

	return(TRUE);
}
//mtm icon colors
void ColorPalette_icon(MGLGRID *parentgrid)
{

	MGLGRID *IconColorGrid = parentgrid->IconColorGrid;

	IconColorGrid->parent_w->visible = TRUE;
	IconColorGrid->gridcells[0][0].bg = (GLfloat*)Iconcolor0;
	IconColorGrid->gridcells[0][1].bg = (GLfloat*)Iconcolor2;
	IconColorGrid->gridcells[0][2].bg = (GLfloat*)Iconcolor4;
	IconColorGrid->gridcells[0][3].bg = (GLfloat*)Iconcolor6;
	IconColorGrid->gridcells[0][4].bg = (GLfloat*)Iconcolor8;
	IconColorGrid->gridcells[0][5].bg = (GLfloat*)Iconcolor10;
	IconColorGrid->gridcells[0][6].bg = (GLfloat*)Iconcolor12;
	IconColorGrid->gridcells[0][7].bg = (GLfloat*)Iconcolor14;


	IconColorGrid->gridcells[1][0].bg = (GLfloat*)Iconcolor1;
	IconColorGrid->gridcells[1][1].bg = (GLfloat*)Iconcolor3;
	IconColorGrid->gridcells[1][2].bg = (GLfloat*)Iconcolor5;
	IconColorGrid->gridcells[1][3].bg = (GLfloat*)Iconcolor7;
	IconColorGrid->gridcells[1][4].bg = (GLfloat*)Iconcolor9;
	IconColorGrid->gridcells[1][5].bg = (GLfloat*)Iconcolor11;
	IconColorGrid->gridcells[1][6].bg = (GLfloat*)Iconcolor13;
	IconColorGrid->gridcells[1][7].bg = (GLfloat*)Iconcolor15;



}
int Iconmatrix_callback(struct InstanceData* id, MGLGRID *IconColorGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, c, vorow;
	//	char *cptr1, *cptr2;
	//	int back[3];
	//	int fore[3];
	//	char  *xcolname;
	extern int re_exec(const char *lp);
	extern char *re_comp(const char *pat);

	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	IconColorGrid->parentgrid->parentgrid->icon_set = 1;


	if (IconColorGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - IconColorGrid->gridrect.y1) / IconColorGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - IconColorGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= IconColorGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	vorow = IconColorGrid->nrows - r - 1;

	// see which column was selected
	c = MGLCalcCol(IconColorGrid, x);
	if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <1)
		vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
	strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Actions", char), 64, "Icon:", _TRUNCATE);
	SecondTabGrid->aircraftcolor_change_MLAT = 1;
	SecondTabGrid->aircraftcolor_change_ASDEX = 1;
	SecondTabGrid->aircraftcolor_change_Noise = 1;
	SecondTabGrid->aircraftcolor_change_Gate = 1;
	SecondTabGrid->aircraftcolor_change_ASD = 1;
	SecondTabGrid->aircraftcolor_change = 1;


	if (vorow == 0)
	{
		if (c == 0)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 0;
		}
		else if (c == 1)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 2;
		}
		else if (c == 2)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 4;
		}
		else if (c == 3)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 6;
		}
		else if (c == 4)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 8;
		}
		else if (c == 5)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 10;
		}
		else if (c == 6)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 12;
		}
		else if (c == 7)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 14;
		}
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[2] = 1;	
	}

	else if (vorow == 1)
	{
		if (c == 0)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 1;
		}
		else if (c == 1)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 3;
		}
		else if (c == 2)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 5;
		}
		else if (c == 3)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 7;
		}
		else if (c == 4)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 9;
		}
		else if (c == 5)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 11;
		}
		else if (c == 6)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 13;
		}
		else if (c == 7)
		{
			SecondTabGrid->icon_set = 1;
			IconColorGrid->icon_color = 15;
		}		
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[2] = 1;
	}



	return(TRUE);
}
//mtm icon color changes
void CreateColorMatrix_icon(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	//creates the drop down color grid//
	widgets *w, *lastw, *panel_widget;
	int r, i;
	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}
	w = (widgets *)vo_malloc(sizeof(widgets));

	//If it is Foreground color palette
	w->gregion.x1 = SecondTabGrid->gridrect.x1 + 310;
	w->gregion.y1 = SecondTabGrid->gridrect.y1 + 100 + MGL_GRID_DEF_ROW_HEIGHT;
	w->gregion.x2 = 1520;
	w->gregion.y2 = 820;
	panel_widget = w;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "colormatrixicon", 2, 8, w->gregion.x1, w->gregion.y1);
	panel_widget = w;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
	w->type = WGT_GRID;
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->IconColorGrid = w->grid;

	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color454545;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	//"Col" to keep all the columns as same width
	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[7], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);

	ColorPalette_icon(parentgrid);

	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->IconColorGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->IconColorGrid->marginx;
	w->gregion.y1 = parentgrid->IconColorGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->IconColorGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;

	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Color Palette Icon", _TRUNCATE);

	w->visible = 1;
	w->grid = parentgrid->IconColorGrid;

	//Row callback is called when a particular color is selected on the color palette
	for (i = 0; i < parentgrid->IconColorGrid->nrows; i++){

		parentgrid->IconColorGrid->gridrows[i].row_callback = Iconmatrix_callback;
	}

	id->m_poglgui = oglgui;


}

int DropDownColor(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, c, vorow,colsize;
	extern int re_exec(const char *lp);
	extern char *re_comp(const char *pat);
	MGLGRID *topgrid = AlertsColorGrid->parentgrid;
	char *cptr;
	int  red_ival, green_ival, blue_ival, fg_ival, bg_ival;
	char *cptr1,*cptr2;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	if (AlertsColorGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / AlertsColorGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= AlertsColorGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = AlertsColorGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + AlertsColorGrid->iVscrollPos;
	AlertsColorGrid->row_number = vorow;

	// see which column was selected
	c = MGLCalcCol(AlertsColorGrid, x);

	if (c == 1)
	{
		if (vorow == 0)
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[0] == 1)
			{
				cptr = SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_fg;
				colsize = sizeof(SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_fg);
				strncpy_s(cptr, colsize, "", _TRUNCATE);
				topgrid->Actions[SecondTabGrid->childgrid->CurActionNum].ActionType |= ACTION_COLOR_FG;
				cptr1 = SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_fg;
				red_ival = 0;
				green_ival = 0;
				blue_ival = 0;
				fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
				SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[0].fg = fg_ival;
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[0] = 0;
			}
		}

		else if (vorow == 1)
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[1] == 1)
			{
				cptr = SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_bg;
				colsize = sizeof(SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_bg);
				strncpy_s(cptr, colsize, "", _TRUNCATE);
				topgrid->Actions[SecondTabGrid->childgrid->CurActionNum].ActionType |= ACTION_COLOR_BG;
				cptr2 = SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].row_bg;
				red_ival = 192;
				green_ival = 192;
				blue_ival = 192;
				bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
				SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[1].bg = bg_ival;
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[1] = 0;
			}
		}
		else if (vorow == 2)
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[2] == 1)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[2] = 0;
				SecondTabGrid->childgrid->icon_set = 0;
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Actions", char), 64, "", _TRUNCATE);
				SecondTabGrid->aircraftcolor_change_MLAT = 1;
				SecondTabGrid->aircraftcolor_change_ASDEX = 1;
				SecondTabGrid->aircraftcolor_change_Noise = 1;
				SecondTabGrid->aircraftcolor_change_Gate = 1;
				SecondTabGrid->aircraftcolor_change_ASD = 1;
				SecondTabGrid->aircraftcolor_change = 1;
			}
		}
		else if (vorow == 3)
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] == 0)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 1;
				topgrid->Actions[topgrid->CurActionNum].filter_active = 1;
			}
			else if (SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] == 1)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 0;
				topgrid->Actions[topgrid->CurActionNum].filter_active = 0;
			}
			topgrid->Actions[topgrid->CurActionNum].ActionType ^= ACTION_FILTER;			
			SecondTabGrid->childgrid->iVscrollPos = 0;
		}
		
		else if (vorow == 4) {
			if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->email_enabled == 1) 
			{
				SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->email_enabled = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[4] = 0;
			}
			else if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->email_enabled == 0)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->email_enabled = 1;
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[4] = 1;
			}

			//MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->childgrid->AlertsColorGrid->gridrect.x2 + MGL_GRID_DEF_ROW_HEIGHT, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->gridrect.y1 - (2 * MGL_GRID_DEF_ROW_HEIGHT));
		}
		else if (vorow == 5) //sy
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[5] == 0) 
			{
				SecondTabGrid->childgrid->showflights_check = 1;
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[5] = 1;
			} 
			else if (SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[5] == 1) 
			{
				SecondTabGrid->childgrid->showflights_check = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[5] = 0;
			}
			
		}
	}
	else if (c == 2)
	{
		if (vorow == 0)
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] == 0)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 1;
				if (!AlertsColorGrid->ForegroundColorGrid)
					CreateColorMatrix_foreground(id, AlertsColorGrid, 1);
				else
					AlertsColorGrid->ForegroundColorGrid->parent_w->visible = TRUE;
				if (AlertsColorGrid->BackgroundColorGrid)
				{
					if (AlertsColorGrid->BackgroundColorGrid->parent_w->visible == TRUE)
					{
						AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
						SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
					}
				}
				

			}
			else if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] == 1)
			{
				AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
			}

			if (AlertsColorGrid->IconColorGrid){
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
				AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
			}
			if (AlertsColorGrid->EMailGrid)
			{
				if (AlertsColorGrid->EMailGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
			}
			if (AlertsColorGrid->SubjectGrid)
			{
				if (AlertsColorGrid->SubjectGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
			}
		}
		else if (vorow == 1)
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] == 0)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 1;
				if (!AlertsColorGrid->BackgroundColorGrid)
					CreateColorMatrix_background(id, AlertsColorGrid, 1);
				else
					AlertsColorGrid->BackgroundColorGrid->parent_w->visible = TRUE;
				if (AlertsColorGrid->ForegroundColorGrid)
				{
					if (AlertsColorGrid->ForegroundColorGrid->parent_w->visible == TRUE)
					{
						AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
						SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
					}
				}
			}
			else if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] == 1)
			{
				AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
			}
			if (AlertsColorGrid->IconColorGrid){
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
				AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
			}
			if (AlertsColorGrid->EMailGrid)
			{
				if (AlertsColorGrid->EMailGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
			}
			if (AlertsColorGrid->SubjectGrid)
			{
				if (AlertsColorGrid->SubjectGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
			}

		}

		else if (vorow == 2)
		{
			if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] == 0)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 1;
				CreateColorMatrix_icon(id, AlertsColorGrid, 1);
				AlertsColorGrid->IconColorGrid->parent_w->visible = 1;
			}
			else if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] == 1){
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
				AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
			}

			if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] == 1)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
				AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
			}
			else if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] == 1)
			{
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
				AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
			}
			if (AlertsColorGrid->EMailGrid)
			{
				if (AlertsColorGrid->EMailGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
			}
			if (AlertsColorGrid->SubjectGrid)
			{
				if (AlertsColorGrid->SubjectGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
			}

		}
		
		else if (vorow == 4)
		{
			
			if (AlertsColorGrid->EMailGrid)
			{
				if (AlertsColorGrid->EMailGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
				else if (AlertsColorGrid->EMailGrid->parent_w->visible == FALSE)
				{
					AlertsColorGrid->EMailGrid->parent_w->visible = TRUE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 1;
				}
				if (AlertsColorGrid->ForegroundColorGrid)
					AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->BackgroundColorGrid)
					AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->IconColorGrid)
					AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
			}

			else if (!AlertsColorGrid->EMailGrid)
			{
				CreateEMailGrid(id, SecondTabGrid->childgrid);
				FillEMailVO(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid);						
				if (AlertsColorGrid->ForegroundColorGrid)
					AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->BackgroundColorGrid)
					AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->IconColorGrid)
					AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
			}

			if (AlertsColorGrid->SubjectGrid)
			{
				if (AlertsColorGrid->SubjectGrid->parent_w->visible == TRUE)
				{
					AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;
				}
				else if (AlertsColorGrid->SubjectGrid->parent_w->visible == FALSE)
				{
					AlertsColorGrid->SubjectGrid->parent_w->visible = TRUE;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 1;
				}
				if (AlertsColorGrid->ForegroundColorGrid)
					AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->BackgroundColorGrid)
					AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->IconColorGrid)
					AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 1;
			}

			else if (!AlertsColorGrid->SubjectGrid)
			{
				CreateSubjectGrid(id, SecondTabGrid->childgrid);
				FillSubjectVO(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid);
				if (AlertsColorGrid->ForegroundColorGrid)
					AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->BackgroundColorGrid)
					AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
				if (AlertsColorGrid->IconColorGrid)
					AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
				SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 1;
			}
			
		}
	}

		

		

	

	return(TRUE);
}


///callback for "Actions" subgrid///

int GridActions_callback(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, c, vorow;
	VO *vo;


	extern int re_exec(const char *lp);
	extern char *re_comp(const char *pat);
	MGLGRID *topgrid = AlertsColorGrid->parentgrid;

	vo = AlertsColorGrid->vo;

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	if (AlertsColorGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / AlertsColorGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - AlertsColorGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= AlertsColorGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = AlertsColorGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + AlertsColorGrid->iVscrollPos;



	AlertsColorGrid->row_number = vorow;

	// set the active input row in the grid
	//AlertsColorGrid->textrow = vorow;
	//strcpy_s(	AlertsColorGrid->textcolname, sizeof(AlertsColorGrid->textcolname) , "Value1" );


	// see which column was selected
	c = MGLCalcCol(AlertsColorGrid, x);




	if (vorow == 0){

		if (AlertsColorGrid->GridActions)
			AlertsColorGrid->GridActions->parent_w->visible = 0;
		if (AlertsColorGrid->Databookgrid)
			AlertsColorGrid->Databookgrid->parent_w->visible = 0;
		if (AlertsColorGrid->Notifygrid)
			AlertsColorGrid->Notifygrid->parent_w->visible = 0;


		CreateParametersGrid(id, AlertsColorGrid, TRUE);
		MGLInitAlertsColorsVO_me(AlertsColorGrid);
	}

	else if (vorow == 1)
	{
		if (AlertsColorGrid->GridActions)
			AlertsColorGrid->GridActions->parent_w->visible = 0;
		if (AlertsColorGrid->Databookgrid)
			AlertsColorGrid->Databookgrid->parent_w->visible = 0;
		if (AlertsColorGrid->Notifygrid)
			AlertsColorGrid->Notifygrid->parent_w->visible = 0;

		IconsGrid(id, AlertsColorGrid, TRUE);

	}

	else if (vorow == 2)
	{
		if (AlertsColorGrid->GridActions)
			AlertsColorGrid->GridActions->parent_w->visible = 0;
		if (AlertsColorGrid->IconGrid)
			AlertsColorGrid->IconGrid->parent_w->visible = 0;
		if (AlertsColorGrid->Notifygrid)
			AlertsColorGrid->Notifygrid->parent_w->visible = 0;


		DatabookGrid(id, AlertsColorGrid, TRUE);
	}

	else if (vorow == 3)
	{
		if (AlertsColorGrid->IconGrid)
			AlertsColorGrid->IconGrid->parent_w->visible = 0;
		if (AlertsColorGrid->Databookgrid)
			AlertsColorGrid->Databookgrid->parent_w->visible = 0;
		if (AlertsColorGrid->GridActions)
			AlertsColorGrid->GridActions->parent_w->visible = 0;

		NotifyGrid(id, AlertsColorGrid, TRUE);

	}




	//AlertsColorGrid->AlertsColorGrid->parent_w->visible =1;


	return(TRUE);
}

void CreateColorMatrix_foreground(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	//creates the drop down color grid//
	widgets *w, *lastw, *panel_widget;
	int r, i;
	assert(id);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "colormatrixforeground");
	//w->gregion.x1 = 1220;
	//w->gregion.y1 = 300;
	w->gregion.x1 = SecondTabGrid->gridrect.x1 + 320;
	w->gregion.y1 = SecondTabGrid->gridrect.y1 + 100 + MGL_GRID_DEF_ROW_HEIGHT;
	w->gregion.x2 = 1520;
	w->gregion.y2 = 820;
	panel_widget = w;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "colormatrixforeground", 5, 5, w->gregion.x1, w->gregion.y1);

	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->ForegroundColorGrid = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color454545;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	//"Col" to keep all the columns as same width
	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);

	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);

	ColorPalette_foreground(parentgrid);

	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->ForegroundColorGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->ForegroundColorGrid->marginx;
	w->gregion.y1 = parentgrid->ForegroundColorGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->ForegroundColorGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;


	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Color Palette FG", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->ForegroundColorGrid;

	//Row callback is called when a particular color is selected on the color palette
	for (i = 0; i < parentgrid->ForegroundColorGrid->nrows; i++){

		parentgrid->ForegroundColorGrid->gridrows[i].row_callback = AlertsColorCallback_colorpalettefg;
	}

	id->m_poglgui = oglgui;

}
void CreateColorMatrix_background(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	//creates the drop down color grid//
	widgets *w, *lastw, *panel_widget;
	int r, i;
	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}
	w = (widgets *)vo_malloc(sizeof(widgets));

	//If it is Foreground color palette
	w->gregion.x1 = SecondTabGrid->gridrect.x1 + 320;
	w->gregion.y1 = SecondTabGrid->gridrect.y1 + 100 + MGL_GRID_DEF_ROW_HEIGHT;
	w->gregion.x2 = 1520;
	w->gregion.y2 = 820;
	panel_widget = w;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "colormatrixbackground", 5, 5, w->gregion.x1, w->gregion.y1);
	panel_widget = w;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
	w->type = WGT_GRID;


	strcpy_s(w->name, sizeof(w->name), "colormatrixbackground");

	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->BackgroundColorGrid = w->grid;

	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)color868686;
			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color454545;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	//"Col" to keep all the columns as same width
	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);

	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Col", _TRUNCATE);

	ColorPalette_background(parentgrid);

	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->BackgroundColorGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->BackgroundColorGrid->marginx;
	w->gregion.y1 = parentgrid->BackgroundColorGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->BackgroundColorGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;

	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Color Palette BG", _TRUNCATE);

	w->visible = 1;
	w->grid = parentgrid->BackgroundColorGrid;

	//Row callback is called when a particular color is selected on the color palette
	for (i = 0; i < parentgrid->BackgroundColorGrid->nrows; i++){

		parentgrid->BackgroundColorGrid->gridrows[i].row_callback = AlertsColorCallback_colorpalettefg;
	}

	id->m_poglgui = oglgui;

}



void GridToDisaplaySave(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	//this grid displays a save message once we click on save
	widgets *w, *lastw, *panel_widget;
	int r;
	assert(id);


	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}
	w = (widgets *)vo_malloc(sizeof(widgets));



	strcpy_s(w->name, sizeof(w->name), "Saved");
	w->gregion.x1 = 700;
	w->gregion.y1 = 150;
	w->gregion.x2 = 400;
	w->gregion.y2 = 670;



	//if it is background color palette


	panel_widget = w;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;


	w->grid = MGLGridCreate(id, parentgrid, "Saved", 1, 1, w->gregion.x1, w->gregion.y1);



	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->AlertsColorGrid = w->grid;
	r = 0;
	w->grid->gridrows[r].fg = (GLfloat*)color868686;
	w->grid->gridrows[r + 1].fg = (GLfloat*)yellow;


	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}



	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->AlertsColorGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->AlertsColorGrid->marginx;
	w->gregion.y1 = parentgrid->AlertsColorGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->AlertsColorGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;


	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Saved", _TRUNCATE);
	id->m_poglgui = oglgui;
}


//void CreateParametersGrid(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
//{
//	widgets *w, *lastw, *panel_widget;
//	int r, i;
//
//	assert(id);
//
//	// find last widget in list and add it here
//	for (w = oglgui->widgets; w; w = w->np){
//		lastw = w;
//	}
//
//	// Create panel for holding the two grids plus buttons
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "AlertPanel");
//	w->gregion.x1 = 20;
//	w->gregion.y1 = 310;
//	w->width = 500;
//	w->height = 300;
//	//w->gregion.x2 = 200;
//	//w->gregion.y2 = 570;
//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;
//
//	w->type = WGT_PANEL;
//	w->visible = 0;  // don't show panel
//	w->bg = (GLfloat*)gridgrey0;
//	w->fg = (GLfloat*)white;
//	panel_widget = w;
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//		lastw = w;
//	}
//
//	// Create Actions grid
//
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "AlertsColorWidget");
//	w->gregion.x1 = 20;
//	w->gregion.y1 = 370;
//	w->gregion.x2 = 400;
//	w->gregion.y2 = 670;
//	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
//
//	w->type = WGT_GRID;
//	w->grid = MGLGridCreate(id, parentgrid, "AlertsColorGrid", 6, 2, w->gregion.x1, w->gregion.y1);
//	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
//	w->grid->iVscrollPos = 0;
//	w->grid->GScrollWidth = 20;
//	w->visible = 1;
//	w->grid->AllowScrollbar = TRUE;
//	w->grid->HasTitleBar = TRUE;
//	// add default colors for grid and the rows
//	w->grid->bg = (GLfloat*)gridgrey0;
//	w->grid->fg = (GLfloat*)white;
//	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
//	//parentgrid->AlertsColorGrid = w->grid;
//	parentgrid->AlertsColorGrid = w->grid;
//
//
//
//	for (r = 0; r < w->grid->nrows; r += 2){
//		if (r == 0){
//			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
//		}
//		else {
//			w->grid->gridrows[r].fg = (GLfloat*)color868686;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
//		}
//		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
//		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
//	}
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//		lastw = w;
//	}
//
//	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Color", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Settings", _TRUNCATE);
//
//
//
//	strncpy_s(w->grid->gridcols[1].pngfilename, sizeof(w->grid->gridcols[1].pngfilename), "down-reorder-arrow.png", _TRUNCATE);
//	w->grid->gridcols[1].texturesize = 16;
//	w->grid->gridcols[1].img_displaysize = 10;
//
//
//
//
//
//	// set row callbacks
//	for (i = 0; i < parentgrid->AlertsColorGrid->nrows; i++){
//		parentgrid->AlertsColorGrid->gridrows[i].row_callback = DropDownColor;
//	}
//
//	// set fg and bg colors for cells
//
//
//
//	// set TITLE text widgets
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	parentgrid->AlertsColorGrid->title_widgets = w;
//
//	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
//	// give location of widget relative to title_rect
//	w->gregion.x1 = parentgrid->AlertsColorGrid->marginx;
//	w->gregion.y1 = parentgrid->AlertsColorGrid->marginy;
//	w->gregion.x2 = w->gregion.x1 + 120;
//	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->AlertsColorGrid->marginy;
//	w->type = WGT_TEXT;
//	w->wgt_mouse_cb = title_view_cb;
//	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Actions", _TRUNCATE);
//	w->visible = 1;
//	w->grid = parentgrid->AlertsColorGrid;
//
//
//}

int cal_mnum(char * m){

	int month_numeric = -1;
	char *month_list[] = { "January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December" };
	
	if (strcmp(m, month_list[0]) == 0)
		month_numeric= 0;
	else if (strcmp(m, month_list[1]) == 0)
		month_numeric= 1;
	else if (strcmp(m, month_list[2]) == 0)
		month_numeric= 2;
	else if (strcmp(m, month_list[3]) == 0)
		month_numeric= 3;
	else if (strcmp(m, month_list[4]) == 0)
		month_numeric= 4;
	else if (strcmp(m, month_list[5]) == 0)
		month_numeric= 5;
	else if (strcmp(m, month_list[6]) == 0)
		month_numeric= 6;
	else if (strcmp(m, month_list[7]) == 0)
		month_numeric= 7;
	else if (strcmp(m, month_list[8]) == 0)
		month_numeric= 8;
	else if (strcmp(m, month_list[9]) == 0)
		month_numeric= 9;
	else if (strcmp(m, month_list[10]) == 0)
		month_numeric= 10;
	else if (strcmp(m, month_list[11]) == 0)
		month_numeric= 11;
	else
		month_numeric= -1;
	
	return month_numeric;
}

int CalCallBack(struct InstanceData* id, MGLGRID *grid, int b, int m, int x, int y){

	widgets *w;
	char monthyear[256];
	char ymd_date[512];
	char *pch;
	char *year_str, *month_str;
	int i = 0, month = 0, year = 0;
	int Startingday = -1, Endingday = -1;
	//array of net days in each month
	int months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int start_row= 2, end_row = -1;
	int start_col = -1, end_col = -1;
	int rev_row=-1,row = -1, col = -1, vorow;
	int diff_row = -1;
	int date = 0;

	extern int re_exec(const char *lp);
	extern char *re_comp(const char *pat);


	memset(monthyear, 0, sizeof(monthyear));
	memset(ymd_date, 0, sizeof(ymd_date));

	//obtain year & month in a str format
	if (!grid || !grid->title_widgets) return -1;
	// look through all title widgets to find the title name widget "TitleWidget"
	for (w = grid->title_widgets; w; w = w->np){
		if (strstr(w->name, "TitleWidget")){
			strncpy_s(monthyear, 256, w->textbuf, _TRUNCATE);
			break;
		}
	}

	pch = strtok(monthyear, " ");
	while (pch != NULL && i < 2){
		if (i == 0)
			month_str= strdup(pch);
		else
			year_str= strdup(pch);
		i++;
		pch = strtok(NULL, " ");
	}

	//obtain year & month in numeric format
	year = atoi(year_str);
	month = cal_mnum(month_str);
	if (month <0)
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):Invalid month\n",CalCallBack);

	if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))
		months[1] = 29;
	else
		months[1] = 28;

	Startingday = get_week_day(1, month + 1, year);
	Endingday = get_week_day(months[month], month + 1, year);

	start_row= 2;
	start_col= Startingday;
	end_col = Endingday;

	if (start_col == 5){
		if (months[month] == 31)
			end_row = 7;
	}
	else if (start_col == 6)
		end_row = 7;
	else
		end_row = 6;


	if (grid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		rev_row = (int)floor(((double)y - grid->gridrect.y1) / grid->gridrows[0].height);
	}
	else {
		rev_row = (int)floor(((double)y - grid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (rev_row < 0 || rev_row >= grid->nrows){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):Invalid Row Accessed\n", CalCallBack);
		return(FALSE);
	}
	// reverse the order
	row = grid->nrows - rev_row - 1;

	// see which column was selected
	col = MGLCalcCol(grid, x);

	diff_row = row - start_row;
	
	if ((row == start_row) && (col >= start_col))
		date = 1 + diff_row * 7 + (col-start_col);
	else if ((row == end_row) && (col <= end_col))
		date = 1 + diff_row * 7 + (col-start_col);
	else if ((row > start_row) && (row < end_row))
		date = 1 + diff_row * 7 + (col-start_col);
	else{
		date = 0;
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):Invalid date\n", CalCallBack);
		return(FALSE);
	}
	_snprintf(ymd_date, 512, "%d%02d%02d", year, month+1, date);
	//printf("\n name=%s", name);
 	return 0;

}


MGLGRID *CreateCalendar(struct InstanceData* id, MGLGRID *CalendarAppGrid)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "CalendarPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of avaliable coloumns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "CalendarWidget");
	w->gregion.x1 = 40;
	w->gregion.y1 = 100;
	w->gregion.x2 = 800;
	w->gregion.y2 = id->m_nHeight +600;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "CalendarGrid", 9, 7, w->gregion.x1, w->gregion.y1);
	w->grid->width = 350;// make column wide enough to fit all region names
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	//>grid->parent_w =
	CalendarAppGrid = w->grid;


	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			//w->grid->gridrows[r].fg = (GLfloat*)yellow;
			w->grid->gridrows[r + 1].fg = (GLfloat*)Text_yellow;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)grey;
			w->grid->gridrows[r + 1].fg = (GLfloat*)grey;
		}

		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)Grid_grey1;
	//w->grid->gridrows[0].fg = (GLfloat*)yellow;
	w->grid->gridrows[0].fg = Text_yellow;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "  SUN", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "  MON", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "  TUE", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "  WED", _TRUNCATE); 
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "  THU", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "  FRI", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, "  SAT", _TRUNCATE);




	//set row callbacks
	for (i = 0; i < CalendarAppGrid->nrows; i++){
		CalendarAppGrid->gridrows[i].row_callback = CalCallBack;
	}
	


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	//parentgrid->AlertsColorGrid->EMailGrid->title_widgets = w;
	CalendarAppGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = CalendarAppGrid->marginx;
	w->gregion.y1 = CalendarAppGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - CalendarAppGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "CALENDAR", _TRUNCATE);
	w->visible = 1;
	w->grid = CalendarAppGrid->EMailGrid;
	id->m_poglgui = oglgui;
	return(CalendarAppGrid);
}




//mtm: E-mail Grid
void CreateEMailGrid(struct InstanceData* id, MGLGRID *parentgrid)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);
	assert(parentgrid);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "EMailPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Actions
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "EMailGridWidget");
	w->gregion.x1 = parentgrid->AlertsColorGrid->gridrect.x2 + 40;
	w->gregion.y1 = parentgrid->AlertsColorGrid->gridrect.y2 - 100;
	w->gregion.x2 = 800;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "EMailGrid", 6, 2, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->AlertsColorGrid->EMailGrid = w->grid;


	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}

		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "  ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "E-MAIL Notifications", _TRUNCATE);
	
	

	//// set row callbacks
	for (i = 0; i < parentgrid->AlertsColorGrid->EMailGrid->nrows; i++){
		parentgrid->AlertsColorGrid->EMailGrid->gridrows[i].row_callback = EMailCallback;
	}



	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->AlertsColorGrid->EMailGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "E-MailTitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->AlertsColorGrid->EMailGrid->marginx;
	w->gregion.y1 = parentgrid->AlertsColorGrid->EMailGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->AlertsColorGrid->EMailGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;	
	w->visible = 1;
	w->grid = parentgrid->AlertsColorGrid->EMailGrid;
	id->m_poglgui = oglgui;
}

//mtm: Subject Grid
void CreateSubjectGrid(struct InstanceData* id, MGLGRID *parentgrid)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);
	assert(parentgrid);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SubjectPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Actions
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SubjectGridWidget");
	w->gregion.x1 = parentgrid->AlertsColorGrid->gridrect.x2 + 40;
	w->gregion.y1 = parentgrid->AlertsColorGrid->EMailGrid->gridrect.y1 - (2 * MGL_GRID_DEF_ROW_HEIGHT);
	w->gregion.x2 = 800;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "SubjectGrid", 2, 2, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->AlertsColorGrid->SubjectGrid = w->grid;


	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}

		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "  ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "                ", _TRUNCATE);



	//// set row callbacks
	for (i = 0; i < parentgrid->AlertsColorGrid->SubjectGrid->nrows; i++){
		parentgrid->AlertsColorGrid->SubjectGrid->gridrows[i].row_callback = SubjectCallback;
	}



	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->AlertsColorGrid->SubjectGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "E-MailTitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->AlertsColorGrid->SubjectGrid->marginx;
	w->gregion.y1 = parentgrid->AlertsColorGrid->SubjectGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->AlertsColorGrid->SubjectGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	w->visible = 1;
	w->grid = parentgrid->AlertsColorGrid->SubjectGrid;
	id->m_poglgui = oglgui;
}

///Build grid with "Actions" as heading - Alerts and Color ///
void CreateAlertsColorGrid(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "AlertsColorGrid");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// Create Actions grid

	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "AlertsColorGrid");
	w->gregion.x1 = SecondTabGrid->gridrect.x1 + 40;
	w->gregion.y1 = SecondTabGrid->gridrect.y1 + 50;
	w->gregion.x2 = 400;
	w->gregion.y2 = 670;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "AlertsColorGrid", 7, 3, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->AlertsColorGrid = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}
	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Selections", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "On/Off", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Options", _TRUNCATE);



	strncpy_s(w->grid->gridcols[1].pngfilename, sizeof(w->grid->gridcols[1].pngfilename), "show-circle-off.png", _TRUNCATE);
	w->grid->gridcols[1].texturesize = 16;
	w->grid->gridcols[1].img_displaysize = 10;

	strncpy_s(w->grid->gridcols[2].pngfilename, sizeof(w->grid->gridcols[2].pngfilename), "zoom-arrow-off.png", _TRUNCATE);
	w->grid->gridcols[2].texturesize = 32;
	w->grid->gridcols[2].img_displaysize = 19;




	// set row callbacks
	for (i = 0; i < parentgrid->AlertsColorGrid->nrows; i++){
		parentgrid->AlertsColorGrid->gridrows[i].row_callback = DropDownColor;
	}

	//	SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 1;
	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->AlertsColorGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->AlertsColorGrid->marginx;
	w->gregion.y1 = parentgrid->AlertsColorGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->AlertsColorGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Actions", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->AlertsColorGrid;

	id->m_poglgui = oglgui;
}
int VisibleRowsColumnCallback(struct InstanceData* id, MGLGRID *RowColumn, int b, int m, int x, int y)
{
	// Toggle the selected action On or OFF
	// allow user to add new actions
	int r, vorow, c;
	char *AppName;
	//	MGLGRID *grid;


	assert(id);

	if (!RowColumn) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (RowColumn->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - RowColumn->gridrect.y1) / RowColumn->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - RowColumn->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}


	// toggle the highlight on or off
	if (r < 0 || r >= RowColumn->nrows){
		return(FALSE);
	}
	// reverse the order
	r = RowColumn->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + RowColumn->iVscrollPos;
	// see which column was selected
	c = MGLCalcCol(RowColumn, x);

	if (c == 1 && vorow >= 0 && vorow < RowColumn->vo->count){

		// user wants to create a new app, so set the text input box and allow him to enter the name of the app
		// popup white text box in place of the "New"
		if (vorow >= 0 && vorow < RowColumn->vo->count){
			// clear out current string
			RowColumn->gridcells[r][c].bg = (GLfloat*)white;
			RowColumn->gridcells[r][c].fg = (GLfloat*)black;
			RowColumn->textrow = vorow;
			RowColumn->parent_w->TextActive = TRUE;
			ActiveTextGrid = RowColumn;
			strcpy_s(RowColumn->textcolname, sizeof(RowColumn->textcolname), "Settings");
			// strcpy_s(DataAppGrid->row[1].row_name, 32, "AppName");
			RowColumn->vo->row_index[vorow].fg = MGLColorHex2Int("Black");
			AppName = VP(RowColumn->vo, vorow, "AppName", char);



		}

	}
	return(TRUE);
}



////mtm
/////Build grid with "No of rows" as heading///
void CreateGridrowcolumns(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "RowColumn");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// Create Actions grid

	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "RowColumn");
	w->gregion.x1 = SecondTabGrid->gridrect.x1 + 60;
	w->gregion.y1 = SecondTabGrid->gridrect.y1 + 80;
	w->gregion.x2 = 400;
	w->gregion.y2 = 670;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "RowColumn", 2, 2, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->RowColumn = w->grid;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}
	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Rows          ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Enter number  ", _TRUNCATE);
	//strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, " ", _TRUNCATE);


	//parentgrid->RowColumn->gridcells[1][1].bg = (GLfloat*)Grid_grey5;
	//	parentgrid->RowColumn->gridcells[1][1].fg = (GLfloat*)Grid_grey3;






	// set row callbacks
	for (i = 0; i < parentgrid->RowColumn->nrows; i++){
		parentgrid->RowColumn->gridrows[i].row_callback = VisibleRowsColumnCallback;
	}


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->RowColumn->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->RowColumn->marginx;
	w->gregion.y1 = parentgrid->RowColumn->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->RowColumn->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Visible rows        ", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->RowColumn;

	id->m_poglgui = oglgui;
}

////mtm
/////Build confirm grid
MGLGRID *CreateConfirmGrid(struct InstanceData* id)
{
	//pops up "SAVED" when save is pressed

	widgets *w, *lastw, *panel_widget;
	int r;


	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* ConfirmGrid = (MGLGRID*)id->ConfirmGrid;
	
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ConfirmDisplay");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Alerts that are active
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ConfirmDisplay");
	w->gregion.x1 = DataAppGrid->gridrect.x1 - 200;
	w->gregion.y1 = DataAppGrid->gridrect.y1 + 35;
	w->gregion.x2 = 300;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "ConfirmDisplay", 3, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	ConfirmGrid = w->grid;



	w->grid->gridrows[0].fg = (GLfloat*)yellow3;
	w->grid->gridrows[0].bg = (GLfloat*)color454545;
	w->grid->gridrows[1].fg = (GLfloat*)white;
	w->grid->gridrows[1].bg = (GLfloat*)color454545;
	w->grid->gridrows[2].fg = (GLfloat*)white;
	w->grid->gridrows[2].bg = (GLfloat*)color454545;



	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	MGLGridText(id, ConfirmGrid, 0, 0, "Do you want to delete the grid?");
	MGLGridText(id, ConfirmGrid, 1, 0, "Yes");
	MGLGridText(id, ConfirmGrid, 2, 0, "No");
	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Do you want to delete the grid?", _TRUNCATE);

	////

	//w = (widgets *)vo_malloc(sizeof(widgets));
	//ConfirmGrid->title_widgets = w;

	//strcpy_s(w->name, sizeof(w->name), "ConfirmGrid");
	//// give location of widget relative to title_rect
	//w->gregion.x1 = ConfirmGrid->marginx;
	//w->gregion.y1 = ConfirmGrid->marginy;
	//w->gregion.x2 = w->gregion.x1 + 120;
	//w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - ConfirmGrid->marginy;
	//w->type = WGT_TEXT;
	//w->wgt_mouse_cb = title_view_cb;
	//strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Do you want to delete the grid?", _TRUNCATE);
	////w->visible = 1;
	//w->grid = ConfirmGrid;

	////// add new widget
	//if (lastw){
	//	lastw->np = w;
	//	lastw = w;
	//}
	for (r = 1; r <ConfirmGrid->nrows; r++){
		ConfirmGrid->gridrows[r].row_callback = ConfirmCallback;
	}

	id->m_poglgui = oglgui;
	id->DataAppGrid = DataAppGrid;
	id->ConfirmGrid = ConfirmGrid;
	return(ConfirmGrid);


}



void CreateParametersGrid1(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	//// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ParametersPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 310;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}


	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ParametersGridWidget");
	w->gregion.x1 = SecondTabGrid->gridrect.x1 + 40;
	/*if (parentgrid->roi_fdo==1)
		w->gregion.y1 = SecondTabGrid->gridrect.y1 + 40 ;
		else*/
	w->gregion.y1 = SecondTabGrid->gridrect.y1 + 40;
	w->gregion.x2 = 1380;
	w->gregion.y2 = 670;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "ParametersGrid", 9, 11, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->ParametersGrid = w->grid;
	strcpy_s(parentgrid->ParametersGrid->textcolname, sizeof(parentgrid->ParametersGrid->textcolname), "Value1");


	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;

#ifdef OLDCOLORS
	for (r = 0; r < w->grid->nrows; r++){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)yellow;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)gridgreyelev;
	}
#endif
	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	w->grid->gridcols[2].val_callback = Cnv2List;

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Column", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Operator", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "List", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Value 1", _TRUNCATE);
	// strncpy_s( w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Value 2"  , _TRUNCATE);

	// we also need to fetch initial filter values from user settings DB

	// set row callbacks
	for (i = 0; i < parentgrid->ParametersGrid->nrows; i++){
		parentgrid->ParametersGrid->gridrows[i].row_callback = ParametersCallback;
	}

	parentgrid->ParametersGrid->move_flag = 30;

	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->ParametersGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "AlertsParametersTitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->ParametersGrid->marginx;
	w->gregion.y1 = parentgrid->ParametersGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->ParametersGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Parameters", _TRUNCATE);
	//w->visible = 1;
	w->grid = parentgrid->ParametersGrid;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	id->m_poglgui = oglgui;
	/*if (parentgrid->roi_fdo == 1)
	{
	parentgrid->ParametersGrid->nrows = 10;
	}*/

}



void SetAlertColors(MGLGRID *parentgrid)
{

	MGLGRID *AlertsColorGrid = parentgrid->AlertsColorGrid;

	AlertsColorGrid->gridcells[1][1].bg = (GLfloat*)red;
	AlertsColorGrid->gridcells[2][1].bg = (GLfloat*)red;
	AlertsColorGrid->gridcells[1][2].bg = (GLfloat*)green;
	AlertsColorGrid->gridcells[2][2].bg = (GLfloat*)green;
	AlertsColorGrid->gridcells[1][3].bg = (GLfloat*)blue;
	AlertsColorGrid->gridcells[2][3].bg = (GLfloat*)blue;
	AlertsColorGrid->gridcells[1][4].bg = (GLfloat*)yellow;
	AlertsColorGrid->gridcells[2][4].bg = (GLfloat*)yellow;

	AlertsColorGrid->gridcells[1][5].bg = (GLfloat*)orange;
	AlertsColorGrid->gridcells[2][5].bg = (GLfloat*)orange;
	AlertsColorGrid->gridcells[1][6].bg = (GLfloat*)silver;
	AlertsColorGrid->gridcells[2][6].bg = (GLfloat*)silver;
	AlertsColorGrid->gridcells[1][7].bg = (GLfloat*)maroon;
	AlertsColorGrid->gridcells[2][7].bg = (GLfloat*)maroon;


	AlertsColorGrid->gridcells[1][4].fg = (GLfloat*)black;
	AlertsColorGrid->gridcells[2][4].fg = (GLfloat*)black;
	AlertsColorGrid->gridcells[1][2].fg = (GLfloat*)black;
	AlertsColorGrid->gridcells[2][2].fg = (GLfloat*)black;

}

void ColorPalette_foreground(MGLGRID *parentgrid)
{

	MGLGRID *ForegroundColorGrid = parentgrid->ForegroundColorGrid;

	//if (strcmp(parentgrid->ForegroundColorGrid->name, "colormatrixforeground") == 0)
	//	parentgrid->flag1 = 1;
	//
	//if (strcmp(parentgrid->ForegroundColorGrid->name, "colormatrixbackground") == 0)
	//	parentgrid->flag2 = 1;



	ForegroundColorGrid->parent_w->visible = TRUE;
	ForegroundColorGrid->gridcells[0][0].bg = (GLfloat*)yellow1;
	ForegroundColorGrid->gridcells[0][1].bg = (GLfloat*)yellow2;
	ForegroundColorGrid->gridcells[0][2].bg = (GLfloat*)yellow3;
	ForegroundColorGrid->gridcells[0][3].bg = (GLfloat*)yellow4;
	ForegroundColorGrid->gridcells[0][4].bg = (GLfloat*)yellow5;


	ForegroundColorGrid->gridcells[1][0].bg = (GLfloat*)red1;
	ForegroundColorGrid->gridcells[1][1].bg = (GLfloat*)red2;
	ForegroundColorGrid->gridcells[1][2].bg = (GLfloat*)red3;
	ForegroundColorGrid->gridcells[1][3].bg = (GLfloat*)red4;
	ForegroundColorGrid->gridcells[1][4].bg = (GLfloat*)red5;

	ForegroundColorGrid->gridcells[2][0].bg = (GLfloat*)violet1;
	ForegroundColorGrid->gridcells[2][1].bg = (GLfloat*)violet2;
	ForegroundColorGrid->gridcells[2][2].bg = (GLfloat*)violet3;
	ForegroundColorGrid->gridcells[2][3].bg = (GLfloat*)violet4;
	ForegroundColorGrid->gridcells[2][4].bg = (GLfloat*)violet5;


	ForegroundColorGrid->gridcells[3][0].bg = (GLfloat*)green1;
	ForegroundColorGrid->gridcells[3][1].bg = (GLfloat*)green2;
	ForegroundColorGrid->gridcells[3][2].bg = (GLfloat*)green3;
	ForegroundColorGrid->gridcells[3][3].bg = (GLfloat*)green4;
	ForegroundColorGrid->gridcells[3][4].bg = (GLfloat*)green5;


	ForegroundColorGrid->gridcells[4][0].bg = (GLfloat*)blue1;
	ForegroundColorGrid->gridcells[4][1].bg = (GLfloat*)blue2;
	ForegroundColorGrid->gridcells[4][2].bg = (GLfloat*)blue3;
	ForegroundColorGrid->gridcells[4][3].bg = (GLfloat*)blue4;
	ForegroundColorGrid->gridcells[4][4].bg = (GLfloat*)blue5;


	ForegroundColorGrid->gridcells[1][4].fg = (GLfloat*)black;
	ForegroundColorGrid->gridcells[2][4].fg = (GLfloat*)black;
	ForegroundColorGrid->gridcells[1][2].fg = (GLfloat*)black;
	ForegroundColorGrid->gridcells[2][2].fg = (GLfloat*)black;

}

void ColorPalette_background(MGLGRID *parentgrid)
{

	MGLGRID *BackgroundColorGrid = parentgrid->BackgroundColorGrid;

	BackgroundColorGrid->parent_w->visible = TRUE;
	BackgroundColorGrid->gridcells[0][0].bg = (GLfloat*)yellow1;
	BackgroundColorGrid->gridcells[0][1].bg = (GLfloat*)yellow2;
	BackgroundColorGrid->gridcells[0][2].bg = (GLfloat*)yellow3;
	BackgroundColorGrid->gridcells[0][3].bg = (GLfloat*)yellow4;
	BackgroundColorGrid->gridcells[0][4].bg = (GLfloat*)yellow5;


	BackgroundColorGrid->gridcells[1][0].bg = (GLfloat*)red1;
	BackgroundColorGrid->gridcells[1][1].bg = (GLfloat*)red2;
	BackgroundColorGrid->gridcells[1][2].bg = (GLfloat*)red3;
	BackgroundColorGrid->gridcells[1][3].bg = (GLfloat*)red4;
	BackgroundColorGrid->gridcells[1][4].bg = (GLfloat*)red5;

	BackgroundColorGrid->gridcells[2][0].bg = (GLfloat*)violet1;
	BackgroundColorGrid->gridcells[2][1].bg = (GLfloat*)violet2;
	BackgroundColorGrid->gridcells[2][2].bg = (GLfloat*)violet3;
	BackgroundColorGrid->gridcells[2][3].bg = (GLfloat*)violet4;
	BackgroundColorGrid->gridcells[2][4].bg = (GLfloat*)violet5;


	BackgroundColorGrid->gridcells[3][0].bg = (GLfloat*)green1;
	BackgroundColorGrid->gridcells[3][1].bg = (GLfloat*)green2;
	BackgroundColorGrid->gridcells[3][2].bg = (GLfloat*)green3;
	BackgroundColorGrid->gridcells[3][3].bg = (GLfloat*)green4;
	BackgroundColorGrid->gridcells[3][4].bg = (GLfloat*)green5;


	BackgroundColorGrid->gridcells[4][0].bg = (GLfloat*)blue1;
	BackgroundColorGrid->gridcells[4][1].bg = (GLfloat*)blue2;
	BackgroundColorGrid->gridcells[4][2].bg = (GLfloat*)blue3;
	BackgroundColorGrid->gridcells[4][3].bg = (GLfloat*)blue4;
	BackgroundColorGrid->gridcells[4][4].bg = (GLfloat*)blue5;


	BackgroundColorGrid->gridcells[1][4].fg = (GLfloat*)black;
	BackgroundColorGrid->gridcells[2][4].fg = (GLfloat*)black;
	BackgroundColorGrid->gridcells[1][2].fg = (GLfloat*)black;
	BackgroundColorGrid->gridcells[2][2].fg = (GLfloat*)black;

}





//void DropDownColorBox(struct InstanceData* id,MGLGRID *parentgrid)
//{
//	// Create a 5*5 block and fill colors
//	widgets *w, *lastw, *panel_widget;
//	int r;
//
//	assert(id);
//
//	// find last widget in list and add it here
//	for (w = oglgui->widgets; w; w = w->np){
//		lastw = w;
//	}
//
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "DropDownColorsWidget");
//	w->gregion.x1 = 20;
//	w->gregion.y1 = 370;
//	w->gregion.x2 = 400;
//	w->gregion.y2 = 670;
//	panel_widget = w;
//	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
//
//	w->type = WGT_GRID;
//	w->grid = MGLGridCreate(id, parentgrid, "DropDownColorsGrid", 5, 5, w->gregion.x1, w->gregion.y1);
//	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
//	w->grid->iVscrollPos = 0;
//	w->grid->GScrollWidth = 20;
//	w->visible = 1;
//	w->grid->AllowScrollbar = TRUE;
//	w->grid->HasTitleBar = TRUE;
//	// add default colors for grid and the rows
//	w->grid->bg = (GLfloat*)gridgrey0;
//	w->grid->fg = (GLfloat*)white;
//	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
//	parentgrid->AlertsColorGrid = w->grid;
//
//
//
//	for (r = 0; r < w->grid->nrows; r += 2){
//		if (r == 0){
//			w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
//		}
//		else {
//			w->grid->gridrows[r].fg = (GLfloat*)color868686;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)color868686;
//		}
//		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
//		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
//	}
//
//	SetAlertColors(parentgrid);
//
//
//}



widgets *AddImage(struct InstanceData* id, widgets *w, char *widgetname, char *pngname, widgets *lastw, int x1, int x2, int y1, int y2,
	int alpha_mode, int img_blend, int filtertype)
{
	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// similar to AddButton, but no callback for this image
	// draw top buttons Applications, Overlays , Filters ,etc
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		if (lastw){
			lastw->np = w;
		}
		else {
			// first widget
			oglgui->widgets = w;
		}
	}


	w->gregion.x1 = x1;
	w->gregion.y1 = y1;
	w->gregion.x2 = x2;  // image is 3x25
	w->gregion.y2 = y2;
	w->type = WGT_PANEL_IMAGE;
	w->visible = 1;
	w->active_image = pngname;
	w->img_alpha_mode = alpha_mode;
	w->img_blend = img_blend;
	w->filtertype = filtertype;
	id->m_poglgui = oglgui;
	return(w);
}





void MGLSetVisibleWidgetLL(widgets *startw, widgets *endw, int visible)
{
	widgets *w;

	for (w = startw; w; w = w->np){
		w->visible = visible;
		if (w == endw){
			return;
		}
	}


}


int SliderArrowOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	widgets *lastw;

	lastw = ZoomSliderW->np->np->np->np->np;
	MGLSetVisibleWidgetLL(ZoomSliderW, lastw, 0);

	return(TRUE);
}




widgets *MGLAddSlider(struct InstanceData* id, widgets *w, char *widgetname, GLfloat *bg, widgets *lastw, int x1, int x2, int y1, int y2,
	GLfloat minval, GLfloat maxval, GLfloat curval)
{
	int slidery, sliderbarx1, sliderbarx2, sliderwidth;   // pixel coordinates of slider background bar
	int imgxsize, imgysize;
	gui* oglgui = (gui*)id->m_poglgui;

	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		if (lastw){
			lastw->np = w;
		}
		else {
			// first widget
			oglgui->widgets = w;
		}
	}
	w->gregion.x1 = x1;
	w->gregion.y1 = y1;
	w->gregion.x2 = x2;
	w->gregion.y2 = y2;
	w->width = w->gregion.x2 - w->gregion.x1;
	w->height = w->gregion.y2 - w->gregion.y1;
	w->bg = (GLfloat*)bg;

	w->type = WGT_SLIDER;
	w->visible = 1;
	w->IsActive = 0;

	w->minval = minval;
	w->maxval = maxval;
	w->curval = curval;
	// add left edge widget


	slidery = y1 + 10;
	LeftSliderEdgeW = AddImage(id, LeftSliderEdgeW, "LeftSliderEdgeW", "slider-large-left-edge.png", w,
		x1 + 10, x1 + 10 + 4, slidery, slidery + 7,
		ALPHA_MODE_BLACK, 0, GL_NEAREST);


	sliderwidth = 80;
	sliderbarx1 = LeftSliderEdgeW->gregion.x2;
	sliderbarx2 = sliderbarx1 + sliderwidth;

	SliderBarBGW = AddImage(id, SliderBarBGW, "SliderBarBGW", "slider-large-1px.png", LeftSliderEdgeW,
		sliderbarx1, sliderbarx2, slidery, slidery + 7,
		ALPHA_MODE_BLACK, 0, GL_NEAREST);


	RightSliderEdgeW = AddImage(id, RightSliderEdgeW, "RightSliderEdgeW", "slider-large-right-edge.png", SliderBarBGW,
		sliderbarx2, sliderbarx2 + 4, slidery, slidery + 7,
		ALPHA_MODE_BLACK, 0, GL_NEAREST);


	// add arrow for user to close slider
	imgxsize = 10;
	imgysize = 8;
	SliderArrowW = MGLAddButton(id, SliderArrowW, "SliderArrowW", "down-reorder-arrow.png", "down-reorder-arrow.png", RightSliderEdgeW,
		RightSliderEdgeW->gregion.x2 + 4, RightSliderEdgeW->gregion.x2 + 4 + imgxsize, slidery, slidery + imgysize,
		ALPHA_MODE_BLACK, 1, GL_LINEAR);
	SliderArrowW->ImgAngle = -90.0;
	SliderArrowW->OnClickCB = SliderArrowOnClickCB;


	SliderBarW = AddImage(id, SliderBarW, "SliderBarW", "slider-large-needle.png", SliderArrowW,
		sliderbarx1, sliderbarx1 + 6, slidery, slidery + 19,
		ALPHA_MODE_BLACK, 0, GL_NEAREST);

	id->m_poglgui = oglgui;
	return(w);
}





widgets *MGLAddButton(struct InstanceData* id, widgets *w, char *widgetname, char *activepng, char *inactivepng, widgets *lastw, int x1, int x2, int y1, int y2,
	int alpha_mode, int img_blend, int filtertype)
{

	gui* oglgui = (gui*)id->m_poglgui;
	// draw top buttons Applications, Overlays , Filters ,etc
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		if (lastw){
			lastw->np = w;
		}
		else {
			// first widget
			oglgui->widgets = w;
		}
	}
	w->gregion.x1 = x1;
	w->gregion.y1 = y1;
	w->gregion.x2 = x2;  // image is 3x25
	w->gregion.y2 = y2;
	w->type = WGT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;
	w->active_image = activepng;
	w->inactive_image = inactivepng;
	w->img_alpha_mode = alpha_mode;
	w->img_blend = img_blend;
	w->filtertype = filtertype;

	id->m_poglgui = oglgui;
	return(w);
}

//mtm to print on build summary grid-roi grid
char* write_layout_column_roi(char full[64], int vorow)
{
	switch (vorow)
	{
	case 0:
		strcpy_s(full, 64, "Region:Show");
		return full;
	case 1:
		strcpy_s(full, 64, "Flight:Show");
		return full;
	case 2:
		strcpy_s(full, 64, "Start:Show");
		return full;
	case 3:
		strcpy_s(full, 64, "End:Show");
		return full;
	case 4:
		strcpy_s(full, 64, "Elapsed:Show");
		return full;
	case 5:
		strcpy_s(full, 64, "In:Show");
		return full;
	case 6:
		strcpy_s(full, 64, "A/D:Show");
		return full;
	case 7:
		strcpy_s(full, 64, "Fix:Show");
		return full;
	}
	return full;
}

//mtm to print on build summary grid-fdo grid
char* write_layout_column_fdo(char full[64], int vorow, MGLGRID *FDOGrid)
{
	VO *vcol;

	full[0] = '\0';
	if (vorow < FDOGrid->vo->vcols->count){
		vcol = V_ROW_PTR(FDOGrid->vo->vcols, vorow);
		strcpy_s(full, 64, vcol->label);
		strcat_s(full,64, ":Show");
	}
	return full;
}
char* write_layout_column_SA(char full[64], int vorow, MGLGRID *SysAlertsGrid)
{
	VO *vcol;

	full[0] = '\0';
	if (vorow < SysAlertsGrid->vo->vcols->count){
		vcol = V_ROW_PTR(SysAlertsGrid->vo->vcols, vorow);
		strcpy_s(full, 64, vcol->label);
		strcat_s(full, 64, ":Show");
	}
	return full;
}

void ToggleGridShow(struct InstanceData* id, MGLGRID *topgrid, int r, int vorow)
{
	int curval;
	char *enter, *criteria, *parameter, *layout, *rowcol;
	char full[512];
	int m, i;
	int t = 10;
	int count = 0;
	int value;
	char check[64] = { 0 };

	assert(topgrid);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* WarningGrid = (MGLGRID*)id->WarningGrid;

	for (i = 0; i < topgrid->SortAvailableGrid->vo->count; i++)
	{
		value = VV(topgrid->SortAvailableGrid->vo, i, "Visible", int);
		if (value == 1)
			count++;

	}

	if (count < 15)
	{
		topgrid->ColLabels[vorow];
		if (vorow >= 0 && topgrid->SortAvailableGrid && vorow < topgrid->SortAvailableGrid->vo->count){
			curval = VV(topgrid->SortAvailableGrid->vo, vorow, "Visible", int);

			if (curval){
				if (count > 1)
				{
					// turn off
					// reverse the video colors
					VV(topgrid->SortAvailableGrid->vo, vorow, "Visible", int) = FALSE;
					topgrid->DisplayCol[vorow] = -1;

					//mtm-- To enter in the build summary grid
					if (topgrid->roi_fdo == 1){
						write_layout_column_roi(full, vorow);//to write into build summary if grid is roi
					}
					else if (topgrid->roi_fdo == 2){
						write_layout_column_fdo(full, vorow, SecondTabGrid->childgrid);//to write into build summary if grid is fdo
					}
					else if (topgrid->roi_fdo == 3){
						write_layout_column_SA(full, vorow, SecondTabGrid->childgrid);//to write into build summary if grid is fdo
					}
					for (m = 0; m < SecondTabGrid->childgrid->BuildSummaryGrid->vo->count; m++)
					{
						enter = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Layout", char);
						if (strcmp(full, enter) == 0)
							break;

					}
					if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count >m)
						strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Layout", char), 64, "", _TRUNCATE);
					for (int j = m + 1; j < SecondTabGrid->childgrid->BuildSummaryGrid->vo->count; j++)
					{
						enter = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, j, "Layout", char);
						strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, j - 1, "Layout", char), 64, enter, _TRUNCATE);
						if (j == SecondTabGrid->childgrid->BuildSummaryGrid->vo->count - 1)
							strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, j, "Layout", char), 64, "", _TRUNCATE);

					}
					if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count - 1 == m)
					{
						layout = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Layout", char);
						parameter = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Parameters", char);
						criteria = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Criteria", char);
						rowcol = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Visiblerows/columns", char);
						if ((strcmp(criteria, "") == 0) && (strcmp(parameter, "") == 0) && (strcmp(layout, "") == 0) && (strcmp(rowcol, "") == 0))
							vo_rm_row(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m);
					}

				}

			}
			else {
				VV(topgrid->SortAvailableGrid->vo, vorow, "Visible", int) = TRUE;
				topgrid->DisplayCol[vorow] = MGLNextVisCol(topgrid, vorow);
				if (topgrid->roi_fdo == 1){
					write_layout_column_roi(full, vorow);//to write into build summary if grid is roi
				}
				else if (topgrid->roi_fdo == 2){
					write_layout_column_fdo(full, vorow,topgrid);//to write into build summary if grid is fdo
				}
				else if (topgrid->roi_fdo == 3){
					write_layout_column_SA(full, vorow, topgrid);//to write into build summary if grid is fdo
				}
				for (m = 0; m < SecondTabGrid->childgrid->BuildSummaryGrid->vo->count; m++)
				{
					enter = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Layout", char);
					if (strcmp(enter, "") == 0)
						break;

				}
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= m)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Layout", char), 64, full, _TRUNCATE);
				

			}
		}
	}

	else if (count >= 15)
	{
		if (vorow >= 0 && topgrid->SortAvailableGrid && vorow < topgrid->SortAvailableGrid->vo->count){
			curval = VV(topgrid->SortAvailableGrid->vo, vorow, "Visible", int);

			if (curval){
				// turn off
				// reverse the video colors
				VV(topgrid->SortAvailableGrid->vo, vorow, "Visible", int) = FALSE;
				topgrid->DisplayCol[vorow] = -1;

				//mtm-- To enter in the build summary grid
				//if (topgrid->roi_fdo == 1){
				//	write_layout_column_roi(full, vorow);//to write into build summary if grid is roi
				//}
				//else if (topgrid->roi_fdo == 2){
				//	write_layout_column_fdo(full, vorow);//to write into build summary if grid is fdo
				//}
				//for (m = 0; m < SecondTabGrid->childgrid->BuildSummaryGrid->vo->count; m++)
				//{
				//	enter = VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Layout", char);
				//	if (strcmp(enter, "") == 0)
				//		break;

				//}
				//if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= m)
				//	vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				//strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, m, "Layout", char), 64, full, _TRUNCATE);

				//OutputDebugString("Turn off Filter color\n");
				//SortAvailableGrid->gridrows[r].fg = (GLfloat*) gridgreyelev;
				//SortAvailableGrid->gridrows[r].bg = (GLfloat*) white;
			}
			else if (curval == 0)
			{
				WarningGrid = CreateWarning(id);
				id->WarningGrid = WarningGrid;
				WarningGrid->parent_w->visible = TRUE;
			}

		}
	}
}

//mtm
void ToggleGridShow_dataapp(MGLGRID *topgrid, int r, int vorow)
{
	int curval;

	assert(topgrid);

	if (vorow >= 0 && (strcmp(topgrid->name, "DataAppSelectorGrid") == 0) && vorow < topgrid->vo->count){
		curval = VV(topgrid->vo, vorow, "Visible", int);
		if (curval){
			// turn off
			// reverse the video colors
			VV(topgrid->vo, vorow, "Visible", int) = FALSE;
			topgrid->DisplayCol[vorow] = -1;
			//OutputDebugString("Turn off Filter color\n");
			//SortAvailableGrid->gridrows[r].fg = (GLfloat*) gridgreyelev;
			//SortAvailableGrid->gridrows[r].bg = (GLfloat*) white;
		}
		else {
			VV(topgrid->vo, vorow, "Visible", int) = TRUE;
			topgrid->DisplayCol[vorow] = MGLNextVisCol_dataapp(topgrid, vorow);
			//OutputDebugString("Turn ON Filter color\n");
			//SortAvailableGrid->gridrows[r].fg = (GLfloat*) white;
			//SortAvailableGrid->gridrows[r].bg = (GLfloat*) gridgreyelev;
		}
	}
}

void TurnOffSortColumn(MGLGRID *SortAvailableGrid, int vorow)
{
	int oldpriority, priority, i;

	assert(SortAvailableGrid);

	// add this row as next sort value
	oldpriority = VV(SortAvailableGrid->vo, vorow, "SortPriority", int);

	VV(SortAvailableGrid->vo, vorow, "SortPriority", int) = 0;

	// need to reorder the remaining values
	for (i = 0; i < SortAvailableGrid->vo->count; i++){
		priority = VV(SortAvailableGrid->vo, i, "SortPriority", int);
		if (priority > oldpriority){
			VV(SortAvailableGrid->vo, i, "SortPriority", int) = priority - 1;
		}
	}
}


int MGLNSortColumns(MGLGRID *SortAvailableGrid)
{
	int i, sortval, curcount;

	assert(SortAvailableGrid);

	// count how many columns are currently being sorted
	for (i = 0, curcount = 0; i < SortAvailableGrid->vo->count; i++){
		sortval = VV(SortAvailableGrid->vo, i, "SortVal", int);
		if (sortval){
			curcount++;
		}
	}
	return(curcount);
}




void TurnOnSortColumn(MGLGRID *SortAvailableGrid, int vorow)
{
	int numsortcols;

	assert(SortAvailableGrid);

	// add this sort as the lowest priority, highest number
	numsortcols = MGLNSortColumns(SortAvailableGrid);

	// add this row as next sort value
	VV(SortAvailableGrid->vo, vorow, "SortPriority", int) = numsortcols;

}

void MGLSortColumnMoveDown(MGLGRID *topgrid, MGLGRID *SortAvailableGrid, int vorow)
{
	int saveval;

	assert(topgrid);

	// change the row order in SortAvailableGrid->vo, and DisplayCol order in source grid

	if (vorow >= 0 && vorow < (topgrid->ncols - 1)){
		saveval = topgrid->DisplayCol[vorow];
		topgrid->DisplayCol[vorow] = topgrid->DisplayCol[vorow + 1]; // default location is same of col number order
		topgrid->DisplayCol[vorow + 1] = saveval;

		vo_move_row(topgrid->SortAvailableGrid->vo, vorow, vorow + 1);
	}


}

void MGLGridColumnMoveDown(MGLGRID *DataAppGrid, int vorow)
{

	assert(DataAppGrid);

	int save_showcheckbox;
	int save_rowcheckbox;
	int save_activecheckbox;

	save_showcheckbox = DataAppGrid->show_checkbox[vorow];
	save_rowcheckbox = DataAppGrid->row_checkbox[vorow];
	save_activecheckbox = DataAppGrid->active_checkbox[vorow];

	// change the row order in DataAppGrid->vo

	if (vorow >= 0 && vorow < DataAppGrid->vo->count - 2){
		vo_move_row(DataAppGrid->vo, vorow, vorow + 1);
		DataAppGrid->show_checkbox[vorow] = DataAppGrid->show_checkbox[vorow + 1];
		DataAppGrid->show_checkbox[vorow + 1] = save_showcheckbox;
		DataAppGrid->row_checkbox[vorow] = DataAppGrid->row_checkbox[vorow + 1];
		DataAppGrid->row_checkbox[vorow + 1] = save_rowcheckbox;
		DataAppGrid->active_checkbox[vorow] = DataAppGrid->active_checkbox[vorow + 1];
		DataAppGrid->active_checkbox[vorow + 1] = save_activecheckbox;
	}


}

void MGLSortPriorityMinus(struct InstanceData* id, MGLGRID *SortAvailableGrid, int vorow)
{
	int SortPriority, i, SortVal, pri, changePriority;

	assert(id);
	assert(SortAvailableGrid);

	SortPriority = VV(SortAvailableGrid->vo, vorow, "SortPriority", int);

	SortVal = VV(SortAvailableGrid->vo, vorow, "SortVal", int);
	if (!SortVal){
		// this value is not set to Asc or Dsc, so it is off, ignore
		return;
	}


	changePriority = SortPriority + 1;

	for (i = 0; i < SortAvailableGrid->vo->count; i++){

		SortVal = VV(SortAvailableGrid->vo, i, "SortVal", int);
		if (!SortVal){
			// this value is not set to Asc or Dsc, so it is off, ignore
			continue;
		}
		pri = VV(SortAvailableGrid->vo, i, "SortPriority", int);
		if (pri == changePriority){
			// swap with selected priority
			VV(SortAvailableGrid->vo, i, "SortPriority", int) = SortPriority;
			VV(SortAvailableGrid->vo, vorow, "SortPriority", int) = changePriority;
			return;
		}
	}

	// should only get here is there is no lower priority to switch with

}



void MGLSortPriorityPlus(struct InstanceData* id, MGLGRID *SortAvailableGrid, int vorow)
{
	int SortPriority, i, SortVal, pri, changePriority;

	assert(id);
	assert(SortAvailableGrid);

	SortPriority = VV(SortAvailableGrid->vo, vorow, "SortPriority", int);

	if (!SortPriority){
		// already highest priority == 0 ){
		return;
	}

	SortVal = VV(SortAvailableGrid->vo, vorow, "SortVal", int);
	if (!SortVal){
		// this value is not set to Asc or Dsc, so it is off, ignore
		return;
	}


	changePriority = SortPriority - 1;

	for (i = 0; i < SortAvailableGrid->vo->count; i++){

		SortVal = VV(SortAvailableGrid->vo, i, "SortVal", int);
		if (!SortVal){
			// this value is not set to Asc or Dsc, so it is off, ignore
			continue;
		}
		pri = VV(SortAvailableGrid->vo, i, "SortPriority", int);
		if (pri == changePriority){
			// swap with selected priority
			VV(SortAvailableGrid->vo, i, "SortPriority", int) = SortPriority;
			VV(SortAvailableGrid->vo, vorow, "SortPriority", int) = changePriority;
			return;
		}
	}

	// should not get here
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "should not get here\n");
}




void MGLSortColumnMoveUp(MGLGRID *topgrid, MGLGRID *SortAvailableGrid, int vorow)
{
	int saveval;
	// change the row order in SortAvailableGrid->vo, and DisplayCol order in source grid

	if (vorow > 0){
		saveval = topgrid->DisplayCol[vorow];
		topgrid->DisplayCol[vorow] = topgrid->DisplayCol[vorow - 1]; // default location is same of col number order
		topgrid->DisplayCol[vorow - 1] = saveval;


		vo_move_row(topgrid->SortAvailableGrid->vo, vorow, vorow - 1);
	}


}

void MGLGridColumnMoveUp(MGLGRID *DataAppGrid, int vorow)
{

	assert(DataAppGrid);

	// change the row order in DataAppGrid->vo
	int save_showcheckbox;
	int save_rowcheckbox;
	int save_activecheckbox;

	save_showcheckbox = DataAppGrid->show_checkbox[vorow];
	save_rowcheckbox = DataAppGrid->row_checkbox[vorow];
	save_activecheckbox = DataAppGrid->active_checkbox[vorow];

	if (vorow > 0 && vorow < DataAppGrid->vo->count - 1){
		vo_move_row(DataAppGrid->vo, vorow, vorow - 1);

		DataAppGrid->show_checkbox[vorow] = DataAppGrid->show_checkbox[vorow - 1];
		DataAppGrid->show_checkbox[vorow - 1] = save_showcheckbox;
		DataAppGrid->row_checkbox[vorow] = DataAppGrid->row_checkbox[vorow - 1];
		DataAppGrid->row_checkbox[vorow - 1] = save_rowcheckbox;
		DataAppGrid->active_checkbox[vorow] = DataAppGrid->active_checkbox[vorow - 1];
		DataAppGrid->active_checkbox[vorow - 1] = save_activecheckbox;
	}



}



void ToggleSortValue(MGLGRID *topgrid, int vorow)
{
	int curval;
	MGLGRID *SortAvailableGrid;

	if (!topgrid){
		return;
	}

	SortAvailableGrid = topgrid->SortAvailableGrid;

	curval = VV(SortAvailableGrid->vo, vorow, "SortVal", int);

	// increment one higher, then take MOD 3 value
	curval++;
	curval = curval % 3;
	VV(SortAvailableGrid->vo, vorow, "SortVal", int) = curval;

	// update sort priority based on new selection
	switch (curval)
	{
	case 0: // OFF, user turned OFF sort
		TurnOffSortColumn(SortAvailableGrid, vorow);
		break;
	case 1:  // Asc , user turned ON Sort
		TurnOnSortColumn(SortAvailableGrid, vorow);
		break;
	case 2:  // Dsc - user switched to Dsc sort, no need to change sort priority
		break;
	default:
		break;
	}
}




#ifdef OLD_REGULAR_EXPRESSION
/* match: search for regexp anywhere in text */
int match(char *regexp, char *text)
{
	extern int matchhere(char *regexp, char *text);


	/* a more advanced regex code can be found here:
	ftp://ftp.sgi.com/other/ARCHIVE/s5/archie/regex.c

	we should implement this when we get a bit of time as it is more powerful and supports more constructs
	*/


	/*

	see http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
	for more details

	c    matches any literal character c
	.    matches any single character
	^    matches the beginning of the input string
	$    matches the end of the input string
	*    matches zero or more occurrences of the previous character

	example  ^AAL  matches AAL1 or AAL followed by any text
	example  AAL* matches  AALL  or AALLL



	if ( match("*df*", "adfrt") ) // fails because no char before first wildcard

	if ( match("df", "adfrt") )   //  matches

	if ( match("AAL*", "AALL") )  //  matches

	if ( match("AAL*", "AAL22") )//  matches

	if ( match("AAL*", "AWL1593") ){ /// fails

	if ( match("AAL*", "AAL") ){ // matches  even with no other characters

	*/

	if (regexp[0] == '^')
		return matchhere(regexp + 1, text);
	do {    /* must look even if string is empty */
		if (matchhere(regexp, text))
			return 1;
	} while (*text++ != '\0');
	return 0;
}

/* matchhere: search for regexp at beginning of text */
int matchhere(char *regexp, char *text)
{
	extern int matchstar(int c, char *regexp, char *text);

	if (regexp[0] == '\0')
		return 1;
	if (regexp[1] == '*')
		return matchstar(regexp[0], regexp + 2, text);
	if (regexp[0] == '$' && regexp[1] == '\0')
		return *text == '\0';
	if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
		return matchhere(regexp + 1, text + 1);
	return 0;
}

/* matchstar: search for c*regexp at beginning of text */
int matchstar(int c, char *regexp, char *text)
{
	do {    /* a * matches zero or more instances */
		if (matchhere(regexp, text))
			return 1;
	} while (*text != '\0' && (*text++ == c || c == '.'));
	return 0;
}

#endif



#ifdef OLD_EXAMPLE
// test regex code


if (re_comp("AAL.*")){
	// some error
}
else if (re_exec("AAL22") == 1){
	// matches
	i = 1;
}
if (re_comp("AAL.*")){
	// some error
}
else if (re_exec("AAR22") == 1){
	// matches
	i = 1;
}
if (re_comp("AAL.*")){
	// some error
}
else if (re_exec("AAL22") == 1){
	// matches
	i = 1;
}
if (re_comp("fo[ob]a[rz]")){
	// some error
}
else if (re_exec("fobar") == 1){
	// matches
	i = 1;
}
if (re_comp("fo[ob]a[rz]")){
	// some error
}
else if (re_exec("fooar") == 1){
	// matches
	i = 1;
}
if (re_comp("fo[ob]a[rz]")){
	// some error
}
else if (re_exec("fooaz") == 1){
	// matches
	i = 1;
}
if (re_comp("fo[ob]a[rz]")){
	// some error
}
else if (re_exec("foxxbar") == 1){
	// matches
	i = 1;
}

if (re_comp("\\(fo.*\\)-\\1")){
	// some error
}
else if (re_exec("foo-foo") == 1){
	// matches
	i = 1;
}
if (re_comp("\\(fo.*\\)-\\1")){
	// some error
}
else if (re_exec("fob-fob") == 1){
	// matches
	i = 1;
}
if (re_comp("\\(foo\\)[1-3]\\1")){
	// some error
}
else if (re_exec("foo1foo") == 1){
	// matches
	i = 1;
}



if (match("*df*", "adfrt")){
	// matches
	i = 1;
}
if (match("df", "adfrt")){
	// matches
	i = 1;
}
if (match("AAL*", "AALL")){
	// matches
	i = 1;
}
if (match("AAL*", "AAL22")){
	// matches
	i = 1;
}
if (match("AAL*", "AWL1593")){
	// matches
	i = 1;
}
if (match("AAL*", "AAL")){
	// matches
	i = 1;
}
#endif





int SortCallback(struct InstanceData* id, MGLGRID *SortAvailableGrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int r, c, vorow;
	MGLGRID *parentgrid = SortAvailableGrid->parentgrid;

	assert(id);
	assert(SortAvailableGrid);

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	// make sure there is at least one action gridconfig
	if (!parentgrid->Actions[parentgrid->CurActionNum].gridconfig){
		parentgrid->Actions[parentgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
	}

	if (SortAvailableGrid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - SortAvailableGrid->gridrect.y1) / SortAvailableGrid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - SortAvailableGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= SortAvailableGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = SortAvailableGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}

	vorow = r - 1 + SortAvailableGrid->iVscrollPos;

	// see which column was selected
	c = MGLCalcCol(SortAvailableGrid, x);

	if (c < 2){
		ToggleGridShow(id, parentgrid, r, vorow);
	}
	if (c == 2){
		// MoveColumn Up
		MGLSortColumnMoveUp(parentgrid, SortAvailableGrid, vorow);
	}
	if (c == 3){
		// MoveColumn Down
		MGLSortColumnMoveDown(parentgrid, SortAvailableGrid, vorow);
	}
	if (c == 4){
		// toggle Sort Value
		ToggleSortValue(parentgrid, vorow);
	}
	if (c == 6){
		// sort priority plus, means lower priority number
		MGLSortPriorityPlus(id, SortAvailableGrid, vorow);
	}
	if (c == 7){
		// sort priority plus, means lower priority number
		MGLSortPriorityMinus(id, SortAvailableGrid, vorow);
	}



	if (parentgrid->UpdateCB){
		parentgrid->UpdateCB(id, parentgrid);
	}

	if (parentgrid->UpdateSACB){
		parentgrid->UpdateSACB(id, parentgrid);
	}

	// for grid recalc so that columns move, etc
	// parentgrid->lastcalccol = 0;
	MGLGridMove(id, parentgrid, parentgrid->gridrect.x1, parentgrid->gridrect.y1);
	return(TRUE);
}





void CreateSortPanel(struct InstanceData* id, MGLGRID *parentgrid)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);
	assert(parentgrid);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SortPanel");
	//w->gregion.x1 = 20;
	//w->gregion.y1 = 370;
	//w->width = 500;
	//w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}


	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "AvailableGridWidget");
	/*w->gregion.x1 = 120;
	w->gregion.y1 = 470;
	w->gregion.x2 = 500;
	w->gregion.y2 = 770;*/


	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "AvailableGrid", 9, 8, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;  // not visible until activated via menu selection
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->SortAvailableGrid = w->grid;




	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		w->grid->gridrows[r].bg = (GLfloat*)SubGrid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)SubGrid_grey2;
	}


	w->grid->gridrows[0].bg = (GLfloat*)SubGrid_grey1;
	w->grid->gridrows[0].fg = (GLfloat*)white;

#ifdef OLDCOLORS
	for (r = 0; r < w->grid->nrows; r++){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)yellow;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)gridgreyelev;
	}
#endif
	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	w->grid->gridcols[1].val_callback = Cnv2HideShow;
	w->grid->gridcols[2].val_callback = Cnv2Plus;
	w->grid->gridcols[3].val_callback = Cnv2Minus;
	w->grid->gridcols[4].val_callback = Cnv2SortAscDsc;

	w->grid->gridcols[5].val_callback = Cnv2SortPriority;
	w->grid->gridcols[6].val_callback = Cnv2Plus;
	w->grid->gridcols[7].val_callback = Cnv2Minus;


	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Available", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Visible", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Move Up", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Down  ", _TRUNCATE);

	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Sort", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "Priority", _TRUNCATE);

	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, " + ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[7], MGL_GRID_LABEL_MAX, " -", _TRUNCATE);



	// draw the up and down re-order images 
	strncpy_s(w->grid->gridcols[2].pngfilename, sizeof(w->grid->gridcols[4].pngfilename), "up-reorder-arrow1.png", _TRUNCATE);
	w->grid->gridcols[2].texturesize = 16;
	w->grid->gridcols[2].img_displaysize = 10;
	strncpy_s(w->grid->gridcols[3].pngfilename, sizeof(w->grid->gridcols[5].pngfilename), "down-reorder-arrow1.png", _TRUNCATE);
	w->grid->gridcols[3].texturesize = 16;
	w->grid->gridcols[3].img_displaysize = 10;

	strncpy_s(w->grid->gridcols[6].pngfilename, sizeof(w->grid->gridcols[4].pngfilename), "up-reorder-arrow1.png", _TRUNCATE);
	w->grid->gridcols[6].texturesize = 16;
	w->grid->gridcols[6].img_displaysize = 10;
	strncpy_s(w->grid->gridcols[7].pngfilename, sizeof(w->grid->gridcols[5].pngfilename), "down-reorder-arrow1.png", _TRUNCATE);
	w->grid->gridcols[7].texturesize = 16;
	w->grid->gridcols[7].img_displaysize = 10;


	// we also need to fetch initial filter values from user settings DB

	// set row callbacks
	for (i = 0; i < parentgrid->SortAvailableGrid->nrows; i++){
		parentgrid->SortAvailableGrid->gridrows[i].row_callback = SortCallback;
	}




	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->SortAvailableGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->SortAvailableGrid->marginx;
	w->gregion.y1 = parentgrid->SortAvailableGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->SortAvailableGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	//strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Layout", _TRUNCATE);
	w->visible = 1;
	w->grid = parentgrid->SortAvailableGrid;

	id->m_poglgui = oglgui;

}



void CrAvailableGridVO(MGLGRID *parentgrid)
{

	VO *AvailableVO;

	assert(parentgrid);

	AvailableVO = (VO *)vo_create(0, NULL);
	vo_set(AvailableVO, V_NAME, "AvailableVO", NULL);
	VOPropAdd(AvailableVO, "Column", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(AvailableVO, "Visible", INTBIND, -1, VO_NO_ROW);
	VOPropAdd(AvailableVO, "CPlus", INTBIND, -1, VO_NO_ROW);  // 0 is Highest
	VOPropAdd(AvailableVO, "CMinus", INTBIND, -1, VO_NO_ROW);  // 0 is Highest
	// this row's value is Active and should NOT be filtered out
	VOPropAdd(AvailableVO, "SortVal", INTBIND, -1, VO_NO_ROW);  // 0 = OFF, 1 = Asc, 2 = Dsc
	VOPropAdd(AvailableVO, "SortPriority", INTBIND, -1, VO_NO_ROW);  // 0 is Highest

	VOPropAdd(AvailableVO, "Plus", INTBIND, -1, VO_NO_ROW);  // 0 is Highest
	VOPropAdd(AvailableVO, "Minus", INTBIND, -1, VO_NO_ROW);  // 0 is Highest

	vo_set(AvailableVO, V_ORDER_COLS, "Column", NULL, NULL);

	parentgrid->SortAvailableGrid->vo = AvailableVO;

}


void FillAvailableColsVO(struct InstanceData* id, MGLGRID *grid)
{
	int i;
	VO *vo, *vcol;

	assert(id);
	assert(grid);

	// fill a vo with all the unique values in this columns for all rows
	if (!grid->SortAvailableGrid->vo){
		CrAvailableGridVO(grid);
	}
	else {
		// already created the SortAvailableGrid->vo 
		//  fall through and load in column data? return;
	}
	vo = grid->vo;
	vo_rm_rows(grid->SortAvailableGrid->vo, 0, grid->SortAvailableGrid->vo->count);



	if (!grid) return;

	if (grid->vo){

		for (i = 0; i < grid->vo->vcols->count; i++){	
			if (grid->roi_fdo == 1)
			{
				vcol = V_ROW_PTR(vo->vcols, i);
				if (strcmp(vcol->label, "") != 0)
				{
					vo_alloc_rows(grid->SortAvailableGrid->vo, 1);
					if (i == 9)
						strncpy_s(VP(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Column", char), 64, "A/D", _TRUNCATE);
					else if (i == 8)
						strncpy_s(VP(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Column", char), 64, "In", _TRUNCATE);
					else
						strncpy_s(VP(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Column", char), 64, vcol->label, _TRUNCATE);
					VV(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Visible", int) = TRUE;
				}
			}

			else if (grid->roi_fdo == 2)
			{
				if (strlen(grid->ColOrderNames[i])){
					vcol = V_ROW_PTR(vo->vcols, i);
					vo_alloc_rows(grid->SortAvailableGrid->vo, 1);
					strncpy_s(VP(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Column", char), 64, vcol->label, _TRUNCATE);
					VV(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Visible", int) = TRUE;
				}
			}

			else if (grid->roi_fdo == 3)
			{
				if (strlen(grid->ColOrderNames[i])){
					vcol = V_ROW_PTR(vo->vcols, i);
					vo_alloc_rows(grid->SortAvailableGrid->vo, 1);
					strncpy_s(VP(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Column", char), 64, vcol->label, _TRUNCATE);
					VV(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Visible", int) = TRUE;
				}
			}
		}
	}
	else {
		// use values stored in grid??? not really needed for non-vo grids ??? may have to implement later
		return;
	}

}




void FillActionsGrid(struct InstanceData *id, MGLGRID *parentgrid, int IsVisible)
{
	int yloc;

	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	FillActionsVO(id, parentgrid);


	yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid
	if (parentgrid->ActionsGrid){
		parentgrid->ActionsGrid->parent_w->visible = 0;  // set parent widget to display the grid
		MGLGridMove(id, parentgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + SecondTabGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * SecondTabGrid->gridrows[0].height));
	}




}

//added mtm//
void FillBackgroundPanelGrid(struct InstanceData *id, MGLGRID *parentgrid, int IsVisible)
{
	int yloc;
	FillBackgroundPanelVO(id, parentgrid);

	yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid
	if (parentgrid->BackgroundPanel){
		parentgrid->BackgroundPanel->parent_w->visible = 1;  // set parent widget to display the grid
		MGLGridMove(id, parentgrid->BackgroundPanel, 200, yloc);
	}

}

//added by mtm
//void FillMainTabGrid(struct InstanceData *id, MGLGRID *parentgrid, int IsVisible)
//{
//	int yloc;
//	FillBackgroundPanelVO(id, parentgrid);
//
//	yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid
//	if (parentgrid->BackgroundPanel){
//		parentgrid->BackgroundPanel->parent_w->visible = 1;  // set parent widget to display the grid
//		MGLGridMove(id, parentgrid->BackgroundPanel, 200, yloc);
//	}
//
//}

void FillActionsGrid1(struct InstanceData *id, MGLGRID *parentgrid, int IsVisible)
{
	int yloc;
	yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid

	if (!parentgrid->Actions[parentgrid->CurActionNum].gridconfig){
		parentgrid->Actions[parentgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));

		// load in data that is stored in etas-db all-etas in table grid_config
		// LoadUserRoiSettings (id, id->m_strUserName, id->m_strRegionArpt, configname, parentgrid->Actions[parentgrid->CurActionNum].gridconfig);
		parentgrid->nActions = 1; // add only one for now if user did not have any stored
		parentgrid->Actions[0].IsActive = TRUE;
		parentgrid->Actions[0].ActionType = ACTION_FILTER;
	}



	//CreateParametersGrid(id, parentgrid, IsVisible);

	if (!parentgrid->AlertsColorGrid){
		CreateAlertsColorGrid(id, parentgrid, FALSE);
		parentgrid->AlertsColorGrid->parent_w->visible = FALSE;
	}
	parentgrid->AlertsColorGrid->parent_w->visible = FALSE;
	MGLInitAlertsColorsVO(parentgrid);

	//MGLInitAlertsColorsVO(parentgrid);
	//if (parentgrid->AlertsColorGrid){
	//	parentgrid->AlertsColorGrid->parent_w->visible = TRUE;  // set parent widget to display the grid
	//MGLGridMove(id, parentgrid->AlertsColorGrid, 950, yloc + 75);  // add 75 since this grid has 75 less height than parameters
	//}
	//CalcColWidths(id, parentgrid->AlertsColorGrid);


}


void FillActionsGrid2(struct InstanceData *id, MGLGRID *parentgrid, int IsVisible)
{
	int yloc;

	yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid
	if (parentgrid->ActionsGrid){
		parentgrid->ActionsGrid->parent_w->visible = FALSE;  // set parent widget to display the grid
		//MGLGridMove(id, parentgrid->ActionsGrid, 380, yloc);
	}


	if (!parentgrid->Actions[parentgrid->CurActionNum].gridconfig){
		parentgrid->Actions[parentgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));

		// load in data that is stored in etas-db all-etas in table grid_config
		// LoadUserRoiSettings (id, id->m_strUserName, id->m_strRegionArpt, configname, parentgrid->Actions[parentgrid->CurActionNum].gridconfig);
		parentgrid->nActions = 1; // add only one for now if user did not have any stored
		parentgrid->Actions[0].IsActive = TRUE;
		parentgrid->Actions[0].ActionType = ACTION_FILTER;
	}

	//// Alerts Grid
	if (!parentgrid->ParametersGrid){
		CreateParametersGrid1(id, parentgrid, FALSE);
		parentgrid->ParametersGrid->parent_w->visible = FALSE;
	}
	MGLFillParametersColsVO(parentgrid);

	CalcColWidths(id, parentgrid->ParametersGrid);
	if (parentgrid->ParametersGrid){
		parentgrid->ParametersGrid->parent_w->visible = FALSE;  // set parent widget to display the grid
		//MGLGridMove(id, parentgrid->ParametersGrid, 600, yloc);
	}


}



int GridMenuCallback(struct InstanceData* id, MGLGRID *settingsgrid, int b, int m, int x, int y)
{
	// pop up one of the GUI settings grids
	int r, yloc;
	widgets *panel_widget;
	MGLGRID *parentgrid, *GridSettingsMenu;

	assert(id);
	assert(settingsgrid);

	parentgrid = settingsgrid->parentgrid;
	GridSettingsMenu = parentgrid->GridSettingsMenu;

	if (!parentgrid->GridSettingsMenu){
		return(FALSE);
	}

	if (GridSettingsMenu->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - GridSettingsMenu->gridrect.y1) / GridSettingsMenu->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - GridSettingsMenu->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}

	if (r < 0 || r >= GridSettingsMenu->nrows){
		return(FALSE);
	}
	// reverse the order
	r = GridSettingsMenu->nrows - r - 1;

	if (r == 0){
		// Actions 
		// Alerts Grid
		if (!parentgrid->ActionsGrid){
			CreateActionsGrid(id, parentgrid);
		}
		FillActionsGrid(id, parentgrid, TRUE);
	}
	else if (r == 1){
		// Sorting Grid
		if (!parentgrid->SortAvailableGrid){
			CreateSortPanel(id, GridSettingsMenu->parentgrid);
		}
		if (parentgrid->SortAvailableGrid){
			parentgrid->SortAvailableGrid->parent_w->visible = TRUE;  // set parent widget to display the grid
			panel_widget = parentgrid->SortAvailableGrid->parent_w->gregion_parentw;
			MGLDrawPanel_new(id, panel_widget);  // calculate actual coordinates from relative panel coordinates
			yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid
			MGLGridMove(id, parentgrid->SortAvailableGrid, panel_widget->aregion.x1 + parentgrid->SortAvailableGrid->gridrows[0].height, yloc);
		}
		FillAvailableColsVO(id, parentgrid);
		//		if ( parentgrid->SortSelectedGrid ){
		//	panel_widget = parentgrid->SortSelectedGrid->parent_w->gregion_parentw;
		// already did this above   MGLDrawPanel(id, panel_widget );  // calculate actual coordinates from relative panel coordinates
		//	MGLGridMove(parentgrid->SortSelectedGrid, panel_widget->aregion.x1 + 300, yloc );
		//}
	}
	else if (r == 2){

		// moved 



	}
	// hide this menu now that a selection has been made
	GridSettingsMenu->parent_w->visible = FALSE;
	return(TRUE);
}

//int ConfirmCallback(struct InstanceData* id, MGLGRID *settingsgrid,/*MGLGRID *npg,*/ int b, int m, int x, int y)
//{
//	// pop up one of the GUI settings grids
//	int r;
//	//	widgets *panel_widget;
//	//	MGLGRID *parentgrid, *GridSettingsMenu;
//
//	assert(id);
//	assert(settingsgrid);
//
//
//
//	if (settingsgrid->gridrows[0].height != MGL_DEFAULT){
//		r = (int)floor(((double)y - settingsgrid->gridrect.y1) / settingsgrid->gridrows[0].height);   // only works if rows all have same height
//	}
//	else {
//		r = (int)floor(((double)y - settingsgrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
//	}
//
//	if (r < 0 || r >= settingsgrid->nrows){
//		return(FALSE);
//	}
//	// reverse the order
//	r = settingsgrid->nrows - r - 1;
//
//	if (r == 0){
//
//
//	}
//	else if (r == 1){
//
//	}
//	else if (r == 2){
//
//		// moved 
//
//
//
//	}
//	// hide this menu now that a selection has been made
//	//GridSettingsMenu->parent_w->visible = FALSE;
//	return(TRUE);
//}

//int GridMenuCallback_me(struct InstanceData* id, MGLGRID *settingsgrid, int b, int m, int x, int y)
//{
//
//	// pop up one of the GUI settings grids
//	int r, yloc;
//	widgets *panel_widget;
//	MGLGRID *parentgrid, *GridSettingsMenu;
//
//	assert(id);
//	assert(settingsgrid);
//
//	parentgrid = settingsgrid->parentgrid;
//	GridSettingsMenu = parentgrid->GridSettingsMenu;
//
//	
//
//	//if (!parentgrid->GridSettingsMenu){
//	//	return(FALSE);
//	//}
//
//	//if (GridSettingsMenu->gridrows[0].height != MGL_DEFAULT){
//	//	r = (int)floor(((double)y - GridSettingsMenu->gridrect.y1) / GridSettingsMenu->gridrows[0].height);   // only works if rows all have same height
//	//}
//	//else {
//	//	r = (int)floor(((double)y - GridSettingsMenu->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
//	//}
//
//	//if (r < 0 || r >= GridSettingsMenu->nrows){
//	//	return(FALSE);
//	//}
//	//// reverse the order
//	//r = GridSettingsMenu->nrows - r - 1;
//
//	r = 0;
//
//
//
//	if (r == 1){
//
//				if (!parentgrid->SortAvailableGrid){
//			CreateSortPanel(id, GridSettingsMenu->parentgrid);
//		}
//
//		if (parentgrid->SortAvailableGrid){
//			parentgrid->SortAvailableGrid->parent_w->visible = TRUE;  // set parent widget to display the grid
//			panel_widget = parentgrid->SortAvailableGrid->parent_w->gregion_parentw;
//			MGLDrawPanel(id, panel_widget);  // calculate actual coordinates from relative panel coordinates
//			yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid
//			MGLGridMove(id, parentgrid->SortAvailableGrid, panel_widget->aregion.x1 + 200, yloc);
//		}
//		FillAvailableColsVO(id, parentgrid);
//	
//		
//	}
//	else if (r == 0){
//	//1//
//	if (!parentgrid->ActionsGrid){
//		CreateActionsGrid(id, parentgrid);
//	}
//	FillActionsGrid(id, parentgrid, TRUE);
//
//
//	//2//
//	if (!parentgrid->SortAvailableGrid){
//		CreateSortPanel(id, GridSettingsMenu->parentgrid);
//	}
//
//	
//	FillActionsGrid1(id, parentgrid, TRUE);
//	
//	FillActionsGrid2(id, parentgrid, TRUE);
//
//
//	}
//
//
//
//	// hide this menu now that a selection has been made
//	GridSettingsMenu->parent_w->visible = FALSE;
//	//parentgrid->ParametersGrid->parent_w->visible = FALSE;
//	return(TRUE);
//}


//mtm//


int GridMenuCallback_rules(struct InstanceData* id, MGLGRID *settingsgrid, int b, int m, int x, int y)
{
	//added mtm
	char buffer[64];
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;

	SecondTabGrid->childgrid->TitleGrid->parent_w->visible = 0;
	SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = 1;
	SecondTabGrid->parent_w->visible = 1;
	strcpy_s(buffer, sizeof(buffer), "Create Grid : ");
	strcat_s(buffer,64,SecondTabGrid->childgrid->name);
	MGLSetTitle(SecondTabGrid, buffer);
	MGLGridMove(id, SecondTabGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - ((SecondTabGrid->nrows - 1)* (DataAppGrid->gridrows[0].height)));
	MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, SecondTabGrid->gridrect.x1, SecondTabGrid->gridrect.y1 - ((SecondTabGrid->childgrid->BuildSummaryGrid->nrows + 0.5) * DataAppGrid->gridrows[0].height));
	//SecondTabGrid->childgrid->BuildSummaryGrid->gridrows[0].fg = (GLfloat*)Grid_grey4;
	SecondTabGrid->gridcells[0][0].fg = (GLfloat*)Text_yellow;              //we want the criteria tab to be open as soon as the grid builder tab is opened
	SecondTabGrid->gridcells[0][1].fg = (GLfloat*)Grid_grey4;
	SecondTabGrid->gridcells[0][2].fg = (GLfloat*)Grid_grey4;
	SecondTabGrid->gridcells[0][3].fg = (GLfloat*)Grid_grey4;
	SecondTabGrid->gridcells[0][4].fg = (GLfloat*)Grid_grey4;
	SecondTabGrid->gridcells[0][5].fg = (GLfloat*)Grid_grey4;
	SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = TRUE;
	SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = TRUE;
	SecondTabGrid->childgrid->to_open_build = 0;
	return(TRUE);
}

//int GridMenuCallback_layout(struct InstanceData* id, MGLGRID *settingsgrid, int b, int m, int x, int y)
//{
//
//	// pop up one of the GUI settings grids
//	int  yloc;
//	widgets *panel_widget;
//	MGLGRID *parentgrid, *GridSettingsMenu;
//	
//	assert(id);
//	assert(settingsgrid);
//
//	parentgrid = settingsgrid->parentgrid;
//	GridSettingsMenu = parentgrid->GridSettingsMenu;
//
//
//			if (!parentgrid->SortAvailableGrid){
//			CreateSortPanel(id, GridSettingsMenu->parentgrid);
//		}
//
//		if (parentgrid->SortAvailableGrid){
//			parentgrid->SortAvailableGrid->parent_w->visible = TRUE;  // set parent widget to display the grid
//			panel_widget = parentgrid->SortAvailableGrid->parent_w->gregion_parentw;
//			MGLDrawPanel(id, panel_widget);  // calculate actual coordinates from relative panel coordinates
//			yloc = parentgrid->gridrect.y2 + 20;  // move 20 pixels above parent grid
//			MGLGridMove(id, parentgrid->SortAvailableGrid, panel_widget->aregion.x1 + 200, yloc);
//		}
//		FillAvailableColsVO(id, parentgrid);
//
//	// hide this menu now that a selection has been made
//	GridSettingsMenu->parent_w->visible = FALSE;
//	//parentgrid->ParametersGrid->parent_w->visible = FALSE;
//	return(TRUE);
//}

int ConfirmCallback(struct InstanceData* id, MGLGRID *ConfirmGrid, int b, int m, int x, int y)
{
	// pop up one of the GUI settings grids
	int r,t,z;
	char *AppName, *AppName2, *AppName1;
	MGLGRID *deletegrid;
	int check = 0;

	assert(id);
	assert(ConfirmGrid);

	extern int DeleteUserXgridSettings(struct InstanceData *id, char *user, char *arpt, char *app_name);

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	
	// see which operator the user selected, add it to the field, then hide the operator menu
	if (ConfirmGrid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - ConfirmGrid->gridrect.y1) / ConfirmGrid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - ConfirmGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}

	if (r < 0 || r >= ConfirmGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = ConfirmGrid->nrows - r - 1;

	if (r == 1)
	{
		//if yes is chosen the row in the DataAppGrid is deleted
		for (t = 0; t < 100; t++)
		{
			if (DataAppGrid->dataapp_confirmdelete[t] == 1)
			{
				//delete
				AppName = VP(DataAppGrid->vo, t, "AppName", char);
				if (strcmp(AppName, "") != 0)
				{
					//DataAppGrid->iVscrollPos = 0;
					deletegrid = VV(DataAppGrid->vo, t, "gridptr", void*);

					if (deletegrid)
						deletegrid->parent_w->visible = FALSE;
					if (SecondTabGrid->childgrid)
					{
						if ((strcmp(SecondTabGrid->childgrid->name, AppName) == 0))
						{
							SecondTabGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->TitleGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = 0;
							SecondTabGrid->childgrid->RowColumn->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 0;
							if (SecondTabGrid->childgrid->AlertsColorGrid->AlertsColorGrid)
								SecondTabGrid->childgrid->AlertsColorGrid->AlertsColorGrid->parent_w->visible = 0;
						}
					}

					if (SecondTabGrid->childgrid)
						AppName2 = SecondTabGrid->childgrid->name;
					else
						AppName2 = "Empty";


					if (DataAppGrid->show_checkbox[t] == 20)
					{
						DataAppGrid->show_checkbox[t] = 0;
						for (z = t; z < DataAppGrid->vo->count - 1; z++)
							DataAppGrid->show_checkbox[z] = DataAppGrid->show_checkbox[z + 1];
						DataAppGrid->show_checkbox[DataAppGrid->vo->count - 1] = 0;

					}

					if (DataAppGrid->row_checkbox[t] == 20){
						DataAppGrid->row_checkbox[t] = 0;
						check = 1;
					}

					if (DataAppGrid->active_checkbox[t] == 20)
					{
						DataAppGrid->active_checkbox[t] = 0;
						for (z = t; z < DataAppGrid->vo->count - 1; z++)
							DataAppGrid->active_checkbox[z] = DataAppGrid->active_checkbox[z + 1];
						DataAppGrid->active_checkbox[DataAppGrid->vo->count - 1] = 0;
					}

					if (!strcmp(AppName, "New")){
						// don't delete if new
					}
					else {
						DeleteUserXgridSettings(id, id->m_strUserName, NULL, VP(DataAppGrid->vo, t, "AppName", char));

						if (SecondTabGrid->childgrid)
						{
							if (strcmp(VP(DataAppGrid->vo,t, "AppName", char), SecondTabGrid->childgrid->BuildSummaryGrid->name) == 0)
							{
								SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = 0;
							}
						}
						vo_rm_row(DataAppGrid->vo, t);
						SecondTabGrid->aircraftcolor_change_MLAT = 1;
						SecondTabGrid->aircraftcolor_change_ASDEX = 1;
						SecondTabGrid->aircraftcolor_change_Noise = 1;
						SecondTabGrid->aircraftcolor_change_Gate = 1;
						SecondTabGrid->aircraftcolor_change_ASD = 1;
						SecondTabGrid->aircraftcolor_change = 1;
						if ((DataAppGrid->vo->count > 0) && (DataAppGrid->vo->count < 9))
						{
							MGLGridMove(id, DataAppCreater, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1 - (2 * MGL_GRID_DEF_ROW_HEIGHT));
							DataAppGrid->nrows--;

						}
						DataAppGrid->iVscrollPos = 0;
					}




					if (check == 0){
						if (AppName2 != "Empty"){
							for (int ii = 0; ii < 100; ii++)
							{
								DataAppGrid->row_checkbox[ii] = 0;

							}

							for (int ii = 0; ii < DataAppGrid->vo->count; ii++)
							{
								AppName1 = VP(DataAppGrid->vo, ii, "AppName", char);

								if (!(strcmp(AppName1, "New") == 0)){
									if (strcmp(AppName1, AppName2) == 0){
										DataAppGrid->row_checkbox[ii] = 20;
										break;
									}
								}


							}
						}
					}


				}				
				break;
			}
		}
	}
	else if(r == 2)
	{
		//if No is chosen the row is not deleted in the datappgrid
	}


	ConfirmGrid->parent_w->visible = FALSE;
	id->DataAppGrid = DataAppGrid;
	id->DataAppCreater = DataAppCreater;
	return(TRUE);
}
int CriteriaConfirmCallback(struct InstanceData* id, MGLGRID *CriteriaConfirmGrid, int b, int m, int x, int y)
{
	// pop up one of the GUI settings grids
	int r,t,line_count;
	


	assert(id);
	assert(CriteriaConfirmGrid);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}


	// see which operator the user selected, add it to the field, then hide the operator menu
	if (CriteriaConfirmGrid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - CriteriaConfirmGrid->gridrect.y1) / CriteriaConfirmGrid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - CriteriaConfirmGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}

	if (r < 0 || r >= CriteriaConfirmGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = CriteriaConfirmGrid->nrows - r - 1;
	if (r == 1)
	{
		for (t = 0; t < SecondTabGrid->childgrid->nActions; t++)
		{
			if (SecondTabGrid->childgrid->ActionsGrid->criteria_confirmdelete[t] == 1)
			{
				if (SecondTabGrid->childgrid->ActionsGrid->gridcells[t + 1][2].bg != (GLfloat*)white)
				{
					MGLFillParametersDelete(id, SecondTabGrid->childgrid->ParametersGrid, t);
					vo_rm_row(SecondTabGrid->childgrid->ActionsGrid->vo, t);
					/*if (strcmp(VP(SecondTabGrid->childgrid->ActionsGrid->vo, t, 0, char), "New") != 0)
						SecondTabGrid->childgrid->ActionsGrid->vo->row_index[SecondTabGrid->childgrid->ActionsGrid->parentgrid->CurActionNum].fg = MGLColorHex2Int("529EFD");*/
				line_count = 0;
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo)
				{
				for (int t = 0; t < SecondTabGrid->childgrid->BuildSummaryGrid->vo->count; t++)
				{
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, t, "Parameters", char), 64, "", _TRUNCATE);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, t, "Criteria", char), 64, "", _TRUNCATE);
				}

				}

				MGLClearParametersGrid(SecondTabGrid->childgrid);
				MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->ActionsGrid->parentgrid->CurActionNum, id);
				InitParametersinBuildSummary(id, SecondTabGrid->childgrid);
				InitCriteriainBuildSummary(id, SecondTabGrid->childgrid);
				}
				if (strcmp(VP(SecondTabGrid->childgrid->ActionsGrid->vo, 0, 0, char), "New") != 0)
				{
					SecondTabGrid->childgrid->ActionsGrid->parentgrid->CurActionNum = 0;
					SecondTabGrid->childgrid->ActionsGrid->vo->row_index[SecondTabGrid->childgrid->ActionsGrid->parentgrid->CurActionNum].fg = MGLColorHex2Int("529EFD");					
					color_actions(id);
					
				}
				else if (strcmp(VP(SecondTabGrid->childgrid->ActionsGrid->vo, 0, 0, char), "New") != 0)
				{
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
					SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[3] = 0;
					if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
						SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 0;
					if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
						SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 0;
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
						SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
				}

			}
				
		}
		
	}
	CriteriaConfirmGrid->parent_w->visible = FALSE;
	return(TRUE);
}


int OperatorMenuCallback(struct InstanceData* id, MGLGRID *operatorgrid, int b, int m, int x, int y)
{
	// pop up one of the GUI settings grids
	int r, opcolnum, vorow;
	MGLGRID *parentgrid, *ParametersGrid;
	char *op;
	VO *vcol;

	assert(id);
	assert(operatorgrid);

	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	parentgrid = operatorgrid->parentgrid;

	// see which operator the user selected, add it to the field, then hide the operator menu
	if (operatorgrid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - operatorgrid->gridrect.y1) / operatorgrid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - operatorgrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}

	if (r < 0 || r >= operatorgrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = operatorgrid->nrows - r - 1;

	switch (r)
	{
	case 0:
		op = "Equal to";
		break;
	case 1:
		op = "Not Equal to";
		break;
	case 2:
		op = "Greater than";
		break;
	case 3:
		op = "Less than";
		break;
	case 4:
		op = "Less than or equal to";
		break;
	case 5:
		op = "Greater than or equal to";
		break;
	case 6:
		op = "Between";
		break;
	default:
		op = "";
		break;
	}

	// set the operator value in the AlertsVO
	// the orange row is the currently active ParametersGrid row
	ParametersGrid = parentgrid->ParametersGrid;

	vcol = vcol_find(ParametersGrid->vo, "Operator");
	opcolnum = vcol->colnum;

	// find out which operator the user selected
	for (vorow = 0; vorow < ParametersGrid->vo->count; vorow++){
		if (ParametersGrid->vo->row_index[vorow].bg == iOrange){
			// found active operator row
			// set the value in the AlertsVO
			strncpy_s(VP(ParametersGrid->vo, vorow, "Operator", char), 32, op, _TRUNCATE);

			// reset the color
			ParametersGrid->vo->row_index[vorow].bg = -1;
		}
	}

#ifdef OLDWAY
	for (gridrow = 1; gridrow < ParametersGrid->nrows; gridrow++){
		if (ParametersGrid->gridcells[gridrow][opcolnum].bg == (GLfloat*)orange){
			// found active operator row
			// set the value in the AlertsVO
			strncpy_s(VP(ParametersGrid->vo, gridrow - 1, "Operator", char), 32, op, _TRUNCATE);

			// reset the color
			ParametersGrid->gridcells[gridrow][opcolnum].bg = (GLfloat*)gridgreyelev;
		}
	}
#endif


	operatorgrid->parent_w->visible = FALSE;
	return(TRUE);
}




MGLGRID *CreateOperatorMenu(struct InstanceData* id, MGLGRID *parentgrid, int x, int y)
{
	int r;
	widgets *w, *lastw;
	MGLGRID *grid;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create MGLGrid
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "OperatorWidget");
	w->gregion.x1 = x;
	w->gregion.y1 = y - 126; // 126 is height of menu, so make menu appear down from mouse click location
	w->gregion.x2 = w->gregion.x1 + 200;
	w->gregion.y2 = w->gregion.y1 + 570;
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "OperatorGrid", 6, 1, w->gregion.x1, w->gregion.y1);
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 1;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	w->grid->parentgrid = parentgrid;

	w->grid->parentgrid->OperatorGrid = w->grid;
	grid = w->grid->parentgrid->OperatorGrid;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)yellow;
	for (r = 0; r < w->grid->nrows; r++){
		w->grid->gridrows[r].fg = (GLfloat*)black;
		w->grid->gridrows[r].bg = (GLfloat*)color868686;
	}


	// add new widget
	if (lastw){
		lastw->np = w;
	}

	MGLGridText(id, grid, 0, 0, "Equal to");
	MGLGridText(id, grid, 1, 0, "Not Equal to");
	MGLGridText(id, grid, 2, 0, "Greater than");
	MGLGridText(id, grid, 3, 0, "Less than");
	MGLGridText(id, grid, 4, 0, "Less than or equal to");
	MGLGridText(id, grid, 5, 0, "Greater than or Equal to");
	//	MGLGridText(id, grid, 6, 0, "Between");

	for (r = 0; r < grid->nrows; r++){
		grid->gridrows[r].row_callback = OperatorMenuCallback;
	}
	id->m_poglgui = oglgui;

	return(grid);
}


MGLGRID *CreateGridSettingsMenu(struct InstanceData* id, MGLGRID *parentgrid)
{
	int r;
	widgets *w, *lastw;
	MGLGRID *grid;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create MGLGrid
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SettingsGridWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->gregion.x2 = 200;
	w->gregion.y2 = 570;
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "SettingsGrid", 2, 1, w->gregion.x1, w->gregion.y1);
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	w->grid->parentgrid = parentgrid;

	w->grid->GridSettingsMenu = w->grid;
	grid = w->grid->GridSettingsMenu;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)yellow;
	w->grid->fg = (GLfloat*)yellow;
	for (r = 0; r < w->grid->nrows; r++){
		w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
		w->grid->gridrows[r].bg = (GLfloat*)color868686;
	}


	// add new widget
	if (lastw){
		lastw->np = w;
	}

#ifdef OLDWAY
	// FillSettingsGrid(id, w->grid);
	MGLGridText(id, grid, 0, 0, "Filter");
	MGLGridText(id, grid, 1, 0, "Sort");
	MGLGridText(id, grid, 2, 0, "Alerts");
#endif

	MGLGridText(id, grid, 0, 0, "Actions");
	MGLGridText(id, grid, 1, 0, "Layout");


	id->m_poglgui = oglgui;

	//grid->gridrows[0].row_callback = GridMenuCallback_me;
	//grid->gridrows[1].row_callback = GridMenuCallback_me;
	//grid->gridrows[2].row_callback = GridMenuCallback;
	return(grid);
}

MGLGRID *CreateGridSettingsMenu_me(struct InstanceData* id, MGLGRID *parentgrid)
{
	int r;
	widgets *w, *lastw;
	MGLGRID *grid;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create MGLGrid
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SettingsGridWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->gregion.x2 = 200;
	w->gregion.y2 = 570;
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "SettingsGrid", 2, 1, w->gregion.x1, w->gregion.y1);
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	w->grid->parentgrid = parentgrid;

	w->grid->GridSettingsMenu = w->grid;
	grid = w->grid->GridSettingsMenu;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)yellow;
	w->grid->fg = (GLfloat*)yellow;
	for (r = 0; r < w->grid->nrows; r++){
		w->grid->gridrows[r].fg = (GLfloat*)colorf8cf53;
		w->grid->gridrows[r].bg = (GLfloat*)color868686;
	}


	// add new widget
	if (lastw){
		lastw->np = w;
	}

#ifdef OLDWAY
	// FillSettingsGrid(id, w->grid);
	MGLGridText(id, grid, 0, 0, "Filter");
	MGLGridText(id, grid, 1, 0, "Sort");
	MGLGridText(id, grid, 2, 0, "Alerts");
#endif

	MGLGridText(id, grid, 0, 0, "Rules");
	MGLGridText(id, grid, 1, 0, "Layout");



	//grid->gridrows[0].row_callback = GridMenuCallback_me;
	//grid->gridrows[1].row_callback = GridMenuCallback_me;

	id->m_poglgui = oglgui;


	return(grid);
}





int MGLHeadersCallback(struct InstanceData* id, MGLGRID *parentgrid, int b, int m, int x, int y)
{
	MGLGRID *GridSettingsMenu;
	int gridmenuheight, rownum, colnum, colstartx;
	// throw up Column Settings menu in a grid

	assert(id);
	assert(parentgrid);

	if (!parentgrid->GridSettingsMenu){
		//	parentgrid->GridSettingsMenu = CreateGridSettingsMenu_me(id, parentgrid); //mtm-commented to disable the gridsettingsmenu to pop up
	}
	if (!(GridSettingsMenu = parentgrid->GridSettingsMenu)) return(FALSE);

	// make sure top of menu is just below the header row of parent grid
	// how big is GridMenu?
	gridmenuheight = GridSettingsMenu->gridrect.y2 - GridSettingsMenu->gridrect.y1;

	if (MGLWhichRowCol(parentgrid, x, y, &rownum, &colnum) == SUCCEED){
		colstartx = parentgrid->gridcells[rownum][colnum].cellrect.x1;
		MGLGridMove(id, parentgrid->GridSettingsMenu, colstartx, parentgrid->gridrect.y2 - MGL_GRID_DEF_ROW_HEIGHT - gridmenuheight);
	}
	// make sure the grid is visible
	parentgrid->GridSettingsMenu->parent_w->visible = TRUE;

	// make sure FilterGrid is invisible
	if (parentgrid->FilterGrid){
		parentgrid->FilterGrid->parent_w->visible = FALSE;
	}
	return(TRUE);
}

//mtm
MGLGRID  *CreateSecondTab(struct InstanceData* id,MGLGRID *SecondTabGrid)
{
	widgets *w, *lastw, *panel_widget;
	int r;

	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	
	
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SecondTab");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;


	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;



	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SecondTabWidget");
	w->gregion.x1 = DataAppGrid->gridrect.x2 - 1;
	w->gregion.y1 = DataAppGrid->gridrect.y1;
	w->gregion.x2 = 400;
	w->gregion.y2 = id->m_nHeight + 250;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "SecondTabGrid", 15, 6, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->grid->edit = 0;//to display the rules-layout icon
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	SecondTabGrid = w->grid;

	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)grey;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)grey;
			w->grid->gridrows[r + 1].fg = (GLfloat*)grey;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)Grid_grey2;
	w->grid->gridrows[0].fg = (GLfloat*)Grid_grey4;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Criteria  ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Parameters           ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Actions", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "      ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Layout              ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "Visible rows", _TRUNCATE);

	w->grid->gridrows[0].row_callback = SecondTabCallback;
	// we also need to fetch initial filter values from user settings DB

	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	SecondTabGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = SecondTabGrid->marginx;
	w->gregion.y1 = SecondTabGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - SecondTabGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Create Grid : New Grid", _TRUNCATE);
	w->grid = SecondTabGrid;

	SecondTabGrid->fixed_width = 2;
	id->m_poglgui = oglgui;
	id->DataAppGrid = DataAppGrid;

	return SecondTabGrid;


}
//mtm - to create build summary grid
void CreateBuildSummary(struct InstanceData* id, MGLGRID *parentgrid)
{
	widgets *w, *lastw, *panel_widget;
	int r;

	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	

	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "BuildSummary");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;


	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;



	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "BuildSummaryWidget");
	w->gregion.x1 = DataAppGrid->gridrect.x2 - 1;
	w->gregion.y1 = DataAppGrid->gridrect.y2 - 180;
	w->gregion.x2 = 400;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "BuildSummaryGrid", 10, 6, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->grid->edit = 0;//to display the rules-layout icon
	w->visible = 0;
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->BuildSummaryGrid = w->grid;

	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey2;
	}

	w->grid->gridrows[0].bg = (GLfloat*)Grid_grey2;
	//	w->grid->gridrows[0].fg = (GLfloat*)Grid_grey4;
	w->grid->gridrows[0].fg = Text_yellow;

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}


	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Criteria  ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Parameters           ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Actions", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "      ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Layout              ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "Visible rows", _TRUNCATE);


	strncpy_s(w->grid->gridcols[3].pngfilename, sizeof(w->grid->gridcols[3].pngfilename), "new.png", _TRUNCATE);
	w->grid->gridcols[3].texturesize = 32;
	w->grid->gridcols[3].img_displaysize = 20;
	//strncpy_s(w->grid->gridcols[3].pngfilename, sizeof(w->grid->gridcols[3].pngfilename), "show-circle-off.png", _TRUNCATE);
	//w->grid->gridcols[3].texturesize = 16;
	//w->grid->gridcols[3].img_displaysize = 10;





	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->BuildSummaryGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->BuildSummaryGrid->marginx;
	w->gregion.y1 = parentgrid->BuildSummaryGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->BuildSummaryGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Build Summary", _TRUNCATE);
	w->visible = 0;
	w->grid = parentgrid->BuildSummaryGrid;

	
	parentgrid->BuildSummaryGrid->fixed_width = 1;
	id->m_poglgui = oglgui;
	id->DataAppGrid = DataAppGrid;

}


//mtm//

//MGLGRID *CreateGridnames(struct InstanceData* id, int count)
//{
//
//	//pops up "SAVED" when save is pressed
//
//	widgets *w, *lastw, *panel_widget;
//
//
//
//	assert(id);
//
//	// find last widget in list and add it here
//	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
//		lastw = w;
//	}
//
//	// Create panel for holding the two grids plus buttons
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "SaveDisplay");
//	w->gregion.x1 = 20;
//	w->gregion.y1 = 370;
//	w->width = 500;
//	w->height = 300;
//	w->type = WGT_PANEL;
//	w->visible = 0;  // don't show panel
//	w->bg = (GLfloat*)gridgrey0;
//	w->fg = (GLfloat*)white;
//	panel_widget = w;
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//	}
//	else {
//		// first widget
//		oglgui->widgets = w;
//	}
//	lastw = w;
//
//	// Create MGLGrid for list of Alerts that are active
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "Gridnames");
//	w->gregion.x1 = 300;
//	w->gregion.y1 = id->m_nHeight - 160;
//	w->gregion.x2 = 300;
//	w->gregion.y2 = id->m_nHeight;
//	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
//
//	w->type = WGT_GRID;
//	w->grid = MGLGridCreate(id, NULL, "Gridnames", count, 1, w->gregion.x1, w->gregion.y1);
//	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
//	w->grid->iVscrollPos = 0;
//	w->grid->GScrollWidth = 20;
//	w->visible = 0;
//	w->grid->AllowScrollbar = FALSE;
//	w->grid->HasTitleBar = FALSE;
//	// add default colors for grid and the rows
//	w->grid->bg = (GLfloat*)gridgrey0;
//	w->grid->fg = (GLfloat*)white;
//	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
//	Gridnames = w->grid;
//
//
//
//	w->grid->gridrows[0].fg = (GLfloat*)yellow;
//	w->grid->gridrows[0].bg = (GLfloat*)color454545;
//
//
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//		lastw = w;
//	}
//
//	/*for (int m = 0; m < count; m++)
//	{
//	MGLGridText(id, Gridnames, m, 0, names[m]);
//	}*/
//	//strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Saved", _TRUNCATE);
//
//
//	//id->flag_check = 2;//to turn the grid on/off
//
//	return(CreateGridnames);
//}

//mtm
//int MainTabCallback(struct InstanceData* id, MGLGRID *parentgrid, int b, int m, int x, int y)
//{
//	// Toggle the selected row region On or OFF
//	int  c, vorow, count;
//
//
//	vorow = 0;
//
//	// see which column was selected
//	c = MGLCalcCol(MainTabGrid, x);
//
//
//	if (c == 0)
//	{
//		if (SecondTabGrid){
//			SecondTabGrid->parent_w->visible = FALSE;
//
//		}
//
//		if (SecondTabGrid->childgrid){
//			if (SecondTabGrid->childgrid->ActionsGrid->parent_w->visible == 1)
//			{
//				SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = 1;
//			}
//			SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = FALSE;
//
//		}
//		if (SecondTabGrid->childgrid){
//			if (SecondTabGrid->childgrid->ParametersGrid->parent_w->visible == 1)
//			{
//				SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = 1;
//				if (SecondTabGrid->childgrid->OperatorGrid)
//					SecondTabGrid->childgrid->OperatorGrid->parent_w->visible = 0;
//			}
//			SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = FALSE;
//		}
//
//		if (SecondTabGrid->childgrid){
//			if (SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible == 1)
//			{
//				SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = 1;
//			}
//			SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = FALSE;
//		}
//		if (SecondTabGrid->childgrid){
//			if (SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible == 1)
//			{
//				SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = 1;
//			}
//			SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = FALSE;
//		}
//		if (SecondTabGrid->childgrid){
//			if (SecondTabGrid->childgrid->RowColumn->parent_w->visible == 1)
//			{
//				SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = 1;
//			}
//			SecondTabGrid->childgrid->RowColumn->parent_w->visible = FALSE;
//
//		}
//		DataAppGrid->parent_w->visible = TRUE;
//		MainTabGrid->gridcells[0][0].bg = (GLfloat*)grey_Tab_bg;
//		MainTabGrid->gridcells[0][1].bg = (GLfloat*)grey_Tab_bg;
//		MainTabGrid->gridcells[0][2].bg = (GLfloat*)Blue_title;
//		MainTabGrid->gridcells[0][3].bg = (GLfloat*)Blue_title;
//
//		if (SecondTabGrid){
//			if (SecondTabGrid->childgrid){
//				if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
//					SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
//				if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
//					SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
//			}
//		}
//
//
//	}
//
//
//	else
//	{
//		DataAppGrid->parent_w->visible = FALSE;
//		if (SecondTabGrid){
//			SecondTabGrid->parent_w->visible = TRUE;
//			MGLGridMove(id, SecondTabGrid, MainTabGrid->gridrect.x1 + 30, MainTabGrid->gridrect.y1 + 30);
//		}
//
//		if (SecondTabGrid->childgrid){
//			if (SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory == 1){
//				SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = TRUE;
//				/*if (SecondTabGrid->childgrid)
//					MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, MainTabGrid->gridrect.x1 + 60, MainTabGrid->gridrect.y1 + 100);*/
//			}
//			if (SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory == 1)
//				SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = TRUE;
//			if (SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory == 1)
//				SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = TRUE;
//			if (SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory == 1)
//				SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = TRUE;
//			if (SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory == 1)
//				SecondTabGrid->childgrid->RowColumn->parent_w->visible = TRUE;
//		}
//
//		MainTabGrid->gridcells[0][0].bg = (GLfloat*)Blue_title;
//		MainTabGrid->gridcells[0][1].bg = (GLfloat*)Blue_title;
//		MainTabGrid->gridcells[0][2].bg = (GLfloat*)grey_Tab_bg;
//		MainTabGrid->gridcells[0][3].bg = (GLfloat*)grey_Tab_bg;
//		count = DataAppGrid->vo->row_index->vo->count;
//		//CreateGridnames(id,count);
//		if (SecondTabGrid){
//			{
//				if (SecondTabGrid->childgrid)
//				{
//					if (SecondTabGrid->childgrid->AlertsColorGrid)
//					{
//						if (SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible == 1){
//							if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
//							{
//								if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] == 1)
//									SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = TRUE;
//							}
//
//							if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
//							{
//								if (SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] == 1)
//									SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = TRUE;
//							}
//
//						}
//					}
//				}
//			}
//		}
//
//	}
//	return(TRUE);
//}
//

//mtm







time_t MGLDtStr2GMTSecs(char *str, char *date_fmt)
{
	time_t secs;
	char *stat_str;
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	if (!str || !date_fmt || !strlen(str)){
		return(-1);
	}

	/* check date_fmt if specified */
	if (date_fmt && strlen(date_fmt)){

		//Win32:08-11-03
		//stat_str = strptime(str, date_fmt, &tm );
		stat_str = strptime(str, date_fmt, &tm);

		/* check fields for invalid numbers */
		if (stat_str && !strlen(stat_str)){
			/* valid conversion */
			/* convert year in next century */
			if (tm.tm_year < 70){
				tm.tm_year += 70;
			}
			tm.tm_isdst = -1;
			// secs = mktime(&tm);
			secs = _mkgmtime(&tm);
			return((int)secs);
		}
		else {
			return(-1);
		}

	}
	return(-1);
}




time_t MGLDtStr2Secs(struct InstanceData* id, char *str, char *date_fmt)
{
	time_t secs;
	char *stat_str;
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	if (!str || !date_fmt || !strlen(str)){
		return(-1);
	}

	/* check date_fmt if specified */
	if (date_fmt && strlen(date_fmt)){

		//Win32:08-11-03
		//stat_str = strptime(str, date_fmt, &tm );
		stat_str = strptime(str, date_fmt, &tm);

		/* check fields for invalid numbers */
		if (stat_str && !strlen(stat_str)){
			/* valid conversion */
			/* convert year in next century */
			if (tm.tm_year < 70){
				tm.tm_year += 70;
			}
			tm.tm_isdst = -1;

			if (id->m_sUserSettings.TimeDisplayLocal == 1)
				secs = mktime(&tm);
			else if (id->m_sUserSettings.TimeDisplayLocal == 0)
				secs = _mkgmtime( &tm );

			
			return((int)secs);
		}
		else {
			return(-1);
		}

	}
	return(-1);
}


/*
secs = time(NULL);
VOGMTime(midnight, secs, "%Y" );
year = atoi(midnight);
VOGMTime(midnight, secs, "%m" );
month = atoi(midnight);
VOGMTime(midnight, secs, "%d" );
day = atoi(midnight);

tmTemp.tm_year = year - 1900; // 1970 starts unix time
tmTemp.tm_mon = month - 1;
tmTemp.tm_mday = day;
tmTemp.tm_hour = 0;
tmTemp.tm_min = 0;
tmTemp.tm_sec = 0;
tmTemp.tm_isdst = -1; // let mktime figure out dst or not, if 0, then it assumes no DST

unixsecs = mktime( &tmTemp );  // mktime assumes tmptr parameters are local time, so it returns seconds in local time
_get_timezone(&tz);
unixsecs -= tz;  // convert back into gmt
// localtime_s( &newtime, &unixsecs );

*/


int MGLConvert2IntervalSecs(struct InstanceData* id,char *cmp_value, char *buf, int buflen)
{
	time_t secs, midnightsecs;
	char midnight[64], fulldatebuf[64];


	// convert user string into unix seconds

	// check format of user string

	if (re_comp("[0-9][0-9]:[0-9][0-9]:[0-9][0-9]")){
		// come error
	}
	else if (re_exec(cmp_value) == 1){
		// matches
		/* if gmtime from user
		struct tm local;
		time_t rtime = DtStr2Secs(tok, "%Y-%m-%dT%H:%M:%SZ");
		if(!localtime_s(&local, &rtime))
		record->eventtime =_mkgmtime(&local);
		*/
		secs = time(0);
		VOGMTime(midnight, secs, "%Y%m%d");
		midnightsecs = MGLDtStr2Secs(id,midnight, "%Y%m%d");

		// add midnight "date" to cmp string which only has time
		sprintf_s(fulldatebuf, sizeof(fulldatebuf), "19700101 %s", cmp_value);

		secs = MGLDtStr2GMTSecs(fulldatebuf, "%Y%m%d %H:%M:%S");

		sprintf_s(buf, buflen, "%d", secs);
		return(TRUE);
	}

	return(FALSE);
}




int MGLConvert2Secs(struct InstanceData* id, char *cmp_value, char *buf, int buflen)
{
	time_t secs, midnightsecs;
	char midnight[64], fulldatebuf[64];

	//	struct tm tmTemp;
	//	int year, month, day;
	//int tz;

	// convert user string into unix seconds

	// check format of user string

	if (re_comp("[0-9][0-9]:[0-9][0-9]:[0-9][0-9]")){
		// come error
	}
	else if (re_exec(cmp_value) == 1){
		// matches
		/* if gmtime from user
		struct tm local;
		time_t rtime = DtStr2Secs(tok, "%Y-%m-%dT%H:%M:%SZ");
		if(!localtime_s(&local, &rtime))
		record->eventtime =_mkgmtime(&local);
		*/

		secs = time(0);
		VOGMTime(midnight, secs, "%Y%m%d");
		midnightsecs = MGLDtStr2Secs(id, midnight, "%Y%m%d");

		// add midnight "date" to cmp string which only has time
		sprintf_s(fulldatebuf, sizeof(fulldatebuf), "%s %s", midnight, cmp_value);

		secs = MGLDtStr2Secs(id,fulldatebuf, "%Y%m%d %H:%M:%S");

		sprintf_s(buf, buflen, "%d", secs);
		return(TRUE);
	}

	return(FALSE);
}



int MGLGridColNum(MGLGRID *topgrid, VO *vcol)
{
	int i;

	assert(topgrid);

	// find out original order number for this column, as user set up in ColOrderNames
	for (i = 0; i < topgrid->ncols; i++){
		if (!strcmp(vcol->name, topgrid->ColOrderNames[i]) || !strcmp(vcol->label, topgrid->ColOrderNames[i])){
			return(i);
		}
	}

	return(-1); // test
}






int MGLCrOrderedCols(MGLGRID *parentgrid)
{
	char sortcolnames[MGL_GRID_MAX_SORT_COLS][MGL_GRID_NAME_MAX];
	int i, nsortcols, sortpriority, sval;
	VO *vo, *vcol, *order_col, *scol;
	char *colname;
	struct row_index *row_index;


	if (!parentgrid || !parentgrid->SortAvailableGrid || !parentgrid->SortAvailableGrid->vo) return(FALSE);

	if (!parentgrid->filteredvo){
		return(FALSE);
	}
	vo = parentgrid->filteredvo;

	for (i = 0; i < MGL_GRID_MAX_SORT_COLS; i++){
		sortcolnames[i][0] = '\0';  // clear out stack struct
	}

	for (i = 0, nsortcols = 0; i < parentgrid->SortAvailableGrid->vo->count && nsortcols < MGL_GRID_MAX_SORT_COLS - 1; i++){
		if ((sval = VV(parentgrid->SortAvailableGrid->vo, i, "SortVal", int)) > 0){
			sortpriority = VV(parentgrid->SortAvailableGrid->vo, i, "SortPriority", int);
			if (sortpriority >= 0 && sortpriority < MGL_GRID_MAX_SORT_COLS){
				strncpy_s(sortcolnames[sortpriority], MGL_GRID_NAME_MAX, VP(parentgrid->SortAvailableGrid->vo, i, "ColName", char), _TRUNCATE);
			}

			// add sort_type property to filteredvo vcol that matches this column name
			if ((vcol = vcol_find(vo, VP(parentgrid->SortAvailableGrid->vo, i, "ColName", char))) || (vcol = MGLFindLabel(vo, VP(parentgrid->SortAvailableGrid->vo, i, "ColName", char)))){
				if ((scol = vcol_find(vcol, "sort_type"))){

				}
				else {
					// we need to add the property
					VOPropAdd(vcol, "sort_type", INTBIND, -1, VO_ADD_ROW);
					scol = vcol_find(vcol, "sort_type");
				}
				VV(vcol, 0, "sort_type", int) = sval - 1;  // vo uses 0 for asc, 1 for dsc, so subtract 1
			}

			nsortcols++;
		}

	}

	if (!nsortcols){
		// no user specified sort order
		return(FALSE);
	}

	// create the order_cols
	if (vo->order_cols){
		// clear out old ones
		vo_free(vo->order_cols);
		vo->order_cols = NULL;
	}

	vo->order_cols = vo_cr_def(sizeof(VO), V_DEFAULT_INDEX);

	for (i = 0; i < MGL_GRID_MAX_SORT_COLS; i++){
		colname = sortcolnames[i];
		if (!strlen(colname)){
			continue;
		}
		if ((vcol = MGLFindLabel(vo, colname))){
			// the label was selected, not column name, but set to vcol->name so vo calls will work
			colname = vcol->name;
		}
		if ((vcol = vcol_find(vo, colname)) ){
			row_index = (struct row_index *)
				vo_alloc(vo->order_cols, sizeof(VO));
			order_col = row_index->datap;
			strncpy_s(order_col->name, V_NAME_SIZE, colname, _TRUNCATE);
			order_col->dbtype = vcol->dbtype;
			order_col->offset = vcol->offset;
		}
		else {
			// column name not found
		}
	}

	return(TRUE);
}



void MGLSortFilteredGrid(MGLGRID *parentgrid)
{

	assert(parentgrid);

	// sort the grid according to user settings

	// get the order columns from AvailableGridVO?
	// for multiple grids , need multiple AvailableGrids

	if (parentgrid->filteredvo){
		if (MGLCrOrderedCols(parentgrid)){
			vo_order(parentgrid->filteredvo);
		}
	}


}




VO *MGLCrSearchVO(char *select_value)
{
	VO *searchvo;
	char *str;
	char *context = NULL;
	char *token;

	searchvo = vo_create(0, NULL);
	vo_set(searchvo, V_NAME, "searchvo", NULL);

	VOPropAdd(searchvo, "listval", NTBSTRINGBIND, 256, VO_NO_ROW);
	vo_rm_rows(searchvo, 0, searchvo->count);


	// parse through comma separated list of values 

	str = select_value;

	for (token = strtok_s(str, ",", &context); token != NULL; token = strtok_s(NULL, ",", &context)){
		str = token;
		vo_alloc_rows(searchvo, 1);
		if (strlen(str) < 255){
			strcpy_s(VP(searchvo, searchvo->count - 1, "listval", char), 255, str);
		}
	}

	return(searchvo);

}



/*
complex rules
from Noel:
These come to mind right away:

FLIFO Update Missing
Actual Out Time is Empty
Scheduled departure time is less than Now - 10 minutes
ETD is empty

FLIFO Follow Up
Actual Out Time is Empty
ETD is not empty
Scheduled departure time is less than Now - 30 minutes
ETD time is less than Now - 10 minutes

Late Arrivals still flying
Destination equals "Airport"
ATA is empty
ETA is not empty
ETA is greater than Scheduled Arrival + 14 Minutes

keywords:

val operator val

x + 3
val op val
if parenthesis, then  on close bracket call val op val  -- this probably not needed right away

time conversions (convert into seconds)
now
minutes, seconds, hours

column names

psuedo code

for each paramter string

put each word into a linked list



while ( (nxtnode = findnxtnode(fieldsll) ){
calc( nxtnode );
}

answer = fieldsll->val   // must always be one left


calc( firstnode )
{
if firstnode.type == val
and firstnode->np.type == op and firstnode->np->np.type == val

switch ( op )
case '+':
answer = atod(n1->val) + atod( n3->val )
break;

firstnode->np = 3rdnode->np;
free( 2ndnode ); free 3rd node
}
firstnode->val = answer;
return;
}




findnxtnode( fieldsll )

lastnode = findclosep( fieldsll );
if ( lastnode ){
firstnode + lastnode->pp->pp;
} else {
firstnode = fieldsll;
}

return( firstnode );
}


last remaining field is the value

*/


int isOperator(char *str)
{
	int c;

	if (str && strlen(str) == 1 && (c = *str)){
		if (c == '+' || c == '-' || c == '*' || c == '/'){
			return(TRUE);
		}
	}

	return(FALSE);

}





CALC_LL *CreateCalcll(char *sourcestr)
{
	char field[64];
	char *str;
	int f, c;
	CALC_LL *field_ll, *cur_ll, *calc_ll = NULL;

	// parse all fields into linked list 

	str = sourcestr;
	while (str && *str != 0)
	{

		//  skip leading white space
		while (*str != 0 && isspace(*str)) str++;
		f = 0;
		while ((c = *str++) != 0 && !isspace(c)){
			if (f >= sizeof(field) - 1){
				break;
			}
			field[f++] = c;
		}
		field[f] = '\0';
		// add to linked list struct
		field_ll = (CALC_LL *)calloc(1, sizeof(CALC_LL));
		strncpy_s(field_ll->val, sizeof(field_ll->val), field, _TRUNCATE);

		if (isOperator(field_ll->val)){
			field_ll->type = CALC_OP;
		}
		else {
			field_ll->type = CALC_VAL;
		}



		// link to prior list
		if (!calc_ll){
			calc_ll = field_ll;
			cur_ll = calc_ll;
		}
		else {
			cur_ll->np = field_ll;
			field_ll->pp = cur_ll;
			cur_ll = field_ll;
		}
		if (c == 0) break;  // see if we are done
	}

	return(calc_ll);

}


CALC_LL *findnxtnode(CALC_LL *calcll)
{
	char *str;
	// for now, we do not support parenthesis, so just return current value

	if (!calcll || !calcll->np || !calcll->np->np){
		// less than 3, we are done
		return(calcll);
	}
	// look for closing parenthesis
	str = calcll->val;
	if (strchr(str, '(') && strchr(str, ')')){
	}


	return(calcll);
}




CALC_LL *ConvertKeywords(struct InstanceData* id, CALC_LL *calcll, MGLGRID *parentgrid, int voindex, VO *vcol)
{
	CALC_LL *cnode;
	char *str, *cmp_value;
	time_t nowsecs;
	char tmpbuf[256];
	VO *cmpvcol;

	// look for keywords like now, null, TimeofDay like 04:00:00 which should be converted into 4 am for today
	for (cnode = calcll; cnode; cnode = cnode->np){
		str = cnode->val;
		if (!strcasecmp(str, "now")){
			// return now in linux seconds
			nowsecs = time(0);
			_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%d", nowsecs);
			continue;
		}
		if (strchr(str, 'Y') && !strcasecmp(vcol->name, "Active")){ // Active colname is for in "In" regions, Y for active. N for inactive, out of region
			// convert 'Y' to a 1
			_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%d", 1);
		}
		if (strchr(str, 'N') && !strcasecmp(vcol->name, "Active")){ // Active colname is for in "In" regions, Y for active. N for inactive, out of region
			// convert 'Y' to a 0
			_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%d", 0);
		}

		if (strchr(str, ':')){
			// TimeOfDay   OR Interval --- assume interval for a single calcll, assume TOD if multiple arguments
			// convert into time since midnight, ie 4:00 becomes 4:00am 16:00 is 4pm
			// if the data_type is INTBIND, but user entered a time string such as "00:20:00", then convert the elapsed time into seconds
			//  LATER, also convert entries like "10 minutes",  "4 hours", etc
			// for now, since we dont require interval string, if column name is "elapsed", then keep duration and do not convert to unix time of day
			if (cnode == calcll && strcasecmp( vcol->name, "Elapsed" ) ){
				// if date string is first argument, assume it is time of day
				if (MGLConvert2Secs(id, str, tmpbuf, sizeof(tmpbuf))){
					_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%s", tmpbuf);
				}
			}
			else {
				// elapsed colname or interval value
				// calc interval values
				if (MGLConvert2IntervalSecs(id, str, tmpbuf, sizeof(tmpbuf))){
					_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%s", tmpbuf);
				}
			}
		}
		if (!strcasecmp(str, "null") || !strcasecmp(str, "empty")){
			// null equates to 0 seconds
			if (vcol->dbtype==13)
				_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%s", "");
			else
				_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%d", 0);
			continue;
		}

		// see if the keyword in a column name, is so, then convert the value
		// we also have to check label names as well, when that is added to vo struct
		if ((cmpvcol = MGLFindCol(parentgrid->vo, str)) || (cmpvcol = MGLFindLabel(parentgrid->vo, str))){
			// substitute for value from this column
			cmp_value = get_vcol_valstr(cmpvcol, V_ROW_PTR(parentgrid->vo, voindex), TRUE);
			_snprintf_s(cnode->val, sizeof(cnode->val), _TRUNCATE, "%s", cmp_value);

			// for debug, get flightid
			//if ((cmpvcol = vcol_find(parentgrid->vo, "FltNum"))){
			////	cmp_value = get_vcol_valstr(cmpvcol, V_ROW_PTR(parentgrid->vo, voindex), TRUE);
			//	strcpy(tmpbuf, cmp_value);
			//}
		}
	}

	return(calcll);
}




void complexcalc(CALC_LL *calcnode)
{
	char op;
	char *opstr;
	double dval1, dval2;
	char *stat_ptr;
	CALC_LL *lastp;

	// combine 3 fields into one using operator and two values
	if (calcnode && calcnode->np && calcnode->np->np && calcnode->type == CALC_VAL && calcnode->np->type == CALC_OP && calcnode->np->np->type == CALC_VAL){
		opstr = calcnode->np->val;
		op = *opstr;
		switch (op){
		case '+':
			dval1 = strtod(calcnode->val, &stat_ptr);
			if (dval1 == 0.0 && calcnode->val == stat_ptr){
				/* bad value */
				strncpy_s(calcnode->val, sizeof(calcnode->val), "error", _TRUNCATE);
				break;
			}
			dval2 = strtod(calcnode->np->np->val, &stat_ptr);
			if (dval2 == 0.0 && calcnode->np->np->val == stat_ptr){
				/* bad value */
				strncpy_s(calcnode->val, sizeof(calcnode->val), "error", _TRUNCATE);
				break;
			}
			_snprintf_s(calcnode->val, sizeof(calcnode->val), _TRUNCATE, "%1.0f", dval1 + dval2);
			break;
		case '-':
			dval1 = strtod(calcnode->val, &stat_ptr);
			if (dval1 == 0.0 && calcnode->val == stat_ptr){
				/* bad value */
				strncpy_s(calcnode->val, sizeof(calcnode->val), "error", _TRUNCATE);
				break;
			}
			dval2 = strtod(calcnode->np->np->val, &stat_ptr);
			if (dval2 == 0.0 && calcnode->np->np->val == stat_ptr){
				/* bad value */
				strncpy_s(calcnode->val, sizeof(calcnode->val), "error", _TRUNCATE);
				break;
			}
			_snprintf_s(calcnode->val, sizeof(calcnode->val), _TRUNCATE, "%1.0f", dval1 - dval2);
			break;
		default:
			strncpy_s(calcnode->val, sizeof(calcnode->val), "error, badop", _TRUNCATE);
			break;
		}
		// delete last two nodes and link calcnode to any remaining links in list
		lastp = calcnode->np->np->np;
		free(calcnode->np->np);
		free(calcnode->np);
		calcnode->np = lastp;
	}

	//firstnode->np = 3rdnode->np;
	//free(2ndnode); free 3rd node

	// to avoid infinite loop, if there are only two ops, get rid of last one
	if (calcnode && calcnode->np && !calcnode->np->np){
		free(calcnode->np);
		calcnode->np = NULL;
	}



	return;
}



int HasOperators(CALC_LL *calcll)
{
	CALC_LL *node;

	for (node = calcll; node; node = node->np){
		if (node->type == CALC_OP){
			return(TRUE);
		}
	}


	return(FALSE);

}





char *ComplexCalcs(struct InstanceData* id, char *sourcestr, char *calcbuf, int calclen, MGLGRID *parentgrid, int voindex, VO *vcol)
{
	CALC_LL *calcll, *nxtnode;
	char *answer;

	calcll = CreateCalcll(sourcestr);
	calcll = ConvertKeywords(id, calcll, parentgrid, voindex, vcol );

	//  put each word into a linked list
	if (!HasOperators(calcll)){
		// no operators, so assume it is a string
		// piece string back together
		for (nxtnode = calcll, answer = NULL; nxtnode; nxtnode = nxtnode->np){
			if (answer){
				answer = strcat_alloc(answer, " ");
				answer = strcat_alloc(answer, nxtnode->val);
			}
			else {
				answer = str_falloc(nxtnode->val);
			}
		}
		strncpy_s(calcbuf, calclen, answer, _TRUNCATE);
		s_free(answer);
		goto calcval;
	}


	while ((nxtnode = findnxtnode(calcll))) {
		complexcalc(nxtnode);
		// if only one calcnode, then break
		if (calcll && !calcll->np){
			break;
		}
	}
	// must always be one left
	strncpy_s(calcbuf, calclen, calcll->val, _TRUNCATE);

calcval:
	// free the calcll
	while (calcll){
		nxtnode = calcll->np;
		free(calcll);
		calcll = nxtnode;
	}

	return(calcbuf);
}




//VO *vcol_find_label(vo, colname)
//VO *vo;
//char *colname;
//{
//	VO *vcol;
//	int i;
//
//
//	if (!vo || !colname || !vo->vcols){
//		return(NULL);
//	}
//	for (i = 0; i < vo->vcols->count; i++){
//		vcol = V_ROW_PTR(vo->vcols, i);
//		if (vcol && (vcol->magic_cookie == VO_MAGIC_NUM) &&
//			strcmp(colname, vcol->label) == 0)
//		{
//			return(vcol);
//		}
//	}
//
//
//	return(NULL);  /* not found */
//}



int MGLFiltersMatch(struct InstanceData* id, MGLGRID *parentgrid, int action, int i)
{
	char *colname, *valstr, *op, *cmp_value;	
	int f;
	VO *vcol, *searchvo, *cmpvcol;
	struct row_index *row_index;
	char calcbuf[512];
	char valbuf[512];	


	if (!parentgrid || !parentgrid->Actions[action].gridconfig){
		return(FALSE);
	}
	if (parentgrid->Actions[action].IsActive==1)
	{
		for (f = 0; f < parentgrid->Actions[action].gridconfig->nFilters; f++){
			// find column in vo that is being filtered		
			if (!strlen((colname = parentgrid->Actions[action].gridconfig->filters[f].field_name))) continue;
			if (parentgrid->Actions[action].gridconfig->filters[f].filter_type == FTYPE_LIST){
				// filter type is a list of values
				if (!(vcol = vcol_find(parentgrid->vo, colname))) continue;
				valstr = get_vcol_valstr(vcol, V_ROW_PTR(parentgrid->vo, i), TRUE);
				if (!(searchvo = parentgrid->Actions[action].gridconfig->filters[f].searchvo)){
					parentgrid->Actions[action].gridconfig->filters[f].searchvo =
						MGLCrSearchVO((char *)&parentgrid->Actions[action].gridconfig->filters[f].select_value);
					parentgrid->Actions[action].gridconfig->filters[f].filter_type = FTYPE_LIST;
				}
				if ((searchvo = parentgrid->Actions[action].gridconfig->filters[f].searchvo)){ // vo list of values
					if ((row_index = vo_search(searchvo, "listval", valstr, NULL))){
						// match found 
						continue;  // next filter
					}
					else {
						return(FALSE);
					}
				}
			}
			else if (parentgrid->Actions[action].gridconfig->filters[f].filter_type == FTYPE_OPERATORS){
				op = parentgrid->Actions[action].gridconfig->filters[f].op;  // op is 2 letter code, ala NE EQ LE
				cmp_value = parentgrid->Actions[action].gridconfig->filters[f].cmp_value;
				if (strcmp(cmp_value, "ASDE - X") == 0)
					strcpy_s(cmp_value, 64, "ASDE-X");
				// see if the current row i matches this filter parameter

				if (!(vcol = vcol_find(parentgrid->vo, colname)) && !(vcol = vcol_find_label(parentgrid->vo, colname))) continue;
				valstr = get_vcol_valstr(vcol, V_ROW_PTR(parentgrid->vo, i), TRUE);
				
				// valstr gets written over on next call to get_vcol_valstr, so copy the value
				strncpy_s(valbuf, sizeof(valbuf) - 1, valstr, _TRUNCATE);



				// we also have to check label names as well, when that is added to vo struct
				if ((cmpvcol = vcol_find(parentgrid->vo, cmp_value))){
					// substitute for value from this column
					cmp_value = get_vcol_valstr(cmpvcol, V_ROW_PTR(parentgrid->vo, i), TRUE);
				}

				cmp_value = ComplexCalcs(id,cmp_value, calcbuf, sizeof(calcbuf), parentgrid, i, vcol);  // process arithmatic values, substitute colnames, keywords like now() etc

				if (MGLMatch_all(parentgrid, action, op, f, cmp_value, valbuf, colname) == FALSE){
					// all filters must match or the row fails  ('OR' can be accomplished with multiple action filters)
					return(FALSE);
				}
					
				
			}
		}  // end each filter
}
	return(TRUE);

}


char *
str_replace(const char *string, const char *substr, const char *replacement){
	char *tok = NULL;
	char *newstr = NULL;
	char *oldstr = NULL;
	char *head = NULL;

	/* if either substr or replacement is NULL, duplicate string a let caller handle it */
	if (substr == NULL || replacement == NULL) return _strdup(string);
	newstr = _strdup(string);
	head = newstr;
	while ((tok = strstr(head, substr))){
		oldstr = newstr;
		newstr = malloc(strlen(oldstr) - strlen(substr) + strlen(replacement) + 1);
		/*failed to alloc mem, free old string and return NULL */
		if (newstr == NULL){
			free(oldstr);
			return NULL;
		}
		memcpy(newstr, oldstr, tok - oldstr);
		memcpy(newstr + (tok - oldstr), replacement, strlen(replacement));
		memcpy(newstr + (tok - oldstr) + strlen(replacement), tok + strlen(substr), strlen(oldstr) - strlen(substr) - (tok - oldstr));
		memset(newstr + strlen(oldstr) - strlen(substr) + strlen(replacement), 0, 1);
		/* move back head right after the last replacement */
		head = newstr + (tok - oldstr) + strlen(replacement);
		free(oldstr);
	}
	return newstr;
}

void MGLAddFilter(struct InstanceData* id, MGLGRID *ParametersGrid, int vorow)
{
	char *opstr, *cmp_value, *colname;
	char tmpbuf[64];
	int maxlen, f, gridcolnum, gridrow;
	VO *vcol;
	MGLGRID *SourceGrid;
	time_t secs, midnightsecs;
	char midnight[64];

	assert(ParametersGrid);

	SourceGrid = ParametersGrid->parentgrid;

	// vorow = gridrow - 1;
	if (vorow < 0 || vorow >= ParametersGrid->vo->count){
		return;
	}
	colname = VP(ParametersGrid->vo, vorow, "Column", char);

	cmp_value = VP(ParametersGrid->vo, vorow, "Value1", char);
	if (!strlen(cmp_value)){
		// user removed this operation
		// clear out the orange background
		ParametersGrid->vo->row_index[vorow].bg = -1;
		return;
	}
	// add space before and after operator, if needed
#ifdef ADD_SPACE_BETWEEN_OPERATORS
	if ((vcol = vcol_find(ParametersGrid->vo, "Value1"))){
		colsize = vcol->size;
		if (strchr(cmp_value, '-') && !strstr(cmp_value, " -") ){
			if ((newstr = str_replace(cmp_value, "-", " - "))){
				strncpy_s(VP(ParametersGrid->vo, vorow, "Value1", char), colsize, newstr, _TRUNCATE);
				free(newstr);
				cmp_value = VP(ParametersGrid->vo, vorow, "Value1", char);
			}
		}
		if (strchr(cmp_value, '+') && !strstr(cmp_value, " +")){
			if ((newstr = str_replace(cmp_value, "+", " + "))){
				strncpy_s(VP(ParametersGrid->vo, vorow, "Value1", char), colsize, newstr, _TRUNCATE);
				free(newstr);
				cmp_value = VP(ParametersGrid->vo, vorow, "Value1", char);
			}
		}
	}
#endif



	(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->nFilters)++;
	f = SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->nFilters - 1;
	gridrow = vorow - ParametersGrid->iVscrollPos + 1;
	if ((opstr = MGLFindAlertsOperator(ParametersGrid, vorow))){
		// if ( strlen( (opstr = VP( ParametersGrid->vo, vorow, "Operator", char )) ) ){

		maxlen = sizeof(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].op);
		strncpy_s(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].op, maxlen, opstr, _TRUNCATE);


		maxlen = sizeof(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].cmp_value);
		strncpy_s(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].cmp_value, maxlen, cmp_value, _TRUNCATE);

		maxlen = sizeof(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].field_name);
		strncpy_s(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].field_name, maxlen, colname, _TRUNCATE);

		if (SourceGrid->vo){
			if ((vcol = vcol_find(SourceGrid->vo, colname)) || (vcol = vcol_find_label(SourceGrid->vo, colname))){
				SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].field_type = vcol->dbtype;
				// if the field is a datetime field, then convert input into unix seconds  (keep display as it is)
				gridcolnum = MGLGridColNum(SourceGrid, vcol);
				if (SourceGrid->gridcols[gridcolnum].display_data_type == DATETIMEBIND){
					if (MGLConvert2Secs(id, cmp_value, tmpbuf, sizeof(tmpbuf))){
						// converted to integer string for unix time like 1389938390. etc
						//	strncpy_s(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].cmp_value, maxlen, tmpbuf, _TRUNCATE);
					}
				}
				else if (vcol->dbtype == INTBIND && strchr(cmp_value, ':')){
					// if the data_type is INTBIND, but user entered a time string such as "00:20:00", then convert the elapsed time into seconds
					if (MGLConvert2Secs(id, cmp_value, tmpbuf, sizeof(tmpbuf))){
						// converted to integer string for unix time like 1389938390. etc
						// we need to strip off only the seconds past midnight to get the interval
						secs = time(0);
						VOGMTime(midnight, secs, "%Y%m%d");
						midnightsecs = MGLDtStr2Secs(id, midnight, "%Y%m%d");

						secs = atoi(tmpbuf);
						secs = secs - midnightsecs;

						// for a time interval, we need to subtract from midnight
						sprintf_s(tmpbuf, sizeof(tmpbuf), "%d", secs);
						//	strncpy_s(SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].cmp_value, maxlen, tmpbuf, _TRUNCATE);
					}
				}

			}
		}
		SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->filters[f].filter_type = FTYPE_OPERATORS;
	}

}



char *MGLActionStandardColor(MGLGRID *topgrid, int CurActionNum, char *hexcolor)
{

	if (!strcmp(hexcolor, "FC4740")){
		return("Red");
	}
	else if (!strcmp(hexcolor, "00FF00")){
		return("Green");
	}
	else if (!strcmp(hexcolor, "0000FF")){
		return("Blue");
	}
	else if (!strcmp(hexcolor, "FFFF00")){  // yellow
		return("Yellow");
	}
	else if (!strcmp(hexcolor, "F4921F")){  //orange
		return("Orange");
	}
	//else if (!strcmp(hexcolor, "999999")){   // silver
	//	return("Silver");
	//}
	//else if (!strcmp(hexcolor, "BF2F37")){ // maroon
	//	return("Maroon");
	//}


	return(NULL);
}




void MGLFillColorsGrid(struct InstanceData* id, MGLGRID *ParametersGrid, int CurActionNum)
{
	MGLGRID *topgrid, *AlertsColorGrid;
	VO *AlertsColorsVO, *vcol;
	int fg_row, bg_row, filterrow;
	char *colorstr;

	assert(id);

	// fill in the colors grid with values in the gridconfig structure

	topgrid = ParametersGrid->parentgrid;

	AlertsColorGrid = topgrid->AlertsColorGrid;
	AlertsColorsVO = AlertsColorGrid->vo;

	// clear out existing values from another action
	MGLInitAlertsColorsVO(topgrid);


	///   topgrid->Actions[topgrid->CurActionNum].ActionType
	///   sounds  -- get from bd 
	///   colors rgb values
	///  popups -- how to format messages

	////   filter type on/off

	// set colored cells if rgb matches
	fg_row = 0;
	bg_row = 1;
	if ((colorstr = MGLActionStandardColor(topgrid, CurActionNum, topgrid->Actions[topgrid->CurActionNum].row_fg))){
		// a pre-defined color was found, so set that color 
		if ((vcol = vcol_find(AlertsColorGrid->vo, colorstr))){
			strncpy_s(VP(AlertsColorGrid->vo, fg_row, colorstr, char), vcol->size, "X", _TRUNCATE);
		}
	}
	if ((colorstr = MGLActionStandardColor(topgrid, CurActionNum, topgrid->Actions[topgrid->CurActionNum].row_bg))){
		// a pre-defined color was found, so set that color 
		if ((vcol = vcol_find(AlertsColorGrid->vo, colorstr))){
			strncpy_s(VP(AlertsColorGrid->vo, bg_row, colorstr, char), vcol->size, "X", _TRUNCATE);
		}
	}

	// update filter value with X if set
	filterrow = 3; // filter X row
	if (topgrid->Actions[topgrid->CurActionNum].ActionType & ACTION_FILTER){
		// set the filter column under the "Red"
		if ((vcol = vcol_find(AlertsColorGrid->vo, "Red"))){
			strncpy_s(VP(AlertsColorGrid->vo, filterrow, "Red", char), vcol->size, "X", _TRUNCATE);
		}
	}
	filterrow = 2;  // alerts X row
	if (topgrid->Actions[topgrid->CurActionNum].ActionType & ACTION_ALERT_LIST){
		// set the alert column under the "Red"
		if ((vcol = vcol_find(AlertsColorGrid->vo, "Red"))){
			strncpy_s(VP(AlertsColorGrid->vo, filterrow, "Red", char), vcol->size, "X", _TRUNCATE);
		}
	}

	CalcColWidths(id, AlertsColorGrid);
}


int MGLHexColortoI(char *hexcolorstr)
{
	int red_ival, green_ival, blue_ival;
	// expectiing string color like "FE8657"

	assert(hexcolorstr);

	red_ival = HexStrtod(&hexcolorstr[0], 2);
	green_ival = HexStrtod(&hexcolorstr[2], 2);
	blue_ival = HexStrtod(&hexcolorstr[4], 2);

	return(red_ival << 16 | green_ival << 8 | blue_ival);
}

void MGLFillParametersDelete(struct InstanceData* id, MGLGRID *ParametersGrid, int vorow)
{
	int listcolnum;
	MGLGRID *SourceGrid;
	VO *vcol;
	
	extern GLfloat orange[3];
	int x;

	assert(id);

	if (!ParametersGrid || !ParametersGrid->parentgrid) return;

	SourceGrid = ParametersGrid->parentgrid;

	if ((vcol = vcol_find(ParametersGrid->vo, "List"))){
		listcolnum = vcol->colnum;
	}


	// go through each row in the alert grid and add in filters for each
	// clear out old ones
	if (SourceGrid->CurActionNum >= SourceGrid->nActions || !SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig){
		return;
	}
	if (SourceGrid->CurActionNum==vorow)
	{
		SourceGrid->CurActionNum = 0;
	//	SourceGrid->ActionsGrid->vo->row_index[0].fg = MGLColorHex2Int("529EFD");
	}
	else if (SourceGrid->CurActionNum > vorow)
	{
		SourceGrid->CurActionNum = SourceGrid->CurActionNum-1;
	//	SourceGrid->ActionsGrid->vo->row_index[SourceGrid->CurActionNum].fg = MGLColorHex2Int("529EFD");
	}
	else if (SourceGrid->CurActionNum < vorow)
	{
		SourceGrid->CurActionNum = SourceGrid->CurActionNum;
	//	SourceGrid->ActionsGrid->vo->row_index[SourceGrid->CurActionNum].fg = MGLColorHex2Int("529EFD");
	}
	SourceGrid->Actions[vorow].gridconfig->nFilters = 0;
	if (vorow == (SourceGrid->nActions - 1))
	{

	}

	else
	{
		memset(&SourceGrid->Actions[vorow], 0, sizeof(SourceGrid->Actions[0]));
		for (x = 0; x < SourceGrid->nActions; x++)
		{
			if (strcmp(SourceGrid->Actions[x].ActionName, "") != 0)
			{
			}
			else
			{
				memcpy(&SourceGrid->Actions[x], &SourceGrid->Actions[x + 1], sizeof(SourceGrid->Actions[x]));
				memset(&SourceGrid->Actions[x + 1], 0, sizeof(SourceGrid->Actions[0]));
			}
		}

	}
	SourceGrid->nActions--;
	

	MGLFillColorsGrid(id, ParametersGrid, SourceGrid->CurActionNum);
	

	if (SourceGrid->UpdateCB){
		SourceGrid->UpdateCB(id, SourceGrid);
	}
}

void MGLFillParameters(struct InstanceData* id, MGLGRID *ParametersGrid)
{
	int listcolnum, vorow;
	MGLGRID *SourceGrid;
	VO *vcol;
	char *opstr, *Value1;
	extern GLfloat orange[3];

	assert(id);

	if (!ParametersGrid || !ParametersGrid->parentgrid) return;

	SourceGrid = ParametersGrid->parentgrid;

	if ((vcol = vcol_find(ParametersGrid->vo, "List"))){
		listcolnum = vcol->colnum;
	}


	// go through each row in the alert grid and add in filters for each
	// clear out old ones
	if (SourceGrid->CurActionNum >= SourceGrid->nActions || !SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig){
		return;
	}
	SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig->nFilters = 0;


	for (vorow = 0; vorow < ParametersGrid->vo->count; vorow++){

		if (strlen((opstr = VP(ParametersGrid->vo, vorow, "Operator", char)))){
			// make sure there is a value
			if (strlen((Value1 = VP(ParametersGrid->vo, vorow, "Value1", char)))){
				MGLAddFilter(id, ParametersGrid, vorow);
			}
		}
	}

	MGLFillColorsGrid(id, ParametersGrid, SourceGrid->CurActionNum);


	if (SourceGrid->UpdateCB){
		SourceGrid->UpdateCB(id, SourceGrid);
	}
}



void MGLClearParametersGrid(MGLGRID *SourceGrid)
{
	int gridrow, c;

	assert(SourceGrid);

	// clear out any orange parameters
	for (gridrow = 0; gridrow < SourceGrid->ParametersGrid->nrows; gridrow++){
		for (c = 1; c < SourceGrid->ParametersGrid->vo->vcols->count; c++){
			SourceGrid->ParametersGrid->gridcells[gridrow][c].bg = (GLfloat*)NULL;
			SourceGrid->ParametersGrid->gridcells[gridrow][c].fg = (GLfloat*)NULL;
		}
	}

	MGLFillParametersColsVO(SourceGrid);



}


int MGLParmColNum(MGLGRID *ParametersGrid, char *op)
{
	VO *vcol;

	assert(ParametersGrid);

	if ((vcol = vcol_find(ParametersGrid->vo, op))){
		return(vcol->colnum);
	}
	return(-1);
}



char *MGLOpName(char *opstr)
{

	assert(opstr);

	if (!strcmp(opstr, "EQ")){
		return("Equal to");
	}
	else if (!strcmp(opstr, "NE")){
		return("Not Equal to");
	}
	else if (!strcmp(opstr, "GT")){
		return("Greater than");
	}
	else if (!strcmp(opstr, "LT")){
		return("Less than");
	}
	else if (!strcmp(opstr, "LE")){
		return("Less than or Equal to");
	}
	else if (!strcmp(opstr, "GE")){
		return("Greater than or Equal to");
	}
	else if (!strcmp(opstr, "BT")){
		return("Between");
	}

	return("UK");
}


void MGLFillParametersGrid(MGLGRID *ActionsGrid, int CurActionNum, struct InstanceData* id)
{
	GridConfig *gridconfig;
	MGLGRID *SourceGrid;
	int f, colnum, r, t;
	char *op, *cmp_value, *field_name, *colname, *opstr;
	VO *vcol;
	extern GLfloat orange[3];
	assert(ActionsGrid);
	char full[512];
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// fill in the parmeters from the selected Action into the alerts grid
	// go through each filter for this action
	SourceGrid = ActionsGrid->parentgrid;

	gridconfig = SourceGrid->Actions[SourceGrid->CurActionNum].gridconfig;
	MGLClearParametersGrid(SourceGrid);  // clear out the old colors and parameter values 

	if (SecondTabGrid)
	{
		if (SecondTabGrid->childgrid)
		{
			if (SecondTabGrid->childgrid->OperatorGrid)
			{
				if (SecondTabGrid->childgrid->OperatorGrid->parent_w->visible == 1)
					SecondTabGrid->childgrid->OperatorGrid->parent_w->visible = 0;
			}
		}
	}

	///mtm:clearing Build summary (Parameters column)
	if (SourceGrid)
	{

		if (SourceGrid->BuildSummaryGrid)
		{
			for (t = 0; t < SourceGrid->BuildSummaryGrid->vo->count; t++)
				strncpy_s(VP(SourceGrid->BuildSummaryGrid->vo, t, "Parameters", char), 64, "", _TRUNCATE);
		}

	}

	for (f = 0; gridconfig && f < gridconfig->nFilters; f++){

		op = gridconfig->filters[f].op;
		cmp_value = gridconfig->filters[f].cmp_value;
		field_name = gridconfig->filters[f].field_name;
		strcpy_s(full, 512, field_name);
		strcat_s(full, 512, " ");
		strcat_s(full, 512, op);
		strcat_s(full, 512, " ");
		strcat_s(full, 512, cmp_value);

		/*if (SourceGrid->BuildSummaryGrid->vo->count <= f)
			vo_alloc_rows(SourceGrid->BuildSummaryGrid->vo, 1);
		strncpy_s(VP(SourceGrid->BuildSummaryGrid->vo, f, "Parameters", char), 64, full, _TRUNCATE);
*/

		if (!strlen(field_name)){
			break;  // bad op
		}
		// set in parameters grid
		//if ( (colnum = MGLParmColNum(SourceGrid->ParametersGrid, op )) < 1 ){
		//	continue;  // bad op
		//}
		if ((vcol = vcol_find(SourceGrid->ParametersGrid->vo, "Operator"))){
			colnum = vcol->colnum;
		}
		else {
			break;
		}

		// get row number in ParametersGrid for the given field name
		for (r = 0; r < SourceGrid->ParametersGrid->vo->count; r++){
			colname = VP(SourceGrid->ParametersGrid->vo, r, "Column", char);
			if (!strcmp(colname, field_name)){
				break;
			}
		}
		// color it orange
		// no longer need to color it SourceGrid->ParametersGrid->gridcells[r + 1][colnum].bg = (GLfloat*) orange;
		// add in value to value1 field
		if (r < SourceGrid->ParametersGrid->vo->count){
			vcol = vcol_find(SourceGrid->ParametersGrid->vo, "Value1");
			strncpy_s(VP(SourceGrid->ParametersGrid->vo, r, "Value1", char), vcol->size, cmp_value, _TRUNCATE);

			vcol = vcol_find(SourceGrid->ParametersGrid->vo, "Operator");
			opstr = MGLOpName(op);
			strncpy_s(VP(SourceGrid->ParametersGrid->vo, r, "Operator", char), vcol->size, opstr, _TRUNCATE);
		}
		else {
			// debug 
			strcpy_s(full, 512, "overflow error detected");
		}
	}
	if (SourceGrid->BuildSummaryGrid)
	{
		InitParametersinBuildSummary(id, SourceGrid);
		InitCriteriainBuildSummary(id, SourceGrid);
	}
}





void MGLFillNewDataApp(MGLGRID *DataAppGrid, struct InstanceData* id)
{
	VO *vcol;
	int valcolnum;
	int gridrow;
	char *AppName;
	char AppName1[64];
	MGLGRID *grid;
	assert(DataAppGrid);

	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(DataAppGrid->vo, DataAppGrid->textcolname);
	valcolnum = vcol->colnum;
	gridrow = DataAppGrid->textrow - DataAppGrid->iVscrollPos + 1;
	// set row colors back to default
	DataAppCreater->character_limit = 0;
	DataAppCreater->shift_limit = 0;

	// and create a another "New" row in DataAppGrid for adding a "new" action. 
	// Clicking on "New" will present a text box that the user fills in to give the action a name
	// create "NEW" row, that user will click on to add an action
	if (!strlen(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char))){
		// do not allow empty name
		// reset text to be active
		DataAppGrid->parent_w->TextActive = FALSE;
		ActiveTextGrid = NULL;		
		DataAppGrid->gridcells[gridrow][valcolnum].bg = (GLfloat*)NULL;
		DataAppGrid->gridcells[gridrow][valcolnum].fg = (GLfloat*)NULL;
		AppName = VP(DataAppGrid->vo, gridrow-1, "AppName", char);
		grid = VV(DataAppGrid->vo, gridrow-1, "gridptr", void *);
		strcpy_s(grid->name,32,AppName);
		strcpy_s(AppName1,64,"Grid Detail : ");
		strcat_s(AppName1,64,AppName);
		MGLSetTitle(grid->TitleGrid, AppName1);
		MGLSetTitle(grid, AppName);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, "New", _TRUNCATE);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 16, "Choose", _TRUNCATE);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "Show", char), 16, "Show", _TRUNCATE);


	}
	else {		
		DataAppGrid->gridcells[gridrow][valcolnum].bg = (GLfloat*)NULL;
		DataAppGrid->gridcells[gridrow][valcolnum].fg = (GLfloat*)NULL;

		vo_alloc_rows(DataAppGrid->vo, 1);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, "   ", _TRUNCATE);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 16, "      ", _TRUNCATE);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "Show", char), 16, "    ", _TRUNCATE);

	}

}


//mtm
void MGLFillNewDataAppCreater(MGLGRID *DataAppCreater)
{
	VO *vcol;
	int valcolnum;


	assert(DataAppCreater);

	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(DataAppCreater->vo, DataAppCreater->textcolname);
	valcolnum = vcol->colnum;
	// set row colors back to default


	// and create a another "New" row in DataAppGrid for adding a "new" action. 
	// Clicking on "New" will present a text box that the user fills in to give the action a name
	// create "NEW" row, that user will click on to add an action
	//if (!strlen(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "DataSource", char))){
	//	// do not allow empty name
	//	// reset text to be active
	//	DataAppGrid->parent_w->TextActive = FALSE;
	//	ActiveTextGrid = NULL;
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, "New", _TRUNCATE);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 16, "Choose", _TRUNCATE);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "Show", char), 16, "Show", _TRUNCATE);


	//}
	//else {
	//	gridrow = DataAppGrid->textrow - DataAppGrid->iVscrollPos + 1;
	//	DataAppGrid->gridcells[gridrow][valcolnum].bg = (GLfloat*)NULL;
	//	DataAppGrid->gridcells[gridrow][valcolnum].fg = (GLfloat*)NULL;

	//	vo_alloc_rows(DataAppGrid->vo, 1);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, "New", _TRUNCATE);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 16, "Choose", _TRUNCATE);
	//	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "Show", char), 16, "Show", _TRUNCATE);

	//}

}


void MGLFillNewAction(struct InstanceData* id, MGLGRID *ActionsGrid)
{
	VO *vcol;
	int valcolnum, vorow;
	char *textcolname, *curstr;
	MGLGRID *topgrid;
	int x;
	int  red_ival, green_ival, blue_ival, fg_ival, bg_ival;

	assert(id);
	assert(ActionsGrid);

	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	
	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(ActionsGrid->vo, ActionsGrid->textcolname);
	valcolnum = vcol->colnum;
	// set row colors back to default
	ActionsGrid->gridcells[ActionsGrid->textrow + 1][valcolnum].bg = (GLfloat*)NULL;
	ActionsGrid->gridcells[ActionsGrid->textrow + 1][valcolnum].fg = (GLfloat*)NULL;

	

	// create a new action with this name, 
	topgrid = ActionsGrid->parentgrid;
	topgrid->CurActionNum = ActionsGrid->textrow;


	if (!topgrid->Actions[topgrid->CurActionNum].gridconfig){
		topgrid->Actions[topgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
	}

	topgrid->Actions[topgrid->CurActionNum].filter_active = 1;
		SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 1;
	//else if (topgrid->Actions[topgrid->CurActionNum].filter_active == 0)
	//	SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 0;
	// load in data that is stored in etas-db all-etas in table grid_config
	// LoadUserRoiSettings (id, id->m_strUserName, id->m_strRegionArpt, configname, ROIGrid->Actions[ROIGrid->CurActionNum].gridconfig);
	topgrid->nActions = ActionsGrid->textrow + 1;
	topgrid->Actions[topgrid->CurActionNum].IsActive = TRUE;
	topgrid->Actions[topgrid->CurActionNum].ActionType = ACTION_FILTER; // default type is ACTION_FILTER

	
	// Set Action name in gridconfig
	vorow = ActionsGrid->textrow;
	textcolname = ActionsGrid->textcolname;
	curstr = VP(ActionsGrid->vo, vorow, textcolname, char);
	strcpy_s(topgrid->Actions[topgrid->CurActionNum].ActionName, sizeof(topgrid->Actions[topgrid->CurActionNum].ActionName),
		curstr);
	for (x = 0; x < ActionsGrid->vo->count; x++)
	{
		ActionsGrid->vo->row_index[x].fg = MGLColorHex2Int("000000");
	}
	if (strcmp(VP(ActionsGrid->vo, vorow, 0, char), "") == 0)
	{
		strncpy_s(VP(ActionsGrid->vo, vorow, "Name", char), 64, "New", _TRUNCATE);
		ActionsGrid->parentgrid->Actions[vorow].IsActive = 0;
	}
	if (strcmp(VP(ActionsGrid->vo, vorow, 0, char), "New") != 0)
	{
		ActionsGrid->vo->row_index[vorow].fg = MGLColorHex2Int("529EFD");
		ActionsGrid->parentgrid->Actions[vorow].IsActive = 1;
		vo_alloc_rows(ActionsGrid->vo, 1);
		strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "Name", char), 64, "New", _TRUNCATE);
		strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "OnOff", char), 16, "On", _TRUNCATE);
	}
	///
	topgrid->Summary[topgrid->nActions].Criteria_rownumber = ActionsGrid->textrow + 1;	
	
	red_ival = 0;
	green_ival = 0;
	blue_ival = 0;

	fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
	SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[0].fg = fg_ival;
	SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[0] = 0;

	bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
	SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[1].bg = bg_ival;
	SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[1] = 0;		

	

	// fill in the ParametersGrid with data from this new Action, which should be blank
	MGLFillParametersGrid(ActionsGrid, topgrid->CurActionNum,id);	
	InitParametersinBuildSummary(id, SecondTabGrid->childgrid);	
	CalcColWidths(id, ActionsGrid->parentgrid->ParametersGrid);
	MGLFillColorsGrid(id, ActionsGrid->parentgrid->ParametersGrid, topgrid->CurActionNum);
	CalcColWidths(id, ActionsGrid);
	
}

void MGLFillEMail(struct InstanceData* id, MGLGRID *EMailGrid)
{
	VO *vcol;	
	int valcolnum,j;

	assert(id);
	assert(EMailGrid);
	
	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(EMailGrid->vo, EMailGrid->textcolname);
	valcolnum = vcol->colnum;
	// set row colors back to default
	EMailGrid->gridcells[EMailGrid->textrow + 1 - EMailGrid->iVscrollPos][valcolnum].bg = (GLfloat*)NULL;
	EMailGrid->gridcells[EMailGrid->textrow + 1 - EMailGrid->iVscrollPos][valcolnum].fg = (GLfloat*)NULL;
	j = EMailGrid->textrow - EMailGrid->iVscrollPos;
	if (EMailGrid->vo->count - 1 == EMailGrid->textrow)
	{
		vo_alloc_row(EMailGrid->vo, 1);
		strcpy_s(VP(EMailGrid->vo, EMailGrid->textrow + 1, "EMail Notifications", char), 64, "Enter EMailID");
	}
	char test[64],temp[64],temp2[64];
	strcpy(test, VP(EMailGrid->vo, j, "EMail Notifications", char));
	if (strcmp(VP(EMailGrid->vo, j, "EMail Notifications", char), "") == 0)
	{
		if ((j == 0) && (EMailGrid->vo->count <= 2))
		{
			EMailGrid->email_enabled = 0;
			EMailGrid->parentgrid->AlertsColorGrid->show_checkbox[4] = 0;
		}
		vo_rm_row(EMailGrid->vo, j);	
		strcpy(VP(EMailGrid->parentgrid->BuildSummaryGrid->vo, j + 1, "Actions", char), "");
		strncpy_s(VP(EMailGrid->vo, 0, "To", char), 64, "To :", _TRUNCATE);
		
	}
	else
	{
		strcpy(temp, VP(EMailGrid->vo, j, "EMail Notifications", char));
		EMailGrid->email_enabled = 1;
		EMailGrid->parentgrid->AlertsColorGrid->show_checkbox[4] = 1;
		if (EMailGrid->parentgrid->BuildSummaryGrid->vo->count < j)
			vo_alloc_row(EMailGrid->parentgrid->BuildSummaryGrid->vo, 1);
		strcpy(temp2, "E-ID:");
		strcat(temp2, temp);
		strcpy(VP(EMailGrid->parentgrid->BuildSummaryGrid->vo, j + 1, "Actions", char), temp2);
	}
	
}

void MGLFillSubject(struct InstanceData* id, MGLGRID *SubjectGrid)
{
	VO *vcol;
	int valcolnum;

	assert(id);
	assert(SubjectGrid);

	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(SubjectGrid->vo, SubjectGrid->textcolname);
	valcolnum = vcol->colnum;
	// set row colors back to default
	SubjectGrid->gridcells[SubjectGrid->textrow + 1 - SubjectGrid->iVscrollPos][valcolnum].bg = (GLfloat*)NULL;
	SubjectGrid->gridcells[SubjectGrid->textrow + 1 - SubjectGrid->iVscrollPos][valcolnum].fg = (GLfloat*)NULL;	

}
//mtm: fill the visible row grid after hitting enter key
void MGLFillRowColumn(struct InstanceData* id, MGLGRID *RowColumn)
{
	VO *vcol;
	int valcolnum, vorow, i;
	char *textcolname;
	char *nrows, *ncolumns;
	int rows, columns;
	MGLGRID *topgrid;
	char full[512];


	assert(id);
	assert(RowColumn);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* WarningGrid = (MGLGRID*)id->WarningGrid;
	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(RowColumn->vo, RowColumn->textcolname);
	valcolnum = vcol->colnum;
	// set row colors back to default
	RowColumn->gridcells[RowColumn->textrow + 1][valcolnum].bg = (GLfloat*)NULL;
	RowColumn->gridcells[RowColumn->textrow + 1][valcolnum].fg = (GLfloat*)NULL;

	// create a new action with this name, 
	topgrid = RowColumn->parentgrid;
	topgrid->CurActionNum = RowColumn->textrow;

	// Set Action name in gridconfig
	vorow = RowColumn->textrow;
	textcolname = RowColumn->textcolname;

	if (topgrid->roi_fdo == 1)
	{
		if (vorow == 0)
		{
			nrows = VP(RowColumn->vo, vorow, textcolname, char);
			rows = atoi(nrows);
			if (strcmp(nrows, "") == 0)
			{
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count == 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, "", _TRUNCATE);
				SecondTabGrid->childgrid->nrows = 16;
				SecondTabGrid->childgrid->height = 16 * MGL_GRID_DEF_ROW_HEIGHT;
			}
			else
			{
				strcpy_s(full, 64, "rows:");
				strcat_s(full, 64, nrows);
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, full, _TRUNCATE);
				if ((rows > 0) && (rows < 16)){
					SecondTabGrid->childgrid->nrows = rows + 1;
					SecondTabGrid->childgrid->height = (rows + 1)* MGL_GRID_DEF_ROW_HEIGHT;
				}
				else
				{
					WarningGrid = CreateWarning(id);
					id->WarningGrid = WarningGrid;
					MGLGridText(id, WarningGrid, 0, 0, "Warning: Cannot show more than 15 rows");
					WarningGrid->parent_w->visible = TRUE;
					SecondTabGrid->childgrid->nrows = 16;
					SecondTabGrid->childgrid->height = 16 * MGL_GRID_DEF_ROW_HEIGHT;
					strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, "", _TRUNCATE);
					strncpy_s(VP(SecondTabGrid->childgrid->RowColumn->vo, 0, "Settings", char), 16, "", _TRUNCATE);
				}
			}
		}

		//else if (vorow == 1)
		//{
		//	ncolumns = VP(RowColumn->vo, vorow, textcolname, char);
		//	columns = atoi(ncolumns);

		//	if (strcmp(ncolumns, "") == 0)
		//	{
		//		if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count == 0)
		//			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
		//		else
		//		{
		//			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 2);
		//		}
		//		strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1, "Visiblerows/columns", char), 64, "", _TRUNCATE);
		//	}
		//	else
		//	{
		//		strcpy_s(full, 512, "cols:");
		//		strcat_s(full, 512, ncolumns);
		//		if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count < 0)
		//			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
		//		if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 1)
		//		{
		//			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
		//			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
		//		}
		//		strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1, "Visiblerows/columns", char), 64, full, _TRUNCATE);
		//		if (columns > 0 && columns < 9)
		//		{
		//			if (columns == 8)
		//			{
		//				for (i = 0; i < 8; i++)
		//				{
		//					SecondTabGrid->childgrid->DisplayCol[i] = i;
		//				}

		//			}
		//			if (columns == 7)
		//			{
		//				SecondTabGrid->childgrid->DisplayCol[7] = -1;
		//				for (i = 0; i < 7; i++)
		//				{
		//					SecondTabGrid->childgrid->DisplayCol[i] = i;
		//				}
		//			}
		//			else if (columns == 6)
		//			{
		//				SecondTabGrid->childgrid->DisplayCol[7] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[6] = -1;
		//				for (i = 0; i < 6; i++)
		//				{
		//					SecondTabGrid->childgrid->DisplayCol[i] = i;
		//				}
		//			}
		//			else if (columns == 5)
		//			{
		//				SecondTabGrid->childgrid->DisplayCol[7] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[6] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[5] = -1;
		//				for (i = 0; i < 5; i++)
		//				{
		//					SecondTabGrid->childgrid->DisplayCol[i] = i;
		//				}

		//			}
		//			else if (columns == 4)
		//			{
		//				SecondTabGrid->childgrid->DisplayCol[7] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[6] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[5] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[4] = -1;
		//				for (i = 0; i < 4; i++)
		//				{
		//					SecondTabGrid->childgrid->DisplayCol[i] = i;
		//				}

		//			}
		//			else if (columns == 3)
		//			{
		//				SecondTabGrid->childgrid->DisplayCol[7] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[6] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[5] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[4] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[3] = -1;
		//				for (i = 0; i < 3; i++)
		//				{
		//					SecondTabGrid->childgrid->DisplayCol[i] = i;
		//				}

		//			}
		//			else if (columns == 2)
		//			{
		//				SecondTabGrid->childgrid->DisplayCol[7] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[6] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[5] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[4] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[3] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[2] = -1;
		//				for (i = 0; i < 2; i++)
		//				{
		//					SecondTabGrid->childgrid->DisplayCol[i] = i;
		//				}

		//			}
		//			else if (columns == 1)
		//			{
		//				SecondTabGrid->childgrid->DisplayCol[7] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[6] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[5] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[4] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[3] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[2] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[1] = -1;
		//				SecondTabGrid->childgrid->DisplayCol[0] = 0;

		//			}



		//		}

				//else
				//	for (i = 0; i < 8; i++)
				//	{
				//		SecondTabGrid->childgrid->DisplayCol[i] = i;
				//	}
		//	}
		}




	//}

	//
	if (topgrid->roi_fdo == 2)
	{
		if (vorow == 0)
		{
			nrows = VP(RowColumn->vo, vorow, textcolname, char);
			rows = atoi(nrows);
			if (strcmp(nrows, "") == 0)
			{
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count == 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, "", _TRUNCATE);
				SecondTabGrid->childgrid->nrows = 16;
				SecondTabGrid->childgrid->height = 16 * MGL_GRID_DEF_ROW_HEIGHT;
			}
			else
			{
				strcpy_s(full, 512, "rows:");
				strcat_s(full, 512, nrows);
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, full, _TRUNCATE);
				if ((rows > 0) && (rows < 16)){
					SecondTabGrid->childgrid->nrows = rows + 1;
					SecondTabGrid->childgrid->height = (rows + 1)* MGL_GRID_DEF_ROW_HEIGHT;
				}
				else
				{
					WarningGrid = CreateWarning(id);
					id->WarningGrid = WarningGrid;
					MGLGridText(id, WarningGrid, 0, 0, "Warning: Cannot show more than 15 rows");
					WarningGrid->parent_w->visible = TRUE;
					SecondTabGrid->childgrid->nrows = 16;
					SecondTabGrid->childgrid->height = 16 * MGL_GRID_DEF_ROW_HEIGHT;
					strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, "", _TRUNCATE);
					strncpy_s(VP(SecondTabGrid->childgrid->RowColumn->vo, 0, "Settings", char), 16, "", _TRUNCATE);
				}
			}
		}

		else if (vorow == 1)
		{
			ncolumns = VP(RowColumn->vo, vorow, textcolname, char);
			columns = atoi(ncolumns);
			if (strcmp(ncolumns, "") == 0)
			{
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count == 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				else
				{
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 2);
				}
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1, "Visiblerows/columns", char), 64, "", _TRUNCATE);
			}
			else
			{
				strcpy_s(full, 512, "cols:");
				strcat_s(full, 512, ncolumns);
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count < 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 1)
				{
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				}
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1, "Visiblerows/columns", char), 64, full, _TRUNCATE);
				if (columns > 0 && columns < 14)
				{
					if (columns == 13)
					{
						for (i = 0; i < 13; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}

					if (columns == 12)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						for (i = 0; i < 12; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}
					}

					if (columns == 11)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						for (i = 0; i < 11; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}
					}
					else if (columns == 10)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						for (i = 0; i < 10; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}
					}
					else if (columns == 9)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;

						for (i = 0; i < 9; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 8)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;

						for (i = 0; i < 8; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 7)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;
						SecondTabGrid->childgrid->DisplayCol[7] = -1;
						for (i = 0; i < 7; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 6)
					{

						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;
						SecondTabGrid->childgrid->DisplayCol[7] = -1;
						SecondTabGrid->childgrid->DisplayCol[6] = -1;
						for (i = 0; i < 6; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 5)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;
						SecondTabGrid->childgrid->DisplayCol[7] = -1;
						SecondTabGrid->childgrid->DisplayCol[6] = -1;
						SecondTabGrid->childgrid->DisplayCol[5] = -1;
						for (i = 0; i < 5; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 4)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;
						SecondTabGrid->childgrid->DisplayCol[7] = -1;
						SecondTabGrid->childgrid->DisplayCol[6] = -1;
						SecondTabGrid->childgrid->DisplayCol[5] = -1;
						SecondTabGrid->childgrid->DisplayCol[4] = -1;
						for (i = 0; i < 4; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 3)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;
						SecondTabGrid->childgrid->DisplayCol[7] = -1;
						SecondTabGrid->childgrid->DisplayCol[6] = -1;
						SecondTabGrid->childgrid->DisplayCol[5] = -1;
						SecondTabGrid->childgrid->DisplayCol[4] = -1;
						SecondTabGrid->childgrid->DisplayCol[3] = -1;
						for (i = 0; i < 3; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 2)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;
						SecondTabGrid->childgrid->DisplayCol[7] = -1;
						SecondTabGrid->childgrid->DisplayCol[6] = -1;
						SecondTabGrid->childgrid->DisplayCol[5] = -1;
						SecondTabGrid->childgrid->DisplayCol[4] = -1;
						SecondTabGrid->childgrid->DisplayCol[3] = -1;
						SecondTabGrid->childgrid->DisplayCol[2] = -1;
						for (i = 0; i < 2; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}
					else if (columns == 1)
					{
						SecondTabGrid->childgrid->DisplayCol[12] = -1;
						SecondTabGrid->childgrid->DisplayCol[11] = -1;
						SecondTabGrid->childgrid->DisplayCol[10] = -1;
						SecondTabGrid->childgrid->DisplayCol[9] = -1;
						SecondTabGrid->childgrid->DisplayCol[8] = -1;
						SecondTabGrid->childgrid->DisplayCol[7] = -1;
						SecondTabGrid->childgrid->DisplayCol[6] = -1;
						SecondTabGrid->childgrid->DisplayCol[5] = -1;
						SecondTabGrid->childgrid->DisplayCol[4] = -1;
						SecondTabGrid->childgrid->DisplayCol[3] = -1;
						SecondTabGrid->childgrid->DisplayCol[2] = -1;
						SecondTabGrid->childgrid->DisplayCol[1] = -1;
						for (i = 0; i < 1; i++)
						{
							SecondTabGrid->childgrid->DisplayCol[i] = i;
						}

					}


				}

				else
					for (i = 0; i < 13; i++)
					{
						SecondTabGrid->childgrid->DisplayCol[i] = i;
					}
			}


		}

	}

	if (topgrid->roi_fdo == 3)
	{
		if (vorow == 0)
		{
			nrows = VP(RowColumn->vo, vorow, textcolname, char);
			rows = atoi(nrows);
			if (strcmp(nrows, "") == 0)
			{
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count == 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, "", _TRUNCATE);
				SecondTabGrid->childgrid->nrows = 16;
				SecondTabGrid->childgrid->height = 16 * MGL_GRID_DEF_ROW_HEIGHT;
			}
			else
			{
				strcpy_s(full, 64, "rows:");
				strcat_s(full, 64, nrows);
				if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 0)
					vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, full, _TRUNCATE);
				if ((rows > 0) && (rows < 16)){
					SecondTabGrid->childgrid->nrows = rows + 1;
					SecondTabGrid->childgrid->height = (rows + 1)* MGL_GRID_DEF_ROW_HEIGHT;
				}
				else
				{
					WarningGrid = CreateWarning(id);
					id->WarningGrid = WarningGrid;
					MGLGridText(id, WarningGrid, 0, 0, "Warning: Cannot show more than 15 rows");
					WarningGrid->parent_w->visible = TRUE;
					SecondTabGrid->childgrid->nrows = 16;
					SecondTabGrid->childgrid->height = 16 * MGL_GRID_DEF_ROW_HEIGHT;
					strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, "", _TRUNCATE);
					strncpy_s(VP(SecondTabGrid->childgrid->RowColumn->vo, 0, "Settings", char), 16, "", _TRUNCATE);
				}
			}
		}
	}
}



void MGLFillAlertParameters(struct InstanceData* id, MGLGRID *ParametersGrid)
{
	VO *vcol;
	int valcolnum;
	char *colname;
	MGLGRID *SourceGrid;
	int line_count;
	char *curstr;

	assert(id);
	assert(ParametersGrid);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* WarningGrid = (MGLGRID*)id->WarningGrid;

	SecondTabGrid->childgrid->iVscrollPos = 0;
	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(ParametersGrid->vo, "Value1");
	valcolnum = vcol->colnum;
	ParametersGrid->gridcells[ParametersGrid->textrow + 1][valcolnum].bg = (GLfloat*)NULL;
	ParametersGrid->gridcells[ParametersGrid->textrow + 1][valcolnum].fg = (GLfloat*)NULL;
	//  put the operator and value results into the filter grid
	// .....
	SourceGrid = ParametersGrid->parentgrid;
	colname = VP(ParametersGrid->vo, ParametersGrid->textrow, "Column", char);


	MGLFillParameters(id, ParametersGrid);
	// reset to the new values and clear out bad ones
	
	MGLFillParametersGrid(SourceGrid->ActionsGrid, SourceGrid->CurActionNum,id);
	curstr = VP(SourceGrid->ActionsGrid->vo, SourceGrid->CurActionNum, "Name", char);
	if (strcmp(curstr, "New") == 0)
	{
		if (SourceGrid->BuildSummaryGrid)//Message is not displayed when we create the grid; Message displayed when we try to use filters without associating it with a criteria name
		{
			
				WarningGrid = CreateWarning(id);
				id->WarningGrid = WarningGrid;
				MGLGridText(id, WarningGrid, 0, 0, "Warning: Give a criteria name");
				WarningGrid->parent_w->visible = TRUE;
			
		}
	}
	//SourceGrid = ActionsGrid->parentgrid;
	line_count = 0;
	if (SecondTabGrid->childgrid->BuildSummaryGrid->vo)
	{
		for (int t = 0; t < SecondTabGrid->childgrid->BuildSummaryGrid->vo->count; t++)
		{
			strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, t, "Parameters", char), 64, "", _TRUNCATE);
			strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, t, "Criteria", char), 64, "", _TRUNCATE);
		}


	}
	line_count = 0;
	InitParametersinBuildSummary(id, SecondTabGrid->childgrid);	
	InitCriteriainBuildSummary(id, SecondTabGrid->childgrid);
	
	CalcColWidths(id, ParametersGrid);
	SecondTabGrid->aircraftcolor_change_MLAT = 1;
	SecondTabGrid->aircraftcolor_change_ASDEX = 1;
	SecondTabGrid->aircraftcolor_change_Noise = 1;
	SecondTabGrid->aircraftcolor_change_Gate = 1;
	SecondTabGrid->aircraftcolor_change_ASD = 1;
	SecondTabGrid->aircraftcolor_change = 1;
	SecondTabGrid->updateroi = 1;
}



//sy
void MGLFillNewSearchGrid(struct InstanceData* id, MGLGRID *SearchGrid)
{
	VO *vcol;
	int valcolnum;
	char *AppName;
	int j = 0;

	

	assert(SearchGrid);

	// the vorow is one less than gridcells since the title row is skipped
	vcol = vcol_find(SearchGrid->vo, SearchGrid->textcolname);
	valcolnum = vcol->colnum;
	// set row colors back to default


	AppName = VP(SearchGrid->vo, 0, "SearchGridApp", char);


	if (strcmp(AppName, "")) {
		FindFlight(id, AppName);

	}

	return;
}


int MGLIsGridKeydown(struct InstanceData* id, WPARAM wParam, int keystate)
{
	//keystate = 1;
	int nProcessed = TRUE;
	int c, len, vorow, valcolnum;
	char *curstr, *textcolname;
	MGLGRID *grid, *SourceGrid;
	VO *vcol;
	widgets *w;
	//extern MGLGRID *DataAppGrid;
	extern MGLGRID *CreateROIGrid(struct InstanceData* id);

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* SearchGrid = (MGLGRID*)id->SearchGrid;

	assert(id);
	
	
	if (!(grid = ActiveTextGrid)){
		nProcessed = FALSE;
		return nProcessed;
	}


	nProcessed = TRUE;

	if (wParam >= 'A' && wParam <= 'Z'){
		if (GetKeyState(VK_SHIFT) < 0) {
			c = wParam;
		}
		else {
			// lower case
			c = wParam + 32;
		}
	}
	else {
		c = -1;
		switch (wParam)
		{
		case VK_OEM_1: //  0xBA ';:' for US
			if (GetKeyState(VK_SHIFT) < 0) {
				c = ':';
			}
			else {
				// lower case
				c = ';';
			}
			break;
		case VK_OEM_PLUS: //  0xBB '+' any country
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '+';
			}
			else {
				// lower case
				c = '=';
			}
			break;
		case VK_OEM_COMMA: //  0xBC ',' any country
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '<';
			}
			else {
				// lower case
				c = ',';
			}
			break;
		case VK_OEM_MINUS: // 0xBD '-' any country
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '_';  // uderline
			}
			else {
				// lower case
				c = '-';
			}
			break;
		case VK_OEM_PERIOD: // 0xBE '.' any country
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '>';
			}
			else {
				// lower case
				c = '.';
			}
			break;
		case VK_OEM_2: //  0xBF  '/?' for US
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '?';
			}
			else {
				// lower case
				c = '/';
			}
			break;
		case VK_OEM_3: //  0xC0 '`~' for US
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '~';
			}
			else {
				// lower case
				c = '`';
			}
			break;
		case VK_OEM_4:	// 0xDB  '[{' for US
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '{';
			}
			else {
				// lower case
				c = '[';
			}
			break;
		case VK_OEM_5:	// 0xDC  '\|' for US
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '|';
			}
			else {
				// lower case
				c = '\\';
			}
			break;
		case VK_OEM_6:	// 0xDD  ']}' for US
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '}';
			}
			else {
				// lower case
				c = ']';
			}
			break;
		case VK_OEM_7:	// 0xDE ''"' for US
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '"';
			}
			else {
				// lower case
				c = '\'';   //   '    escaped with \'
			}
			break;
		case VK_SPACE:
			c = ' ';
			break;
		case VK_PRIOR: // also Page Up
			break;
		case VK_SHIFT:
			//if (strcmp(grid->name, "DataAppCreaterGrid") == 0)
			//	grid->shift_limit++;
			nProcessed = FALSE;
			break;
		case VK_LEFT:
			break;
		case VK_UP:
			break;
		case VK_DOWN:
			break;
		case VK_RIGHT:
			break;
		case VK_DELETE:
			c = VK_BACK;
			break;
		case VK_BACK:
			//	if (strcmp(grid->name, "DataAppCreaterGrid") == 0)
			//	if (grid->character_limit>0)
			//		grid->character_limit--;
			break;
		case VK_CLEAR:
			c = VK_CLEAR;
			break;
		case VK_RETURN:
			c = VK_RETURN;
			// deactivate Text fields
			w = grid->parent_w;
			w->TextActive = FALSE;

			// MGLSetInActiveGridText(id);
			SourceGrid = grid->parentgrid;
			// if we are the ParametersGrid then set the text background for the cell back to default
			if (grid == DataAppCreater)
			{


			}
			else if (SourceGrid && grid == SourceGrid->ParametersGrid){
				MGLFillAlertParameters(id, SourceGrid->ParametersGrid);
			}
			else if (SourceGrid && grid == SourceGrid->ActionsGrid){
				MGLFillNewAction(id, SourceGrid->ActionsGrid);
			}
			else if (grid == DataAppGrid){
				MGLFillNewDataApp(DataAppGrid,id);
			}
			else if (grid == SearchGrid) {
				MGLFillNewSearchGrid(id, SearchGrid);

			}
			else if (grid == SourceGrid->RowColumn)
			{
				MGLFillRowColumn(id, SourceGrid->RowColumn);
				MGLGridMove(id, SecondTabGrid->childgrid, SecondTabGrid->childgrid->gridrect.x1, SecondTabGrid->childgrid->gridrect.y1);
			}
			else if (grid == SourceGrid->AlertsColorGrid->EMailGrid)
			{
				MGLFillEMail(id, SourceGrid->AlertsColorGrid->EMailGrid);			
			}
			else if (grid == SourceGrid->AlertsColorGrid->SubjectGrid)
			{
				MGLFillSubject(id, SourceGrid->AlertsColorGrid->SubjectGrid);
			}
			ActiveTextGrid = NULL;
			return nProcessed;
			break;
		case '0':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = ')';
			}
			else {
				// lower case
				c = '0';
			}
			break;
		case '1':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '!';
			}
			else {
				// lower case
				c = '1';
			}
			break;
		case '2':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '@';
			}
			else {
				// lower case
				c = '2';
			}
			break;
		case '3':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '#';
			}
			else {
				// lower case
				c = '3';
			}
			break;
		case '4':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '$';
			}
			else {
				// lower case
				c = '4';
			}
			break;
		case '5':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '%';
			}
			else {
				// lower case
				c = '5';
			}
			break;
		case '6':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '^';
			}
			else {
				// lower case
				c = '6';
			}
			break;
		case '7':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '&';
			}
			else {
				// lower case
				c = '7';
			}
			break;
		case '8':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '*';
			}
			else {
				// lower case
				c = '8';
			}
			break;
		case '9':
			if (GetKeyState(VK_SHIFT) < 0) {
				c = '(';
			}
			else {
				// lower case
				c = '9';
			}
			break;
		default:
			nProcessed = FALSE;
			break;
		}
	}

	if (nProcessed == TRUE && grid->vo){
		vorow = grid->textrow;
		textcolname = grid->textcolname;
		curstr = VP(grid->vo, vorow, textcolname, char);
		if (vorow >= 0 && vorow < grid->vo->count){
			curstr = VP(grid->vo, vorow, textcolname, char);
			if ((vcol = vcol_find(grid->vo, textcolname))){
				len = strlen(curstr);
				if (strcmp(grid->name, "SubjectGrid") == 0)
				{
					if (len > 32)
						goto next;
				}
					if (len < vcol->size - 1){
					if (c == -1){
						// backspace or ENTER
						if (len > 0){
							curstr[len - 1] = '\0';
						}
					}
					else if (c == VK_RETURN){
						// user has completed input
						valcolnum = vcol->colnum;
					}
					else if (c == VK_CLEAR){
						curstr[0] = '\0';
					}
					else {
						curstr[len] = c;
						curstr[len + 1] = '\0';
					}
				}
			}
		}
	}
next:	return nProcessed;
}



int MGLProcessScrollbarEvent(struct InstanceData* id, widgets *w, int b, int m, int x, int y)
{
	char tmpstr[256];
	VO *vo;

	assert(id);
	assert(w);

	// see if we are in elevator box
	if (InRectangle(x, y, &(w->grid->elev_rect))){
		// in scrollbar
		//printf("MGLProcessScrollbarEvent, x=%d y=%d\n", x, y );
		// if we are in elevator, look for mouse drag 
		if (m == 1){
			/* mouse down event */
			// scrollbar drag events must start inside of the elevator box for scrollbar
			w->grid->nBox = TRUE;
			// assume we are starting a drag
			w->grid->mouse_rect.x1 = x;
			w->grid->mouse_rect.y1 = y;
			_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLProcessScrollbarEvent, MOUSE MOUSE_RECT x=%d y=%d\n", x, y);
			OutputDebugString(tmpstr);

		}
		else if (w->grid->nBox == TRUE){
			/* just ended a drag operation */

			_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLProcessScrollbarEvent, ended a drag operation x=%d y=%d\n", x, y);
			OutputDebugString(tmpstr);
			MoveElevator(w, x, y);
			w->grid->nBox = FALSE;

		}
		else if (w->grid->nBox == FALSE){
			// mouse is up
		}
		//} else if ( w->grid->nBox == TRUE  ){
		// started in elevator, but now outside of elevator
		//printf("MGLProcessScrollbarEvent, ended a drag operation x=%d y=%d\n", x, y );
		// move elevator to this location
		//	MoveElevator( w, x, y );
		//	w->grid->nBox = FALSE;
	}
	else {
		// NOT in elevator
		// are we above or below elevator?

		if (w->grid->vo && w->grid->filteredvo){
			vo = w->grid->filteredvo;
		}
		else if (w->grid->vo){
			vo = w->grid->vo;
		}
		else {
			vo = NULL;
		}

		if (InRectangle(x, y, &(w->grid->scrolltop_rect))){
			// move up one row
			w->grid->iVscrollPos -= 1;
			if (w->grid->iVscrollPos < 0) w->grid->iVscrollPos = 0;
		}
		else if (InRectangle(x, y, &(w->grid->scrollbot_rect))){
			// move down one row
			w->grid->iVscrollPos += 1;

			if (vo){
				if (w->grid->iVscrollPos >= vo->count - w->grid->nrows - 1) w->grid->iVscrollPos = vo->count - (w->grid->nrows - 1);
			}
		}
		else if (y > w->grid->elev_rect.y2){
			// scroll up one page
			printf("MGLProcessScrollbarEvent, scroll up one page x=%d y=%d\n", x, y);
			w->grid->iVscrollPos -= w->grid->nrows - 1;  // subtract one for title row
			if (w->grid->iVscrollPos < 0) w->grid->iVscrollPos = 0;
		}
		else if (y < w->grid->elev_rect.y1){
			// scroll down one page
			printf("MGLProcessScrollbarEvent, scroll DOWN one page x=%d y=%d\n", x, y);
			w->grid->iVscrollPos += w->grid->nrows - 1; // subtract one for title row
			if (vo){
				if (w->grid->iVscrollPos >= vo->count - w->grid->nrows - 1) w->grid->iVscrollPos = vo->count - (w->grid->nrows - 1);
			}
		}

		_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLProcessScrollbarEvent, ended a drag operation x=%d y=%d\n", x, y);
		OutputDebugString(tmpstr);
		w->grid->nBox = FALSE;
	}

	return(TRUE);
}




int MGLHigherZLevel(struct InstanceData* id, int b, int m, int x, int y, widgets *curw)
{

	widgets *w;

	assert(id);

	for (w = curw; w; w = w->np){
		if (!w->visible)
			continue;

		// see where the event is
		if (w->grid) {
			if (w->grid->IsMinimized){
				if (InRectangle(x, y, &(w->grid->title_rect)))
				{
					// matched higher widget
					return(TRUE);
				}
			}
			else {
				// maximized
				if (InRectangle(x, y, &w->grid->gridrect) || InRectangle(x, y, &(w->grid->scrollbar_rect)) ||
					InRectangle(x, y, &(w->grid->title_rect)))
				{
					// matched higher widget
					return(TRUE);
				}
			}
		}
	}

	return(FALSE);

}


void MGLResetRadioWidgets(struct InstanceData* id, widgets *w)
{
	widgets_ll *wll;
	widgets *radiow;

	assert(id);
	assert(w);

	// turn off any other radio buttons that are active besides current w
	for (wll = w->radio_ll; wll; wll = wll->np){
		radiow = wll->w;
		if (radiow != w){
			radiow->IsActive = FALSE;
			radiow->np->IsActive = FALSE;
			radiow->np->np->IsActive = FALSE;
		}
	}

}


int MGLIsGuiMouseEvent(struct InstanceData* id, int b, int m, int x, int y)
{
	panellist *pitem;
	MGLPanel *panel;
	MRECT rect;
	panellist *plist;
	widgets *w, *sibw, *butw;
	int sib;
	char tmpstr[1024];
	int ingrid, stat;

	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;

	MGLGRID* WarningGrid = (MGLGRID*)id->WarningGrid;

	_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent x=%d y=%d\n", x, y);
	OutputDebugString(tmpstr);

	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* SaveGrid = (MGLGRID*)id->SaveGrid;
	
	
	id->SaveGrid = SaveGrid;
	
	if (id->flag_check == 2)
	{
		if (SaveGrid)
			if (SaveGrid->parent_w->visible == 1)
				SaveGrid->parent_w->visible = FALSE;
		if (WarningGrid)
			if (WarningGrid->parent_w->visible == 1)
				WarningGrid->parent_w->visible = FALSE;
	}

	if (DataAppGrid)
	{
		for (int t = 0; t < DataAppGrid->vo->count;t++)
		DataAppGrid->vo->row_index[t].bg = -1;
	}
	if (!oglgui && !G_panellist) return(FALSE);

	if (oglgui){
		for (w = oglgui->widgets; w; w = w->np){
			if (!w->visible)
				continue;

			// see where the event is
			if (w->grid) {
				ingrid = FALSE;

				if ((w->grid->IsMinimized && InRectangle(x, y, &(w->grid->title_rect)))){
					ingrid = TRUE;
				}
				else if (!w->grid->IsMinimized && (InRectangle(x, y, &w->grid->gridrect) || InRectangle(x, y, &(w->grid->scrollbar_rect)) ||
					InRectangle(x, y, &(w->grid->title_rect)))){
					ingrid = TRUE;
				}
				else {
					ingrid = FALSE;
				}

				if (ingrid){
					// in grid
					// see if it matches an overlapping grid or gui layer
					if (MGLHigherZLevel(id, b, m, x, y, w->np)){
						// the mouse location is also over another overlapping widget
						// Z level (draw order) is higher for widget further in w linked list, so skip over this widget
						continue;
					}
					_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent in grid x=%d y=%d m=%d nBox=%d MoveBox=%d\n",
						x, y, m, w->grid->nBox, w->grid->MoveBox);
					OutputDebugString(tmpstr);
					// see if we are in scrollbar
					if (!w->grid->IsMinimized && w->grid->NeedsScrollbar && InRectangle(x, y, &(w->grid->scrollbar_rect))){
						// in scrollbar
						_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent in scrollbar x=%d y=%d m=%d\n", x, y, m);
						OutputDebugString(tmpstr);

						if (m == 1){
							/* mouse down event */
							MGLProcessScrollbarEvent(id, w, b, m, x, y);
							return(TRUE);
						}
					}
					else if (InRectangle(x, y, &(w->grid->title_rect))){
						if (m == 1){
							MGLProcessTitleClick(id, w, b, m, x, y);
							_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent, after TitleClick x=%d y=%d\n", x, y);
							OutputDebugString(tmpstr);
							// leave nBox on since we may still be dragging 
							// w->grid->nBox = FALSE;
						}
						else {
							// mouse up so end drag
							_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent, in TitleClick, mouse up, end drag x=%d y=%d\n", x, y);
							OutputDebugString(tmpstr);
							w->grid->nBox = FALSE;
							w->grid->MoveBox = FALSE;
							return(TRUE);  // still inside grid so return true so map dll will not process mouse event
						}
					}
					else {
						MGLProcessLClick(id, w, b, m, x, y);
						_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent, nBox off after LClick x=%d y=%d\n", x, y);
						OutputDebugString(tmpstr);
						if (w->grid->nBox == TRUE || w->grid->MoveBox == TRUE){
							// ending a drag operation
							_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent, move grid x=%d y=%d\n", x, y);
							OutputDebugString(tmpstr);
							if ((strcmp((w->grid->name), "ActionsGrid") == 0) || (strcmp((w->grid->name), "ParametersGrid") == 0) || (strcmp((w->grid->name), "AlertsColorGrid") == 0) || (strcmp((w->grid->name), "AvailableGrid") == 0) || (strcmp((w->grid->name), "RowColumn") == 0) || (strcmp((w->grid->name), "DataAppSelectorGrid") == 0) || (strcmp((w->grid->name), "SecondTabGrid") == 0) || (strcmp((w->grid->name), "BuildSummaryGrid")))
							{
							}
							else if (strcmp((w->grid->name), "DataAppSelectorGrid") == 0)
							{
							
								MGLGridMove(id, DataAppGrid, w->grid->drag_start_rect.x1 + (x - w->grid->mouse_rect.x1), w->grid->drag_start_rect.y1 - (DataAppGrid->nrows * DataAppGrid->gridrows[0].height) + (y - w->grid->mouse_rect.y1));								
								MGLGridMove(id, DataAppCreater, w->grid->drag_start_rect.x1 + (x - w->grid->mouse_rect.x1), w->grid->drag_start_rect.y1 - ((DataAppGrid->nrows + 1) * DataAppGrid->gridrows[0].height) + (y - w->grid->mouse_rect.y1) - (2 * DataAppGrid->gridrows[0].height));
								if (SecondTabGrid)
								{
									MGLGridMove(id, SecondTabGrid, DataAppGrid->gridrect.x2 - 1, w->grid->drag_start_rect.y1 + (y - w->grid->mouse_rect.y1) - ((SecondTabGrid->nrows - 1) * DataAppGrid->gridrows[0].height) + ((DataAppGrid->nrows - 1) * DataAppGrid->gridrows[0].height));
									if (SecondTabGrid->childgrid)
									{
										if (SecondTabGrid->childgrid->BuildSummaryGrid)
										{
											MGLGridMove(id, SecondTabGrid->childgrid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (SecondTabGrid->childgrid->BuildSummaryGrid->nrows * DataAppGrid->gridrows[0].height) - DataAppGrid->gridrows[0].height);
										}
									}
								}
								if (SecondTabGrid->childgrid)
								{
									if (SecondTabGrid->childgrid->ActionsGrid)
									{
										if (SecondTabGrid->childgrid)
										{
											if (SecondTabGrid->childgrid->ActionsGrid)
											{
												MGLGridMove(id, SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
											}
											if (SecondTabGrid->childgrid->ParametersGrid)
											{
												MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
											}
											if (SecondTabGrid->childgrid->AlertsColorGrid)
											{
												MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
											}
											if (SecondTabGrid->childgrid->SortAvailableGrid)
											{
												MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
											}
											if (SecondTabGrid->childgrid->RowColumn)
											{
												MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));
											}
										}
									}
									if (SecondTabGrid->childgrid->ParametersGrid)
									{
										MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
									}
									if (SecondTabGrid->childgrid->AlertsColorGrid)
									{
										MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
										if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
											MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
										if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
											MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
										if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
											MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
										if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
											MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
										if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
											MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
									}
									if (SecondTabGrid->childgrid->SortAvailableGrid)
									{
										MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
									}
									if (SecondTabGrid->childgrid->RowColumn)
									{
										MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));
									}
								}
							}
							/*else if (strcmp((w->grid->name), "DataAppCreaterGrid") == 0)
							{
							MGLGridMove(id, DataAppCreater, w->grid->drag_start_rect.x1 + (x - w->grid->mouse_rect.x1), w->grid->drag_start_rect.y1 + (y - w->grid->mouse_rect.y1));
							}*/

							else

								MGLGridMove(id, w->grid, w->grid->drag_start_rect.x1 + (x - w->grid->mouse_rect.x1), w->grid->drag_start_rect.y1 + (y - w->grid->mouse_rect.y1));
						}
						w->grid->nBox = FALSE;
						w->grid->MoveBox = FALSE;
					}
					return(TRUE);  // still inside grid so return true so map dll will not process mouse event
				} // InRectangle
				//  allow grid to move if user is dragging title bar outside of grid  w->grid->nBox = FALSE;

				if (m == 1){
				}
				else {
					// mouse is up, end drag 
					_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent, mouse is up, end drag x=%d y=%d\n", x, y);
					OutputDebugString(tmpstr);
					w->grid->nBox = FALSE;
					w->grid->MoveBox = FALSE;
				}
			}  // end w->grid
			// see if this is a button item
			if (m == 1 && (w->type == WGT_BUTTON || w->type == WGT_TEXT_BUTTON)){
				stat = FALSE;
				if (InRectangle(x, y, &w->gregion)){
					// toggle button on or off
					if (w->IsActive){
						w->IsActive = FALSE;
						stat = TRUE;  // item found, so return TRUE unless callback disables
						if (w->nsiblings){
							//  if there are siblings, turn all siblings off
							for (sib = 0; sib < w->nsiblings; sib++){
								sibw = w->siblings[sib];
								sibw->IsActive = FALSE;
								// inactivate callback
								if (sibw->OnClickCB){
									stat = (sibw->OnClickCB)(id, sibw, x, y);
								}
							}
						}
						else {
							// activate callback
							if (w->OnClickCB){
								stat = (w->OnClickCB)(id, w, x, y);
							}
						}
					}
					else {
						w->IsActive = TRUE;
						stat = TRUE;  // item found, so return TRUE unless callback disables
						if (w->nsiblings){
							//  if there are siblings, turn all siblings off
							for (sib = 0; sib < w->nsiblings; sib++){
								sibw = w->siblings[sib];
								sibw->IsActive = TRUE;
								// activate callback
								if (sibw->OnClickCB){
									stat = (sibw->OnClickCB)(id, sibw, x, y);
								}
							}
							butw = w->siblings[0];  // use second widget in group since this has the links to the other radio buttons
							if (butw->radio_ll){
								// this radio but is On, so we must turn off any other radio buttons in this group
								MGLResetRadioWidgets(id, butw);
							}
						}
						else {
							// activate callback
							if (w->OnClickCB){
								stat = (w->OnClickCB)(id, w, x, y);
							}
						}
					}
					if (stat == TRUE){
						return(TRUE);
					}
					else {
						// go to next item
					}
				}
			}  // end WGT_BUTTON

			if (w->type == WGT_PANEL){
				if (InRectangle(x, y, &w->gregion)) {
					stat = TRUE;  // now, do not return since children need to be checked
					// in the future , panels should be checked last if selected, then return true
					//  maybe another loop at the end to check all panels ???
				}
			}


			if (w->type == WGT_SLIDER && m == 1) {
				_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent, check if in '%s' WGT_SLIDER mouse down m=%d x=%d y=%d\n",
					w->name, m, x, y);
				OutputDebugString(tmpstr);
				if (InRectangle(x, y, &w->gregion)) {
					if (m == 1){
						_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent, WGT_SLIDER mouse down x=%d y=%d\n", x, y);
						OutputDebugString(tmpstr);
						// activate callback
						if (w->OnClickCB){
							stat = (w->OnClickCB)(id, w, x, y);
						}
					}
					else {
						// mouse up so end drag
						_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "MGLIsGuiMouseEvent,  WGT_SLIDER mouse up, end drag x=%d y=%d\n", x, y);
						OutputDebugString(tmpstr);
						// activate callback
						if (w->OnClickCB){
							stat = (w->OnClickCB)(id, w, x, y);
						}
					}
				}
				if (stat == TRUE){
					return(stat);
				}
				else {
					// try next item
				}
			}

		}
	}

	// Check other panels
	/* see if the value is in range of any gui panels */
	for (pitem = G_panellist; pitem; pitem = pitem->np){

		if (pitem->itemtype != MGL_PANEL){
			/* error */
			break;
		}

		panel = (MGLPanel *)pitem->panelitem;
		/* check individual panel items first before testing entire panel */
		for (plist = panel->plist; plist; plist = plist->np){
			if (MGLTestPanelItem(panel, plist, b, m, x, y)){
				return(TRUE);
			}
		}

		rect.x1 = panel->x;
		rect.y1 = panel->y;
		rect.x2 = rect.x1 + panel->width;
		rect.y2 = rect.y1 + panel->height;

		if (InRectangle(x, y, &rect)){
			// printf("MGLIsGuiEvent, INRANGE of panel, x=%d y=%d\n", x, y );

			return(TRUE);
		}

	}

	id->m_poglgui = oglgui;
	
	id->DataAppGrid = DataAppGrid;
	id->DataAppCreater = DataAppCreater;

	return(FALSE);
}



void SetAlertAction(struct InstanceData* id, MGLGRID *parentgrid, int action, int vorow)
{
	char KeyStr[128];
	int keycol, i, histrow, f;
	char *fieldname, *parentvalstr, *histvalstr, *colname, *valstr, *op, *cmp_value, *DataSource, *AlertName;
	char alerttext[128];
	VO *vcol, *searchvo, *DataAppVO;
	struct row_index *row_index;
	char tmpbuf[TMPBUF_SIZE];
	MGLGRID *grid;
	void *src_datap, *des_datap;

	assert(id);
	assert(parentgrid);

	// check the master list of Alerts in HistAlertVO to see if we should add this to the alerts
	//   we have to make sure we don't keep adding the same alert


	if (!HistAlertsVO) return;

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	
	// add the new alert to the VO unless it already exists
	// must have KeyFields set for parent grid, otherwise do not add any rows for this object to avoid overflow
	if (!strlen(parentgrid->KeyFields[0])) return;

	// see if this alert is already in HistAlertsVO, and if so, then update, don't add a row
	// get unique value which is concatenation of values from all keyfields
	for (keycol = 0, KeyStr[0] = '\0'; keycol < MGL_GRID_MAX_COLS && strlen(parentgrid->KeyFields[keycol]); keycol++){
		fieldname = parentgrid->KeyFields[keycol];
		if ((vcol = vcol_find(parentgrid->vo, fieldname))){
			parentvalstr = get_vcol_valstr(vcol, V_ROW_PTR(parentgrid->vo, vorow), TRUE);
			// sprintf_s(tmpbuf, sizeof(tmpbuf), "%s", fieldname, parentvalstr );
			if (strlen(KeyStr)) {
				// put in separator
				strcat_s(KeyStr, sizeof(KeyStr), ", ");
			}
			strcat_s(KeyStr, sizeof(KeyStr), parentvalstr);
		}
		else {
			// bad column name
			return;
		}
	}

	for (i = 0, histrow = -1; i < HistAlertsVO->count; i++){
		histvalstr = VP(HistAlertsVO, i, "KeyStr", char);
		if (!strcmp(histvalstr, KeyStr)){
			// already matches
			histrow = i;
			break;
		}
	}

	// this row matched the filter
	/* text for alert is comprised of
	Action Name plus  1-n (Filter field value, ie. FltNum: JBU1022, Origin: KJFK, OuttoOff: 30:01)
	*/
	// go through each filter for this action to get Fieldname Value for string
	alerttext[0] = '\0';
	for (f = 0; f < parentgrid->Actions[action].gridconfig->nFilters; f++){
		// find column in vo that is being filtered
		if (!strlen((colname = parentgrid->Actions[action].gridconfig->filters[f].field_name))) continue;
		if (parentgrid->Actions[action].gridconfig->filters[f].filter_type == FTYPE_LIST){
			// filter type is a list of values
			if (!(vcol = vcol_find(parentgrid->vo, colname))) continue;
			valstr = get_vcol_valstr(vcol, V_ROW_PTR(parentgrid->vo, vorow), TRUE);
			if (!(searchvo = parentgrid->Actions[action].gridconfig->filters[f].searchvo)){
				parentgrid->Actions[action].gridconfig->filters[f].searchvo =
					MGLCrSearchVO((char *)&parentgrid->Actions[action].gridconfig->filters[f].select_value);
				parentgrid->Actions[action].gridconfig->filters[f].filter_type = FTYPE_LIST;
			}
			if ((searchvo = parentgrid->Actions[action].gridconfig->filters[f].searchvo)){ // vo list of values
				if ((row_index = vo_search(searchvo, "listval", valstr, NULL))){
					// match found 
					sprintf_s(tmpbuf, TMPBUF_SIZE, "%s: %s", colname, valstr);
					strcat_s(alerttext, sizeof(alerttext), tmpbuf);
					continue;  // next filter
				}
				else {
					// should not get here since this row was cleared before
					return;
				}
			}
		}
		else if (parentgrid->Actions[action].gridconfig->filters[f].filter_type == FTYPE_OPERATORS){
			op = parentgrid->Actions[action].gridconfig->filters[f].op;  // op is 2 letter code, ala NE EQ LE
			cmp_value = parentgrid->Actions[action].gridconfig->filters[f].cmp_value;
			// see if the current row i matches this filter parameter
			if (!(vcol = vcol_find(parentgrid->vo, colname))) continue;
			valstr = get_vcol_valstr(vcol, V_ROW_PTR(parentgrid->vo, vorow), TRUE);

			// if cmp_value is a column name, the substitute the value from the vo for that column
			//    for example if cmp_value = Destin, then get that value from this row
			if ((vcol = vcol_find(parentgrid->vo, cmp_value))){
				// substitute for value from this column
				cmp_value = get_vcol_valstr(vcol, V_ROW_PTR(parentgrid->vo, i), TRUE);
			}


			if (MGLMatch_all(parentgrid, action, op, f, cmp_value, valstr, colname) == FALSE){
				// all filters must match or the row fails  ('OR' can be accomplished with multiple action filters)
				// should not get here since this row was cleared before
				return;
			}
			if (strlen(alerttext)){
				sprintf_s(tmpbuf, TMPBUF_SIZE, ": %s", colname, valstr);
			}
			else {
				sprintf_s(tmpbuf, TMPBUF_SIZE, "%s", valstr);
			}
			strcat_s(alerttext, sizeof(alerttext), tmpbuf);
		}
	}  // end each filter



	if (!parentgrid->ActionsGrid || !parentgrid->ActionsGrid->vo){
		// we need to create the actions grid here???
		if (!parentgrid->ActionsGrid){
			CreateActionsGrid(id, parentgrid);
			FillActionsGrid(id, parentgrid, FALSE);
		}
		if (!parentgrid->ActionsGrid || !parentgrid->ActionsGrid->vo){
			return;
		}
	}

	AlertName = VP(parentgrid->ActionsGrid->vo, action, "Action", char);


	if (histrow == -1){
		// add new row
		vo_alloc_rows(HistAlertsVO, 1);
		histrow = HistAlertsVO->count - 1;

		// find out which row in VO we want, see CrDataAppVO for definitiono of DataAppVO
		DataAppVO = DataAppGrid->vo;
		for (i = 0; i < DataAppVO->count; i++){
			grid = VV(DataAppGrid->vo, i, "gridptr", void *);
			if (grid == parentgrid){
				// found match
				DataSource = VP(DataAppVO, i, "DataSource", char);
				break;
			}
		}


		// id col no longer exists VV( HistAlertsVO, histrow, "ID", int ) = GAlertsID++;
		GAlertsID++;
		vcol = vcol_find(HistAlertsVO, "Alert");
		strncpy_s(VP(HistAlertsVO, histrow, "Alert", char), vcol->size, AlertName, _TRUNCATE);

		vcol = vcol_find(HistAlertsVO, "Trigger");
		strncpy_s(VP(HistAlertsVO, histrow, "Trigger", char), vcol->size, alerttext, _TRUNCATE);
		VV(HistAlertsVO, histrow, "Time", int) = time(0);
		VV(HistAlertsVO, histrow, "Ack", int) = 0;
		VV(HistAlertsVO, histrow, "Del", int) = 0;
		vcol = vcol_find(HistAlertsVO, "KeyStr");
		strncpy_s(VP(HistAlertsVO, histrow, "KeyStr", char), vcol->size, KeyStr, _TRUNCATE);

		VV(HistAlertsVO, histrow, "IsVisible", int) = 1;



		if ((vcol = vcol_find(HistAlertsVO, "DataSource"))){
			strncpy_s(VP(HistAlertsVO, histrow, "DataSource", char), vcol->size, DataSource, _TRUNCATE);
		}

	}

	// update the Main Alert Grid with Active Alerts
	// free the old rows
	vo_rm_rows(AlertsGrid->vo, 0, AlertsGrid->vo->count);
	for (i = 0; i < HistAlertsVO->count; i++){
		// if row is active , copy to AlertsGrid->vo
		if (VV(HistAlertsVO, i, "IsVisible", int)){

			vo_alloc_rows(AlertsGrid->vo, 1);
			des_datap = AlertsGrid->vo->row_index[AlertsGrid->vo->count - 1].datap;
			src_datap = HistAlertsVO->row_index[i].datap;
			// int vo_copy_all( VO *des_vo, VO *src_vo, void *des_datap, void *src_datap )
			vo_copy_all(AlertsGrid->vo, HistAlertsVO, des_datap, src_datap);

			// if Ack is set then color it grey
			if (VV(HistAlertsVO, i, "Ack", int)){
				AlertsGrid->vo->row_index[AlertsGrid->vo->count - 1].fg = MGLHexColortoI("868686");
			}
		}
	}
	CalcColWidths(id, AlertsGrid);
	id->DataAppGrid = DataAppGrid;
}


void CrParametersVO(MGLGRID *topgrid)
{

	VO *ParametersVO;

	assert(topgrid);

	ParametersVO = (VO *)vo_create(0, NULL);
	vo_set(ParametersVO, V_NAME, "ParametersVO", NULL);
	VOPropAdd(ParametersVO, "Column", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(ParametersVO, "Operator", NTBSTRINGBIND, 32, VO_NO_ROW);


	VOPropAdd(ParametersVO, "List", INTBIND, -1, VO_NO_ROW);  // clicking in this column spawns select list of possible values from main grid

	VOPropAdd(ParametersVO, "Value1", NTBSTRINGBIND, 256, VO_NO_ROW);
	VOPropAdd(ParametersVO, "Value2", NTBSTRINGBIND, 256, VO_NO_ROW);

	vo_set(ParametersVO, V_ORDER_COLS, "Column", NULL, NULL);

	topgrid->ParametersGrid->vo = ParametersVO;


}



void MGLFillParametersColsVO(MGLGRID *parentgrid)
{
	int i;
	VO *vo, *vcol;
	

	if (!parentgrid) return;

	vo = parentgrid->vo;

	// fill a vo with all the unique values in this columns for all rows
	if (!parentgrid->ParametersGrid->vo){
		CrParametersVO(parentgrid);
	}
	MGLSetlabelNamesFromVO(parentgrid);
	vo_rm_rows(parentgrid->ParametersGrid->vo, 0, parentgrid->ParametersGrid->vo->count);
	if (parentgrid->vo){

		for (i = 0; i < parentgrid->vo->vcols->count; i++){
			if(parentgrid->roi_fdo == 1)
			{
				vcol = V_ROW_PTR(vo->vcols, i);
				if (strcmp(vcol->label, "") != 0)
				{
					vo_alloc_rows(parentgrid->ParametersGrid->vo, 1);
					if (i == 9)
						strncpy_s(VP(parentgrid->ParametersGrid->vo, parentgrid->ParametersGrid->vo->count - 1, "Column", char), 64, "A/D", _TRUNCATE);
					else if (i == 8)
						strncpy_s(VP(parentgrid->ParametersGrid->vo, parentgrid->ParametersGrid->vo->count - 1, "Column", char), 64, "In", _TRUNCATE);
					else
						strncpy_s(VP(parentgrid->ParametersGrid->vo, parentgrid->ParametersGrid->vo->count - 1, "Column", char), 64, vcol->label, _TRUNCATE);
				}
			}
			else if (parentgrid->roi_fdo == 2)
			{
				if (strlen(parentgrid->ColOrderNames[i])){
					vcol = V_ROW_PTR(vo->vcols, i);
					vo_alloc_rows(parentgrid->ParametersGrid->vo, 1);
					strncpy_s(VP(parentgrid->ParametersGrid->vo, parentgrid->ParametersGrid->vo->count - 1, "Column", char), 64, vcol->label, _TRUNCATE);
				}
			}
			else if (parentgrid->roi_fdo == 3)
			{
				if (strlen(parentgrid->ColOrderNames[i])){
					vcol = V_ROW_PTR(vo->vcols, i);
					vo_alloc_rows(parentgrid->ParametersGrid->vo, 1);
					strncpy_s(VP(parentgrid->ParametersGrid->vo, parentgrid->ParametersGrid->vo->count - 1, "Column", char), 64, vcol->label, _TRUNCATE);
				}
			}
		}

		/*for (i = 0; i < parentgrid->vo->vcols->count; i++){
		if (strlen(parentgrid->ColOrderNames[i])){
		colname = parentgrid->ColOrderNames[i];
		vo_alloc_rows(parentgrid->ParametersGrid->vo, 1);
		strncpy_s(VP(parentgrid->ParametersGrid->vo, parentgrid->ParametersGrid->vo->count - 1, "Column", char), 64, colname, _TRUNCATE);
		}
		}*/
	}
	else {
		// use values stored in grid??? not really needed for non-vo grids ??? may have to implement later
		return;
	}

}

void check_highlightedfg(struct InstanceData* id, MGLGRID *ROIGrid, int vorow)
{
	char *FltNum;
	int count1 = 0;
	int count2 = 0;
	int co = 0;

	if (ROIGrid->roi_fdo == 1)
	{
		//  ROIGrid
		if (vorow >= 0){
			//To obtain the flightnumber and the region name of the ROIGrids
			// FltNum = VP(ROIGrid->filteredvo, ROIGrid->filteredvo->row_index[vorow].rownum, "FltNum", char);
			FltNum = VP(ROIGrid->filteredvo, ROIGrid->filteredvo->row_index[vorow].rownum, "flightid", char);

			if (strcmp(ROIGrid->Flightnum, FltNum) == 0)
			{
				ROIGrid->filteredvo->row_index[ROIGrid->filteredvo->count - 1].fg = MGLColorHex2Int("529EFD");
			}
#ifdef OLDWAY
			Region = VP(ROIGrid->filteredvo, ROIGrid->filteredvo->row_index[vorow].rownum, "Region", char);


			//Isolating the Flightnumber
			while (FltNum[co] != '\0'){
				count1++;
				co++;
			}

			co = 0;

			while (Region[co] != '\0'){
				if (Region[co] == '-'){
					count2--;
				}
				if (Region[co] == ' ')
				{
					count2--;
				}

				count2++;
				co++;
			}


			Flightnum = (char *)malloc(count1 - count2 + 2);

			for (i = 0; i < (count1 - count2 + 1); i++)
				Flightnum[i] = 0;

			for (i = 0; i < (count1 - count2); i++)
			{
				Flightnum[i] = FltNum[i];

			}

			Flightnum[count1 - count2 + 1] = '\0';

			//Color the selected flightnumber
			if (strcmp(ROIGrid->Flightnum, Flightnum) == 0)
			{
				ROIGrid->filteredvo->row_index[ROIGrid->filteredvo->count - 1].fg = MGLColorHex2Int("529EFD");
			}
			free(Flightnum);
#endif
		}
	}
	else if (ROIGrid->roi_fdo == 2)
	{
		// FDO
		if (vorow >= 0){
			//To obtain the flightnumber and the region name of the ROIGrids
			FltNum = VP(ROIGrid->filteredvo, ROIGrid->filteredvo->row_index[vorow].rownum, "FltNum", char);
			//Color the selected flightnumber
			if ((strcmp(ROIGrid->Flightnum, "") != 0))
			{
				if (strcmp(ROIGrid->Flightnum, FltNum) == 0)
				{
					ROIGrid->filteredvo->row_index[ROIGrid->filteredvo->count - 1].fg = MGLColorHex2Int("529EFD");
				}
			}
		}
	}
}



void CrFilteredGrid(struct InstanceData* id, MGLGRID *parentgrid)
{
	int i, action, red_ival, green_ival, blue_ival, fg_ival, bg_ival;
	int match;
	void *src_datap, *des_datap;
	char *colorstr;
	VO *fvcol, *pvcol;
	extern int vo_copy(VO *des_vo, ...);
	extern int vo_copy_all(VO *des_vo, VO *src_vo, void *des_datap, void *src_datap);
	int check = 0;


	assert(id);


	// use the grid->GridConfig values to filter the source data into a filtered subset of rows
	// use GridConfig, or vo objects to store filter parameters
	if (!parentgrid || !parentgrid->vo || !parentgrid->Actions[parentgrid->CurActionNum].gridconfig){

		// can't filter
		return;
	}
	/*
	for each row in source vo
	for each column that has a filter
	// following line uses hash or binary search of list of acceptable values
	does value in source vo match all of the accepted filter values?   If not, then reject row
	end each filter column
	end each row loop
	*/
	// for each row in vo, set flag if row should be displayed in RowIsFiltered array

	if (parentgrid->filteredvo && parentgrid->filteredvo->count){
		// remove the rows, but keep the structure
		vo_rm_rows(parentgrid->filteredvo, 0, parentgrid->filteredvo->count);
	}

	if (!parentgrid->filteredvo){
		parentgrid->filteredvo = vo_dupnew(parentgrid->vo);
		// add in labels
		for (i = 0; i < parentgrid->vo->vcols->count; i++){
			pvcol = V_ROW_PTR(parentgrid->vo->vcols, i);
			fvcol = V_ROW_PTR(parentgrid->filteredvo->vcols, i);
			strcpy_s(fvcol->label, V_NAME_SIZE, pvcol->label);
		}

		if (parentgrid->filteredvo && parentgrid->filteredvo->count){
			vo_rm_rows(parentgrid->filteredvo, 0, parentgrid->filteredvo->count);
		}
	}
	if (parentgrid->Flut){
		free(parentgrid->Flut);
		parentgrid->Flut = NULL;
	}
	for (i = 0; i < parentgrid->vo->count; i++){
		match = TRUE; // each row is included unless if fails a filter
		//parentgrid->active_icon = 1;
		for (action = 0; action < parentgrid->nActions; action++){
			if (!parentgrid->Actions[action].IsActive) continue; // not active
			if (!(parentgrid->Actions[action].ActionType & ACTION_FILTER)) continue; // do not filter out rows for this action
			if (MGLFiltersMatch(id,parentgrid, action, i)){
			}
			else {
				match = FALSE;
			}

		} // end each action
		//  if any filter in any action failed, then do not copy the row into filteredvo
		if (match == TRUE){
			// this row passed filter, copy to the filtered VO
			vo_alloc_rows(parentgrid->filteredvo, 1);
			des_datap = parentgrid->filteredvo->row_index[parentgrid->filteredvo->count - 1].datap;
			src_datap = parentgrid->vo->row_index[i].datap;
			// int vo_copy_all( VO *des_vo, VO *src_vo, void *des_datap, void *src_datap )
			vo_copy_all(parentgrid->filteredvo, parentgrid->vo, des_datap, src_datap);
			// the row was added to the filteredvo, check for matches with other options, like color
			// clear out row_index values for new row since row_index is re-used
			//		FltNum = VP(parentgrid->filteredvo, parentgrid->filteredvo->row_index[parentgrid->filteredvo->count - 1].rownum, "FltNum", char);
			parentgrid->filteredvo->row_index[parentgrid->filteredvo->count - 1].fg = -1;
			parentgrid->filteredvo->row_index[parentgrid->filteredvo->count - 1].bg = -1;
			for (action = 0; action < parentgrid->nActions; action++){
				if (!parentgrid->Actions[action].IsActive) continue; // not active
				if ((parentgrid->Actions[action].ActionType & ACTION_COLOR_FG)){
					// color action found
					if (MGLFiltersMatch(id,parentgrid, action, i)){
						// color the row by setting fg color in vo row_index struct

						colorstr = parentgrid->Actions[action].row_fg; // Foreground FG
						if (strlen(colorstr)){
							// fg is set, format char row_fg[7];  // in rgb hex  RRGGBB, FF00FE 
							// 	pColor[0] = (GLfloat) HexStrtod( &str[1], 2 ) / 255.0f;
							red_ival = HexStrtod(&colorstr[0], 2);
							green_ival = HexStrtod(&colorstr[2], 2);
							blue_ival = HexStrtod(&colorstr[4], 2);

							fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;

							parentgrid->filteredvo->row_index[parentgrid->filteredvo->count - 1].fg = fg_ival;
								
						

						}
						check_highlightedfg(id, parentgrid, parentgrid->filteredvo->count - 1);//To highlight selected row or selected icon or just the filter fg and bg colors
					}
				}
				if ((parentgrid->Actions[action].ActionType & ACTION_COLOR_BG)){
					if (MGLFiltersMatch(id,parentgrid, action, i)){
						// color the row by setting fg color in vo row_index struct
						colorstr = parentgrid->Actions[action].row_bg;  // Background BG
						if (strlen(colorstr)){
							// bg is set, format char row_fg[7];  // in rgb hex  RRGGBB, FF00FE 
							// 	pColor[0] = (GLfloat) HexStrtod( &str[1], 2 ) / 255.0f;
							red_ival = HexStrtod(&colorstr[0], 2);
							green_ival = HexStrtod(&colorstr[2], 2);
							blue_ival = HexStrtod(&colorstr[4], 2);
							bg_ival = 0;
							bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
							parentgrid->filteredvo->row_index[parentgrid->filteredvo->count - 1].bg = bg_ival;
							
								check_highlightedfg(id, parentgrid, parentgrid->filteredvo->count - 1);//To highlight selected row or selected icon or just the filter fg and bg colors
						}
					}
				}
				if (!((parentgrid->Actions[action].ActionType & ACTION_COLOR_BG)) && !((parentgrid->Actions[action].ActionType & ACTION_COLOR_FG)))
				{
					
						check_highlightedfg(id, parentgrid, parentgrid->filteredvo->count - 1);
				}
				// check for Alerts to be added to Alerts List Grid
				if ((parentgrid->Actions[action].ActionType & ACTION_ALERT_LIST)){
					if (MGLFiltersMatch(id,parentgrid, action, i)){
						SetAlertAction(id, parentgrid, action, i);  // action only set once per matching items
					}
				}


			} // end each action
		} // end match== TRUE if
		else if (match == FALSE)
		{
			
				check_highlightedfg(id, parentgrid, parentgrid->filteredvo->count - 1);//To highlight selected row or selected icon or just the filter fg and bg colors
		}

	} // end each i in parentvo (unfiltered)

	check = 20;
}




int UpdateTarmacGrid(struct InstanceData* id, MGLGRID *grid)
{

	assert(id);

	CrFilteredGrid(id, grid);
	MGLSortFilteredGrid(grid);
	CalcColWidths(id, grid);
	return(TRUE);
}


char *CnvSecs2HHMMSS_GMT(char *valstr)
{
	static char timebuf[64];
	time_t secs;
	char *str;

	assert(valstr);

	secs = atol(valstr);

	if (secs > 0){
		str = VOGMTime(timebuf, secs, "%H:%M:%S");
	}
	else {
		return("");
	}

	return(timebuf);


#ifdef USE_USER_TIME
	if (id->m_sUserSettings.TimeDisplayLocal)
	{
		VOTimeFmt(stime, startsecs, "%H:%M:%S");
		if (endsecs){
			VOTimeFmt(etime, endsecs, "%H:%M:%S");
		}
		else {
			etime[0] = '\0';
		}
	}
	else{
		VOGMTime(stime, startsecs, "%H:%M:%S");
		if (endsecs){
			VOGMTime(etime, endsecs, "%H:%M:%S");
		}
		else {
			etime[0] = '\0';
		}
	}
#endif


}

char *CnvSecs2HHMMSS(struct InstanceData* id, char *valstr)
{
	static char timebuf[64];
	time_t secs;
	char *str;

	assert(valstr);

	secs = atol(valstr);

	if (secs > 0){
		if(id->m_sUserSettings.TimeDisplayLocal == 1)
			str = VOTimeFmt(timebuf, secs, "%H:%M:%S");
		else if (id->m_sUserSettings.TimeDisplayLocal == 0)
			str = VOGMTime(timebuf, secs, "%H:%M:%S");
	}
	else {
		return("");
	}

	return(timebuf);


#ifdef USE_USER_TIME
	if (id->m_sUserSettings.TimeDisplayLocal)
	{
		VOTimeFmt(stime, startsecs, "%H:%M:%S");
		if (endsecs){
			VOTimeFmt(etime, endsecs, "%H:%M:%S");
		}
		else {
			etime[0] = '\0';
		}
	}
	else{
		VOGMTime(stime, startsecs, "%H:%M:%S");
		if (endsecs){
			VOGMTime(etime, endsecs, "%H:%M:%S");
		}
		else {
			etime[0] = '\0';
		}
	}
#endif


}




MGLGRID *CreateTarmacGrid(struct InstanceData* id)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;
	MGLGRID *TarmacGrid;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "TarmacGridWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of tarmac delays
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "TarmacWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = id->m_nHeight - 300;
	w->gregion.x2 = 400;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "TarmacGrid", 6, 10, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = TRUE;
	w->grid->edit = TRUE;//to display the rules-layout icon
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	TarmacGrid = w->grid;
	TarmacGrid->UpdateCB = UpdateTarmacGrid;


	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)yellow;
			w->grid->gridrows[r + 1].fg = (GLfloat*)colorfc4740;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)colorfc4740;
			w->grid->gridrows[r + 1].fg = (GLfloat*)colorfc4740;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	/*
	VOPropAdd( TDVO, "FltNum", NTBSTRINGBIND, FLTNUM_SIZE, VO_NO_ROW );
	VOPropAdd( TDVO, "TailNum", NTBSTRINGBIND, N_TAIL_SIZE, VO_NO_ROW );
	VOPropAdd( TDVO, "Orig", NTBSTRINGBIND, ORIG_DES_SIZE, VO_NO_ROW );
	VOPropAdd( TDVO, "Dest", NTBSTRINGBIND, ORIG_DES_SIZE, VO_NO_ROW );
	VOPropAdd( TDVO, "arrdep", NTBSTRINGBIND, 2, VO_NO_ROW );
	VOPropAdd( TDVO, "ATA", NTBSTRINGBIND, 6, VO_NO_ROW );
	VOPropAdd( TDVO, "STA", NTBSTRINGBIND, 6, VO_NO_ROW );
	VOPropAdd( TDVO, "OUT", NTBSTRINGBIND, 6, VO_NO_ROW );
	VOPropAdd( TDVO, "STD", NTBSTRINGBIND, 6, VO_NO_ROW );
	VOPropAdd( TDVO, "Delay", INTBIND, -1, VO_NO_ROW );
	*/



	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Flight", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "TailNum", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Orig", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Dest", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "A/D", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "ATA", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, "STA", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[7], MGL_GRID_LABEL_MAX, "OUT", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[8], MGL_GRID_LABEL_MAX, "STD", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[9], MGL_GRID_LABEL_MAX, "Delay", _TRUNCATE);


	w->grid->gridcols[5].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[6].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[7].val_callback = CnvSecs2HHMMSS;

	w->grid->gridcols[9].val_callback = CnvElapsedSecs2HHMMSS;  // Elapsed delay


	// allow user to configure layout
	w->grid->gridrows[0].row_callback = MGLHeadersCallback;

	// we also need to fetch initial filter values from user settings DB
	// set row callbacks
	for (i = 0; i < TarmacGrid->nrows; i++){
		//		TarmacGrid->gridrows[i].row_callback = TarmacCallback;
	}


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	TarmacGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = TarmacGrid->marginx;
	w->gregion.y1 = TarmacGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - TarmacGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Tarmac Tracker", _TRUNCATE);
	w->visible = 1;
	w->grid = TarmacGrid;

	id->m_poglgui = oglgui;

	return(TarmacGrid);
}




int AlertsCallback(struct InstanceData* id, MGLGRID *AlertsGrid, int b, int m, int x, int y)
{
	// turn off or erase alerts depending upon user action
	int r, vorow, c, i;
	VO *vcol;
	char *KeyStr, *histvalstr;

	assert(id);

	if (!AlertsGrid) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (AlertsGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - AlertsGrid->gridrect.y1) / AlertsGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - AlertsGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= AlertsGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = AlertsGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + AlertsGrid->iVscrollPos;

	// see which column was selected
	c = MGLCalcCol(AlertsGrid, x);

	// see if we selected the del column
	if ((vcol = vcol_find(AlertsGrid->vo, "Del"))){
		if (c == vcol->colnum){
			// user wants to delete this row 
			// we have to delete it from the master HistAlertsVO
			KeyStr = VP(AlertsGrid->vo, vorow, "KeyStr", char);
			for (i = 0; i < HistAlertsVO->count; i++){
				histvalstr = VP(HistAlertsVO, i, "KeyStr", char);
				if (!strcmp(histvalstr, KeyStr)){
					// found match to make invisible (don't delete or else it will alert again )
					VV(HistAlertsVO, i, "IsVisible", int) = FALSE;

					// remove from AlertsVO also
					vo_rm_row(AlertsGrid->vo, vorow);
					break;
				}
			}
		}
	}

	// see if we selected the ACK column
	if ((vcol = vcol_find(AlertsGrid->vo, "Ack"))){
		if (c == vcol->colnum){
			// ACK this row, set bg equal to grey
			// do it in historical and visible grids
			AlertsGrid->vo->row_index[vorow].fg = MGLHexColortoI("868686");

			KeyStr = VP(AlertsGrid->vo, vorow, "KeyStr", char);
			for (i = 0; i < HistAlertsVO->count; i++){
				histvalstr = VP(HistAlertsVO, i, "KeyStr", char);
				if (!strcmp(histvalstr, KeyStr)){
					// found match to ACK
					VV(HistAlertsVO, i, "Ack", int) = TRUE;
					break;
				}
			}
		}
	}



	return(TRUE);
}


MGLGRID *CreateAlertsGrid(struct InstanceData* id)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "AlertGridWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Alerts that are active
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "AlertsWidget");
	w->gregion.x1 = 400;
	w->gregion.y1 = id->m_nHeight - 120;
	w->gregion.x2 = 400;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "AlertsGrid", 6, 6, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	AlertsGrid = w->grid;


	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)yellow;
			w->grid->gridrows[r + 1].fg = (GLfloat*)colorfc4740;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)colorfc4740;
			w->grid->gridrows[r + 1].fg = (GLfloat*)colorfc4740;
		}
		w->grid->gridrows[r].bg = (GLfloat*)color504e4f;
		w->grid->gridrows[r + 1].bg = (GLfloat*)color454545;
	}

	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	// strncpy_s( w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "ID" , _TRUNCATE);
	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Key", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Alert", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Trigger", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Time", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Ack", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "Del", _TRUNCATE);

	w->grid->gridcols[3].val_callback = CnvSecs2HHMMSS;

	// draw the delete red x in the Del column
	// strncpy_s( w->grid->gridcols[5].pngfilename, 32, "redx32.png", _TRUNCATE);
	strncpy_s(w->grid->gridcols[4].pngfilename, sizeof(w->grid->gridcols[4].pngfilename), "alert-check-red-on.png", _TRUNCATE);
	w->grid->gridcols[4].texturesize = 16;
	w->grid->gridcols[4].img_displaysize = 10;
	strncpy_s(w->grid->gridcols[5].pngfilename, sizeof(w->grid->gridcols[5].pngfilename), "alert-red-x.png", _TRUNCATE);
	w->grid->gridcols[5].texturesize = 16;
	w->grid->gridcols[5].img_displaysize = 10;

	// we also need to fetch initial filter values from user settings DB
	// set row callbacks
	for (i = 0; i < AlertsGrid->nrows; i++){
		AlertsGrid->gridrows[i].row_callback = AlertsCallback;
	}


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	AlertsGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = AlertsGrid->marginx;
	w->gregion.y1 = AlertsGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - AlertsGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Alert Tracker", _TRUNCATE);
	w->visible = 1;
	w->grid = AlertsGrid;

	id->m_poglgui = oglgui;

	return(AlertsGrid);
}


MGLGRID *CreateSave(struct InstanceData* id)
{

	//pops up "SAVED" when save is pressed

	widgets *w, *lastw, *panel_widget;



	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SaveGrid = (MGLGRID*)id->SaveGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SaveDisplay");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Alerts that are active
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SaveDisplay");
	w->gregion.x1 = 300;
	w->gregion.y1 = id->m_nHeight - 160;
	w->gregion.x2 = 300;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "SaveDisplay", 1, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	SaveGrid = w->grid;



	w->grid->gridrows[0].fg = (GLfloat*)white;
	w->grid->gridrows[0].bg = (GLfloat*)color454545;



	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	MGLGridText(id, SaveGrid, 0, 0, "Saved");
	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Saved", _TRUNCATE);


	id->flag_check = 2;//to turn the grid on/off
	id->m_poglgui = oglgui;

	id->SaveGrid = SaveGrid;
	return(SaveGrid);
}





int CalcTextSize(char *linestr, SIZE *size)
{
	int len;
	// char errorbuf[ TMPBUF_SIZE ];

	assert(linestr);

	if ((len = strlen(linestr))){
		if (GetTextExtentPoint32(wglGetCurrentDC(), linestr, len, size) == 0){
			// function failed
			// sprintf_s(errorbuf, TMPBUF_SIZE, "error is %s", GetLastErrorStr());
		}
		else {
			return(TRUE);
		}
	}
	return(0);

}

//mtm
int SecondTabCallback(struct InstanceData* id, MGLGRID *parentgrid, int b, int m, int x, int y)
{
	// Toggle the selected row region On or OFF
	int c, vorow;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	vorow = 0;

	// see which column was selected
	c = MGLCalcCol(SecondTabGrid, x);


	//Corresponding colors are changed and sorresponding grids are made visible


	if (c == 0)
	{

		SecondTabGrid->gridcells[0][0].fg = (GLfloat*)Text_yellow;
		SecondTabGrid->gridcells[0][1].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][2].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][4].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][5].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = TRUE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = TRUE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;

		

	}


	else if (c == 1)
	{


		if (!SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig){
			SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));			
			SecondTabGrid->childgrid->nActions = 1; 
			SecondTabGrid->childgrid->Actions[0].IsActive = TRUE;
			SecondTabGrid->childgrid->Actions[0].ActionType = ACTION_FILTER;
		}
	
		SecondTabGrid->gridcells[0][0].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][1].fg = (GLfloat*)Text_yellow;
		SecondTabGrid->gridcells[0][2].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][4].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][5].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = FALSE;

		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = TRUE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = TRUE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
		MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + SecondTabGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * SecondTabGrid->gridrows[0].height));

	}

	else if (c == 2)
	{
		if (!SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig){
			SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
			SecondTabGrid->childgrid->nActions = 1; 
			SecondTabGrid->childgrid->Actions[0].IsActive = TRUE;
			SecondTabGrid->childgrid->Actions[0].ActionType = ACTION_FILTER;
		}

		SecondTabGrid->gridcells[0][0].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][1].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][2].fg = (GLfloat*)Text_yellow;
		SecondTabGrid->gridcells[0][4].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][5].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = TRUE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = TRUE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = FALSE;
		
		if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 0;		
		if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 0;
		if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
		if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[0] = 0;
		SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[1] = 0;
		SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[2] = 0;
		SecondTabGrid->childgrid->AlertsColorGrid->row_checkbox[4] = 0;



	}

	else if (c == 4)
	{

		if (!SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig){
			SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
			SecondTabGrid->childgrid->nActions = 1; // add only one for now if user did not have any stored
			SecondTabGrid->childgrid->Actions[0].IsActive = TRUE;
			SecondTabGrid->childgrid->Actions[0].ActionType = ACTION_FILTER;
		}
		
		SecondTabGrid->gridcells[0][0].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][1].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][2].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][5].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][4].fg = (GLfloat*)Text_yellow;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = TRUE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = TRUE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;

	}

	else if (c == 5)
	{
		SecondTabGrid->gridcells[0][0].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][1].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][2].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][4].fg = (GLfloat*)Grid_grey4;
		SecondTabGrid->gridcells[0][5].fg = (GLfloat*)Text_yellow;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible = TRUE;
		SecondTabGrid->childgrid->ActionsGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->ParametersGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible_memory = FALSE;
		SecondTabGrid->childgrid->RowColumn->parent_w->visible_memory = TRUE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
		if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
			SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
	}



	return(TRUE);
}

widgets *MGLAddTextButton(struct InstanceData* id, widgets *w, char *buttonname, char *textstr, widgets *lastw, int x1, int y1, int fontindex)
{
	int textwidth = 40;  // get from extents 
	SIZE size;
	int nlines, maxwidth, len, tmHeight, tmAscent, tmDescent, tmInternalLeading;
	char *membuf, *eolstr;
	char linebuf[256];

	assert(id);

	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), buttonname);
		// add new widget
		lastw->np = w;
	}
	w->gregion.x1 = x1;
	w->gregion.y1 = y1;

	w->type = WGT_TEXT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;  // button state is off
	w->fontindex = fontindex;  // index into GUIFont, 0 to GUI_FONTCOUNT
	strcpy_s(w->textbuf, sizeof(w->textbuf), textstr);

	// calculate size of text
	// this routine is only called on resize of window, or startup
	SetGUIFontIndex(id, w->fontindex, 1);
	membuf = w->textbuf;

	maxwidth = 0;
	nlines = 0;

	// there may be multiple lines, so we must process each line
	do {
		if ((eolstr = strchr(membuf, '\n'))){
			len = eolstr - membuf;
			if (len > 255){
				break;  // some problem with the data
			}
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = eolstr;
			membuf++; // skip over eol char
		}
		else {
			len = strlen(membuf);
			strncpy_s(linebuf, _countof(linebuf), membuf, len);
			membuf = NULL;
		}
		CalcTextSize(linebuf, &size);
		if (size.cx > maxwidth) maxwidth = size.cx;
		nlines++;
	} while (membuf && *membuf != '\0');

	tmHeight = id->m_guiFonts[fontindex].m_tmCustom.tmHeight;
	tmAscent = id->m_guiFonts[fontindex].m_tmCustom.tmAscent;
	tmDescent = id->m_guiFonts[fontindex].m_tmCustom.tmDescent;
	tmInternalLeading = id->m_guiFonts[fontindex].m_tmCustom.tmInternalLeading;

	w->textheight = size.cy - tmInternalLeading;
	w->gregion.x2 = w->gregion.x1 + maxwidth;
	w->gregion.y2 = w->gregion.y1 + (w->textheight * nlines);

	return(w);
}




widgets *AddNextTextButton(struct InstanceData* id, widgets *w, char *buttonname, char *buttontext, widgets *lastw, int bufsize)
{
	int xstart = 0;
	widgets *ButtonW = NULL;
	widgets *bw, *sibw;
	int sib;

	assert(id);

	if (lastw){
		xstart = lastw->gregion.x2;
	}

	// add pretext buffer pix
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), "leftbuffer");
		lastw->np = w;
		lastw = w;
	}
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + bufsize;  // image is 1x25, but expand it to buffer we need
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;  // button state is off
	// w->active_image = "1px-horizontal.png";
	w->inactive_image = "nav-bg-px.png";
	w->active_image = "selected-px.png";
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 1;
	w->filtertype = GL_NEAREST;
	ButtonW = w; // ButtonW acually points to first left buffer 

	xstart = w->gregion.x2;
	// draw top buttons Applications, Overlays , Filters ,etc
	if (!w->np){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), buttonname);
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	else {
		w = w->np; // next widget is already created
	}

#ifdef OLDWAY_USES_IMAGE_WITH_TEXT
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + imgwidth;
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;  // button state is off
	w->active_image = imagenameOn;
	w->inactive_image = imagenameOff;
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 1;
	w->filtertype = GL_NEAREST;
	// use first buffer widget instead --- ButtonW = w;
#endif


	w = MGLAddTextButton(id, ButtonW->np, buttontext, buttontext, ButtonW,
		xstart, ButtonW->gregion.y1 + 10, 3);


	ButtonW->gregion.x2 = ButtonW->gregion.x1 + (bufsize * 2) + w->gregion.x2 - w->gregion.x1;

	// set up pointers to sibling widgets so if any are clicked on, the rest can be updated
	ButtonW->nsiblings = 2;
	bw = ButtonW->np;
	bw->nsiblings = 2;
	for (sib = 0, sibw = ButtonW; sib < 2; sib++, sibw = sibw->np){
		ButtonW->siblings[sib] = sibw;
		bw->siblings[sib] = sibw;
	}



#ifdef OLDWAY
	xstart = textAppW->gregion.x2;
	// add buffer pix
	if (!w->np){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), "rightbuffer");
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	else {
		w = w->np; // next widget is already created
	}
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + bufsize;  // image is 1x25, but expand it to buffer we need
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;  // button state is off
	// w->active_image = "1px-horizontal.png";
	w->inactive_image = "nav-bg-px.png";
	w->active_image = "selected-px.png";
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 1;
	w->filtertype = GL_NEAREST;

	// set up pointers to sibling widgets so if any are clicked on, the rest can be updated
	ButtonW->nsiblings = 3;
	bw = ButtonW->np;
	bw->nsiblings = 3;
	rightw = bw->np;
	rightw->nsiblings = 3;
	for (sib = 0, sibw = ButtonW; sib < 3; sib++, sibw = sibw->np){
		ButtonW->siblings[sib] = sibw;
		bw->siblings[sib] = sibw;
		rightw->siblings[sib] = sibw;
	}

#endif


	return(ButtonW);  // returns middle item, so calling routine must beware
}




widgets *AddNextButton(struct InstanceData* id, widgets *w, char *buttonname, char *imagenameOn, char *imagenameOff, int imgwidth, widgets *lastw, int bufsize)
{
	int xstart = 0;
	widgets *ButtonW = NULL, *bw, *sibw, *rightw;
	int sib;

	assert(id);

	if (lastw){
		xstart = lastw->gregion.x2;
	}

	// add pretext buffer pix
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), "leftbuffer");
		lastw->np = w;
		lastw = w;
	}
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + bufsize;  // image is 1x25, but expand it to buffer we need
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;  // button state is off
	// w->active_image = "1px-horizontal.png";
	w->inactive_image = "nav-bg-px.png";
	w->active_image = "selected-px.png";
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 1;
	w->filtertype = GL_NEAREST;
	ButtonW = w; // ButtonW acually points to first left buffer 

	xstart = w->gregion.x2;
	// draw top buttons Applications, Overlays , Filters ,etc
	if (!w->np){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), buttonname);
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	else {
		w = w->np; // next widget is already created
	}
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + imgwidth;
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;  // button state is off
	w->active_image = imagenameOn;
	w->inactive_image = imagenameOff;
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 1;
	w->filtertype = GL_NEAREST;
	// use first buffer widget instead --- ButtonW = w;


	xstart = w->gregion.x2;
	// add buffer pix
	if (!w->np){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), "rightbuffer");
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	else {
		w = w->np; // next widget is already created
	}
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + bufsize;  // image is 1x25, but expand it to buffer we need
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_BUTTON;
	w->visible = 1;
	w->IsActive = 0;  // button state is off
	// w->active_image = "1px-horizontal.png";
	w->inactive_image = "nav-bg-px.png";
	w->active_image = "selected-px.png";
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 1;
	w->filtertype = GL_NEAREST;

	// set up pointers to sibling widgets so if any are clicked on, the rest can be updated
	ButtonW->nsiblings = 3;
	bw = ButtonW->np;
	bw->nsiblings = 3;
	rightw = bw->np;
	rightw->nsiblings = 3;
	for (sib = 0, sibw = ButtonW; sib < 3; sib++, sibw = sibw->np){
		ButtonW->siblings[sib] = sibw;
		bw->siblings[sib] = sibw;
		rightw->siblings[sib] = sibw;
	}


	return(ButtonW);  // returns middle item, so calling routine must beware
}



widgets *AddButSeparator(struct InstanceData* id, widgets *w, char *widgetname, widgets *lastw, widgets *bufw)
{
	int xstart = 0;

	assert(id);

	if (lastw){
		xstart = bufw->gregion.x2;
	}
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + 3;  // imgwidth of separator is 3
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_PANEL_IMAGE;
	w->visible = 1;
	w->active_image = "separator.png";
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 0;
	w->filtertype = GL_LINEAR;


	return(w);

}



widgets *AddButSeparatorOLD(struct InstanceData* id, widgets *w, char *widgetname, widgets *lastw)
{
	int xstart = 0;

	assert(id);

	if (lastw){
		xstart = lastw->gregion.x2;
	}
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	w->gregion.x1 = xstart;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + 3;  // image is 32x25
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_PANEL_IMAGE;
	w->visible = 1;
	w->active_image = "separator.png";
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 0;
	w->filtertype = GL_LINEAR;


	return(w);

}


widgets *MGLAddPanel(struct InstanceData* id, widgets *w, char *widgetname, GLfloat *bg, widgets *lastw, int x1, int x2, int y1, int y2)
{
	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// similar to AddButton, but no callback for this image
	// draw top buttons Applications, Overlays , Filters ,etc
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		if (lastw){
			lastw->np = w;
		}
		else {
			// first widget
			oglgui->widgets = w;
		}
	}

	w->bg = (GLfloat*)bg;

	w->gregion.x1 = x1;
	w->gregion.y1 = y1;
	w->gregion.x2 = x2;  // image is 3x25
	w->gregion.y2 = y2;
	w->width = w->gregion.x2 - w->gregion.x1;
	w->height = w->gregion.y2 - w->gregion.y1;
	w->type = WGT_PANEL;
	w->visible = 1;

	id->m_poglgui = oglgui;

	return(w);
}





widgets *AddText(struct InstanceData* id, widgets *w, char *widgetname, char *textstr, widgets *lastw,
	int x1, int x2, int y1, int y2)
{
	assert(id);

	// Text for lights 
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	w->gregion.x1 = x1;
	w->gregion.y1 = y1;
	w->gregion.x2 = x2;
	w->gregion.y2 = y2;
	w->type = WGT_TEXT;
	w->visible = 1;
	strcpy_s(w->textbuf, sizeof(w->textbuf), textstr);

	return(w);
}





widgets *AddEdge(struct InstanceData* id, widgets *w, char *widgetname, char *pngname, widgets *lastw, int startx)
{
	assert(id);

	// draw top buttons Applications, Overlays , Filters ,etc
	if (!w){
		w = (widgets *)vo_malloc(sizeof(widgets));
		strcpy_s(w->name, sizeof(w->name), widgetname);
		// add new widget
		lastw->np = w;
		lastw = w;
	}
	w->gregion.x1 = startx;
	w->gregion.y1 = id->m_nHeight - 40;
	w->gregion.x2 = w->gregion.x1 + 3;  // image is 3x25
	w->gregion.y2 = w->gregion.y1 + 25;
	w->type = WGT_PANEL_IMAGE;
	w->visible = 1;
	w->active_image = pngname;
	w->img_alpha_mode = ALPHA_MODE_BLACK;
	w->img_blend = 1;
	w->filtertype = GL_NEAREST;

	return(w);
}



void DbgOutputWidgets(widgets *w)
{

	char tmpstr[256];

	OutputDebugString("DbgOutputWidgets\n\n");
	while (w){
		sprintf_s(tmpstr, _countof(tmpstr), "%s img='%s' x1=%d y1=%d x2=%d y2=%d\n", w->name, w->active_image,
			w->gregion.x1, w->gregion.y1, w->gregion.x2, w->gregion.y2);

		OutputDebugString(tmpstr);
		w = w->np;
	}

}


widgets_ll *MGLMakeRadioButtons(struct InstanceData *id, ...)
{
	va_list ap;
	widgets_ll *radio_ll = NULL, *nextll;
	widgets *w;

	assert(id);

	va_start(ap, id);

	while ((w = (widgets *)va_arg(ap, char *))){
		if (!radio_ll){
			radio_ll = (widgets_ll *)vo_malloc(sizeof(widgets_ll));
			nextll = radio_ll;
		}
		else {
			nextll->np = (widgets_ll *)vo_malloc(sizeof(widgets_ll));
			nextll = nextll->np;
		}
		nextll->w = w;
		w->radio_ll = radio_ll; // place linked list in all widgets in list
	}

	return(radio_ll);
}



ZoomToSize(struct InstanceData* id, double newxrange, double newyrange)
{
	double xrange, yrange, newratio, xdiff, ydiff;
	char strDebug[1024] = { 0 };
	int zout;

	xrange = id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin;
	yrange = id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin;

	xdiff = fabs(newxrange - xrange); // xdiff is the amount to adjust
	xdiff /= 2.0; // divide in half since we move min and max sides
	ydiff = fabs(newyrange - yrange);
	ydiff /= 2.0;


	newratio = newxrange / xrange;

	if (newratio > 1.0) zout = TRUE;
	else zout = FALSE;

	newratio /= 2.0; // divide by half since we are moving both sides

	sprintf_s(strDebug, sizeof(strDebug), "START newrange=%g newratio=%g xrange=%g xmin=%g xmax=%g ymin=%g ymax=%g\n",
		newxrange, newratio, xrange,
		id->m_sCurLayout.m_dWorldXmin, id->m_sCurLayout.m_dWorldXmax,
		id->m_sCurLayout.m_dWorldYmin, id->m_sCurLayout.m_dWorldYmax);
	OutputDebugString(strDebug);

	if (zout == TRUE){
		// zoomout
		id->m_sCurLayout.m_dWorldXmin -= xdiff;
		id->m_sCurLayout.m_dWorldYmin -= ydiff;
		id->m_sCurLayout.m_dWorldXmax += xdiff;
		id->m_sCurLayout.m_dWorldYmax += ydiff;
		id->m_sCurLayout.m_dWorldYSize = (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin);
		id->m_sCurLayout.m_dWorldXSize = (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin);
	}
	else {
		// zoomin
		id->m_sCurLayout.m_dWorldXmin += xdiff;
		id->m_sCurLayout.m_dWorldYmin += ydiff;
		id->m_sCurLayout.m_dWorldXmax -= xdiff;
		id->m_sCurLayout.m_dWorldYmax -= ydiff;
		id->m_sCurLayout.m_dWorldYSize = (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin);
		id->m_sCurLayout.m_dWorldXSize = (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin);
	}


#ifdef OLDWAY
	if (zout == TRUE){
		// zoomout
		id->m_sCurLayout.m_dWorldXmin -= xrange * newratio;
		id->m_sCurLayout.m_dWorldYmin -= yrange * newratio;
		id->m_sCurLayout.m_dWorldXmax += xrange * newratio;
		id->m_sCurLayout.m_dWorldYmax += yrange * newratio;
		id->m_sCurLayout.m_dWorldYSize = (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin);
		id->m_sCurLayout.m_dWorldXSize = (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin);
	}
	else {
		// zoomin
		id->m_sCurLayout.m_dWorldXmin += xrange * newratio;
		id->m_sCurLayout.m_dWorldYmin += yrange * newratio;
		id->m_sCurLayout.m_dWorldXmax -= xrange * newratio;
		id->m_sCurLayout.m_dWorldYmax -= yrange * newratio;
		id->m_sCurLayout.m_dWorldYSize = (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin);
		id->m_sCurLayout.m_dWorldXSize = (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin);
	}
#endif


	if (id->m_sCurLayout.m_dWorldXmax < id->m_sCurLayout.m_dWorldXmin){
		//  error
		return(FALSE);
	}
	if (id->m_sCurLayout.m_dWorldYmax < id->m_sCurLayout.m_dWorldYmin){
		//  error
		return(FALSE);
	}

	xrange = id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin;
	yrange = id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin;


	sprintf_s(strDebug, sizeof(strDebug), "END newratio=%g new xrange=%g xmin=%g xmax=%g ymin=%g ymax=%g\n",
		newratio, xrange,
		id->m_sCurLayout.m_dWorldXmin, id->m_sCurLayout.m_dWorldXmax,
		id->m_sCurLayout.m_dWorldYmin, id->m_sCurLayout.m_dWorldYmax);
	OutputDebugString(strDebug);



	return(TRUE);



}



int SliderOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	// find out where in the slider the user clicked an move the slider bar as appropriate
	double barsize, worldpercent, maxworldxsize, maxworldysize, newXrange, newYrange;
	double xrange, yrange;

	if (w->IsActive){

	}
	else {
	}

	// if user clicked on the bar, then see if he is dragging
	if (InRectangle(x, y, &(SliderBarBGW->gregion)) ||
		InRectangle(x, y, &(LeftSliderEdgeW->gregion)) ||
		InRectangle(x, y, &(RightSliderEdgeW->gregion)))
	{

		// move bar to where the user clicked, ala Map Quest interface
		SliderBarW->gregion.x1 = x;
		SliderBarW->gregion.x2 = SliderBarW->gregion.x1 + 6;

		// change map world coordinates to reflect choise user made
		maxworldxsize = 75.25;
		maxworldysize = 28.0;
		barsize = 80.0 + (4 * 2);  // barsize is length plus edges
		worldpercent = ((double)x - LeftSliderEdgeW->gregion.x1) / barsize;

		//worldxsize = maxworldxsize * worldpercent;
		//worldysize = maxworldysize * worldpercent;

		// ZoomToSize( id, worldxsize , worldysize );
		xrange = id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin;
		yrange = id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin;

		newXrange = worldpercent * maxworldxsize;
		// newYrange = worldpercent * maxworldysize;
		// match aspect ratio of screen
		newYrange = (newXrange * yrange) / xrange;

		ZoomToSize(id, newXrange, newYrange);

		return(TRUE);
	}
	return(FALSE);
}






int SideArrowOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	MRECT sidebarrect;
	MGLGRID* SearchGrid = (MGLGRID*)id->SearchGrid;
	id->SearchGrid = SearchGrid;

	if (w->IsActive){
		// open up the panel to full size and activate all of the widgets
		// maximize the side bar
		sidebarrect.x1 = id->m_nWidth - 63; // side edge is 23, so add 40
		sidebarrect.x2 = id->m_nWidth;
		sidebarrect.y1 = 0;
		sidebarrect.y2 = id->m_nHeight - 51;
		SideBarW->gregion = sidebarrect;
		SideSeparatorW->gregion.x1 -= 28;
		SideSeparatorW->gregion.x2 -= 28;
		BlkSeparatorW->gregion.x1 -= 28;
		BlkSeparatorW->gregion.x2 -= 28;
		SideBarArrowW->ImgAngle = -90.0;
		SideBarArrowW->gregion.x1 -= 28;
		SideBarArrowW->gregion.x2 -= 28;
		// set side bar widgets to the correct location

		ZoomSliderW->gregion.x1 = sidebarrect.x1 - ZoomSliderW->width;
		ZoomSliderW->gregion.x2 = sidebarrect.x1 - 1;
		ZoomSliderW->gregion.y1 = USAMapW->gregion.y1;
		ZoomSliderW->gregion.y2 = SubNavSeparatorBotW_O->gregion.y1; // should be just below subnav bar 

		LeftSliderEdgeW->gregion.x1 = ZoomSliderW->gregion.x1 + 10;
		LeftSliderEdgeW->gregion.x2 = LeftSliderEdgeW->gregion.x1 + 4;  // img X is 4 pixels
		LeftSliderEdgeW->gregion.y1 = ZoomSliderW->gregion.y1 + 10;
		LeftSliderEdgeW->gregion.y2 = LeftSliderEdgeW->gregion.y1 + 7;  // img Y is 7 pixels

		SliderBarBGW->gregion.x1 = LeftSliderEdgeW->gregion.x2;
		SliderBarBGW->gregion.x2 = SliderBarBGW->gregion.x1 + 80;
		SliderBarBGW->gregion.y1 = LeftSliderEdgeW->gregion.y1;
		SliderBarBGW->gregion.y2 = LeftSliderEdgeW->gregion.y2;

		RightSliderEdgeW->gregion.x1 = SliderBarBGW->gregion.x2;
		RightSliderEdgeW->gregion.x2 = RightSliderEdgeW->gregion.x1 + 4;
		RightSliderEdgeW->gregion.y1 = LeftSliderEdgeW->gregion.y1;
		RightSliderEdgeW->gregion.y2 = LeftSliderEdgeW->gregion.y2;


		SetZoomBarW(id);

		SliderArrowW->gregion.x1 = RightSliderEdgeW->gregion.x2 + 4;
		SliderArrowW->gregion.x2 = SliderArrowW->gregion.x1 + 10;
		SliderArrowW->gregion.y1 = LeftSliderEdgeW->gregion.y1;
		SliderArrowW->gregion.y2 = LeftSliderEdgeW->gregion.y1 + 8; // tot size of image is 19


		MGLSetVisibleWidgetLL(USAMapW, LastSideBarW, 1);
	}
	else {
		// minimize the side bar
		sidebarrect.x1 = id->m_nWidth - 35; // side edge is 23, so add 12
		sidebarrect.x2 = id->m_nWidth;
		sidebarrect.y1 = 0;
		sidebarrect.y2 = id->m_nHeight - 51;
		SideBarW->gregion = sidebarrect;
		SideSeparatorW->gregion.x1 += 28;
		SideSeparatorW->gregion.x2 += 28;
		BlkSeparatorW->gregion.x1 += 28;
		BlkSeparatorW->gregion.x2 += 28;
		SideBarArrowW->ImgAngle = 90.0;
		SideBarArrowW->gregion.x1 += 28;
		SideBarArrowW->gregion.x2 += 28;
		MGLSetVisibleWidgetLL(USAMapW, LastSideBarW, 0);
		SearchGrid->parent_w->visible = FALSE;
	}

	return(TRUE);
}

//sy
int SearchCallback(struct InstanceData* id, MGLGRID *SearchGrid, int b, int m, int x, int y)
{
	// Callback function for the search grid	
	int r, vorow, c;
	char *AppName;
	int check = 0;


	assert(id);

	if (!SearchGrid) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (SearchGrid->gridrows[0].height != MGL_DEFAULT) {
		// only works if rows all have same height
		r = (int)floor(((double)y - SearchGrid->gridrect.y1) / SearchGrid->gridrows[0].height);
	} else {
		r = (int)floor(((double)y - SearchGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}


	if (r < 0 || r >= SearchGrid->nrows) {
		return(FALSE);
	}



	// see which column was selected
	c = 0;

	r = SearchGrid->nrows - r - 1;
	vorow = r - 1;


	//columns 3,5,7 are the ones in use. c==2(column 3) corresponds to "Add New", c==5 (column 6) corresponds to "Source", c==7 (column 8) corresponds to "Create" 
	if (c == 0 && vorow == 0) {
		int t = 0;
		AppName = VP(SearchGrid->vo, vorow, "SearchGridApp", char);
		if (!strcmp(AppName, "Enter flight no")) {//Spaces in the string is to adjust the look in the GUI
			// user wants to create a new app, so set the text input box and allow him to enter the name of the app
			// popup white text box in place of the "New"
			// clear out current string
			AppName[0] = '\0';
			SearchGrid->textrow = vorow;
			SearchGrid->parent_w->TextActive = TRUE;
			ActiveTextGrid = SearchGrid;
			strcpy_s(SearchGrid->textcolname, sizeof(SearchGrid->textcolname), "SearchGridApp");
			SearchGrid->vo->row_index[vorow].fg = MGLColorHex2Int("Black");
			AppName = VP(SearchGrid->vo, vorow, "SearchGridApp", char);

		}

		else if (!strcmp(AppName, "")) {
			// user wants to create a new app, so set the text input box and allow him to enter the name of the app
			// popup white text box in place of the "New"
			// clear out current string
			AppName[0] = '\0';
			SearchGrid->textrow = vorow;
			SearchGrid->parent_w->TextActive = TRUE;
			ActiveTextGrid = SearchGrid;
			strcpy_s(SearchGrid->textcolname, sizeof(SearchGrid->textcolname), "SearchGridApp");
			SearchGrid->vo->row_index[vorow].fg = MGLColorHex2Int("Black");
			AppName = VP(SearchGrid->vo, vorow, "SearchGridApp", char);

		} else {
			// user wants to create a new app, so set the text input box and allow him to enter the name of the app
			// popup white text box in place of the "New"
			// clear out current string
			AppName[0] = '\0';
			SearchGrid->textrow = vorow;
			SearchGrid->parent_w->TextActive = TRUE;
			ActiveTextGrid = SearchGrid;
			strcpy_s(SearchGrid->textcolname, sizeof(SearchGrid->textcolname), "SearchGridApp");
			SearchGrid->vo->row_index[vorow].fg = MGLColorHex2Int("Black");
			AppName = VP(SearchGrid->vo, vorow, "SearchGridApp", char);
		}

	}



	return(TRUE);
}

//sy
void CrSearchGridVO(MGLGRID *SearchGrid)
{

	VO *SearchGridVO;

	assert(SearchGrid);

	SearchGridVO = (VO *)vo_create(0, NULL);
	vo_set(SearchGridVO, V_NAME, "SearchGridVO", NULL);
	VOPropAdd(SearchGridVO, "SearchGridApp", NTBSTRINGBIND, 64, VO_NO_ROW);



	// VOPropAdd( DataAppVO, "OnOff", NTBSTRINGBIND, 16, VO_NO_ROW );  

	SearchGrid->vo = SearchGridVO;

}

//sy
void FillSearchGridVO(struct InstanceData* id, MGLGRID *SearchGrid)
{

	assert(id);
	assert(SearchGrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!SearchGrid->vo) {
		CrSearchGridVO(SearchGrid);
	} else {
		// already created the DataAppGrid->vo
		return;
	}

	//vo_rm_rows(SearchGrid->vo, 0, SearchGrid->vo->count);
	CalcColWidths(id, SearchGrid);
}



//sy
MGLGRID *CreateSearchGrid(struct InstanceData* id)
{

	//pops up "SAVED" when save is pressed

	widgets *w, *lastw, *panel_widget;



	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SearchGrid = (MGLGRID*)id->SearchGrid;

	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np) {
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "FlghtSearchPanel");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;

	// add new widget
	if (lastw) {
		lastw->np = w;
	} else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// Create MGLGrid for list of Alerts that are active
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SearchGrid");
	w->gregion.x1 = 300;
	w->gregion.y1 = id->m_nHeight - 160;
	w->gregion.x2 = 300;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "SearchGrid", 2, 1, w->gregion.x1, w->gregion.y1);
	w->grid->width = 125;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	SearchGrid = w->grid;



	w->grid->gridrows[0].fg = (GLfloat*)black;
	w->grid->gridrows[0].bg = (GLfloat*)white;

	w->grid->gridrows[1].fg = (GLfloat*)black;
	w->grid->gridrows[1].bg = (GLfloat*)white;


	SearchGrid->gridrows[1].row_callback = SearchCallback;


	// add new widget
	if (lastw) {
		lastw->np = w;
		lastw = w;
	}


	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "              ", _TRUNCATE);


	//id->flag_check = 2;//to turn the grid on/off
	id->SearchGrid = SearchGrid;
	return(SearchGrid);
}

//sy
int FindFlightOnClick(struct InstanceData* id, widgets *w, int x, int y) //Find Flight Callback (sy)
{
	MGLGRID* SearchGrid = (MGLGRID*)id->SearchGrid;
	id->SearchGrid = SearchGrid;

	if (w->IsActive)// to open the search box 
	{
		SearchGrid = CreateSearchGrid(id);
		MGLGridMove(id, SearchGrid, FindFlightW->gregion.x1 - SearchGrid->width, FindFlightW->gregion.y1);
		SearchGrid->parent_w->visible = TRUE;
		FillSearchGridVO(id, SearchGrid);

		vo_alloc_rows(SearchGrid->vo, 1);
		strncpy_s(VP(SearchGrid->vo, SearchGrid->vo->count - 1, "SearchGridApp", char), 64, "Flight/Airport", _TRUNCATE);

	} else // for minimising 
	{
		SearchGrid->parent_w->visible = FALSE;

	}

	return(TRUE);
}

int DisableFilterOnClick(struct InstanceData* id, widgets *w, int x, int y)// Disable filter callback (sy)
{

	if (id->filter_check != 1) {
		id->filter_check = 1; //disable filter;
	} else {
		id->filter_check = 2;
	}




	return(TRUE);
}


void CreateRightBar(struct InstanceData* id)
{
	widgets *w, *lastw;
	MRECT sidebarrect;
	int imgxsize, imgysize, SubButtonsSpacing, panelx1, ZoomWidth, x1, x2, y1, y2;

	MGLGRID* SearchGrid = (MGLGRID*)id->SearchGrid;
	

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	sidebarrect.x1 = id->m_nWidth - 35; // side edge is 23, so add 12
	sidebarrect.x2 = id->m_nWidth;
	sidebarrect.y1 = 0;
	sidebarrect.y2 = id->m_nHeight - 51;

	SubButtonsSpacing = 20;

	SideBarW = MGLAddPanel(id, SideBarW, "SideBarW", (GLfloat *)color454545, lastw,
		sidebarrect.x1, sidebarrect.x2, sidebarrect.y1, sidebarrect.y2);

	// add light grey and black lines to edges 
	SideSeparatorW = MGLAddPanel(id, SideSeparatorW, "SideSeparatorW", (GLfloat *)color868686, SideBarW,
		SideBarW->gregion.x1 - 2, SideBarW->gregion.x1 - 1, SideBarW->gregion.y1, SideBarW->gregion.y2);

	// add light grey and black lines to edges 
	BlkSeparatorW = MGLAddPanel(id, BlkSeparatorW, "BlkSeparatorW", (GLfloat *)black, SideSeparatorW,
		SideBarW->gregion.x1 - 1, SideBarW->gregion.x1, SideBarW->gregion.y1, SideBarW->gregion.y2);

	// place sidebar popout arrow at far right just below passur logo
	imgxsize = 10;
	imgysize = 8;
	SideBarArrowW = MGLAddButton(id, SideBarArrowW, "SideBarArrowW", "down-reorder-arrow.png", "down-reorder-arrow.png", BlkSeparatorW,
		sidebarrect.x1 + 1, sidebarrect.x1 + 1 + imgxsize, sidebarrect.y2 - 10 - imgysize, sidebarrect.y2 - 10,
		ALPHA_MODE_BLACK, 1, GL_LINEAR);
	SideBarArrowW->ImgAngle = 90.0;
	SideBarArrowW->OnClickCB = SideArrowOnClickCB;


	imgxsize = 36;
	imgysize = 23;
	panelx1 = id->m_nWidth - 63;  // 23 for window loss + 40
	USAMapW = MGLAddButton(id, USAMapW, "USAMapW", "map-on.png", "map-off.png", SideBarArrowW,
		panelx1 + 1, panelx1 + 1 + imgxsize, SideBarArrowW->gregion.y1 - SubButtonsSpacing - imgysize, SideBarArrowW->gregion.y1 - SubButtonsSpacing,
		ALPHA_MODE_BLACK, 1, GL_LINEAR);

	imgxsize = 26;
	imgysize = 21;
	FindFlightW = MGLAddButton(id, FindFlightW, "FindFlightW", "find-flight-on.png", "find-flight-off.png", USAMapW,
		panelx1 + 1, panelx1 + 1 + imgxsize, USAMapW->gregion.y1 - SubButtonsSpacing - imgysize, USAMapW->gregion.y1 - SubButtonsSpacing,
		ALPHA_MODE_BLACK, 1, GL_LINEAR);
	FindFlightW->OnClickCB = FindFlightOnClick;
	SearchGrid = CreateSearchGrid(id);
	id->SearchGrid = SearchGrid;
	MGLGridMove(id, SearchGrid, FindFlightW->gregion.x1 - SearchGrid->width, FindFlightW->gregion.y1);

	imgxsize = 29;
	imgysize = 41;
	DisableFiltersW = MGLAddButton(id, DisableFiltersW, "DisableFiltersW", "enable-filters-alt.png", "disable-filters-alt.png", FindFlightW,
		panelx1 + 1, panelx1 + 1 + imgxsize, FindFlightW->gregion.y1 - SubButtonsSpacing - imgysize, FindFlightW->gregion.y1 - SubButtonsSpacing,
		ALPHA_MODE_BLACK, 1, GL_LINEAR);
	DisableFiltersW->OnClickCB = DisableFilterOnClick;

	imgxsize = 25;
	imgysize = 25;
	HelpW = MGLAddButton(id, HelpW, "DisableFiltersW", "help-icon.png", "help-icon.png", DisableFiltersW,
		panelx1 + 1, panelx1 + 1 + imgxsize, DisableFiltersW->gregion.y1 - SubButtonsSpacing - imgysize, DisableFiltersW->gregion.y1 - SubButtonsSpacing,
		ALPHA_MODE_BLACK, 1, GL_LINEAR);


	// put zoom slider next to map on side bar, which will be under the subnavbar
	ZoomWidth = 120;
	x1 = sidebarrect.x1 - ZoomWidth;
	x2 = sidebarrect.x1 - 1;
	y1 = USAMapW->gregion.y1;
	y2 = SubNavSeparatorBotW_A->gregion.y1 - 1; // should be just below subnav bar 
	ZoomSliderW = MGLAddSlider(id, ZoomSliderW, "ZoomSliderW", (GLfloat *)color454545, HelpW, x1, x2, y1, y2, 0.0, 100.0, 50.0);
	ZoomSliderW->OnClickCB = SliderOnClickCB;


	// default is to minimize the side bar and make the icons invisible
	LastSideBarW = SliderBarW;
	MGLSetVisibleWidgetLL(USAMapW, LastSideBarW, 0);
	id->m_poglgui = oglgui;

}



void CreateOverlaysSubNav(struct InstanceData* id)
{
	widgets *w, *lastw;
	MRECT subnavrect;
	int SubButtonsSpacing, space_top_bot;
	SIZE size;
	int fontindex, textheight, nlines, panelheight, totheight;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	subnavrect.x1 = 0;
	subnavrect.x2 = id->m_nWidth;
	subnavrect.y1 = id->m_nHeight - 83;
	subnavrect.y2 = id->m_nHeight - 50;

	SubButtonsSpacing = 22;

	OverlaysBarW = MGLAddPanel(id, OverlaysBarW, "OverlaysBarW", (GLfloat *)color454545, lastw,
		subnavrect.x1, subnavrect.x2, subnavrect.y1, subnavrect.y2);

	// add separator bars at top and bottom
	SubNavSeparatorTopW_O = MGLAddPanel(id, SubNavSeparatorTopW_O, "SubNavSeparatorTopW_O", (GLfloat *)color868686, OverlaysBarW,
		subnavrect.x1, subnavrect.x2, subnavrect.y2, subnavrect.y2 - 1);

	//  white on bottom , then dark on line above it
	SubNavSeparatorDarkW_O = MGLAddPanel(id, SubNavSeparatorDarkW_O, "SubNavSeparatorDarkW_O", (GLfloat *)black, SubNavSeparatorTopW_O,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 1, subnavrect.y1);
	SubNavSeparatorBotW_O = MGLAddPanel(id, SubNavSeparatorBotW_O, "SubNavSeparatorBotW_O", (GLfloat *)color868686, SubNavSeparatorDarkW_O,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 2, subnavrect.y1 - 1);

	// we need to calculate text height to place the widget, so calculate it
	fontindex = 3;
	SetGUIFontIndex(id, fontindex, 1);
	CalcTextSize("PASSUR", &size);

	textheight = size.cy - id->m_guiFonts[fontindex].m_tmCustom.tmInternalLeading;

	// height to start text is size of panel - height of text / 2, so we have equal space on top and bottom
	nlines = 1;
	panelheight = subnavrect.y2 - subnavrect.y1;
	totheight = textheight * nlines;
	space_top_bot = panelheight - totheight;
	PassurButW = MGLAddTextButton(id, PassurButW, "PassurButW", "PASSUR", SubNavSeparatorBotW_O,
		subnavrect.x1 + 280, subnavrect.y1 + (space_top_bot / 2), fontindex);


	ASDIButW = MGLAddTextButton(id, ASDIButW, "ASDIButW", "ASDI", PassurButW,
		PassurButW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	ASDEXButW = MGLAddTextButton(id, ASDEXButW, "ASDEXButW", "ASDE-X", ASDIButW,
		ASDIButW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	ASDSBButW = MGLAddTextButton(id, ASDSBButW, "ASDSBButW", "ADS-B", ASDEXButW,
		ASDEXButW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	AirwaysButW = MGLAddTextButton(id, AirwaysButW, "AirwaysButW", "Airways", ASDSBButW,
		ASDSBButW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	WxButW = MGLAddTextButton(id, WxButW, "WxButW", "Weather", AirwaysButW,
		AirwaysButW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);


	// FlightPlanW
	nlines = 2;
	panelheight = subnavrect.y2 - subnavrect.y1;
	totheight = textheight * nlines;
	space_top_bot = panelheight - totheight;
	FlightPlanW = MGLAddTextButton(id, FlightPlanW, "FlightPlanW", "Flight\nPlan", WxButW,
		WxButW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	AircraftTrailsW = MGLAddTextButton(id, AircraftTrailsW, "AircraftTrailsW", "Aircraft\nTrails", FlightPlanW,
		FlightPlanW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	AircraftRingsW = MGLAddTextButton(id, AircraftRingsW, "AircraftRingsW", "Aircraft\nRings", AircraftTrailsW,
		AircraftTrailsW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	RangeRingsW = MGLAddTextButton(id, RangeRingsW, "RangeRingsW", "Range\nRings", AircraftRingsW,
		AircraftRingsW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	AirportStatusW = MGLAddTextButton(id, AirportStatusW, "AirportStatusW", "Airport\nStatus", RangeRingsW,
		RangeRingsW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	//redefined again for "runways"
	nlines = 1;
	panelheight = subnavrect.y2 - subnavrect.y1;
	totheight = textheight * nlines;
	space_top_bot = panelheight - totheight;

	RunwaysW = MGLAddTextButton(id, RunwaysW, "RunwaysW", "Airport\nStatus", AirportStatusW,
		AirportStatusW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	// TerminalFixesW onwards 
	nlines = 2;
	panelheight = subnavrect.y2 - subnavrect.y1;
	totheight = textheight * nlines;
	space_top_bot = panelheight - totheight;
	
	TerminalFixesW = MGLAddTextButton(id, TerminalFixesW, "TerminalFixesW", "Terminal\nFixes", RunwaysW,
		RunwaysW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	HighFixesW = MGLAddTextButton(id, HighFixesW, "HighFixesW", "High\nFixes", TerminalFixesW,
		TerminalFixesW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	LowFixesW = MGLAddTextButton(id, LowFixesW, "LowFixesW", "Low\nFixes", HighFixesW,
		HighFixesW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	id->m_poglgui = oglgui;

}

void CreateApplicationsSubNav(struct InstanceData* id)
{
	widgets *w, *lastw;
	MRECT subnavrect;
	int SubButtonsSpacing, space_top_bot;
	SIZE size;
	int fontindex, textheight, nlines, panelheight, totheight;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;

	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	subnavrect.x1 = 0;
	subnavrect.x2 = id->m_nWidth;
	subnavrect.y1 = id->m_nHeight - 83;
	subnavrect.y2 = id->m_nHeight - 50;

	SubButtonsSpacing = 22;



	ApplicationsBarW = MGLAddPanel(id, ApplicationsBarW, "ApplicationsBarW", (GLfloat *)color454545, lastw,
		subnavrect.x1, subnavrect.x2, subnavrect.y1, subnavrect.y2);

	// add separator bars at top and bottom
	SubNavSeparatorTopW_A = MGLAddPanel(id, SubNavSeparatorTopW_A, "SubNavSeparatorTopW_A", (GLfloat *)color868686, ApplicationsBarW,
		subnavrect.x1, subnavrect.x2, subnavrect.y2, subnavrect.y2 - 1);

	//  white on bottom , then dark on line above it
	SubNavSeparatorDarkW_A = MGLAddPanel(id, SubNavSeparatorDarkW_A, "SubNavSeparatorDarkW_A", (GLfloat *)black, SubNavSeparatorTopW_A,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 1, subnavrect.y1);
	SubNavSeparatorBotW_A = MGLAddPanel(id, SubNavSeparatorBotW_A, "SubNavSeparatorBotW_A", (GLfloat *)color868686, SubNavSeparatorDarkW_A,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 2, subnavrect.y1 - 1);

	// we need to calculate text height to place the widget, so calculate it
	fontindex = 3;
	SetGUIFontIndex(id, fontindex, 1);
	CalcTextSize("PASSUR", &size);

	textheight = size.cy - id->m_guiFonts[fontindex].m_tmCustom.tmInternalLeading;



	// FlightPlanW
	nlines = 1;
	panelheight = subnavrect.y2 - subnavrect.y1;
	totheight = textheight * nlines;
	space_top_bot = panelheight - totheight;

	FindFlightTopW = MGLAddTextButton(id, FindFlightTopW, "FindFlightTopW", "Find Flight", SubNavSeparatorBotW_A,
		subnavrect.x1 + 280, subnavrect.y1 + (space_top_bot / 2), fontindex);
	//SearchGrid_T = CreateSearchGrid_T(id);


	FindAirportTopW = MGLAddTextButton(id, FindAirportTopW, "FindAirportTopW", "Find Airport", FindFlightTopW,
		FindFlightTopW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	AirlineLookupW = MGLAddTextButton(id, AirlineLookupW, "AirlineLookupW", "Airline Lookup", FindAirportTopW,
		FindAirportTopW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	GateConflictsW = MGLAddTextButton(id, GateConflictsW, "GateConflictsW", "Gate Conflicts", AirlineLookupW,
		AirlineLookupW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	RegionOfInterestW = MGLAddTextButton(id, RegionOfInterestW, "RegionOfInterestW", "Region Of Interest", GateConflictsW,
		GateConflictsW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);


}

void CreateSettingsSubNav(struct InstanceData* id)
{


	widgets *w, *lastw;
	MRECT subnavrect;
	int SubButtonsSpacing, space_top_bot;
	SIZE size;
	int fontindex, textheight, nlines, panelheight, totheight;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;

	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	subnavrect.x1 = 0;
	subnavrect.x2 = id->m_nWidth;
	subnavrect.y1 = id->m_nHeight - 83;
	subnavrect.y2 = id->m_nHeight - 50;

	SubButtonsSpacing = 22;



	SettingsBarW = MGLAddPanel(id, SettingsBarW, "SettingsBarW", (GLfloat *)color454545, lastw,
		subnavrect.x1, subnavrect.x2, subnavrect.y1, subnavrect.y2);

	// add separator bars at top and bottom
	SubNavSeparatorTopW_S = MGLAddPanel(id, SubNavSeparatorTopW_S, "SubNavSeparatorTopW_S", (GLfloat *)color868686, SettingsBarW,
		subnavrect.x1, subnavrect.x2, subnavrect.y2, subnavrect.y2 - 1);

	//  white on bottom , then dark on line above it
	SubNavSeparatorDarkW_S = MGLAddPanel(id, SubNavSeparatorDarkW_S, "SubNavSeparatorDarkW_S", (GLfloat *)black, SubNavSeparatorTopW_S,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 1, subnavrect.y1);
	SubNavSeparatorBotW_S = MGLAddPanel(id, SubNavSeparatorBotW_S, "SubNavSeparatorBotW_S", (GLfloat *)color868686, SubNavSeparatorDarkW_S,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 2, subnavrect.y1 - 1);

	// we need to calculate text height to place the widget, so calculate it
	fontindex = 3;
	SetGUIFontIndex(id, fontindex, 1);
	CalcTextSize("PASSUR", &size);

	textheight = size.cy - id->m_guiFonts[fontindex].m_tmCustom.tmInternalLeading;

	//*ALertsW, *ColorsW, *TagsW, *WorldMapW, *ARTCC_W, *SIDS_W, *STARS_W, *LocalTimeW, *LKP_W, *AllSurfaceW, *VehiclesW, *VehicleLabelW, RecordDurationW;

	// FlightPlanW
	nlines = 1;
	panelheight = subnavrect.y2 - subnavrect.y1;
	totheight = textheight * nlines;
	space_top_bot = panelheight - totheight;

	AlertsW = MGLAddTextButton(id, AlertsW, "AlertsW", "Alerts", SubNavSeparatorBotW_S,
		subnavrect.x1 + 280, subnavrect.y1 + (space_top_bot / 2), fontindex);


	ColorsW = MGLAddTextButton(id, ColorsW, "ColorsW", "Colors", AlertsW,
		AlertsW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	TagsW = MGLAddTextButton(id, TagsW, "TagsW", "Tags", ColorsW,
		ColorsW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	WorldMapW = MGLAddTextButton(id, WorldMapW, "WorldMapW", "World Map", TagsW,
		TagsW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	ARTCC_W = MGLAddTextButton(id, ARTCC_W, "ARTCC_W", "ARTCC", WorldMapW,
		WorldMapW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	SIDS_W = MGLAddTextButton(id, SIDS_W, "SIDS_W", "SIDS", ARTCC_W,
		ARTCC_W->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	STARS_W = MGLAddTextButton(id, STARS_W, "STARS_W", "STARS", SIDS_W,
		SIDS_W->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	LocalTimeW = MGLAddTextButton(id, LocalTimeW, "LocalTimeW", "Local Time", STARS_W,
		STARS_W->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	LKP_W = MGLAddTextButton(id, LKP_W, "LKP_W", "LKP", LocalTimeW,
		LocalTimeW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	AllSurfaceW = MGLAddTextButton(id, AllSurfaceW, "AllSurfaceW", "All Surface", LKP_W,
		LKP_W->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	VehiclesW = MGLAddTextButton(id, VehiclesW, "VehiclesW ", "Vehicles", AllSurfaceW,
		AllSurfaceW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	VehicleLabelW = MGLAddTextButton(id, VehicleLabelW, "VehicleLabelW", "Vehicle Label", VehiclesW,
		VehiclesW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);

	RecordDurationW = MGLAddTextButton(id, RecordDurationW, "RecordDurationW", "Record Duration", VehicleLabelW,
		VehicleLabelW->gregion.x2 + SubButtonsSpacing, subnavrect.y1 + (space_top_bot / 2), fontindex);





}

void CreateAirportsSubNav(struct InstanceData* id)
{


	widgets *w, *lastw;
	MRECT subnavrect;
	int SubButtonsSpacing, space_top_bot;
	SIZE size;
	int fontindex, textheight, nlines, panelheight, totheight;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;

	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np) {
		lastw = w;
	}

	subnavrect.x1 = 0;
	subnavrect.x2 = id->m_nWidth;
	subnavrect.y1 = id->m_nHeight - 83;
	subnavrect.y2 = id->m_nHeight - 50;

	SubButtonsSpacing = 22;



	AirportsBarW = MGLAddPanel(id, AirportsBarW, "AirportsBarW", (GLfloat *)color454545, lastw,
		subnavrect.x1, subnavrect.x2, subnavrect.y1, subnavrect.y2);

	// add separator bars at top and bottom
	SubNavSeparatorTopW_Ai = MGLAddPanel(id, SubNavSeparatorTopW_Ai, "SubNavSeparatorTopW_Ai", (GLfloat *)color868686, AirportsBarW,
		subnavrect.x1, subnavrect.x2, subnavrect.y2, subnavrect.y2 - 1);

	//  white on bottom , then dark on line above it
	SubNavSeparatorDarkW_Ai = MGLAddPanel(id, SubNavSeparatorDarkW_Ai, "SubNavSeparatorDarkW_Ai", (GLfloat *)black, SubNavSeparatorTopW_Ai,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 1, subnavrect.y1);
	SubNavSeparatorBotW_Ai = MGLAddPanel(id, SubNavSeparatorBotW_Ai, "SubNavSeparatorBotW_Ai", (GLfloat *)color868686, SubNavSeparatorDarkW_Ai,
		subnavrect.x1, subnavrect.x2, subnavrect.y1 - 2, subnavrect.y1 - 1);

	// we need to calculate text height to place the widget, so calculate it
	fontindex = 3;
	SetGUIFontIndex(id, fontindex, 1);
	CalcTextSize("PASSUR", &size);

	textheight = size.cy - id->m_guiFonts[fontindex].m_tmCustom.tmInternalLeading;



	// PASSUR Airport
	nlines = 1;
	panelheight = subnavrect.y2 - subnavrect.y1;
	totheight = textheight * nlines;
	space_top_bot = panelheight - totheight;

	PASSURAirportW = MGLAddTextButton(id, PASSURAirportW, "PASSURAirportW", "PASSUR Airport", SubNavSeparatorBotW_Ai,
		subnavrect.x1 + 280, subnavrect.y1 + (space_top_bot / 2), fontindex);




}




#ifdef OLDWAY
//imgxsize = 44;
//imgysize = 23;
//PassurButW = MGLAddButton(id, PassurButW, "passurbut", "passur-selected.png", "passur-off.png", SubNavSeparatorBotW, 
//	subnavrect.x1 + 280, subnavrect.x1 + 280 + imgxsize, subnavrect.y1 + 5, subnavrect.y1 + 5 + imgysize, ALPHA_MODE_BLACK, 1, GL_LINEAR );

//imgxsize = 26;
//imgysize = 23;
//ASDIButW = MGLAddButton(id, ASDIButW, "ASDIButW", "asdi-selected.png", "asdi-off.png", PassurButW, 
//	PassurButW->gregion.x2 + SubButtonsSpacing, PassurButW->gregion.x2 + SubButtonsSpacing + imgxsize, 
//	PassurButW->gregion.y1, PassurButW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST );
//imgxsize = 27;
//imgysize = 23;
//FlightPlanW = MGLAddButton(id, FlightPlanW, "FlightPlanW", "flight-plan-selected.png", "flight-plan-off.png", WxButW, 
//	WxButW->gregion.x2 + SubButtonsSpacing, WxButW->gregion.x2 + SubButtonsSpacing + imgxsize, 
//	WxButW->gregion.y1, WxButW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST );

imgxsize = 41;
imgysize = 23;
ASDEXButW = MGLAddButton(id, ASDEXButW, "ASDEXButW", "asdex-selected.png", "asdex-off.png", ASDIButW,
	ASDIButW->gregion.x2 + SubButtonsSpacing, ASDIButW->gregion.x2 + SubButtonsSpacing + imgxsize,
	ASDIButW->gregion.y1, ASDIButW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);


imgxsize = 33;
imgysize = 23;
ASDSBButW = MGLAddButton(id, ASDSBButW, "ASDSBButW", "adsb-selected.png", "adsb-off.png", ASDEXButW,
	ASDEXButW->gregion.x2 + SubButtonsSpacing, ASDEXButW->gregion.x2 + SubButtonsSpacing + imgxsize,
	ASDEXButW->gregion.y1, ASDEXButW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);

imgxsize = 38;
imgysize = 23;
AirwaysButW = MGLAddButton(id, AirwaysButW, "AirwaysButW", "airways-selected.png", "airways-off.png", ASDSBButW,
	ASDSBButW->gregion.x2 + SubButtonsSpacing, ASDSBButW->gregion.x2 + SubButtonsSpacing + imgxsize,
	ASDSBButW->gregion.y1, ASDSBButW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);



imgxsize = 42;
imgysize = 23;
WxButW = MGLAddButton(id, WxButW, "WxButW", "weather-selected.png", "weather-off.png", AirwaysButW,
	AirwaysButW->gregion.x2 + SubButtonsSpacing, AirwaysButW->gregion.x2 + SubButtonsSpacing + imgxsize,
	AirwaysButW->gregion.y1, AirwaysButW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);


imgxsize = 35;
imgysize = 23;
AircraftTrailsW = MGLAddButton(id, AircraftTrailsW, "AircraftTrailsW", "aircraft-trails-selected.png", "aircraft-trails-off.png", FlightPlanW,
	FlightPlanW->gregion.x2 + SubButtonsSpacing, FlightPlanW->gregion.x2 + SubButtonsSpacing + imgxsize,
	FlightPlanW->gregion.y1, FlightPlanW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);



/*  image missing for trail times
imgxsize = 35;
imgysize = 23;
TrailTimesW = MGLAddButton(id, TrailTimesW, "AircraftTrailsW", "aircraft-trails-selected.png", "aircraft-trails-off.png", AircraftTrailsW,
AircraftTrailsW->gregion.x2 + SubButtonsSpacing, AircraftTrailsW->gregion.x2 + SubButtonsSpacing + imgxsize,
AircraftTrailsW->gregion.y1, AircraftTrailsW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST );



imgxsize = 35;
imgysize = 23;
AircraftRingsW = MGLAddButton(id, AircraftRingsW, "AircraftRingsW", "aircraft-rings-selected.png", "aircraft-rings-off.png", AircraftTrailsW,
AircraftTrailsW->gregion.x2 + SubButtonsSpacing, AircraftTrailsW->gregion.x2 + SubButtonsSpacing + imgxsize,
AircraftTrailsW->gregion.y1, AircraftTrailsW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST );

imgxsize = 33;
imgysize = 23;
AirportStatusW = MGLAddButton(id, AirportStatusW, "AirportStatusW", "airport-status-selected.png", "airport-status-off.png", RangeRingsW,
RangeRingsW->gregion.x2 + SubButtonsSpacing, RangeRingsW->gregion.x2 + SubButtonsSpacing + imgxsize,
RangeRingsW->gregion.y1, RangeRingsW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST );
*/


imgxsize = 44;
imgysize = 23;
RunwaysW = MGLAddButton(id, RunwaysW, "RunwaysW", "runways-selected.png", "runways-off.png", AirportStatusW,
	AirportStatusW->gregion.x2 + SubButtonsSpacing, AirportStatusW->gregion.x2 + SubButtonsSpacing + imgxsize,
	AirportStatusW->gregion.y1, AirportStatusW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);


imgxsize = 41;
imgysize = 23;
TerminalFixesW = MGLAddButton(id, TerminalFixesW, "AirportStatusW", "terminal-fixes-selected.png", "terminal-fixes-off.png", RunwaysW,
	RunwaysW->gregion.x2 + SubButtonsSpacing, RunwaysW->gregion.x2 + SubButtonsSpacing + imgxsize,
	RunwaysW->gregion.y1, RunwaysW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);




imgxsize = 25;
imgysize = 23;
HighFixesW = MGLAddButton(id, HighFixesW, "AirportStatusW", "high-fixes-selected.png", "high-fixes-off.png", TerminalFixesW,
	TerminalFixesW->gregion.x2 + SubButtonsSpacing, TerminalFixesW->gregion.x2 + SubButtonsSpacing + imgxsize,
	TerminalFixesW->gregion.y1, TerminalFixesW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);

imgxsize = 25;
imgysize = 23;
LowFixesW = MGLAddButton(id, LowFixesW, "LowFixesW", "low-fixes-selected.png", "low-fixes-off.png", HighFixesW,
	HighFixesW->gregion.x2 + SubButtonsSpacing, HighFixesW->gregion.x2 + SubButtonsSpacing + imgxsize,
	HighFixesW->gregion.y1, HighFixesW->gregion.y1 + imgysize, ALPHA_MODE_BLACK, 1, GL_NEAREST);


#endif



void SetGridPos(struct guistruct* g)
{
	widgets* w, *gw;
	float height, pos;
	int n;

	assert(g);

	w = gw = g->widgets;

	// Get Height from tab
	height = (float)(gw->np->gregion.y2 - gw->np->gregion.y1);

	// Access Thumb
	for (n = 0; n < 4; n++)
		w = w->np;

	// Pos w-r-t bar
	pos = (w->gregion.y2 + w->gregion.y1) / 2.0f - gw->np->gregion.y1;

	gw->grid->iVscrollPos = (int)(20 - (pos / height * 20));
}






void InActivateAllSubNavs()
{
	widgets *w;
	int subnav;

	if (!SubNavStartWidgets[0] || !SubNavStartWidgets[1] || !SubNavStartWidgets[4] || !SubNavStartWidgets[5]) {
		SubNavStartWidgets[0] = ApplicationsBarW;
		SubNavStartWidgets[1] = OverlaysBarW;
		SubNavStartWidgets[2] = NULL;
		SubNavStartWidgets[3] = NULL;
		SubNavStartWidgets[4] = SettingsBarW;
		SubNavStartWidgets[5] = AirportsBarW;
		SubNavStartWidgets[6] = NULL;

		SubNavLastWidgets[0] = RegionOfInterestW;
		SubNavLastWidgets[1] = LowFixesW;
		SubNavLastWidgets[2] = NULL;
		SubNavLastWidgets[3] = NULL;
		SubNavLastWidgets[4] = RecordDurationW;
		SubNavLastWidgets[5] = PASSURAirportW;
		SubNavLastWidgets[6] = NULL;
	}

	for (subnav = 0; subnav < N_SUBNAVS; subnav++){
		for (w = SubNavStartWidgets[subnav]; w; w = w->np){
			w->visible = 0;
			if (w == SubNavLastWidgets[subnav]){
				break;
			}
		}
	}

}

void ActivateLayoutsSubNav()
{
	//widgets *w;


	// make overlays visible, make all other subnav invisible, like applications, etc
	InActivateAllSubNavs();

}




void ActivateApplicationsSubNav()
{
	widgets *w;
	int subnav_app;

	// make applications visible, make all other subnav invisible etc

	InActivateAllSubNavs();

	subnav_app = 0; // applications is index number 1
	for (w = SubNavStartWidgets[subnav_app]; w; w = w->np) {
		w->visible = 1;
		if (w == SubNavLastWidgets[subnav_app]) {
			break;
		}
	}

}


void ActivateFiltersSubNav()
{
	//widgets *w;
	//int count;

	// make overlays visible, make all other subnav invisible, like applications, etc

	InActivateAllSubNavs();

}

void ActivateAirportsSubNav()
{
	widgets *w;
	int subnav_ove;

	// make overlays visible, make all other subnav invisible, like applications, etc

	InActivateAllSubNavs();

	subnav_ove = 5; // overlays is index number 1
	for (w = SubNavStartWidgets[subnav_ove]; w; w = w->np) {
		w->visible = 1;
		if (w == SubNavLastWidgets[subnav_ove]) {
			break;
		}
	}
}


void ActivateSettingsSubNav()
{
	widgets *w;
	int subnav_ove;

	// make overlays visible, make all other subnav invisible, like applications, etc

	InActivateAllSubNavs();

	subnav_ove = 4; // overlays is index number 1
	for (w = SubNavStartWidgets[subnav_ove]; w; w = w->np) {
		w->visible = 1;
		if (w == SubNavLastWidgets[subnav_ove]) {
			break;
		}
	}

}


void ActivateOverlaysSubNav()
{
	widgets *w;
	int subnav_ove;

	// make overlays visible, make all other subnav invisible, like applications, etc

	InActivateAllSubNavs();

	subnav_ove = 1; // overlays is index number 1
	for (w = SubNavStartWidgets[subnav_ove]; w; w = w->np) {
		w->visible = 1;
		if (w == SubNavLastWidgets[subnav_ove]) {
			break;
		}
	}

}




int AirportsOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	extern void CreateAirportsSubNav(struct InstanceData* id);

	assert(id);

	if (!AirportsBarW) {
		CreateAirportsSubNav(id);
	}

	if (w->IsActive) {
		InActivateAllSubNavs();
		ActivateAirportsSubNav();
	} else {
		InActivateAllSubNavs();
	}
	return(TRUE);
}



int SettingsOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	extern void CreateSettingsSubNav(struct InstanceData* id);

	assert(id);

	if (!SettingsBarW) {
		CreateSettingsSubNav(id);
	}

	if (w->IsActive) {
		InActivateAllSubNavs();
		ActivateSettingsSubNav();
	} else {
		InActivateAllSubNavs();
	}
	return(TRUE);

}


int LayoutsOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	assert(id);

	//extern void CreateOverlaysSubNav(struct InstanceData* id );

	///if ( !FiltersBarW ){
	//	CreateFiltersSubNav(id);
	//}

	ActivateLayoutsSubNav();
	return(TRUE);
}


int FiltersOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	assert(id);

	//extern void CreateOverlaysSubNav(struct InstanceData* id );

	//if ( !FiltersBarW ){
	//	CreateFiltersSubNav(id);
	//}

	ActivateFiltersSubNav();
	return(TRUE);
}



int ApplicationsOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	extern void CreateApplicationsSubNav(struct InstanceData* id);

	assert(id);

	if (!ApplicationsBarW) {
		CreateApplicationsSubNav(id);
	}

	if (w->IsActive) {
		InActivateAllSubNavs();
		ActivateApplicationsSubNav();
	} else {
		InActivateAllSubNavs();
	}
	return(TRUE);
}


int OverlaysOnClickCB(struct InstanceData* id, widgets *w, int x, int y)
{
	extern void CreateOverlaysSubNav(struct InstanceData* id);

	assert(id);

	if (!OverlaysBarW){
		CreateOverlaysSubNav(id);
	}

	if (w->IsActive){
		ActivateOverlaysSubNav();
	}
	else {
		InActivateAllSubNavs();
	}


	return(TRUE);
}





void CreateTopGUI(struct InstanceData* id)
{
	widgets *w, *lastw;
	int bufsize, ystart;
	// int imgxsize, imgysize;
	int passurlogoleft, defaultrightpitm, totalspace, fontindex;
	struct row_index *row_index;
	char *arpt;
	static char lettername0[6], lettername1[6], lettername2[6];
	extern int IsValidAsdexAirport(struct InstanceData* id, char* strArpt);
	static int firsttime = TRUE;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	if (firsttime){
		BuildGUIFonts(id);
		BuildVerdanaFonts(id);
		SetGUIFontIndex(id, 2, 0);
		firsttime = FALSE;
	}

	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	TopBarW = AddImage(id, TopBarW, "TopBarBut", "top-bar-1px.png", lastw, 0, id->m_nWidth, id->m_nHeight - 50, id->m_nHeight,
		ALPHA_MODE_BLACK, 0, GL_NEAREST);


	// 155x38 is image size
	ystart = id->m_nHeight - 50 + 5; // 50 is height of bar, 5 is margin from bottom
	//WebTrackerTextW = MGLAddButton(id, WebTrackerTextW, "WebTrackerTextW", "webtrackertext.png", "webtrackertext.png", TopBarW, 5, 160, ystart, ystart + 38, 
	//	ALPHA_MODE_BLACK, 0, GL_NEAREST);

	// try new text for airport code 
	// put in airport three letter code, if there is one
	arpt = "N/A";
	if (id->m_sCurLayout.m_bShowPassurAircraft) {
		arpt = id->m_strPassurArpts[0];
	}
	else if (id->m_sCurLayout.m_bShowASDEXAircraft){
		if (IsValidAsdexAirport(id, id->m_strAsdexArpt)){
			arpt = id->m_strAsdexArpt;
		}
	}

	fontindex = 10;
	//AirportTextW = MGLAddTextButton( id, AirportTextW, "AirportTextW", arpt, TopBarW, 22, id->m_nHeight - 38, 15 );
	//AirportTextW->IsActive = TRUE;  // selected color is gold
	//AirportTextW->fontbase = id->m_guiFonts[fontindex].m_glListBase - 32;


	// try verdana font

	AirportText2W = MGLAddTextButton(id, AirportText2W, "AirportText2W", arpt, TopBarW, 22, id->m_nHeight - 38, fontindex);
	AirportText2W->IsActive = TRUE;  // selected color is gold
	AirportText2W->fontbase = id->m_verdanaFonts[fontindex].m_glListBase - 32;

	AirportText3W = MGLAddTextButton(id, AirportText3W, "AirportText3W", arpt, AirportText2W, 82, id->m_nHeight - 38, fontindex);
	AirportText3W->IsActive = TRUE;  // selected color is gold
	AirportText3W->fontbase = id->m_arFTFonts[fontindex].m_glListBase;


	if (id->m_pAirportsVO && (row_index = vo_search((VO*)id->m_pAirportsVO, "passur_code", arpt, NULL))){
		arpt = VP((VO *)id->m_pAirportsVO, row_index->rownum, "icao_code", char);
		//strcpy_s(AirportTextW->textbuf, sizeof(AirportTextW->textbuf), arpt);
		//StrUpper( AirportTextW->textbuf );
		strcpy_s(AirportText2W->textbuf, sizeof(AirportTextW->textbuf), arpt);
		StrUpper(AirportText2W->textbuf);

		strcpy_s(AirportText3W->textbuf, sizeof(AirportTextW->textbuf), arpt);
		StrUpper(AirportText3W->textbuf);
	}

	ILightW = AddImage(id, ILightW, "ILightW", "green-light.png", AirportText3W, 176, 188, id->m_nHeight - 40, id->m_nHeight - 40 + 12,
		ALPHA_MODE_BLACK, 1, GL_NEAREST);
	ILightsTextW = AddText(id, ILightsTextW, "ILightsText", "I", ILightW, ILightW->gregion.x1 + 6, ILightW->gregion.x1 + 12,
		id->m_nHeight - 22, id->m_nHeight - 22 + 12);

	XLightW = AddImage(id, XLightW, "XLightW", "red-light.png", ILightsTextW,
		ILightW->gregion.x1 + NET_LIGHTS_X_SPACE, ILightW->gregion.x1 + NET_LIGHTS_X_SPACE + 12, id->m_nHeight - 40, id->m_nHeight - 40 + 12,
		ALPHA_MODE_BLACK, 1, GL_NEAREST);
	XLightsTextW = AddText(id, XLightsTextW, "XLightsText", "X", XLightW, XLightW->gregion.x1 + 3, XLightW->gregion.x1 + 12,
		id->m_nHeight - 22, id->m_nHeight - 22 + 12);

	BLightW = AddImage(id, BLightW, "BLightW", "off-light.png", XLightsTextW,
		XLightW->gregion.x1 + NET_LIGHTS_X_SPACE, XLightW->gregion.x1 + NET_LIGHTS_X_SPACE + 12, id->m_nHeight - 40, id->m_nHeight - 40 + 12,
		ALPHA_MODE_BLACK, 1, GL_NEAREST);
	BLightsTextW = AddText(id, BLightsTextW, "BLightsText", "B", BLightW, BLightW->gregion.x1 + 3, BLightW->gregion.x1 + 12,
		id->m_nHeight - 22, id->m_nHeight - 22 + 12);

	//DbgOutputWidgets( WebTrackerTextW );
	LeftEdgeW = AddEdge(id, LeftEdgeW, "leftedge", "left-edge.png", BLightsTextW, BLightsTextW->gregion.x2 + NET_LIGHTS_X_SPACE);


	//DbgOutputWidgets( WebTrackerTextW );

	// calculate rough size of free space between buttons and the passur logo
	passurlogoleft = id->m_nWidth - 81 - 5 - 23;
	defaultrightpitm = 710;

	totalspace = passurlogoleft - defaultrightpitm;
	// add in some space between left and right log
	if (totalspace > 500){
		totalspace -= 200;
	}
	else if (totalspace > 250) {
		totalspace -= 150;
	}
	else if (totalspace > 100) {
		totalspace -= 50;
	}
	bufsize = totalspace / 12;

	// AppButtonW actually points to the first buffer before the button  LeftBufW->ButW->RightBufW
	// 
	//AppButtonBufW = AddNextButton(id, AppButtonBufW, "ApplicationsButton", "applications-selected.png", "applications-on.png", 66, LeftEdgeW, bufsize );
	//AppButtonBufW->OnClickCB = ApplicationsOnClickCB;

	AppButtonBufW = AddNextTextButton(id, AppButtonBufW, "ApplicationsButton", "Applications", LeftEdgeW, bufsize);
	AppButtonBufW->OnClickCB = ApplicationsOnClickCB;

	//DbgOutputWidgets( WebTrackerTextW );

	// no need to keep copies to all separator widgets, just use button->np
	SeparatorW = AddButSeparator(id, AppButtonBufW->np->np, "separatorWidget", AppButtonBufW->np, AppButtonBufW);


	OverlaysButtonBufW = AddNextTextButton(id, OverlaysButtonBufW, "OverlaysButtonBufW", "Overlays", SeparatorW, bufsize);
	OverlaysButtonBufW->OnClickCB = OverlaysOnClickCB;


	//OverlaysButtonBufW = AddNextButton(id, OverlaysButtonBufW, "OverlaysButtonBufW", "overlays-selected.png", "overlays-off.png", 46, SeparatorW, bufsize );
	//OverlaysButtonBufW->OnClickCB = OverlaysOnClickCB;

	//DbgOutputWidgets( AppButtonBufW );

	SeparatorW = AddButSeparator(id, OverlaysButtonBufW->np->np, "separatorWidget", OverlaysButtonBufW->np, OverlaysButtonBufW);

	FiltersButtonBufW = AddNextTextButton(id, FiltersButtonBufW, "FiltersButtonBufW", "Filters", SeparatorW, bufsize);
	//FiltersButtonBufW = AddNextButton(id, FiltersButtonBufW, "filtersbut", "filters-selected.png", "filters-off.png", 33, SeparatorW, bufsize );
	FiltersButtonBufW->OnClickCB = FiltersOnClickCB;

	SeparatorW = AddButSeparator(id, FiltersButtonBufW->np->np, "separatorWidget", FiltersButtonBufW->np, FiltersButtonBufW);
	LayoutsButtonBufW = AddNextTextButton(id, LayoutsButtonBufW, "LayoutsButtonBufW", "Layouts", SeparatorW, bufsize);
	// LayoutsButtonBufW = AddNextButton(id, LayoutsButtonBufW, "layoutsbut", "layouts-selected.png", "layouts-off.png", 41, SeparatorW, bufsize );
	LayoutsButtonBufW->OnClickCB = LayoutsOnClickCB;

	SeparatorW = AddButSeparator(id, LayoutsButtonBufW->np->np, "separatorWidget", LayoutsButtonBufW->np, LayoutsButtonBufW);
	SettingsButtonBufW = AddNextTextButton(id, SettingsButtonBufW, "SettingsButtonBufW", "Settings", SeparatorW, bufsize);
	// SettingsButtonBufW = AddNextButton(id, SettingsButtonBufW, "settingsbut", "settings-selected.png", "settings-off.png", 43, SeparatorW, bufsize );
	SettingsButtonBufW->OnClickCB = SettingsOnClickCB;

	SeparatorW = AddButSeparator(id, SettingsButtonBufW->np->np, "separatorWidget", SettingsButtonBufW->np, SettingsButtonBufW);
	AirportsButtonBufW = AddNextTextButton(id, AirportsButtonBufW, "AirportsButtonBufW", "Airports", SeparatorW, bufsize);
	// AirportsButtonBufW = AddNextButton(id, AirportsButtonBufW, "airportsbut", "airports-selected.png","airports-off.png", 43, SeparatorW, bufsize );
	AirportsButtonBufW->OnClickCB = AirportsOnClickCB;

	// no separator on last widget lastw = AddButSeparator(id, "separatorWidget", lastw );
	//DbgOutputWidgets( AppButtonBufW );
	RightEdgeW = AddEdge(id, RightEdgeW, "RightEdgeW", "right-edge.png", AirportsButtonBufW->np, AirportsButtonBufW->gregion.x2);
	//DbgOutputWidgets( AppButtonBufW );

	ReplayButW = MGLAddButton(id, ReplayButW, "replaybut", "replay-on.png", "replay-off.png", RightEdgeW, RightEdgeW->gregion.x2 + 30, RightEdgeW->gregion.x2 + 76,
		id->m_nHeight - 40, id->m_nHeight - 40 + 25, ALPHA_MODE_BLACK, 1, GL_LINEAR);

	//DbgOutputWidgets( AppButtonBufW );
	// pitm image is 57x20
	PITMButW = MGLAddButton(id, PITMButW, "pitmbut", "PITM-on.png", "PITM-off.png", ReplayButW, ReplayButW->gregion.x2 + 30, ReplayButW->gregion.x2 + 30 + 57,
		id->m_nHeight - 40, id->m_nHeight - 40 + 20, ALPHA_MODE_BLACK, 1, GL_LINEAR);

	//DbgOutputWidgets( AppButtonBufW );
	// for some reason, the m_nWidth is 23 pixels wider than what the user sees on the screen, so subtract additional 23
	// image width is 81x24
	PassurLogoW = AddImage(id, PassurLogoW, "passurbut", "passur-logo.png", PITMButW, id->m_nWidth - 81 - 5 - 23, id->m_nWidth - 5 - 23,
		id->m_nHeight - 40, id->m_nHeight - 40 + 24, ALPHA_MODE_BLACK, 1, GL_LINEAR);

	//DbgOutputWidgets( AppButtonBufW );


	MGLMakeRadioButtons(id, AppButtonBufW, OverlaysButtonBufW, FiltersButtonBufW, LayoutsButtonBufW, SettingsButtonBufW, AirportsButtonBufW, NULL);
	id->m_poglgui = oglgui;

}


void FillFDODiversions(struct InstanceData* id, MGLGRID *FDOGrid)
{


	// retrieve all diversion information and put it into the vo


	// see MergeFDOVOFromDMXVO and DMXVO 
}



int UpdateFDO(struct InstanceData* id, MGLGRID *FDOGrid)
{
	extern void CrFilteredGrid(struct InstanceData* id, MGLGRID *parentgrid);
	extern void FillFDOVOFromAir(struct InstanceData *id, MGLGRID *FDOGrid);
	extern int 	FillFDOFromGFDO(struct InstanceData *id, MGLGRID *FDOGrid);

	assert(id);

	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// FillFDOVO(id, FDOGrid);
	// FillFDOVOFromAir(id, FDOGrid);
	FillFDOFromGFDO(id, FDOGrid);
	// not yet implemented FillFDODiversions(id, FDOGrid);
	// FillFDOVO(id, FDOGrid );
	MGLSetColNamesFromVO(FDOGrid);
	CrFilteredGrid(id, FDOGrid);
	MGLSortFilteredGrid(FDOGrid);
	CalcColWidths(id, FDOGrid);
	SecondTabGrid->aircraftcolor_change_MLAT = 1;
	SecondTabGrid->aircraftcolor_change_ASDEX = 1;
	SecondTabGrid->aircraftcolor_change_Noise = 1;
	SecondTabGrid->aircraftcolor_change_Gate = 1;
	SecondTabGrid->aircraftcolor_change_ASD = 1;
	SecondTabGrid->aircraftcolor_change = 1;
	return(TRUE);
}

int UpdateSA(struct InstanceData* id, MGLGRID *SysAlertsGrid)
{
	
	extern int 	FillSysAlertsFromGSA(struct InstanceData *id, MGLGRID *SysAlertsGrid);

	assert(id);

	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	FillSysAlertsFromGSA(id, SysAlertsGrid);
	MGLSetColNamesFromVO(SysAlertsGrid);
	CrFilteredGrid(id, SysAlertsGrid);
	MGLSortFilteredGrid(SysAlertsGrid);
	CalcColWidths(id, SysAlertsGrid);
	/*SecondTabGrid->aircraftcolor_change_MLAT = 1;
	SecondTabGrid->aircraftcolor_change_ASDEX = 1;
	SecondTabGrid->aircraftcolor_change_Noise = 1;
	SecondTabGrid->aircraftcolor_change_Gate = 1;
	SecondTabGrid->aircraftcolor_change_ASD = 1;
	SecondTabGrid->aircraftcolor_change = 1;*/
	return(TRUE);
}
//mtm to highlight selected aircraft
int ROICallback(struct InstanceData* id, MGLGRID *Grid, int b, int m, int x, int y)
{
	// Toggle the selected action On or OFF
	// allow user to add new actions;

	VO *vo = NULL, *vo1;
	int r, vorow, i,j, flightindex;
	char *FltNum;
	// *Region, *Flightnum;
	int count1 = 0;
	int count2 = 0;
	int co = 0;
	char *DataSource;
	VO *DataAppVO;
	MGLGRID *ROIGrid = NULL;

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	
	//finding the row number//
	if (!Grid) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (Grid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - Grid->gridrect.y1) / Grid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - Grid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= Grid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = Grid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + Grid->iVscrollPos;

	//vo = DataAppGrid->filteredvo;

	if (Grid->filteredvo && Grid->filteredvo->count){
		vo = Grid->filteredvo;
	}
	else if (Grid->vo  && Grid->vo->count){
		vo = Grid->vo;
	}
	else {
		// no rows yet, so exit
		return(FALSE);
	}


	///extracting the flight ID

	// FltNum = VP(vo, vo->row_index[vorow].rownum, "FltNum", char);  // this defaults to uniqueid since FltNum colname not in grid, not what we wanted , use flightid
	FltNum = VP(vo, vo->row_index[vorow].rownum, "flightid", char);
	//  Region = VP(vo, vo->row_index[vorow].rownum, "Region", char);


#ifdef OLDWAY
	while (FltNum[co] != '\0'){
		count1++;
		co++;
	}

	co = 0;

	while (Region[co] != '\0'){
		if (Region[co] == '-'){
			count2--;
		}
		if (Region[co] == ' ')
		{
			count2--;
		}

		count2++;
		co++;
	}


	Flightnum = (char *)malloc(count1 - count2+2);

	for (i = 0; i < (count1 - count2 + 1); i++)
		Flightnum[i] = 0;

	for (i = 0; i < (count1 - count2); i++)
	{
		Flightnum[i] = FltNum[i];

	}

	Flightnum[count1 - count2 + 1] = '\0';
#endif

	flightindex = FindUserFlight(id, FltNum);//to highlight the flight

	vo1 = Grid->vo;
	for (i = 0; i < vo1->count; i++)
	{
		vo1->row_index[i].grid_fg = 0;

	}

	if (flightindex >= 0)
	{
		vo1->row_index[vorow].grid_fg = 1;
		strcpy_s(Grid->Flightnum, 20, FltNum);
		strcpy_s(id->m_selectedaircraft, 20, Grid->Flightnum);
		for (i = 0; DataAppGrid && (DataAppVO = DataAppGrid->vo) && i < DataAppVO->count; i++){
			DataSource = VP(DataAppVO, i, "DataSource", char);
			if (!strcmp(DataSource, "ROI")){
				ROIGrid = VV(DataAppGrid->vo, i, "gridptr", void *);
				strcpy_s(ROIGrid->Flightnum, 20, FltNum);				
			}
					
		}
	
		SecondTabGrid->aircraftcolor_change_MLAT = 1;
		SecondTabGrid->aircraftcolor_change_ASDEX = 1;
		SecondTabGrid->aircraftcolor_change_Noise = 1;
		SecondTabGrid->aircraftcolor_change_Gate = 1;
		SecondTabGrid->aircraftcolor_change_ASD = 1;
		SecondTabGrid->aircraftcolor_change = 1;

		if (Grid->icon_set == 1)
			id->m_selectedaircraftcolor = Grid->AlertsColorGrid->IconColorGrid->icon_color;
		else
			id->m_selectedaircraftcolor = 4;

		for (j = 0; j < vo->count; j++)
		{
			if (j == vorow)
				vo->row_index[j].fg = MGLColorHex2Int("529EFD");
			else
				vo->row_index[j].fg = MGLColorHex2Int("FFFFFF");
		}
	}



	// free(Flightnum);


	if (m == 1){
		// ignore mouse down, only activate on mouse up
		return(FALSE);
	}

	id->DataAppGrid = DataAppGrid;
	// To be done
	return(TRUE);

}


#ifdef NOT_NEEDED
//mtm-color vorow in roi when an icon(flight) is selected on the map
int ColorVorow_iconselected(struct InstanceData* id, char *FltNum1, char *destination )
{
	int i;
	char *DataSource;
	VO *DataAppVO;
	MGLGRID *ROIGrid = NULL;
	MGLGRID *FDOGrid = NULL;

	//id->m_pSelectedAircraft=
	//id->m_nSelectedTrackID=;
	//id->m_nSelectedAirIndex=;

	for (i = 0; DataAppGrid && (DataAppVO = DataAppGrid->vo) && i < DataAppVO->count; i++){
		DataSource = VP(DataAppVO, i, "DataSource", char);
		if (!strcmp(DataSource, "ROI")){
			ROIGrid = VV(DataAppGrid->vo, i, "gridptr", void *);	
			if (strcmp(ROIGrid->Flightnum, FltNum1) != 0)
			{
				strcpy_s(ROIGrid->Flightnum,20, FltNum1);
				CrFilteredGrid(id, ROIGrid);
			}
			strcpy_s(ROIGrid->Flightnum,20, FltNum1);
			ROIGrid->mm = 3;
		}
		else if (!strcmp(DataSource, "FDO"))
		{
			FDOGrid = VV(DataAppGrid->vo, i, "gridptr", void *);
			if (strcmp(FDOGrid->Flightnum, FltNum1) != 0)
			{
				strcpy_s(FDOGrid->Flightnum,20,FltNum1);
				//   mhm xxxxxxxxxxxxxxx  this kills system and causes white screen  if (FDOGrid->parent_w->visible)
				//  	UpdateFDO(id, FDOGrid);
			}
			strcpy_s(FDOGrid->Flightnum,20,FltNum1);
			FDOGrid->mm = 3;

			coloraircraft(struct InstanceData* id, char* flightnum, char *destination);
			
		}

	}

	return 1;
	// To be done


}

#endif


int FDOCallback(struct InstanceData* id, MGLGRID *FDOGrid, int b, int m, int x, int y)
{
	// Toggle the selected action On or OFF
	// allow user to add new actions;

	//modified mtm

	VO *vo;
	int r, vorow, flightindex,i,j;
	char *FltNum;
	char *DataSource;
	VO *DataAppVO;
	MGLGRID *Grid;
	char savefltnum[FLTNUM_SIZE];

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	
	//to find the row in FDO grid which is selected
	if (!FDOGrid) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (FDOGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - FDOGrid->gridrect.y1) / FDOGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - FDOGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}
	// toggle the highlight on or off
	if (r < 0 || r >= FDOGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = FDOGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + FDOGrid->iVscrollPos;

	if (FDOGrid->filteredvo && FDOGrid->filteredvo->count){
		vo = FDOGrid->filteredvo;
	}
	else if (FDOGrid->vo  && FDOGrid->vo->count){
		vo = FDOGrid->vo;
	}
	else {
		return(FALSE);
	}

	if ( vorow >= vo->count){
		return(FALSE); // dont core dump
	}
	vo = vo->row_index[vorow].vo;
	FltNum = VP(vo, vo->row_index[vorow].rownum, "FltNum", char);//querying the flight id

	// we need to save FltNum, because calls to Update FDO below might invalidate FltNum pointer into the vo
	savefltnum[0] = '\0';
	if (FltNum && strlen(FltNum) < FLTNUM_SIZE){
		strcpy_s(savefltnum, FLTNUM_SIZE, FltNum);
	}
	else {
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) bad fltnum\n");
		return(FALSE);
	}
	flightindex = FindUserFlight(id, savefltnum);//to highlight the selected flight

	if (flightindex >= 0)
	{

		for (i = 0; DataAppGrid && (DataAppVO = DataAppGrid->vo) && i < DataAppVO->count - 1; i++){
			DataSource = VP(DataAppVO, i, "DataSource", char);
			Grid = VV(DataAppGrid->vo, i, "gridptr", void *);
			if (Grid->roi_fdo == 2)
			{
				strcpy_s(Grid->Flightnum, 20, savefltnum);
				strcpy_s(id->m_selectedaircraft, 20, Grid->Flightnum);
				SecondTabGrid->aircraftcolor_change_MLAT = 1;
				SecondTabGrid->aircraftcolor_change_ASDEX = 1;
				SecondTabGrid->aircraftcolor_change_Noise = 1;
				SecondTabGrid->aircraftcolor_change_Gate = 1;
				SecondTabGrid->aircraftcolor_change_ASD = 1;
				SecondTabGrid->aircraftcolor_change = 1;
				//if (Grid->parent_w->visible == 1)
				//	vo->row_index[vorow].fg = MGLColorHex2Int("529EFD");
			}
		}
		if (FDOGrid->icon_set == 1)
			id->m_selectedaircraftcolor = FDOGrid->AlertsColorGrid->IconColorGrid->icon_color;
		else
			id->m_selectedaircraftcolor = 4;
		for (j = 0; j < vo->count; j++)
		{
			if (j == vorow)
				vo->row_index[j].fg = MGLColorHex2Int("529EFD");
			else
				vo->row_index[j].fg = MGLColorHex2Int("FFFFFF");
		}
	}


	/*for (i = 0; i < vo->count; i++)
	{
	vo->row_index[i].fg= MGLColorHex2Int("000000");

	}
	if (flightindex >= 0)
	vo->row_index[vorow].fg = MGLColorHex2Int("529EFD");*/

	id->DataAppGrid = DataAppGrid;
	// To be done
	return(TRUE);

}

int fdo_grid_text_cb(struct InstanceData* id, widgets *w)
{
	assert(id);
	assert(w);

	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, id->m_strAirportMap, _TRUNCATE);
	return(TRUE);
}




MGLGRID *CreateFDOGrid(struct InstanceData* id)
{
	widgets *w, *lastw, *panel_widget, *prevw;
	int r, i, col;
	MGLGRID *FDOGrid;

	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "FDOGrid");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;


	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;



	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "FDOWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = MAX(id->m_nHeight - 660, 75);
	w->gregion.x2 = 400;  //  N.A.
	w->gregion.y2 = id->m_nHeight;  //  N.A.
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "FDOGrid", 16, 60, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = TRUE;
	w->grid->edit = FALSE;//to display the rules-layout icon
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	FDOGrid = w->grid;
	FDOGrid->UpdateCB = UpdateFDO;
	FDOGrid->roi_fdo = 2;




	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)Text_yellow;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey3;
	}


	w->grid->flag1 = 1;//to enable mglgridmove for fdo grid

	w->grid->gridrows[0].bg = (GLfloat*)Grid_grey2;
	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	//  strncpy_s( w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Available" , _TRUNCATE);

	MGLSetColNamesFromVO(w->grid);
	// set unique key for this grid to be used for alerts, and to avoid dup alerts

	strncpy_s(w->grid->KeyFields[0], MGL_GRID_LABEL_MAX, "FltNum", _TRUNCATE); // keyfields must match vcol name, not label

	col = 0;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Flight ID", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Origin", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Destination", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "A/C Type (FAA)", _TRUNCATE); col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ETOn (PASSUR)", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ATOn (PASSUR)", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "STD", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ETD", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ATOff (PASSUR)", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "STA", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Speed", _TRUNCATE); col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "N Miles", _TRUNCATE);
	// make n miles hide fractional parts
	strncpy_s(w->grid->gridcols[col].display_fmt, 32, "%1.0f", _TRUNCATE); col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Beacon", _TRUNCATE);
	strncpy_s(w->grid->gridcols[col].display_fmt, 32, "%4o", _TRUNCATE); col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Altitude", _TRUNCATE); col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ATD (Carrier)", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ATOff (Carrier)", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ATOn (Carrier)", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "ATA (Carrier)", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Gate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "D ETA", _TRUNCATE);
	//w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "D STA", _TRUNCATE);
	//w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "A/C Type (Carrier)", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Drag Off", _TRUNCATE);
//	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Pax Closeout", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Dep Gate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "PrevPubDepGate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "prevpubarrgate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "previntdprtgate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "previntarrgate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "defaultdprtgateind", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Arr Gate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "arr_gate_time", _TRUNCATE);
	//w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "dprt_gate_time", _TRUNCATE);
	//w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "default_arr_gate_ind", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "lst_upd_time", _TRUNCATE);
	//w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Tbfm_Offtime", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Tbfm_Ontime", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Tbfm_Freezetime", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Cargo_Door_Clsd", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Cargo_Door_Open", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Pax_Door_Clsd", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Pax_Door_Open", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Bridge_On_Time", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Crew_Out_Time", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Brake_Released", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Brake_Set", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;


	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Dep_Gate", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Arr_Gate", _TRUNCATE); col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Acars_Init", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-msgTime", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-old_fid", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-A/C Flt Pln Status", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-A/C Current Status", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-STA Freeze Flag", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-Rwy Freeze Flag", _TRUNCATE); col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-rwy_freeze", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-A/C Type", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-TRACON Asgn Rwy", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-Arr Runway", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-EDCT Status", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-EDCT", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-ATOff", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-STA at Rwy", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM-ETA at Rwy", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Assigned Gate Status", _TRUNCATE); col++;



	//10/12/2016

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM Time", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "TBFM Status", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Boarding Started", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Pax Door Closed", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Ready to Depart", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++; 
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "onlinetsgmt", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "draggateid", _TRUNCATE); col++;

	// 11/1/2016
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Flt_at_Gate", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "Gt_Exp_Clr_Time", _TRUNCATE); 
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS; col++;

	//

	w->grid->flag1 = 1;//to enable mglgridmove for fdo grid


	//column order names//
	col = 0;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Flight", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Origin", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Destination", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Type", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "ETA", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "ATA", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "STD", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "ETD", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "ATD", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "STA", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Speed", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "N Miles", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Beacon", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "altitude", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Out", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Off", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "On", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "In", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Gate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "D ETA", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "D STA", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "D ACTYPE", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Drag Off", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Psgr Closeout", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "IntDepGate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "PrevPubDepGate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "prevpubarrgate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "previntdprtgate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "previntarrgate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "defaultdprtgateind", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "int_arr_gate", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "arr_gate_time", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "dprt_gate_time", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "default_arr_gate_ind", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "lst_upd_time", _TRUNCATE); col++;

	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "tbfm_offtime", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "tbfm_ontime", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "tbfm_freezetime", _TRUNCATE); col++;

	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "cargo_door_clsd", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "cargo_door_open", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "pax_door_clsd", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "pax_door_open", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "bridge_on_time", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "crew_out_time", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "brake_released", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "brake_set", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "dep_gate", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "arr_gate", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "acars_init", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "first_msg", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "old_fid", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "fps", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "acs", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "sfzflag", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "rfzflag", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "rwy_freeze", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "actype", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "tracon_rwy", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "arr_rwy", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "edc_status", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "edc_time", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "tbfm_etd", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "sta_at_rwy", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "eta_at_rwy", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "Assigned Gate Status", _TRUNCATE); col++;


	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "estdprtctrlgmtts", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "estdprtctrlgmttssrc", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "brdngstrtdgmtts", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "paxdrclsgmtts", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "readytodprtgmtts", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "onlinetsgmt", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "draggateid", _TRUNCATE); col++;
	
	

	// we also need to fetch initial filter values from user settings DB

	// set row callbacks
	// do not set the row callback for row 0, since that is for the headers callback
	for (i = 1; i < FDOGrid->nrows; i++){
		FDOGrid->gridrows[i].row_callback = FDOCallback;
	}


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	FDOGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = FDOGrid->marginx;
	w->gregion.y1 = FDOGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - FDOGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Flight Data", _TRUNCATE);
	w->visible = 1;
	w->grid = FDOGrid;
	prevw = w;

	//// create text widget for Displaying the Current Airport
	w->np = (widgets *)vo_malloc(sizeof(widgets));
	w = w->np;
	strcpy_s(w->name, sizeof(w->name), "ArptTextWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = prevw->gregion.x2+50;
	w->gregion.y1 = FDOGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 45;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - FDOGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_airport_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, id->m_strAirportMap, _TRUNCATE);
	w->visible = 1;
	w->grid =FDOGrid;
	w->wgt_text_cb = fdo_grid_text_cb;
	prevw = w;

	// create text widget for Displaying the current configuration layout (i.e. ARRIVALS, ELAPSED, etc)
	w->np = (widgets *)vo_malloc(sizeof(widgets));
	w = w->np;
	strcpy_s(w->name, sizeof(w->name), "ConfigTextWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = prevw->gregion.x2;
	w->gregion.y1 = FDOGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 100;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - FDOGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_layout_cb;
	//strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "ARRIVALS", _TRUNCATE);
	//w->grid->heading_callback = MGLHeadersCallback;
	w->visible = 1;
	w->grid = FDOGrid;

	//FDOGrid->nrows = 15;
	FDOGrid->active_icon=1;
	FDOGrid->fixed_width = 0;
	id->m_poglgui = oglgui;

	return(FDOGrid);
}



MGLGRID *CreateSysAlertsGrid(struct InstanceData* id)
{
	widgets *w, *lastw, *panel_widget, *prevw;
	int r, i, col;
	MGLGRID *SysAlertsGrid;

	gui* oglgui = (gui*)id->m_poglgui;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SysAlertsGrid");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;


	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;



	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "SysAlertsWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = MAX(id->m_nHeight - 660, 75);
	w->gregion.x2 = 400;  //  N.A.
	w->gregion.y2 = id->m_nHeight;  //  N.A.
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "SysAlertsGrid", 16, 9, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = TRUE;
	w->grid->edit = FALSE;//to display the rules-layout icon
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	SysAlertsGrid = w->grid;
	//FillSysAlertsFromGSA(id, SysAlertsGrid);
	SysAlertsGrid->UpdateSACB = UpdateSA;
	SysAlertsGrid->roi_fdo = 3;




	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)Text_yellow;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey3;
	}


	w->grid->flag1 = 1;//to enable mglgridmove for fdo grid

	w->grid->gridrows[0].bg = (GLfloat*)Grid_grey2;
	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	//  strncpy_s( w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Available" , _TRUNCATE);

	MGLSetColNamesFromVO(w->grid);
	// set unique key for this grid to be used for alerts, and to avoid dup alerts

	/*strncpy_s(w->grid->KeyFields[0], MGL_GRID_LABEL_MAX, "Fl", _TRUNCATE); // keyfields must match vcol name, not label*/

	col = 0;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "seqnum", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "appname", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "username", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "alertname", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "priority", _TRUNCATE); col++;
	

	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "starttime", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "endtime", _TRUNCATE);
	w->grid->gridcols[col].val_callback = CnvSecs2HHMMSS;
	w->grid->gridcols[col].display_data_type = DATETIMEBIND; col++;
	
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "comments", _TRUNCATE); col++;
	strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "alertsource", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColLabels[col], MGL_GRID_LABEL_MAX, "keyvalues", _TRUNCATE); col++;

	w->grid->flag1 = 1;//to enable mglgridmove for fdo grid


	//column order names//
	col = 0;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "seqnum", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "appname", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "username", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "alertname", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "priority", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "starttime", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "endtime", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "comments", _TRUNCATE); col++;
	strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "alertsource", _TRUNCATE); col++;
	//strncpy_s(w->grid->ColOrderNames[col], MGL_GRID_LABEL_MAX, "keyvalues", _TRUNCATE); col++;
	
	

	// we also need to fetch initial filter values from user settings DB

	// set row callbacks
	// do not set the row callback for row 0, since that is for the headers callback
	/*for (i = 1; i < FDOGrid->nrows; i++){
		FDOGrid->gridrows[i].row_callback = FDOCallback;
	}*/


	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	SysAlertsGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = SysAlertsGrid->marginx;
	w->gregion.y1 = SysAlertsGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - SysAlertsGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Flight Data", _TRUNCATE);
	w->visible = 1;
	w->grid = SysAlertsGrid;
	prevw = w;

	///create text widget for Displaying the Current Airport
	w->np = (widgets *)vo_malloc(sizeof(widgets));
	w = w->np;
	strcpy_s(w->name, sizeof(w->name), "ArptTextWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = prevw->gregion.x2 + 50;
	w->gregion.y1 = SysAlertsGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 45;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - SysAlertsGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_airport_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, id->m_strAirportMap, _TRUNCATE);
	w->visible = 1;
	w->grid = SysAlertsGrid;
	w->wgt_text_cb = fdo_grid_text_cb;
	prevw = w;

	// create text widget for Displaying the current configuration layout (i.e. ARRIVALS, ELAPSED, etc)
	w->np = (widgets *)vo_malloc(sizeof(widgets));
	w = w->np;
	strcpy_s(w->name, sizeof(w->name), "ConfigTextWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = prevw->gregion.x2;
	w->gregion.y1 = SysAlertsGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 100;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - SysAlertsGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_layout_cb;
	//strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "ARRIVALS", _TRUNCATE);
	//w->grid->heading_callback = MGLHeadersCallback;
	w->visible = 1;
	w->grid = SysAlertsGrid;

	//FDOGrid->nrows = 15;
	SysAlertsGrid->active_icon = 1;
	SysAlertsGrid->fixed_width = 0;
	id->m_poglgui = oglgui;

	return(SysAlertsGrid);
}











int DataTypeMenuCallback(struct InstanceData* id, MGLGRID *DataTypeMenuGrid, int b, int m, int x, int y)
{
	// pop up one of the GUI settings grids
	int r;
	//	MGLGRID *FDOGrid, *ROIGrid;
	int addnewrow = FALSE;
	extern MGLGRID *CreateROIGrid(struct InstanceData* id);
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;

	assert(id);

	if (DataTypeMenuGrid->gridrows[0].height != MGL_DEFAULT){
		r = (int)floor(((double)y - DataTypeMenuGrid->gridrect.y1) / DataTypeMenuGrid->gridrows[0].height);   // only works if rows all have same height
	}
	else {
		r = (int)floor(((double)y - DataTypeMenuGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}

	if (r < 0 || r >= DataTypeMenuGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = DataTypeMenuGrid->nrows - r - 1;

	// find "Choose" row in DataAppGrid
	if (!DataAppCreater || !DataAppCreater->vo) return(FALSE);

	if (r == 0){        // ROI
		strncpy_s(VP(DataAppCreater->vo, 0, "DataSource", char), 64, "ROI", _TRUNCATE);
		DataAppCreater->grid_type = "ROIGrid";//1 for roi
		//ROIGrid = CreateROIGrid(id); // top level widget for all data grids
		// name the gird based on AppName
		//strncpy_s(ROIGrid->name, sizeof(ROIGrid->name), VP(DataAppCreater->vo, 0, "DataSource", char), _TRUNCATE);
		//  filled from UpdateROIGridVO
		// fill in the ptr to the grid 
		//	VV(DataAppCreater->vo, 0, "gridptr", void *) = ROIGrid;

		//To check and fill in pointers if they are "New" grid and not "ROI"//
		/*if (approw != 0){
			ROIGrid->check_newgrid = 1;
			ROIGrid->rownumbindataapp = approw;
			}*/

		// fill in any prior settings from the user that are stored in the DB
		//  this is a new grid so don't load???   LoadXGrid( id, FDOGrid, VP(DataAppGrid->vo, approw, "AppName", char ), NULL );
		// display the new grid
		//DataAppGrid->gridrows[approw + 1].fg = (GLfloat*)black;
		//ROIGrid->parent_w->visible = TRUE;
		/*MGLSetTitle(ROIGrid, VP(DataAppGrid->vo, approw, "AppName", char));
		addnewrow = TRUE;*/

	}

	else if (r == 1){    // FDO
		strncpy_s(VP(DataAppCreater->vo, 0, "DataSource", char), 64, "FDO", _TRUNCATE);
		DataAppCreater->grid_type = "FDOGrid";
		//	FDOGrid = CreateFDOGrid(id); // top level widget for all data grids
		//	// name the gird based on AppName
		//	strncpy_s(FDOGrid->name, sizeof(FDOGrid->name), VP(DataAppGrid->vo, approw, "AppName", char), _TRUNCATE);
		//	FillFDOVO(id, FDOGrid);
		//	// fill in the ptr to the grid 
		//	VV(DataAppGrid->vo, approw, "gridptr", void *) = FDOGrid;
		//	// fill in any prior settings from the user that are stored in the DB
		//	//  this is a new grid so don't load???   LoadXGrid( id, FDOGrid, VP(DataAppGrid->vo, approw, "AppName", char ), NULL );
		//	// display the new grid

		//	//To check and fill in pointers if they are "New" grid and not "FDO"//

		//	if (approw != 0){
		//		FDOGrid->check_newgrid = 2;
		//		FDOGrid->rownumbindataapp = approw;
		//	}
		//	DataAppGrid->gridrows[approw + 1].fg = (GLfloat*)black;
		//	//FDOGrid->parent_w->visible = TRUE;
		//	MGLSetTitle(FDOGrid, VP(DataAppGrid->vo, approw, "AppName", char));
		//	addnewrow = TRUE;
		//}
		//else if (r == 2){     // Tarmac
		//	strncpy_s(VP(DataAppGrid->vo, approw, "DataSource", char), 64, "Tarmac", _TRUNCATE);
		//	// create the tarmac grid and pop it up
		//}
		//// hide this menu now that a selection has been made
		//DataTypeMenuGrid->parent_w->visible = FALSE;
		////if (addnewrow == TRUE){
		////	// check to make sure the current last row is not "New"
		////	if (strcmp(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), "New")){
		////		vo_alloc_rows(DataAppGrid->vo, 1);
		////		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, "New", _TRUNCATE);
		////		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 16, "Choose", _TRUNCATE);
		////		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "Show", char), 16, "Show", _TRUNCATE);
		////		
		////	}
	}

	else if (r == 2){    // FDO
		strncpy_s(VP(DataAppCreater->vo, 0, "DataSource", char), 64, "Sys", _TRUNCATE);
		DataAppCreater->grid_type = "SysAlertsGrid";
	}

	DataTypeMenuGrid->parent_w->visible = FALSE;
	id->DataAppCreater = DataAppCreater;
	return(TRUE);
}
////Pop up the grid list
//MGLGRID *CreateGridList(struct InstanceData* id, MGLGRID *parentgrid)
//{
//	int r,i;
//	widgets *w, *lastw;
//	MGLGRID *grid;
//
//	assert(id);
//	gui* oglgui = (gui*)id->m_poglgui;
//	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
//	// find last widget in list and add it here
//	for (w = oglgui->widgets; w; w = w->np){
//		lastw = w;
//	}
//
//	// Create MGLGrid
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "Grid_ListWidget");
//	w->gregion.x1 = DataAppCreater->gridrect.x1 + 70;
//	w->gregion.y1 = DataAppCreater->gridrect.y1 - 30;
//	w->gregion.x2 = 200;
//	w->gregion.y2 = 570;
//	w->type = WGT_GRID;
//	w->grid = MGLGridCreate(id, parentgrid, "Grid_ListGrid", 8, 1, w->gregion.x1, w->gregion.y1);
//	w->grid->iVscrollPos = 0;
//	w->grid->GScrollWidth = 20;
//	w->visible = 0;
//	w->grid->AllowScrollbar = TRUE;
//	w->grid->HasTitleBar = FALSE;
//	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
//	w->grid->parentgrid = parentgrid;
//
//	grid = w->grid;
//	// add default colors for grid and the rows
//	w->grid->bg = (GLfloat*)gridgrey0;
//	w->grid->fg = (GLfloat*)yellow;
//
//	for (r = 0; r < w->grid->nrows; r ++){
//
//		w->grid->gridrows[r].fg = (GLfloat*)white;		
//		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
//	
//	}
//	w->grid->gridrows[0].fg = (GLfloat*)Text_yellow;
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//	}
//
//	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "New/Copy GRID", _TRUNCATE);
//
//
//	for (i = 0; i < w->grid->nrows; i++){
//		w->grid->gridrows[i].row_callback = GridListCallback;
//	}
//
//	id->m_poglgui = oglgui;
//	id->DataAppCreater = DataAppCreater;
//
//	return(grid);
//}

//void FillGridList(struct InstanceData* id, MGLGRID *GridList)
//{
//
//	assert(id);
//	assert(GridList);
//
//	// fill a vo with all the unique values in this columns for all rows
//	if (!GridList->vo){
//		CrGridListVO(GridList);
//	}
//	else {
//		// already created the DataAppGrid->vo
//		return;
//	}
//
//	vo_alloc_rows(GridList->vo, 1);
//
//	CalcColWidths(id, GridList);
//}
//void AddVo_GridList(struct InstanceData* id, MGLGRID *Grid_List)
//{
//	int i;	
//	char *Datasource;
//	VO *DataAppVO;
//	MGLGRID *Grid;
//	strncpy_s(VP(Grid_List->vo, 0, "GridNames", char), 64, "New", _TRUNCATE);
//	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
//	
//	for (i = 0; DataAppGrid && (DataAppVO = DataAppGrid->vo) && i < DataAppVO->count-1; i++){
//		Grid = VV(DataAppGrid->vo, i, "gridptr", void *);
//		vo_alloc_rows(Grid_List->vo, 1);
//		strncpy_s(VP(Grid_List->vo, Grid_List->vo->count-1, "GridNames", char), 64, Grid->name, _TRUNCATE);
//	}
//	id->DataAppGrid = DataAppGrid;
//}

//void Copy_CriteriaGrid(id, GridName_DataApp)
//{
//
//}
//int Grid_ReportCallback(struct InstanceData* id, MGLGRID *Grid_Report, int b, int m, int x, int y)
//{
//	// pop up one of the GUI settings grids
//	int r;
//	
//	
//
//	assert(id);
//
//	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
//	MGLGRID* Grid_List = (MGLGRID*)id->Grid_List;
//	
//
//	if (Grid_Report->gridrows[0].height != MGL_DEFAULT){
//		r = (int)floor(((double)y - Grid_Report->gridrect.y1) / Grid_Report->gridrows[0].height);   // only works if rows all have same height
//	}
//	else {
//		r = (int)floor(((double)y - Grid_Report->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
//	}
//
//	if (r < 0 || r >= Grid_Report->nrows){
//		return(FALSE);
//	}
//	// reverse the order
//	r = Grid_Report->nrows - r - 1;
//
//
//	
//
//	if (r == 0){ 
//		
//			Grid_List = CreateGridList(id, Grid_Report);
//			id->Grid_List = Grid_List;
//			FillGridList(id, Grid_List);
//			AddVo_GridList(id, Grid_List);
//			Grid_List->parent_w->visible = TRUE;		
//	   	    Grid_Report->gridrows[0].fg = (GLfloat*)red;
//			MGLGridMove(id, Grid_List, DataAppGrid->gridrect.x1 + 80, DataAppGrid->gridrect.y1 - (10 * MGL_GRID_DEF_ROW_HEIGHT));
//	     }
//
//	id->DataAppGrid = DataAppGrid;
//	return(TRUE);
//}

//int GridListCallback(struct InstanceData* id, MGLGRID *Grid_List, int b, int m, int x, int y)
//{
//	// pop up one of the GUI settings grids
//	int r,i,vorow;
//	char *Datasource;
//	VO *DataAppVO;
//	MGLGRID *Grid;
//	char GridName_list[64], GridName_DataApp[64];
//
//
//	assert(id);
//	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
//	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
//	MGLGRID* Grid_Report = (MGLGRID*)id->Grid_Report;
//
//	
//
//	if (!Grid_List) return(FALSE);
//	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up
//
//	if (Grid_List->gridrows[0].height != MGL_DEFAULT){
//		r = (int)floor(((double)y - Grid_List->gridrect.y1) / Grid_List->gridrows[0].height);   // only works if rows all have same height
//	}
//	else {
//		r = (int)floor(((double)y - Grid_List->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
//	}
//
//	if (r < 0 || r >= Grid_List->nrows){
//		return(FALSE);
//	}
//	// reverse the order
//	r = Grid_List->nrows - r - 1;
//
//	Grid_List->parent_w->visible = FALSE;
//	if (r > 0)
//	{
//		if (r == 1){
//
//			strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "AppName", char), 64, "Enter Name", _TRUNCATE);	
//
//		}
//		else
//		{
//			strncpy_s(GridName_list, 64, VP(Grid_List->vo, r - 1 + Grid_List->iVscrollPos, "GridNames", char), _TRUNCATE);
//			for (i = 0; i < DataAppGrid->vo->count - 1; i++){
//				Grid = VV(DataAppGrid->vo, i, "gridptr", void *);
//				Datasource = VP(DataAppGrid->vo, i, "DataSource", char);
//				strcpy_s(GridName_DataApp, 64, Grid->name);
//				if (strcmp(GridName_DataApp, GridName_list) == 0)						
//					break;	
//
//			}
//			
//			strcat(GridName_DataApp, "_copy");
//			strcpy_s(DataAppGrid->copy_create, 64, "Copy");
//			DataAppGrid->vonumber = i;
//			strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "AppName", char), 64, GridName_DataApp, _TRUNCATE);
//			strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "DataSource", char), 64, Datasource, _TRUNCATE);
//			strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "Create", char), 64, "Copy", _TRUNCATE);
//			if (strcmp(Datasource, "ROI") == 0)
//				DataAppCreater->grid_type = "ROIGrid";
//			else if (strcmp(Datasource, "FDO") == 0)
//				DataAppCreater->grid_type = "FDOGrid";		
//			
//		}
//	}
//	
//	Grid_Report->gridrows[0].fg = (GLfloat*)Grid_grey2;
//	Grid_List->parent_w->visible = FALSE;
//	Grid_Report->parent_w->visible = FALSE;
//	id->DataAppGrid = DataAppGrid;
//	id->DataAppCreater = DataAppCreater;
//	return(TRUE);
//}
//mtm- Type : Grid or report
//MGLGRID *CreateGrid_Report(struct InstanceData* id, MGLGRID *parentgrid)
//{
//	int r;
//	widgets *w, *lastw;
//	MGLGRID *grid;
//
//	assert(id);
//	gui* oglgui = (gui*)id->m_poglgui;
//	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
//	// find last widget in list and add it here
//	for (w = oglgui->widgets; w; w = w->np){
//		lastw = w;
//	}
//
//	// Create MGLGrid
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "Grid_ReportWidget");
//	w->gregion.x1 = DataAppCreater->gridrect.x1 + 20;
//	w->gregion.y1 = DataAppCreater->gridrect.y1 - 30;
//	w->gregion.x2 = 200;
//	w->gregion.y2 = 570;
//	w->type = WGT_GRID;
//	w->grid = MGLGridCreate(id, parentgrid, "Grid_ReportGrid", 2, 1, w->gregion.x1, w->gregion.y1);
//	w->grid->iVscrollPos = 0;
//	w->grid->GScrollWidth = 20;
//	w->visible = 0;
//	w->grid->AllowScrollbar = FALSE;
//	w->grid->HasTitleBar = FALSE;
//	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
//	w->grid->parentgrid = parentgrid;
//
//	grid = w->grid;
//	// add default colors for grid and the rows
//	w->grid->bg = (GLfloat*)gridgrey0;
//	w->grid->fg = (GLfloat*)yellow;
//	for (r = 0; r < w->grid->nrows; r++){
//		w->grid->gridrows[r].fg = (GLfloat*)Grid_grey2;
//		w->grid->gridrows[r].bg = (GLfloat*)white;
//	}
//
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//	}
//
//	MGLGridText(id, grid, 0, 0, "GRID");
//	MGLGridText(id, grid, 1, 0, "Report");
//	
//
//	grid->gridrows[0].row_callback = Grid_ReportCallback;
//	id->m_poglgui = oglgui;
//	id->DataAppCreater = DataAppCreater;
//
//	return(grid);
//}





MGLGRID *CreateDataTypeMenuGrid(struct InstanceData* id, MGLGRID *parentgrid)
{
	int r;
	widgets *w, *lastw;
	MGLGRID *grid;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create MGLGrid
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "DataTypeMenuWidget");
	w->gregion.x1 = DataAppCreater->gridrect.x1 + 80;
	w->gregion.y1 = DataAppCreater->gridrect.y1 - 30;
	w->gregion.x2 = 200;
	w->gregion.y2 = 570;
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, parentgrid, "DataTypeGrid", 3, 1, w->gregion.x1, w->gregion.y1);
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	w->grid->parentgrid = parentgrid;

	grid = w->grid;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)yellow;
	for (r = 0; r < w->grid->nrows; r++){
		w->grid->gridrows[r].fg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r].bg = (GLfloat*)white;
	}


	// add new widget
	if (lastw){
		lastw->np = w;
	}

	MGLGridText(id, grid, 0, 0, "ROI");
	MGLGridText(id, grid, 1, 0, "FDO");
	MGLGridText(id, grid, 2, 0, "SysAlerts");

	grid->gridrows[0].row_callback = DataTypeMenuCallback;
	grid->gridrows[1].row_callback = DataTypeMenuCallback;
	grid->gridrows[2].row_callback = DataTypeMenuCallback;
	id->m_poglgui = oglgui;
	id->DataAppCreater = DataAppCreater;

	return(grid);
}





char *CreateXGridParmatersStr(struct InstanceData *id, MGLGRID *grid, char *AppName, char *DataSource,int i)
{
	int action, f, col, SortPriority, IsVisible, ActionType, IsActive, SortVal, s, rowsize, colsize;
	char *op, *cmp_value, *ActionName, *ColName, *soundname, *row_fg, *row_bg;
	int filteron, count;
	char *arpt, *outstr, *searchstr;
	char tmpbuf[1024];
	VO *AvailableVO, *searchvo;
	char *rsize, *csize;
	char encodebuf[1024];
	

	assert(id);

	arpt = "";

	outstr = str_falloc("Tablename: \"xgridapps\"\n");
	sprintf_s(tmpbuf, sizeof(tmpbuf), "AppName: \"%s\"\n", AppName);
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "username: \"%s\"\n", id->m_strUserName);
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "arpt: \"%s\"\n", arpt);
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "datasource: \"%s\"\n", DataSource);
	outstr = strcat_alloc(outstr, tmpbuf);

	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"xgriddefaults\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "minx1: \"%d\"\n", grid->gridrectmin.x1); // minimized x location
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "miny1: \"%d\"\n", grid->gridrectmin.y1); // minimized y location
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "minx2: \"%d\"\n", grid->gridrectmin.x2); // minimized x location
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "miny2: \"%d\"\n", grid->gridrectmin.y2); // minimized y location
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "maxx1: \"%d\"\n", grid->gridrectmax.x1);  // maximized
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "maxy1: \"%d\"\n", grid->gridrectmax.y1);
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "maxx2: \"%d\"\n", grid->gridrectmax.x2);  // maximized
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "maxy2: \"%d\"\n", grid->gridrectmax.y2);
	outstr = strcat_alloc(outstr, tmpbuf);

	if (!grid->SortAvailableGrid) {
		CreateSortPanel(id, grid);
		FillAvailableColsVO(id, grid);
	}

	if (!grid->SortAvailableGrid || !(AvailableVO = grid->SortAvailableGrid->vo)) {
		return(NULL);
	}

	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"xgridcolumns\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	for (col = 0; col < AvailableVO->count; col++){

		ColName = VP(AvailableVO, col, "Column", char);
		IsVisible = VV(AvailableVO, col, "Visible", int);
		SortPriority = VV(AvailableVO, col, "SortPriority", int);
		SortVal = VV(AvailableVO, col, "SortVal", int);


		sprintf_s(tmpbuf, sizeof(tmpbuf), "colname: \"%s\"\n", ColName);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "IsVisible: \"%d\"\n", IsVisible);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "SortPriority: \"%d\"\n", SortPriority);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "col_order: \"%d\"\n", col);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "SortType: \"%d\"\n", SortVal);
		outstr = strcat_alloc(outstr, tmpbuf);
	}


	for (action = 0; action < grid->nActions; action++){

		sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"xgridactions\"\n");
		outstr = strcat_alloc(outstr, tmpbuf);
		ActionName = grid->Actions[action].ActionName;
		ActionType = grid->Actions[action].ActionType;
		IsActive = grid->Actions[action].IsActive;
		row_fg = grid->Actions[action].row_fg;  // ex:  FF00B7
		row_bg = grid->Actions[action].row_bg;  // ex:  FF00B7
		soundname = grid->Actions[action].soundname;
		filteron = grid->Actions[action].filter_active;


		sprintf_s(tmpbuf, sizeof(tmpbuf), "actionname: \"%s\"\n", ActionName);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "actiontype: \"%d\"\n", ActionType);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "isactive: \"%d\"\n", IsActive);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "Filter On: \"%d\"\n", filteron);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "row_fg: \"%s\"\n", row_fg);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "row_bg: \"%s\"\n", row_bg);
		outstr = strcat_alloc(outstr, tmpbuf);
		sprintf_s(tmpbuf, sizeof(tmpbuf), "soundname: \"%s\"\n", soundname);
		outstr = strcat_alloc(outstr, tmpbuf);

		if (grid->Actions[action].gridconfig) {

			// save filter parameters 
			if (grid->Actions[action].gridconfig->nFilters){
				sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"xgridparameters\"\n");
				outstr = strcat_alloc(outstr, tmpbuf);
			}

			for (f = 0; f < grid->Actions[action].gridconfig->nFilters; f++){
				// find column in vo that is being filtered
				searchstr = NULL;
				if (!strlen((ColName = grid->Actions[action].gridconfig->filters[f].field_name))) continue;
				if (grid->Actions[action].gridconfig->filters[f].filter_type == FTYPE_LIST){
					// filter type is a list of values
					sprintf_s(tmpbuf, sizeof(tmpbuf), "field_type: \"%d\"\n", grid->Actions[action].gridconfig->filters[f].field_type);
					outstr = strcat_alloc(outstr, tmpbuf);
					if ((searchvo = grid->Actions[action].gridconfig->filters[f].searchvo) && searchvo->count){ // vo list of values
						// get list valuesd from searchvo
						// create string of each value separated by commas, etc
						for (s = 0; s < searchvo->count; s++){
							sprintf_s(tmpbuf, sizeof(tmpbuf), "\"%s\",", VP(searchvo, s, "listval", char));
							searchstr = strcat_alloc(searchstr, tmpbuf);
						}
					}
					else {
						// error, the searchstr was not filled, searchvo empty??
						searchstr = "";
					}
				}
				else if (grid->Actions[action].gridconfig->filters[f].filter_type == FTYPE_OPERATORS){
					op = grid->Actions[action].gridconfig->filters[f].op;  // op is 2 letter code, ala NE EQ LE
					cmp_value = grid->Actions[action].gridconfig->filters[f].cmp_value;
					// save op, value, and colname in DB
				}
				if (searchstr){
					sprintf_s(tmpbuf, sizeof(tmpbuf), "fieldname: \"%s\"\n", ColName);
					outstr = strcat_alloc(outstr, tmpbuf);
					sprintf_s(tmpbuf, sizeof(tmpbuf), "select_value: \"%s\"\n", searchstr);
					outstr = strcat_alloc(outstr, tmpbuf);
				}
				else {
					sprintf_s(tmpbuf, sizeof(tmpbuf), "fieldname: \"%s\"\n", ColName);
					outstr = strcat_alloc(outstr, tmpbuf);
					sprintf_s(tmpbuf, sizeof(tmpbuf), "op: \"%s\"\n", op);
					outstr = strcat_alloc(outstr, tmpbuf);

					// save in case of + or - keys
					urlencode(encodebuf, cmp_value, 0);

					sprintf_s(tmpbuf, sizeof(tmpbuf), "cmp_value: \"%s\"\n", encodebuf);

					
					outstr = strcat_alloc(outstr, tmpbuf);
					sprintf_s(tmpbuf, sizeof(tmpbuf), "filter_type: \"%d\"\n", grid->Actions[action].gridconfig->filters[f].filter_type);
					outstr = strcat_alloc(outstr, tmpbuf);
					sprintf_s(tmpbuf, sizeof(tmpbuf), "field_type: \"%d\"\n", grid->Actions[action].gridconfig->filters[f].field_type);
					outstr = strcat_alloc(outstr, tmpbuf);
					sprintf_s(tmpbuf, sizeof(tmpbuf), "exclude_selected: \"%d\"\n", grid->Actions[action].gridconfig->filters[f].exclude_selected);
					outstr = strcat_alloc(outstr, tmpbuf);
				}

			}  // end each filter
		}
	} // end each action


	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"xrowsize\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	rsize = VP(grid->RowColumn->vo, 0, "Settings", char);
	rowsize = atoi(rsize);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Rows: \"%d\"\n", rowsize);
	outstr = strcat_alloc(outstr, tmpbuf);

	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"xcolsize\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	csize = VP(grid->RowColumn->vo, 1, "Settings", char);
	colsize = atoi(csize);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Cols: \"%d\"\n", colsize);
	outstr = strcat_alloc(outstr, tmpbuf);


	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"IconColor\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	int color = grid->AlertsColorGrid->IconColorGrid->icon_color;
	if (grid->icon_set ==1)
		sprintf_s(tmpbuf, sizeof(tmpbuf), "Color: \"%d\"\n", color);
	else
		sprintf_s(tmpbuf, sizeof(tmpbuf), "Color: \"%d\"\n", 100);
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"ActiveGrid\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Active: \"%d\"\n", grid->active_icon);
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"ShowGrid\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Show: \"%d\"\n", grid->parent_w->visible);
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"OrderOfGrid\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Order: \"%d\"\n",i);
	outstr = strcat_alloc(outstr, tmpbuf);

	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"EMailGrid_ID\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	//to save the email id and subject message
	for (count = 0; count < grid->AlertsColorGrid->EMailGrid->vo->count; count++)
	{
		if (strcmp("", VP(grid->AlertsColorGrid->EMailGrid->vo, 0, "EMail Notifications", char)) == 0)
			sprintf_s(tmpbuf, sizeof(tmpbuf), "EMailID: \"%s\"\n", "");
		else
			sprintf_s(tmpbuf, sizeof(tmpbuf), "EMailID: \"%s\"\n", VP(grid->AlertsColorGrid->EMailGrid->vo, count, "EMail Notifications", char));
		outstr = strcat_alloc(outstr, tmpbuf);
	}
	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"EmailAlerts\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	sprintf_s(tmpbuf, sizeof(tmpbuf), "email_enabled: \"%d\"\n", grid->AlertsColorGrid->EMailGrid->email_enabled);
	outstr = strcat_alloc(outstr, tmpbuf);

	sprintf_s(tmpbuf, sizeof(tmpbuf), "Tablename: \"EMailGrid_Subject\"\n");
	outstr = strcat_alloc(outstr, tmpbuf);
	if (strcmp("", VP(grid->AlertsColorGrid->SubjectGrid->vo, 0, "Subject Line", char)) == 0)
		sprintf_s(tmpbuf, sizeof(tmpbuf), "Subject: \"%s\"\n", "");
	else
		sprintf_s(tmpbuf, sizeof(tmpbuf), "Subject: \"%s\"\n", VP(grid->AlertsColorGrid->SubjectGrid->vo, 0, "Subject Line", char));
	outstr = strcat_alloc(outstr, tmpbuf);

	return(outstr);

}

char *LoadTESTDataFromDB(struct InstanceData *id, char *AppName, char *Arpt)
{
	// test program to just create a string with data from the current string as a test instead of from DB on server
	MGLGRID *grid;
	char *DataSource, *outstr;
	VO *DataAppVO;
	int i;

	assert(id);

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	

	// Arpt name is required for ROI's and tarmac but not FDO objects
	
	if (!(DataAppVO = DataAppGrid->vo)) return(NULL);

	// we may need a gridproperties table to store xloc, yloc, for open and closed

	// find out which row in VO we want, see CrDataAppVO for definition of DataAppVO
	for (i = 0; i < DataAppVO->count; i++){
		if (!strcmp(AppName, VP(DataAppVO, i, "AppName", char))){
			// found match
			DataSource = VP(DataAppVO, i, "DataSource", char);
			grid = VV(DataAppGrid->vo, i, "gridptr", void *);
			break;
		}
	}
	if (!grid) return(NULL);



	outstr = CreateXGridParmatersStr(id, grid, AppName, DataSource,i);
	id->DataAppGrid = DataAppGrid;
	return(outstr);
}





void RegionFilterCB(struct InstanceData* id, MGLGRID *grid, int c, char *colname)
{
	int i;
	PREGION **Region;
	char *name;
	VO *searchvo = NULL;
	struct row_index *row_index;
	extern PREGION **CreateRegionStructs(struct InstanceData* id);

	assert(id);

	vo_alloc_rows(grid->FilterGrid->vo, id->m_nRegions);

	if (!(Region = CreateRegionStructs(id))){
		return;
	}

	// get the current List searchvo and fill in values based on the current settings
	// see if we can find the filter number 
	//for (f = 0; f < grid->Actions[grid->CurActionNum].gridconfig->nFilters; f++){
	//	if (!strcmp(grid->Actions[grid->CurActionNum].gridconfig->filters[f].field_name, colname)){
	//		break;  // found match
	//	}
	//}
	//if (f >= grid->Actions[grid->CurActionNum].gridconfig->nFilters){
	//	// did not find a filter
	//}
	//else if ((searchvo = grid->Actions[grid->CurActionNum].gridconfig->filters[f].searchvo)){
	//}


	// put regions names into FilterGrid->vo
	for (i = 0; i < id->m_nRegions; i++){
		name = Region[i]->name;
		strncpy_s(VP(grid->FilterGrid->vo, i, "valstr", char), 64, name, _TRUNCATE);
		VV(grid->FilterGrid->vo, i, "ValActive", int) = TRUE; // default is to show all values

		// see if this value is in the searchvo
		if (searchvo && (row_index = vo_search(searchvo, "listval", name, NULL))){
			VV(grid->FilterGrid->vo, i, "ValActive", int) = TRUE;
		}
		else {
			VV(grid->FilterGrid->vo, i, "ValActive", int) = FALSE;
		}

	}

	vo_order(grid->FilterGrid->vo);
	grid->FilterGrid->vo = grid->FilterGrid->vo;



	strncpy_s(grid->FilterGrid->vo->name, sizeof(grid->FilterGrid->vo->name), grid->ColOrderNames[c], _TRUNCATE);
}



int roi_grid_text_cb(struct InstanceData* id, widgets *w)
{
	assert(id);
	assert(w);

	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, id->m_strAirportMap, _TRUNCATE);
	return(TRUE);
}




MGLGRID *CreateROIGrid(struct InstanceData* id)
{
	widgets *w, *lastw, *prevw;
	int r, i;
	static GLfloat gridbg[3];
	static GLfloat gridfg[3];
	MGLGRID *ROIGrid;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	// find last widget in list and add it here
	for (w = oglgui->widgets; w; w = w->np){
		lastw = w;
	}

	// Create MGLGrid
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "ROIGridWidget");
	w->gregion.x1 = 20;
	w->gregion.y1 = 71;
	w->gregion.x2 = 200;
	w->gregion.y2 = 450;
	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "ROIGrid", 16, 8, w->gregion.x1, w->gregion.y1);
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->visible = 0;
	w->grid->AllowScrollbar = TRUE;
	w->grid->edit = FALSE;//to display the rules-layout icon
	w->grid->HasTitleBar = TRUE;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	strcpy_s(w->name, sizeof(w->name), "ROIGridWidget");
	w->grid->roi_fdo = 1;

	ROIGrid = w->grid;
	ROIGrid->roi_fdo = 1;

	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;

	strncpy_s(w->grid->ColOrderNames[0], MGL_GRID_LABEL_MAX, "Region", _TRUNCATE);
	strncpy_s(w->grid->ColOrderNames[1], MGL_GRID_LABEL_MAX, "flightid", _TRUNCATE);
	strncpy_s(w->grid->ColOrderNames[2], MGL_GRID_LABEL_MAX, "starttime", _TRUNCATE);
	strncpy_s(w->grid->ColOrderNames[3], MGL_GRID_LABEL_MAX, "endtime", _TRUNCATE);
	strncpy_s(w->grid->ColOrderNames[4], MGL_GRID_LABEL_MAX, "elapsed", _TRUNCATE);
	strncpy_s(w->grid->ColOrderNames[5], MGL_GRID_LABEL_MAX, "Active", _TRUNCATE);
	strncpy_s(w->grid->ColOrderNames[6], MGL_GRID_LABEL_MAX, "arrdep", _TRUNCATE);
	strncpy_s(w->grid->ColOrderNames[7], MGL_GRID_LABEL_MAX, "fix", _TRUNCATE);


	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Region", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "Flight", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Start", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "End", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "Elapsed", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "In", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, "A/D", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[7], MGL_GRID_LABEL_MAX, "Fix", _TRUNCATE);



	w->grid->gridcols[2].val_callback = CnvSecs2HHMMSS;
	// set data_type to DATETIME so the filtering parameters know what they are converting
	//w->grid->gridcols[2].display_data_type = DATETIMEBIND;
	w->grid->gridcols[3].val_callback = CnvSecs2HHMMSS;
	//w->grid->gridcols[3].display_data_type = DATETIMEBIND;


	w->grid->gridcols[4].val_callback = CnvElapsedSecs2HHMMSS;


	w->grid->gridcols[5].val_callback = Cnv2YN;

	w->grid->gridcols[0].filter_callback = RegionFilterCB;
	w->grid->gridcols[1].filter_callback = UniqueFilterCB;
	w->grid->gridcols[5].filter_callback = UniqueFilterCB;
	w->grid->gridcols[6].filter_callback = UniqueFilterCB;
	w->grid->gridcols[7].filter_callback = UniqueFilterCB;



	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)Text_yellow;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey3;
	}


	w->grid->flag1 = 1;//to enable mglgridmove for fdo grid

	w->grid->gridrows[0].bg = (GLfloat*)Grid_grey2;

	// ggui->widgets = w;
	// nw = w;
	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;

	// add in callback for the column header row so if user selects a column heading, 
	//     then it will popup a menu for Filter, Sort, or Column Selector
	w->grid->gridrows[0].row_callback = MGLHeadersCallback;


	// create the title bar widgets, text for View name (Regions), Text for Airport, Text for Layout (Arrivals)
	//              clicking on layout "ARRIVALS" will cause a grid list to pop up with all other layouts
	// Create Title Button, use up png for now

	// 
#ifdef REDX_EXAMPLE
	w = (widgets *)vo_malloc(sizeof(widgets));

	strcpy_s(w->name, sizeof(w->name), "TitleRedxWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = 350;
	w->gregion.y1 = 0;
	w->gregion.x2 = w->gregion.x1 + 32;
	w->gregion.y2 = w->gregion.y1 + 32;
	w->type = WGT_BUTTON;
	w->wgt_mouse_cb = redx_cb;
	w->active_image = "redx32.png";
	w->texsize = 32;
	w->visible = 1;
	w->grid = ROIGrid;
	nw = w;
	ROIGrid->title_widgets = w;

	// add next widget to linked list
	// create text widget for VIEW NAME
	w->np = (widgets *)vo_malloc(sizeof(widgets));
	w = w->np;
#else
	w = (widgets *)vo_malloc(sizeof(widgets));
	ROIGrid->title_widgets = w;
#endif


	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = ROIGrid->marginx;
	w->gregion.y1 = ROIGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - ROIGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	//strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Regions of Interest", _TRUNCATE);
	w->visible = 0;  // not visible until user selects the tile, or loaded from properties
	w->grid = ROIGrid;

	prevw = w;


	//// create text widget for Displaying the Current Airport
	w->np = (widgets *)vo_malloc(sizeof(widgets));
	w = w->np;
	strcpy_s(w->name, sizeof(w->name), "ArptTextWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = prevw->gregion.x2;
	w->gregion.y1 = ROIGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 45;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - ROIGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_airport_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, id->m_strAirportMap, _TRUNCATE);
	w->visible = 1;
	w->grid = ROIGrid;
	w->wgt_text_cb = roi_grid_text_cb;
	prevw = w;

	// create text widget for Displaying the current configuration layout (i.e. ARRIVALS, ELAPSED, etc)
	w->np = (widgets *)vo_malloc(sizeof(widgets));
	w = w->np;
	strcpy_s(w->name, sizeof(w->name), "ConfigTextWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = prevw->gregion.x2+50;
	w->gregion.y1 = ROIGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 100;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - ROIGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_layout_cb;
	//strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "ARRIVALS", _TRUNCATE);
	//w->grid->heading_callback = MGLHeadersCallback;
	w->visible = 1;
	w->grid = ROIGrid;


	for (i = 1; i < ROIGrid->nrows; i++){
		ROIGrid->gridrows[i].row_callback = ROICallback;
	}
	//ROIGrid->nrows = 15;
	ROIGrid->active_icon = 1;
	ROIGrid->fixed_width = 0;
	SecondTabGrid->updateroi = 1;
	id->m_poglgui = oglgui;

	return (ROIGrid);
}




int SaveUserXgridSettings(struct InstanceData *id, char *user, char *arpt, char *app_name, char * data)
{
	char	tmpstr[32768];
	char server_name_test[] = "https://www54.passur.com/fcgi/xSurf.fcg";
	struct MemoryStruct chunk;
	extern char* GetNxtSurfSrvr(struct InstanceData* id);
	int colstartx, datalen;
	char *encodebuf = NULL;

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	id->DataAppGrid = DataAppGrid;
	MGLGRID* SaveGrid = (MGLGRID*)id->SaveGrid;
	SaveGrid = CreateSave(id); // top level widget for all data grids
	id->SaveGrid = SaveGrid;


	colstartx = DataAppGrid->gridcells[0][1].cellrect.x1;
	MGLGridMove(id, SaveGrid, DataAppGrid->gridrect.x2 - (3 * MGL_GRID_DEF_ROW_HEIGHT), DataAppGrid->gridrect.y2 - (3 * MGL_GRID_DEF_ROW_HEIGHT));
	SaveGrid->parent_w->visible = TRUE;
	// fill in the ptr to the grid 
	//FillDefaultAlertsVO(id, AlertsGrid);
	//CalcColWidths(id, AlertsGrid);
	//VV(DataAppVO, i, "gridptr", void *) = SaveGrid;

	assert(id);

	if (user == NULL || strlen(user) == 0) {
		return (FALSE);
	}

	if (app_name == NULL || strlen(app_name) == 0) {
		return (FALSE);
	}

	if (arpt == NULL) {
		return (FALSE);
	}

	if (data == NULL || strlen(data) == 0) {
		return (FALSE);
	}

	chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	memset(tmpstr, 0, sizeof(tmpstr));

	datalen = strlen(data);
	encodebuf = calloc(1, datalen * 2);
	urlencode(encodebuf, data, 0);


	_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "Action=SaveXgridConfig&app_name=%s&user_name=%s&arpt=%s&data=%s", app_name, user, arpt, data);

	free(encodebuf);

	if (!GetCurlFormPostResults(id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE)){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to save XGRID config\n", GetCurrentThreadId());
		GetNxtSurfSrvr(id);
	}
	else {

		// Check for valid data from server
		if (chunk.memory) {
			if (!strstr(chunk.memory, "404 Not Found")) {
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__,
					"saved xgrid to DB str='%s'\n", data);
				free_chunk_memory(&chunk);

				return (TRUE);
			}
			else {
				free_chunk_memory(&chunk);
				return (FALSE);
			}
		}

		return (FALSE);
	}



	return (FALSE);
}








int SaveXGrid(struct InstanceData *id, char *AppName, char *Arpt)
{
	// don't use grids since they are reused by other objects MGLGRID *ParametersGrid, *ActionsGrid, *grid;
	MGLGRID *grid = NULL;
	VO *DataAppVO;
	int i;
	char *DataSource;
	char *outstr;

	assert(id);

	
	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	id->DataAppGrid = DataAppGrid;
	
	// Arpt name is required for ROI's and tarmac but not FDO objects
	if (!(DataAppVO = DataAppGrid->vo)) return(FALSE);

	// we may need a gridproperties table to store xloc, yloc, for open and closed

	// find out which row in VO we want, see CrDataAppVO for definition of DataAppVO
	for (i = 0; i < DataAppVO->count; i++){
		if (!strcmp(AppName, VP(DataAppVO, i, "AppName", char))){
			// found match
			DataSource = VP(DataAppVO, i, "DataSource", char);
			grid = VV(DataAppGrid->vo, i, "gridptr", void *);
			break;
		}
	}
	if (!grid){

		return(FAIL);
	}

	outstr = CreateXGridParmatersStr(id, grid, AppName, DataSource,i);

	// struct InstanceData *id, char *user, char *arpt, char *app_name, char * data)
	SaveUserXgridSettings(id, id->m_strUserName, "", AppName, outstr);
//	ListUserXgridAppnames(id, id->m_strUserName, "", DataAppVO);

	s_free(outstr);

	return(SUCCEED);
}




int DeleteUserXgridSettings(struct InstanceData *id, char *user, char *arpt, char *app_name)
{
	char	tmpstr[32768];
	int trycount = 0;

	struct	MemoryStruct chunk;

	if (user == NULL || strlen(user) == 0) {
		return (FALSE);
	}

	if (app_name == NULL || strlen(app_name) == 0) {
		return (FALSE);
	}

	if (!arpt){
		arpt = "";
	}
	chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	memset(tmpstr, 0, sizeof(tmpstr));
	_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "Action=DeleteXgridConfig&app_name=%s&user_name=%s&arpt=%s", app_name, user, arpt);


	while (!GetCurlFormPostResults(id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE) && trycount++ < 4)
	{
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to delete XGRID config\n", GetCurrentThreadId());
		GetNxtSurfSrvr(id);
	}

	// Check for valid data from server
	if (chunk.memory) {
		if (!strstr(chunk.memory, "404 Not Found")) {
			if (strncmp(chunk.memory, "DELETED", 6)) {
				free_chunk_memory(&chunk);
				return (FALSE);
			}
			else {
				// Here if we successfully deleted record 
				free_chunk_memory(&chunk);
				return (TRUE);
			}

			free_chunk_memory(&chunk);
		}
	}

	return (FALSE);
}
//mtm
void CrSecondTabVO(MGLGRID *grid)
{

	VO *gridVO;

	assert(grid);

	gridVO = (VO *)vo_create(0, NULL);
	vo_set(gridVO, V_NAME, "SecondTabVO", NULL);
	// VOPropAdd( DataAppVO, "OnOff", NTBSTRINGBIND, 16, VO_NO_ROW );  

	grid->vo = gridVO;

}
//mtm
void FillSecondTabVO(struct InstanceData* id, MGLGRID *grid)
{

	assert(id);
	assert(grid);

	// fill a vo with all the unique values in this columns for all rows
	if (!grid->vo){
		CrSecondTabVO(grid);
	}
	else {
		// already created the DataAppGrid->vo
		return;
	}

	vo_rm_rows(grid->vo, 0, grid->vo->count);
	CalcColWidths(id, grid);
}

//added by mtm//
//void CrMainTabVO(MGLGRID *MainTabGrid)
//{
//
//	VO *MainTabVO;
//
//	assert(MainTabGrid);
//
//	MainTabVO = (VO *)vo_create(0, NULL);
//	vo_set(MainTabVO, V_NAME, "DataAppVO", NULL);
//	// VOPropAdd( DataAppVO, "OnOff", NTBSTRINGBIND, 16, VO_NO_ROW );  
//
//	MainTabGrid->vo = MainTabVO;
//
//}

int   DataAppCallback(struct InstanceData* id, MGLGRID *DataAppGrid, int b, int m, int x, int y)
{
	// Toggle the selected action On or OFF
	// allow user to add new actions
	int r, vorow, vorow_icon, c,t,l,j;
	char *AppName;
	char AppName3[64];
	MGLGRID *grid, *testgrid;
	widgets *panel_widget;	
	int check = 0;

	assert(id);

	if (!DataAppGrid) return(FALSE);
	
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* WarningGrid = (MGLGRID*)id->WarningGrid;
	MGLGRID* ConfirmGrid = (MGLGRID*)id->ConfirmGrid;

	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (DataAppGrid->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - DataAppGrid->gridrect.y1) / DataAppGrid->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - DataAppGrid->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}


	// toggle the highlight on or off
	if (r < 0 || r >= DataAppGrid->nrows){
		return(FALSE);
	}
	// reverse the order
	r = DataAppGrid->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + DataAppGrid->iVscrollPos;
	vorow_icon = vorow - DataAppGrid->iVscrollPos;
	// see which column was selected
	c = MGLCalcCol(DataAppGrid, x);

	if (c == 0)
	{
		AppName = VP(DataAppGrid->vo, vorow, "AppName", char);

		if (!strcmp(AppName, "New") || !strlen(AppName)){
			// user must choose a name first, so skip 
		}
		else
		{
			//moving the grid up 
			MGLGridColumnMoveUp(DataAppGrid, vorow);
			SecondTabGrid->aircraftcolor_change_MLAT = 1;
			SecondTabGrid->aircraftcolor_change_ASDEX = 1;
			SecondTabGrid->aircraftcolor_change_Noise = 1;
			SecondTabGrid->aircraftcolor_change_Gate = 1;
			SecondTabGrid->aircraftcolor_change_ASD = 1;
			SecondTabGrid->aircraftcolor_change = 1;
			if (vorow>=0)
			DataAppGrid->vo->row_index[vorow - 1].bg = MGLColorHex2Int("529EFD");
		}

	}

	else if (c == 1)
	{
		AppName = VP(DataAppGrid->vo, vorow, "AppName", char);

		if (!strcmp(AppName, "New") || !strlen(AppName)){
			// user must choose a name first, so skip 
		}
		else
		{
			//moving the grid down
			MGLGridColumnMoveDown(DataAppGrid, vorow);
			SecondTabGrid->aircraftcolor_change_MLAT = 1;
			SecondTabGrid->aircraftcolor_change_ASDEX = 1;
			SecondTabGrid->aircraftcolor_change_Noise = 1;
			SecondTabGrid->aircraftcolor_change_Gate = 1;
			SecondTabGrid->aircraftcolor_change_ASD = 1;
			SecondTabGrid->aircraftcolor_change = 1;
			if (vorow < DataAppGrid->vo->count-2)
				DataAppGrid->vo->row_index[vorow + 1].bg = MGLColorHex2Int("529EFD");
		}
	}

	else if (c == 2 && vorow >= 0 && vorow < DataAppGrid->vo->count){
		//to delete a particular grid
		//When a "x" corresponding to a particular grid is clicked a Confirm grid pops up 
		id->ConfirmGrid = ConfirmGrid;

		if (DataAppGrid->del != 1)
		{
			if (!ConfirmGrid)
			{
				ConfirmGrid = CreateConfirmGrid(id);				
				ConfirmGrid->parent_w->visible = 1;
				for (t = 0; t < 100; t++)
					DataAppGrid->dataapp_confirmdelete[t] = 0;

				DataAppGrid->dataapp_confirmdelete[vorow] = 1;

			}

			else if (ConfirmGrid)
			{
				if (ConfirmGrid->parent_w->visible == 1)
				{
				}
				else
				{
					ConfirmGrid->parent_w->visible = 1;
					for (t = 0; t < 100; t++)
						DataAppGrid->dataapp_confirmdelete[t] = 0;

					DataAppGrid->dataapp_confirmdelete[vorow] = 1;
				}

			}
			MGLGridMove(id, ConfirmGrid, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y2);
			
		}
	}

	if (c == 5 && vorow >= 0 && vorow < DataAppGrid->vo->count){

		AppName = VP(DataAppGrid->vo, vorow, "AppName", char);

		if (!strcmp(AppName, "New") || !strlen(AppName)){
			// user must choose a name first, so skip 
		}
		else
		{
			grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
			
				if (DataAppGrid->active_checkbox[vorow] == 0){
					DataAppGrid->active_checkbox[vorow] = 20;
					grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
					for (j = 0; j < grid->nActions; j++)
						grid->Actions[j].IsActive = 1;				
					grid->active_icon = 1;
					SecondTabGrid->aircraftcolor_change_MLAT = 1;
					SecondTabGrid->aircraftcolor_change_ASDEX = 1;
					SecondTabGrid->aircraftcolor_change_Noise = 1;
					SecondTabGrid->aircraftcolor_change_Gate = 1;
					SecondTabGrid->aircraftcolor_change_ASD = 1;
					SecondTabGrid->aircraftcolor_change = 1;
				}
				else if (DataAppGrid->active_checkbox[vorow] == 20){
					if (grid->parent_w->visible == 0)
					{
						DataAppGrid->active_checkbox[vorow] = 0;
						grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
						for (j = 0; j < grid->nActions; j++)
							grid->Actions[j].IsActive = 0;					
						grid->active_icon = 0;
						SecondTabGrid->aircraftcolor_change_MLAT = 1;
						SecondTabGrid->aircraftcolor_change_ASDEX = 1;
						SecondTabGrid->aircraftcolor_change_Noise = 1;
						SecondTabGrid->aircraftcolor_change_Gate = 1;
						SecondTabGrid->aircraftcolor_change_ASD = 1;
						SecondTabGrid->aircraftcolor_change = 1;
					}
					else
					{
						DataAppGrid->show_checkbox[vorow] = 0;					
						grid->parent_w->visible = 0;
						DataAppGrid->active_checkbox[vorow] = 0;
						grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
						for (j = 0; j < grid->nActions; j++)
							grid->Actions[j].IsActive = 0;					
						grid->active_icon = 0;
						SecondTabGrid->aircraftcolor_change_MLAT = 1;
						SecondTabGrid->aircraftcolor_change_ASDEX = 1;
						SecondTabGrid->aircraftcolor_change_Noise = 1;
						SecondTabGrid->aircraftcolor_change_Gate = 1;
						SecondTabGrid->aircraftcolor_change_ASD = 1;
						SecondTabGrid->aircraftcolor_change = 1;
					}
				}
			
		}
		
	}

	if (c == 6 && vorow >= 0 && vorow < DataAppGrid->vo->count){

		//Shows and hides the ROI/FDO grid 

		AppName = VP(DataAppGrid->vo, vorow, "AppName", char);


		if (!strcmp(AppName, "New") || !strlen(AppName)){
			// user must choose a name first, so skip 
		}

		else
		{
			grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
			if ((grid->active_icon == 1) || (grid->active_icon == 2))
			{

				if (strcmp("Choose", VP(DataAppGrid->vo, vorow, "DataSource", char))){
					if (DataAppGrid->show_checkbox[vorow] == 0){
						DataAppGrid->show_checkbox[vorow] = 20;//setting the flag corresponding show_checkbox[row in the DataAppGrid] to the particular grid = 20.
						//While rendering either the hide or the show icon is rendered according to the value of the flag
						AppName = VP(DataAppGrid->vo, vorow, "AppName", char);
						SecondTabGrid->childgrid;
						grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
						strcpy_s(grid->name,32,AppName);
						MGLSetTitle(grid, AppName);
						strcpy_s(AppName3,64,"Grid Detail : ");
						strcat_s(AppName3,64,AppName);
						MGLSetTitle(grid->TitleGrid, AppName3);
						grid->parent_w->visible = 1;
						MGLGridMove(id, grid, grid->gridrect.x1, grid->gridrect.y1);
						if (grid->roi_fdo == 2)
							UpdateFDO(id, grid);
						/*if (grid->roi_fdo==3)
							GetSysAlertsData(id);*/

					}

					else if (DataAppGrid->show_checkbox[vorow] == 20)
					{
						DataAppGrid->show_checkbox[vorow] = 0;
						AppName = VP(DataAppGrid->vo, vorow, "AppName", char);
						grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
						grid->parent_w->visible = 0;

					}

				}
			}

			else
			{
				WarningGrid = CreateWarning(id);
				id->WarningGrid = WarningGrid;
				MGLGridText(id, WarningGrid, 0, 0, "Warning! Grid is inactive, activate it to show");
				WarningGrid->parent_w->visible = TRUE;
				MGLGridMove(id, WarningGrid, DataAppGrid->gridrect.x2 - (9 * MGL_GRID_DEF_ROW_HEIGHT), DataAppGrid->gridrect.y2);
			}
		}

	}

	else if (c == 7 && vorow >= 0 && vorow < DataAppGrid->vo->count){
		// saves the grid
		AppName = VP(DataAppGrid->vo, vorow, "AppName", char);
		if (!strcmp(AppName, "New")){
			// don't save new, it is a placeholder for user to pick a new app
		}
		else {
			SaveXGrid(id, VP(DataAppGrid->vo, vorow, "AppName", char), NULL);
		}

	}

	else if (c == 8 && vorow >= 0 && vorow < DataAppGrid->vo->count){
		//Brings up the secondtabgrid

		AppName = VP(DataAppGrid->vo, vorow, "AppName", char);
		if (!strcmp(AppName, "New") || !strlen(AppName)){
			// user must choose a name first, so skip 
		}
		else
		{
		grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
		strcpy_s(grid->name,32,AppName);
		strcpy_s(AppName3,64,"Grid Detail : ");
		strcat_s(AppName3,64,AppName);
		MGLSetTitle(grid->TitleGrid, AppName3);
		MGLSetTitle(grid, AppName);

		if (!strcmp(AppName, "New") || !strlen(AppName)){
			// user must choose a name first, so skip 
		}

		else{
			if (strcmp("Choose", VP(DataAppGrid->vo, vorow, "DataSource", char))){
				DataAppGrid->callback = 20;                                      //The particular row whose details has to be shown is set to row_checkbox[row] =20. While rendering the row which has corresponding
				//flag set to 20 has the build icon with yellow color.
				for (int xx = 0; xx < 100; xx++)
					DataAppGrid->row_checkbox[xx] = 0;

					DataAppGrid->row_checkbox[vorow] = 20;

				if (SecondTabGrid->childgrid)
				{
					SecondTabGrid->childgrid->ActionsGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->ParametersGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->AlertsColorGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->SortAvailableGrid->parent_w->visible = FALSE;
					SecondTabGrid->childgrid->RowColumn->parent_w->visible = FALSE;

					for (l = 0; l < DataAppGrid->vo->count - 1; l++)
					{
						testgrid = VV(DataAppGrid->vo, l, "gridptr", void *);
						if (!testgrid)
							return(FALSE);
							testgrid->TitleGrid->parent_w->visible = FALSE;
						testgrid->BuildSummaryGrid->parent_w->visible = FALSE;
						SecondTabGrid->parent_w->visible = FALSE;
						SecondTabGrid->childgrid->to_open_build = FALSE;
						SecondTabGrid->childgrid->to_open_details = FALSE;
						testgrid->ActionsGrid->parent_w->visible = FALSE;
						testgrid->ParametersGrid->parent_w->visible = FALSE;
						testgrid->AlertsColorGrid->parent_w->visible = FALSE;
						testgrid->SortAvailableGrid->parent_w->visible = FALSE;
						testgrid->RowColumn->parent_w->visible = FALSE;
						if (testgrid->AlertsColorGrid->ForegroundColorGrid)
							testgrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
						if (testgrid->AlertsColorGrid->BackgroundColorGrid)
							testgrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
						if (testgrid->AlertsColorGrid->IconColorGrid)
							testgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;
						if (testgrid->AlertsColorGrid->EMailGrid)
							testgrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
						if (testgrid->AlertsColorGrid->SubjectGrid)
							testgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
					}

				}

				SecondTabGrid->gridcells[0][0].fg = (GLfloat*)Grid_grey4;
				SecondTabGrid->gridcells[0][1].fg = (GLfloat*)Grid_grey4;
				SecondTabGrid->gridcells[0][2].fg = (GLfloat*)Grid_grey4;
				SecondTabGrid->gridcells[0][3].fg = (GLfloat*)Grid_grey4;
				SecondTabGrid->gridcells[0][4].fg = (GLfloat*)Grid_grey4;

				grid = VV(DataAppGrid->vo, vorow, "gridptr", void *);
				SecondTabGrid->childgrid = grid;
				SecondTabGrid->childgrid->to_open_build = TRUE;
				SecondTabGrid->childgrid->BuildSummaryGrid->parent_w->visible = TRUE;

				SecondTabGrid->childgrid->BuildSummaryGrid->gridrows[0].fg = Text_yellow;
				SecondTabGrid->childgrid->TitleGrid->parent_w->visible = TRUE;

				//To adjust the positions of the build grid				
				MGLGridMove(id, DataAppGrid, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1);
				MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1);	
				
				MGLGridMove(id, grid->TitleGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 + DataAppGrid->gridrows[0].height);
				MGLGridMove(id, grid->BuildSummaryGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (grid->BuildSummaryGrid->nrows*DataAppGrid->gridrows[0].height) - 0.75 * DataAppGrid->gridrows[0].height);
				MGLGridMove(id, SecondTabGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - SecondTabGrid->nrows * DataAppGrid->gridrows[0].height - DataAppGrid->gridrows[0].height);
				MGLGridMove(id, SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				MGLGridMove(id, SecondTabGrid->childgrid->ParametersGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				MGLGridMove(id, SecondTabGrid->childgrid->RowColumn, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (4 * DataAppGrid->gridrows[0].height));

				if (SecondTabGrid->childgrid->AlertsColorGrid)
				{
					if (SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->BackgroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->ForegroundColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
					if (SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid)
						MGLGridMove(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid, SecondTabGrid->gridrect.x1 + (17 * DataAppGrid->gridrows[0].height), SecondTabGrid->gridrect.y1 + (6 * DataAppGrid->gridrows[0].height));
				}



				if (!SecondTabGrid->childgrid->ActionsGrid){

					CreateActionsGrid(id, grid);
					grid->ActionsGrid->parent_w->visible = FALSE;
					FillActionsGrid(id, grid, FALSE);



					CreateParametersGrid1(id, grid, 0);
					MGLFillParametersColsVO(grid);
					grid->ParametersGrid->parent_w->visible = FALSE;


					CreateAlertsColorGrid(id, grid, 0);
					MGLInitAlertsColorsVO(grid);
					grid->AlertsColorGrid->parent_w->visible = FALSE;


					CreateGridrowcolumns(id, grid, 0);
					MGLInitGridofRowsColumnsVO(grid);




					CreateSortPanel(id, grid);
					grid->RowColumn->parent_w->visible = FALSE;
					FillAvailableColsVO(id, grid);
					FillActionsGrid1(id, grid, FALSE);
					FillActionsGrid2(id, grid, FALSE);
					MGLFillParameters(id, grid->ParametersGrid);
				}

				if (grid->SortAvailableGrid){
					grid->SortAvailableGrid->parent_w->visible = FALSE;
					panel_widget = grid->SortAvailableGrid->parent_w->gregion_parentw;
					MGLDrawPanel(id, panel_widget);  // set parent widget to display the grid
					MGLGridMove(id, SecondTabGrid->childgrid->SortAvailableGrid, SecondTabGrid->gridrect.x1 + DataAppGrid->gridrows[0].height, SecondTabGrid->gridrect.y1 + (3 * DataAppGrid->gridrows[0].height));
				}

			}
		}

	}
	id->resize = 2;
	MGLDrawGUI(id);

	}
	


	id->DataAppCreater = DataAppCreater;
	return(TRUE);
}
void CrBuildSummaryVO(MGLGRID *BuildSummaryGrid)
{

	VO *BuildSummaryVO;

	assert(BuildSummaryGrid);

	BuildSummaryVO = (VO *)vo_create(0, NULL);
	vo_set(BuildSummaryVO, V_NAME, "BuildSummaryVO", NULL);
	VOPropAdd(BuildSummaryVO, "Criteria", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(BuildSummaryVO, "Parameters", NTBSTRINGBIND, 512, VO_NO_ROW);
	VOPropAdd(BuildSummaryVO, "Actions", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(BuildSummaryVO, "IconSpace", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(BuildSummaryVO, "Layout", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(BuildSummaryVO, "Visiblerows/columns", NTBSTRINGBIND, 64, VO_NO_ROW);


	BuildSummaryGrid->vo = BuildSummaryVO;

}

//mtm
void FillBuildSummaryVO(struct InstanceData* id, MGLGRID *BuildSummaryGrid)
{

	assert(id);
	assert(BuildSummaryGrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!BuildSummaryGrid->vo){
		CrBuildSummaryVO(BuildSummaryGrid);
	}
	else {
		// already created the DataAppGrid->vo
		return;
	}

	//vo_alloc_rows(BuildSummaryGrid->vo, 1);
	CalcColWidths(id, BuildSummaryGrid);
}

//mtm title-Grid Detail
void CreateTitleGrid(struct InstanceData* id, char *Appname, MGLGRID *parentgrid)
{
	widgets *w, *lastw, *panel_widget;
	int r;
	char appname1[64];

	assert(id);
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	strcpy_s(appname1, 64, "  Grid Detail : ");
	strcat_s(appname1, 64, Appname);
	// find last widget in list and add it here
	gui* oglgui = (gui*)id->m_poglgui;
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "TitleGrid");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;


	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;



	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "TitleGridWidget");
	w->gregion.x1 = DataAppGrid->gridrect.x2 - 1;
	w->gregion.y1 = DataAppGrid->gridrect.y2 - MGL_GRID_DEF_ROW_HEIGHT;
	w->gregion.x2 = 400;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "TitleGridGrid", 1, 6, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;	
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = TRUE;
	w->grid->edit = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	parentgrid->TitleGrid = w->grid;

	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey2;
	}


	w->grid->gridrows[0].fg = Text_yellow;



	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "            ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "            ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "           ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "      ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "             ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "                    ", _TRUNCATE);





	//w->grid->gridcols[2].val_callback = Cnv2HideShow_mtm;

	// we also need to fetch initial filter values from user settings DB



	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	parentgrid->TitleGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = parentgrid->TitleGrid->marginx;
	w->gregion.y1 = parentgrid->TitleGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - parentgrid->TitleGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, appname1, _TRUNCATE);
	w->visible = 0;
	w->grid = parentgrid->TitleGrid;

	parentgrid->TitleGrid->nrows = 1;
	parentgrid->TitleGrid->fixed_width = 2;
	id->m_poglgui = oglgui;
}
//mtm grid_manager title

//void CreateGridManagerGrid(struct InstanceData* id)
//{
//	widgets *w, *lastw, *panel_widget;
//	int r;
//
//
//	assert(id);
//
//	
//	//Setting void pointers to MGLGRID struct
//	gui* oglgui = (gui*)id->m_poglgui;
//	//MGLGRID* GridManager = (MGLGRID*)id->GridManager;
//	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
//	id->DataAppGrid = DataAppGrid;
//
//	// find last widget in list and add it here
//	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
//		lastw = w;
//	}
//
//	// Create panel for holding the two grids plus buttons
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "TitleGrid");
//	w->gregion.x1 = 20;
//	w->gregion.y1 = 370;
//	w->width = 500;
//	w->height = 300;
//	//w->gregion.x2 = 200;
//	//w->gregion.y2 = 570;
//	//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;
//
//	w->type = WGT_PANEL;
//	w->visible = 0;  // don't show panel
//	w->bg = (GLfloat*)gridgrey0;
//	w->fg = (GLfloat*)white;
//	panel_widget = w;
//
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//	}
//	else {
//		// first widget
//		oglgui->widgets = w;
//	}
//	lastw = w;
//
//
//
//	// Create MGLGrid for list of Available columns
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "GridManagerWidget");
//	w->gregion.x1 = DataAppGrid->gridrect.x1;
//	w->gregion.y1 = DataAppGrid->gridrect.y2;
//	w->gregion.x2 = 400;
//	w->gregion.y2 = id->m_nHeight;
//	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
//
//	w->type = WGT_GRID;
//	w->grid = MGLGridCreate(id, NULL, "GridManagerGrid", 1, 9, w->gregion.x1, w->gregion.y1);
//	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
//	w->grid->iVscrollPos = 0;
//	w->grid->GScrollWidth = 20;
//	w->grid->RulesLayout = 0;//to display the rules-layout icon
//	w->visible = 0;
//	w->grid->AllowScrollbar = FALSE;
//	w->grid->HasTitleBar = TRUE;
//	// add default colors for grid and the rows
//	w->grid->bg = (GLfloat*)gridgrey0;
//	w->grid->fg = (GLfloat*)white;
//	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
//	GridManager = w->grid;
//
//	for (r = 0; r < w->grid->nrows; r += 2){
//		if (r == 0){
//			w->grid->gridrows[r].fg = (GLfloat*)white;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
//		}
//		else {
//			w->grid->gridrows[r].fg = (GLfloat*)white;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
//		}
//		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
//		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey2;
//	}
//
//
//	w->grid->gridrows[0].fg = Text_yellow;
//
//
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//		lastw = w;
//	}
//	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "    ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "    ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "                      ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "      ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, "      ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[7], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[8], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
//	
//
//
//
//
//	//w->grid->gridcols[2].val_callback = Cnv2HideShow_mtm;
//
//	// we also need to fetch initial filter values from user settings DB
//
//
//
//	// set TITLE text widgets
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	GridManager->title_widgets = w;
//
//	strcpy_s(w->name, sizeof(w->name), "GridManagerwidget");
//	// give location of widget relative to title_rect
//	w->gregion.x1 = GridManager->marginx;
//	w->gregion.y1 = GridManager->marginy;
//	w->gregion.x2 = w->gregion.x1 + 120;
//	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - GridManager->marginy;
//	w->type = WGT_TEXT;
//	w->wgt_mouse_cb = title_view_cb;
//	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "  GRID Manager", _TRUNCATE);
//	w->visible = 1;
//	w->grid = GridManager;
//
//	GridManager->fixed_width = 2;
//	id->m_poglgui = oglgui;
//	id->GridManager = GridManager;
//
//
//}


//mtm to create a new line in dataappselector grid with the grid name (ROI or FDO)
MGLGRID *AddRowDataAppGrid(struct InstanceData* id)
{
	MGLGRID *ROIGrid, *FDOGrid, *SysAlertsGrid;
	char *AppName;
	int i;
	char full[512];
	int count;

	extern void* CreateRegionVO(struct InstanceData* id, const char* strVoName);

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	id->DataAppGrid = DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	id->DataAppCreater = DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	id->SecondTabGrid = SecondTabGrid;

	if (DataAppCreater->grid_type == "ROIGrid")
	{
		//Creating ROIGrid 
		ROIGrid = CreateROIGrid(id);
		ROIGrid->vo = CreateRegionVO(id, "OpenglROI");
		MGLSetlabelNamesFromVO(ROIGrid);
		VV(DataAppGrid->vo, DataAppGrid->vo->count - 1, "gridptr", void *) = ROIGrid;
		MGLSetTitle(ROIGrid, VP(DataAppCreater->vo, 0, "AppName", char));
		strcpy_s(ROIGrid->name, MGL_GRID_NAME_MAX, VP(DataAppCreater->vo, 0, "AppName", char));

		//Write the text in DataAppCreator grid in DataAppGrid
		AppName = VP(DataAppCreater->vo, 0, "AppName", char);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, AppName, _TRUNCATE);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 64, "ROI", _TRUNCATE);
		DataAppGrid->active_checkbox[DataAppGrid->vo->count - 1] = 20;


		//Clear DataAppCreator Grid after writing into DataAppGrid
		AppName[0] = '\0';
		strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "AppName", char), 64, "  Add New", _TRUNCATE);
		strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "DataSource", char), 64, "  Source", _TRUNCATE);

		//Increase a vo row to accomodate next grid that can be entered
		vo_alloc_rows(DataAppGrid->vo, 1);

		//To adjust the positions of DataAppGrid and DataAppCreator grid
		if (DataAppGrid->nrows <= 9)
			DataAppGrid->nrows++;
		MGLGridMove(id, DataAppGrid, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1);
		if (DataAppGrid->vo->count <= 8)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1 - DataAppGrid->gridrows[0].height);
		else if (DataAppGrid->vo->count > 8)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1);
		if (DataAppGrid->vo->count == 9)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1 - (DataAppGrid->gridrows[0].height));


		//Creating and filling build grids

		CreateActionsGrid(id, ROIGrid);
		ROIGrid->ActionsGrid->parent_w->visible = FALSE;
		FillActionsGrid(id, ROIGrid, FALSE);



		CreateParametersGrid1(id, ROIGrid, 0);	
		MGLFillParametersColsVO(ROIGrid);		
		ROIGrid->ParametersGrid->parent_w->visible = FALSE;


		CreateAlertsColorGrid(id, ROIGrid, 0);
		MGLInitAlertsColorsVO(ROIGrid);
		ROIGrid->AlertsColorGrid->parent_w->visible = FALSE;


		CreateGridrowcolumns(id, ROIGrid, 0);
		MGLInitGridofRowsColumnsVO(ROIGrid);

		
		CreateSortPanel(id, ROIGrid);
		ROIGrid->RowColumn->parent_w->visible = FALSE;		
		FillAvailableColsVO(id, ROIGrid);

		FillActionsGrid1(id, ROIGrid, FALSE);
		FillActionsGrid2(id, ROIGrid, FALSE);
		MGLFillParameters(id, ROIGrid->ParametersGrid);

	
		MGLFillParametersGrid(ROIGrid->ActionsGrid, ROIGrid->CurActionNum,id);
		MGLFillParametersColsVO(ROIGrid);
		FillAvailableColsVO(id, ROIGrid);
		CreateColorMatrix_foreground(id, ROIGrid->AlertsColorGrid, 0);
		CreateColorMatrix_background(id, ROIGrid->AlertsColorGrid, 0);
		CreateColorMatrix_icon(id, ROIGrid->AlertsColorGrid, 0);
		CreateEMailGrid(id, ROIGrid);
		FillEMailVO(id, ROIGrid->AlertsColorGrid->EMailGrid);
		CreateSubjectGrid(id, ROIGrid);
		FillSubjectVO(id, ROIGrid->AlertsColorGrid->SubjectGrid);
		ROIGrid->AlertsColorGrid->EMailGrid->parent_w->visible = FALSE;
		ROIGrid->AlertsColorGrid->SubjectGrid->parent_w->visible = FALSE;
		ROIGrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = FALSE;
		ROIGrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = FALSE;
		ROIGrid->AlertsColorGrid->IconColorGrid->parent_w->visible = FALSE;

		if (!ROIGrid->Actions[ROIGrid->CurActionNum].gridconfig){
			ROIGrid->Actions[ROIGrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));			
			ROIGrid->nActions = 1; 
			ROIGrid->Actions[0].IsActive = TRUE;
			ROIGrid->Actions[0].ActionType = ACTION_FILTER;
		}
		ROIGrid->AlertsColorGrid->show_checkbox[0] = 0;
		ROIGrid->AlertsColorGrid->show_checkbox[1] = 0;
		ROIGrid->AlertsColorGrid->show_checkbox[2] = 0;
		ROIGrid->AlertsColorGrid->show_checkbox[3] = 1;//show_checkbox - helps decide the show or hide icon
		ROIGrid->Actions[0].filter_active = 1;

		CreateBuildSummary(id, ROIGrid);
		FillBuildSummaryVO(id, ROIGrid->BuildSummaryGrid);
		CreateTitleGrid(id, ROIGrid->name, ROIGrid);
		ROIGrid->TitleGrid->parent_w->visible = FALSE;

		for (int x = 0; x < ROIGrid->SortAvailableGrid->vo->count; x++)
		{
			if (ROIGrid->DisplayCol[x] != -1)
			{
				write_layout_column_roi(full, x);
				vo_alloc_rows(ROIGrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(ROIGrid->BuildSummaryGrid->vo, x, "Layout", char), 64, full, _TRUNCATE);
			}
		}

		return(ROIGrid);
	}

	else if (DataAppCreater->grid_type == "FDOGrid")
	{
		//Creating FDOGrid 
		FDOGrid = CreateFDOGrid(id);
		FillFDOFromGFDO(id, FDOGrid);
		MGLSetlabelNamesFromVO(FDOGrid);
		VV(DataAppGrid->vo, DataAppGrid->vo->count - 1, "gridptr", void *) = FDOGrid;
		MGLSetTitle(FDOGrid, VP(DataAppCreater->vo, 0, "AppName", char));
		strcpy_s(FDOGrid->name,MGL_GRID_NAME_MAX,VP(DataAppCreater->vo, 0, "AppName", char));

		//Write the text in DataAppCreator grid in DataAppGrid
		AppName = VP(DataAppCreater->vo, 0, "AppName", char);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, AppName, _TRUNCATE);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 64, "FDO", _TRUNCATE);
		DataAppGrid->active_checkbox[DataAppGrid->vo->count - 1] = 20;

		//Clear DataAppCreator Grid after writing into DataAppGrid
		AppName[0] = '\0';
		strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "AppName", char), 64, "  Add New", _TRUNCATE);
		strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "DataSource", char), 64, "  Source", _TRUNCATE);


		//Increase a vo row to accomodate next grid that can be entered
		vo_alloc_rows(DataAppGrid->vo, 1);

		//To adjust the positions of DataAppGrid and DataAppCreator grid
		if (DataAppGrid->nrows <= 9)
			DataAppGrid->nrows++;
		MGLGridMove(id, DataAppGrid, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1);
		if (DataAppGrid->vo->count <= 8)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1 - DataAppGrid->gridrows[0].height);
		else if (DataAppGrid->vo->count > 8)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1);
		if (DataAppGrid->vo->count == 9)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1 - (DataAppGrid->gridrows[0].height));

		//Creating and filling build grids
		
		CreateBuildSummary(id, FDOGrid);//Creates Build summary grid
		FillBuildSummaryVO(id, FDOGrid->BuildSummaryGrid);//Fills the VO of the build summary grid

		CreateActionsGrid(id, FDOGrid);//Creates Criteria grid
		FDOGrid->ActionsGrid->parent_w->visible = FALSE;
		FillActionsGrid(id, FDOGrid, FALSE);//Fills criteria grid



		CreateParametersGrid1(id, FDOGrid, 0);//Creates Parameters grid
		MGLFillParametersColsVO(FDOGrid);//Fills Parameters grid
		FDOGrid->ParametersGrid->parent_w->visible = FALSE;


		CreateAlertsColorGrid(id, FDOGrid, 0);//Creates actions grid
		MGLInitAlertsColorsVO(FDOGrid);//Fills actions grid VO
		FDOGrid->AlertsColorGrid->parent_w->visible = FALSE;

		CreateGridrowcolumns(id, FDOGrid, 0);//Creates row column grid
		MGLInitGridofRowsColumnsVO(FDOGrid);//Fills row column grid VO


		CreateSortPanel(id, FDOGrid);//Creates Layout grid
		FDOGrid->RowColumn->parent_w->visible = FALSE;

		FillAvailableColsVO(id, FDOGrid);//Fills layout grid VO

		FillActionsGrid1(id, FDOGrid, FALSE);
		FillActionsGrid2(id, FDOGrid, FALSE);
		MGLFillParameters(id, FDOGrid->ParametersGrid);


		MGLFillParametersGrid(FDOGrid->ActionsGrid, FDOGrid->CurActionNum,id);
	//	MGLFillParametersColsVO(FDOGrid);
	//	FillAvailableColsVO(id, FDOGrid);
		CreateColorMatrix_foreground(id, FDOGrid->AlertsColorGrid, 0);//creates actions--foreground grid
		CreateColorMatrix_background(id, FDOGrid->AlertsColorGrid, 0);//creates actions--background grid
		CreateColorMatrix_icon(id, FDOGrid->AlertsColorGrid, 0); //creates actions -- Icon color grid
		CreateEMailGrid(id, FDOGrid);//Creates email grid
		FillEMailVO(id, FDOGrid->AlertsColorGrid->EMailGrid);//Fills email grid
		CreateSubjectGrid(id, FDOGrid);//Creates subject grid
		FillSubjectVO(id, FDOGrid->AlertsColorGrid->SubjectGrid);	//Filla subject grid	
		FDOGrid->AlertsColorGrid->EMailGrid->parent_w->visible = 0;
		FDOGrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 0;
		FDOGrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 0;
		FDOGrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
		FDOGrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 0;

		if (!FDOGrid->Actions[FDOGrid->CurActionNum].gridconfig){
			FDOGrid->Actions[FDOGrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));			
			FDOGrid->nActions = 1; 
			FDOGrid->Actions[0].IsActive = TRUE;
			FDOGrid->Actions[0].ActionType = ACTION_FILTER;
		}
		FDOGrid->AlertsColorGrid->show_checkbox[0] = 0;
		FDOGrid->AlertsColorGrid->show_checkbox[1] = 0;
		FDOGrid->AlertsColorGrid->show_checkbox[2] = 0;
		FDOGrid->AlertsColorGrid->show_checkbox[3] = 1;//show_checkbox - helps decide the show or hide icon
		FDOGrid->Actions[0].filter_active = 1;

		CreateTitleGrid(id, FDOGrid->name, FDOGrid);//Creates Title grid
		FDOGrid->TitleGrid->parent_w->visible = 0;

		MGLGridMove(id, SecondTabGrid, DataAppGrid->gridrect.x2 - 1, DataAppGrid->gridrect.y2 - (SecondTabGrid->nrows* (MGL_GRID_DEF_ROW_HEIGHT)));
		//keeping 5 columns of FDO visible as default
		for (i = 0; i < 3; i++)
		{
			FDOGrid->DisplayCol[i] = i;
			VV(FDOGrid->SortAvailableGrid->vo, i, "Visible", int) = 1;
		}
		for (i = 3; i < FDOGrid->ncols; i++)
		{
			VV(FDOGrid->SortAvailableGrid->vo, i, "Visible", int) = 0;
			FDOGrid->DisplayCol[i] = -1;
		}
		FDOGrid->DisplayCol[6] = 6;
		VV(FDOGrid->SortAvailableGrid->vo, 6, "Visible", int) = 1;
		FDOGrid->DisplayCol[9] = 9;
		VV(FDOGrid->SortAvailableGrid->vo, 9, "Visible", int) = 1;

		count = -1;
		strcpy_s(full, 64, "");
		for (int x = 0; x < FDOGrid->SortAvailableGrid->vo->count; x++)
		{
			if (FDOGrid->DisplayCol[x] != -1)
			{
				write_layout_column_fdo(full, x,FDOGrid);
				count++;
				vo_alloc_rows(FDOGrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(FDOGrid->BuildSummaryGrid->vo, count, "Layout", char), 64, full, _TRUNCATE);
			}
		}
		return(FDOGrid);
	}
	else if (DataAppCreater->grid_type == "SysAlertsGrid")
	{
		//Creating SysAlertsGrid 
		SysAlertsGrid = CreateSysAlertsGrid(id);
		FillSysAlertsFromGSA(id, SysAlertsGrid);
		MGLSetlabelNamesFromVO(SysAlertsGrid);
		VV(DataAppGrid->vo, DataAppGrid->vo->count - 1, "gridptr", void *) = SysAlertsGrid;
		MGLSetTitle(SysAlertsGrid, VP(DataAppCreater->vo, 0, "AppName", char));
		strcpy_s(SysAlertsGrid->name, MGL_GRID_NAME_MAX, VP(DataAppCreater->vo, 0, "AppName", char));

		//Write the text in DataAppCreator grid in DataAppGrid
		AppName = VP(DataAppCreater->vo, 0, "AppName", char);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, AppName, _TRUNCATE);
		strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 64, "SysAlerts", _TRUNCATE);
		DataAppGrid->active_checkbox[DataAppGrid->vo->count - 1] = 20;

		//Clear DataAppCreator Grid after writing into DataAppGrid
		AppName[0] = '\0';
		strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "AppName", char), 64, "  Add New", _TRUNCATE);
		strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "DataSource", char), 64, "  Source", _TRUNCATE);


		//Increase a vo row to accomodate next grid that can be entered
		vo_alloc_rows(DataAppGrid->vo, 1);

		//To adjust the positions of DataAppGrid and DataAppCreator grid
		if (DataAppGrid->nrows <= 9)
			DataAppGrid->nrows++;
		MGLGridMove(id, DataAppGrid, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1);
		if (DataAppGrid->vo->count <= 8)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1 - MGL_GRID_DEF_ROW_HEIGHT);
		else if (DataAppGrid->vo->count > 8)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1);
		if (DataAppGrid->vo->count == 9)
			MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1 - (MGL_GRID_DEF_ROW_HEIGHT));

		//Creating and filling build grids

		CreateBuildSummary(id, SysAlertsGrid);//Create Build summary Grid
		FillBuildSummaryVO(id, SysAlertsGrid->BuildSummaryGrid);//fill Build Summary Grid

		CreateActionsGrid(id, SysAlertsGrid);//Create Criteria Grid
		SysAlertsGrid->ActionsGrid->parent_w->visible = FALSE;//Visisbility of Criteria Grid
		FillActionsGrid(id, SysAlertsGrid, FALSE);//Fill Criteria Grid VO



		CreateParametersGrid1(id, SysAlertsGrid, 0);//Create Parameters Grid
		MGLFillParametersColsVO(SysAlertsGrid);//Fill Parameters Grid
		SysAlertsGrid->ParametersGrid->parent_w->visible = FALSE;//visibilty of Parameters Grid


		CreateAlertsColorGrid(id, SysAlertsGrid, 0);//Create Actions Grid
		MGLInitAlertsColorsVO(SysAlertsGrid);//Fill Actions Grid
		SysAlertsGrid->AlertsColorGrid->parent_w->visible = FALSE; //Visibility of Actions Grid

		CreateGridrowcolumns(id, SysAlertsGrid, 0);//Create Visible Rows grid
		MGLInitGridofRowsColumnsVO(SysAlertsGrid);//Fill visible Rows Coloumns


		CreateSortPanel(id, SysAlertsGrid);//Create Layout Grid
		SysAlertsGrid->RowColumn->parent_w->visible = FALSE;//Show Layout grid

		FillAvailableColsVO(id, SysAlertsGrid);//Fill Layout Grid

		FillActionsGrid1(id, SysAlertsGrid, FALSE);
		FillActionsGrid2(id, SysAlertsGrid, FALSE);
		MGLFillParameters(id, SysAlertsGrid->ParametersGrid);


		MGLFillParametersGrid(SysAlertsGrid->ActionsGrid, SysAlertsGrid->CurActionNum, id);
		//MGLFillParametersColsVO(SysAlertsGrid);
		//FillAvailableColsVO(id, SysAlertsGrid);
		CreateColorMatrix_foreground(id, SysAlertsGrid->AlertsColorGrid, 0);//Create Foreground Color Grid
		CreateColorMatrix_background(id, SysAlertsGrid->AlertsColorGrid, 0);//Create Background Color Grid
		CreateColorMatrix_icon(id, SysAlertsGrid->AlertsColorGrid, 0);//Create Icon color grid
		CreateEMailGrid(id, SysAlertsGrid);//Create Email Grid
		FillEMailVO(id, SysAlertsGrid->AlertsColorGrid->EMailGrid);//Fill Email Grid
		CreateSubjectGrid(id, SysAlertsGrid);//Create Subject Grid
		FillSubjectVO(id, SysAlertsGrid->AlertsColorGrid->SubjectGrid);//Fill Subject Grid
		SysAlertsGrid->AlertsColorGrid->EMailGrid->parent_w->visible = 0;				/* makes Email - subject grid invisble*/
		SysAlertsGrid->AlertsColorGrid->ForegroundColorGrid->parent_w->visible = 0;
		SysAlertsGrid->AlertsColorGrid->BackgroundColorGrid->parent_w->visible = 0;
		SysAlertsGrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
		SysAlertsGrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 0;

		if (!SysAlertsGrid->Actions[SysAlertsGrid->CurActionNum].gridconfig){
			SysAlertsGrid->Actions[SysAlertsGrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
			SysAlertsGrid->nActions = 1;
			SysAlertsGrid->Actions[0].IsActive = TRUE;
			SysAlertsGrid->Actions[0].ActionType = ACTION_FILTER;
		}
		SysAlertsGrid->AlertsColorGrid->show_checkbox[0] = 0;
		SysAlertsGrid->AlertsColorGrid->show_checkbox[1] = 0;
		SysAlertsGrid->AlertsColorGrid->show_checkbox[2] = 0;
		SysAlertsGrid->AlertsColorGrid->show_checkbox[3] = 1;//show_checkbox - helps decide the show or hide icon
		SysAlertsGrid->Actions[0].filter_active = 1;

		CreateTitleGrid(id, SysAlertsGrid->name, SysAlertsGrid);// Creates Title Grid
		SysAlertsGrid->TitleGrid->parent_w->visible = 0;//Sets visibility of the title grid

		//keeping 5 columns of FDO visible as default
	/*or (i = 0; i < 3; i++)
		{
			SysAlertsGrid->DisplayCol[i] = i;
			VV(SysAlertsGrid->SortAvailableGrid->vo, i, "Visible", int) = 1;
		}
		for (i = 3; i < SysAlertsGrid->ncols; i++)
		{
			VV(SysAlertsGrid->SortAvailableGrid->vo, i, "Visible", int) = 0;
			SysAlertsGrid->DisplayCol[i] = -1;
		}
		/*SysAlertsGrid->DisplayCol[6] = 6;
		VV(SysAlertsGrid->SortAvailableGrid->vo, 6, "Visible", int) = 1;
		SysAlertsGrid->DisplayCol[9] = 9;
		VV(SysAlertsGrid->SortAvailableGrid->vo, 9, "Visible", int) = 1;*/

	/*	count = -1;
		strcpy_s(full, 64, "");
		for (int x = 0; x < SysAlertsGrid->SortAvailableGrid->vo->count; x++)
		{
			if (SysAlertsGrid->DisplayCol[x] != -1)
			{
				write_layout_column_fdo(full, x, SysAlertsGrid);
				count++;
				vo_alloc_rows(SysAlertsGrid->BuildSummaryGrid->vo, 1);
				strncpy_s(VP(SysAlertsGrid->BuildSummaryGrid->vo, count, "Layout", char), 64, full, _TRUNCATE);
			}
		}*/
		//SysAlertsGrid->DisplayCol[9] = -1;
		//SysAlertsGrid->DisplayCol[7] = -1;
		return(SysAlertsGrid);
	}

	else
		return(NULL);
	

}

//void MGLCopyAction(struct InstanceData* id, MGLGRID *ActionsGrid,int i)
//{
//	VO *vcol;
//	int valcolnum, vorow;
//	char *textcolname, *curstr;
//	MGLGRID *topgrid;
//	int x;
//	int  red_ival, green_ival, blue_ival, fg_ival, bg_ival;
//
//	assert(id);
//	assert(ActionsGrid);
//	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
//	
//	// create a new action with this name, 
//	topgrid = ActionsGrid->parentgrid;
//	topgrid->CurActionNum = i;
//
//
//	if (!topgrid->Actions[topgrid->CurActionNum].gridconfig){
//		topgrid->Actions[topgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
//	}
//
//	topgrid->Actions[topgrid->CurActionNum].filter_active = 1;
//	SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 1;
//	//else if (topgrid->Actions[topgrid->CurActionNum].filter_active == 0)
//	//	SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 0;
//	// load in data that is stored in etas-db all-etas in table grid_config
//	// LoadUserRoiSettings (id, id->m_strUserName, id->m_strRegionArpt, configname, ROIGrid->Actions[ROIGrid->CurActionNum].gridconfig);
//	topgrid->nActions = i+1;
//	topgrid->Actions[topgrid->CurActionNum].IsActive = TRUE;
//	topgrid->Actions[topgrid->CurActionNum].ActionType = ACTION_FILTER; // default type is ACTION_FILTER
//
//
//	// Set Action name in gridconfig
//	vorow = i;
//	textcolname = ActionsGrid->textcolname;
//	curstr = VP(ActionsGrid->vo, vorow, textcolname, char);
//	strcpy_s(topgrid->Actions[topgrid->CurActionNum].ActionName, sizeof(topgrid->Actions[topgrid->CurActionNum].ActionName),
//		curstr);
//	for (x = 0; x < ActionsGrid->vo->count; x++)
//	{
//		ActionsGrid->vo->row_index[x].fg = MGLColorHex2Int("000000");
//	}
//	/*if (strcmp(VP(ActionsGrid->vo, vorow, 0, char), "") == 0)
//	{
//		strncpy_s(VP(ActionsGrid->vo, vorow, "Name", char), 64, "New", _TRUNCATE);
//		ActionsGrid->parentgrid->Actions[vorow].IsActive = 0;
//	}
//	if (strcmp(VP(ActionsGrid->vo, vorow, 0, char), "New") != 0)
//	{
//		ActionsGrid->vo->row_index[vorow].fg = MGLColorHex2Int("529EFD");
//		ActionsGrid->parentgrid->Actions[vorow].IsActive = 1;
//		vo_alloc_rows(ActionsGrid->vo, 1);
//		strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "Name", char), 64, "New", _TRUNCATE);
//		strncpy_s(VP(ActionsGrid->vo, ActionsGrid->vo->count - 1, "OnOff", char), 16, "On", _TRUNCATE);
//	}*/
//	///
//	topgrid->Summary[topgrid->nActions].Criteria_rownumber = ActionsGrid->textrow + 1;
//
//	red_ival = 0;
//	green_ival = 0;
//	blue_ival = 0;
//
//	fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
//	SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[0].fg = fg_ival;
//	SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[0] = 0;
//
//	bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
//	SecondTabGrid->childgrid->AlertsColorGrid->vo->row_index[1].bg = bg_ival;
//	SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[1] = 0;
//
//
//
//	// fill in the ParametersGrid with data from this new Action, which should be blank
//	MGLFillParametersGrid(ActionsGrid, topgrid->CurActionNum, id);
//	InitParametersinBuildSummary(id, SecondTabGrid->childgrid);
//	CalcColWidths(id, ActionsGrid->parentgrid->ParametersGrid);
//	MGLFillColorsGrid(id, ActionsGrid->parentgrid->ParametersGrid, topgrid->CurActionNum);
//	CalcColWidths(id, ActionsGrid);
//
//}

//mtm to copy the criteria of the grid selected into the new grid criteria
//void Copy_Criteria(struct InstanceData* id, MGLGRID *GridDestination)
//{
//	MGLGRID *GridSource;
//	int i;
//	char Name[64],Name_source[64];
//	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
//	id->DataAppGrid = DataAppGrid;
//	GridSource = VV(DataAppGrid->vo, DataAppGrid->vonumber, "gridptr", void *);
//	
//		for (i = 0; i < 100; i++)
//		{
//			if (i < GridSource->ActionsGrid->vo->count)
//			{
//				strncpy_s(Name_source, 64, VP(GridSource->ActionsGrid->vo, i, "Name", char), _TRUNCATE);
//				if (strcmp(Name_source, "") != 0)
//				{
//					if (GridDestination->ActionsGrid->vo->count <= GridSource->nActions)
//						vo_alloc_rows(GridDestination->ActionsGrid->vo, 1);					
//					strncpy_s(VP(GridDestination->ActionsGrid->vo, i, "Name", char), 64, VP(GridSource->ActionsGrid->vo, i, "Name", char), _TRUNCATE);					
//					strncpy_s(VP(GridDestination->ActionsGrid->vo, i, "OnOff", char), 16, VP(GridSource->ActionsGrid->vo, i, "OnOff", char), _TRUNCATE);
//					if (strcmp(Name_source, "New")!=0)
//						MGLCopyAction(id,GridDestination->ActionsGrid,i);
//				}
//			}
//			else
//				break;
//		}
//	
//
//		//Filling in the Criteria of the selected grid build summary into new grid build summary
//		InitCriteriainBuildSummary(id, GridDestination);
//}

//void MGLCopyParameters(struct InstanceData* id, MGLGRID *ParametersGrid)
//{
//	VO *vcol;
//	int valcolnum;
//	char *colname;
//	MGLGRID *SourceGrid;
//	int line_count;
//	char *curstr;
//
//	assert(id);
//	assert(ParametersGrid);
//	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
//	MGLGRID* WarningGrid = (MGLGRID*)id->WarningGrid;
//
//	SecondTabGrid->childgrid->iVscrollPos = 0;
//	// the vorow is one less than gridcells since the title row is skipped
//	vcol = vcol_find(ParametersGrid->vo, "Value1");
//	valcolnum = vcol->colnum;
//	ParametersGrid->gridcells[ParametersGrid->textrow + 1][valcolnum].bg = (GLfloat*)NULL;
//	ParametersGrid->gridcells[ParametersGrid->textrow + 1][valcolnum].fg = (GLfloat*)NULL;
//	//  put the operator and value results into the filter grid
//	// .....
//	SourceGrid = ParametersGrid->parentgrid;
//	colname = VP(ParametersGrid->vo, ParametersGrid->textrow, "Column", char);
//
//
//	MGLFillParameters(id, ParametersGrid);
//	// reset to the new values and clear out bad ones
//	//colname = VP(ParametersGrid->vo, ParametersGrid->textrow, "Column", char);
//	//cmp_value = VP(ParametersGrid->vo, ParametersGrid->textrow, "Value1", char);
//	//field_name = VP(ParametersGrid->vo, ParametersGrid->textrow, "Operator", char);	
//	MGLFillParametersGrid(SourceGrid->ActionsGrid, SourceGrid->CurActionNum, id);
//	curstr = VP(SourceGrid->ActionsGrid->vo, SourceGrid->CurActionNum, "Name", char);
//	if (strcmp(curstr, "New") == 0)
//	{
//		if (SourceGrid->BuildSummaryGrid)//Message is not displayed when we create the grid; Message displayed when we try to use filters without associating it with a criteria name
//		{
//
//			WarningGrid = CreateWarning(id);
//			id->WarningGrid = WarningGrid;
//			MGLGridText(id, WarningGrid, 0, 0, "Warning: Give a criteria name");
//			WarningGrid->parent_w->visible = TRUE;
//
//		}
//	}
//	//SourceGrid = ActionsGrid->parentgrid;
//	line_count = 0;
//	if (SecondTabGrid->childgrid->BuildSummaryGrid->vo)
//	{
//		for (int t = 0; t < SecondTabGrid->childgrid->BuildSummaryGrid->vo->count; t++)
//		{
//			strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, t, "Parameters", char), 64, "", _TRUNCATE);
//			strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, t, "Criteria", char), 64, "", _TRUNCATE);
//		}
//
//
//	}
//	line_count = 0;
//	InitParametersinBuildSummary(id, SecondTabGrid->childgrid);
//
//
//	/*for (int t = 0; t < SecondTabGrid->childgrid->nActions; t++)
//	{
//	gridconfig = SecondTabGrid->childgrid->Actions[t].gridconfig;
//	if (t == 0)
//	{
//	if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count < 1)
//	vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
//	strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Criteria", char), 64, SecondTabGrid->childgrid->Actions[t].ActionName, _TRUNCATE);
//	}
//	else
//	{
//
//	if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count < line_count)
//	vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
//	strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, line_count, "Criteria", char), 64, SecondTabGrid->childgrid->Actions[t].ActionName, _TRUNCATE);
//	}
//	for (int f = 0; gridconfig && f < gridconfig->nFilters; f++)
//	{
//	op = gridconfig->filters[f].op;
//	if (strcmp(op, "") != 0)
//	{
//	line_count++;
//
//	}
//	}
//
//
//	}*/
//	InitCriteriainBuildSummary(id, SecondTabGrid->childgrid);
//
//
//
//	CalcColWidths(id, ParametersGrid);
//	SecondTabGrid->aircraftcolor_change_MLAT = 1;
//	SecondTabGrid->aircraftcolor_change_ASDEX = 1;
//	SecondTabGrid->aircraftcolor_change_Noise = 1;
//	SecondTabGrid->aircraftcolor_change_Gate = 1;
//	SecondTabGrid->aircraftcolor_change_ASD = 1;
//	SecondTabGrid->aircraftcolor_change = 1;
//	SecondTabGrid->updateroi = 1;
//}

//char *FieldName(char *fieldname)
//{
//	if (strcmp("EQ", fieldname) == 0)
//		return "Equal to";
//	else if (strcmp("NE", fieldname) == 0)
//		return "Not Equal to";
//	else if (strcmp("GT", fieldname) == 0)
//		return "Greater than";
//	else if (strcmp("LT", fieldname) == 0)
//		return "Less than";
//	else if (strcmp("LE", fieldname) == 0)
//		return "Less than or equal to";
//	else if (strcmp("GE", fieldname) == 0)
//		return "Greater than or equal to";
//	else
//		return "";
//
//}

//void Copy_Parameters(struct InstanceData* id, MGLGRID *GridDestination)
//{
//
//	MGLGRID *GridSource;
//	int i,j,k;
//	char Parameter_Operator[32],Value[256];
//	GridConfig *gridconfig;
//	char *op, *cmp_value, *field_name,*op_full;
//
//	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
//	id->DataAppGrid = DataAppGrid;
//	GridSource = VV(DataAppGrid->vo, DataAppGrid->vonumber, "gridptr", void *);
//
//	assert(id);
//	assert(GridDestination);
//	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
//
//
//	
//	//MGLClearParametersGrid(ActionsGrid->parentgrid); // clear out current settings from action parameters
//	
//	//MGLFillParametersGrid(ActionsGrid, ActionsGrid->parentgrid->CurActionNum, id);
//
//	//for (i = 0; i < GridSource->nActions; i++)
//	//{
//	//	GridDestination->CurActionNum = i;
//	//	for (j = 0; j < GridDestination->ParametersGrid->vo->count; j++)
//	//	{
//	//		strcpy_s(Parameter_Operator, 32, VP(GridSource->ParametersGrid->vo, j, "Operator", char));
//	//		if (strcmp(Parameter_Operator, ""))
//	//		{
//	//			strcpy_s(VP(GridDestination->ParametersGrid->vo, j, "Operator", char), 32, VP(GridSource->ParametersGrid->vo, j, "Operator", char));
//	//			strcpy_s(VP(GridDestination->ParametersGrid->vo, j, "Value1", char), 32, VP(GridSource->ParametersGrid->vo, j, "Value1", char));
//	//			
//	//		}
//	//	}
//
//	//	MGLFillParameters(id, GridDestination->ParametersGrid);
//	//	MGLFillParametersGrid(GridDestination->ActionsGrid, i, id);
//	//	//MGLCopyParameters(id, ParametersGrid);
//	//}
//
//	for (i = 0; i < GridSource->nActions; i++)
//	{
//		GridDestination->CurActionNum = i;
//		gridconfig = GridSource->Actions[i].gridconfig;		
//		for (j = 0; gridconfig && j < gridconfig->nFilters; j++){
//			op = gridconfig->filters[j].op;
//			cmp_value = gridconfig->filters[j].cmp_value;
//			field_name = gridconfig->filters[j].field_name;			
//			op_full = FieldName(op);
//
//			for (k = 0; k < GridDestination->ParametersGrid->vo->count; k++)
//			  {
//				  if (strcmp(field_name, VP(GridSource->ParametersGrid->vo, k, "Column", char)) == 0)
//					  break;
//				
//			  }
//			strcpy_s(VP(GridDestination->ParametersGrid->vo, k, "Operator", char), 32, "Equal to");
//			strcpy_s(VP(GridDestination->ParametersGrid->vo, k, "Value1", char), 256, cmp_value);
//			MGLFillParameters(id, GridDestination->ParametersGrid);
//			MGLFillParametersGrid(GridDestination->ActionsGrid, i, id);
//		}
//		
//	}
//	
//	SecondTabGrid->updateroi = 1;
//
//
//}
//void Copy_Grid(struct InstanceData* id, MGLGRID *Grid)
//{
//	int t;
//	t = 100;	
//	Copy_Criteria(id, Grid);
//	Copy_Parameters(id, Grid);
//
//}
//mtm
int DataAppCreatorCallback(struct InstanceData* id, MGLGRID *DataAppCreater, int b, int m, int x, int y)
{
// Callback function for the dataappcreator grid	
	int r, vorow, c, gridmenuheight, colstartx;
	char *AppName, *DataSource;	
	int check = 0;
	MGLGRID *Grid;
	
	assert(id);

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	id->DataAppGrid = DataAppGrid;
	MGLGRID* DataTypeMenuGrid = (MGLGRID*)id->DataTypeMenuGrid;	
	MGLGRID* Grid_Report = (MGLGRID*)id->Grid_Report;
	MGLGRID* Grid_List = (MGLGRID*)id->Grid_List;
	

	if (!DataAppCreater) return(FALSE);
	if (m == 1) return(FALSE); // ignore mouse down, only activate on mouse up

	if (DataAppCreater->gridrows[0].height != MGL_DEFAULT){
		// only works if rows all have same height
		r = (int)floor(((double)y - DataAppCreater->gridrect.y1) / DataAppCreater->gridrows[0].height);
	}
	else {
		r = (int)floor(((double)y - DataAppCreater->gridrect.y1) / (double)MGL_GRID_DEF_ROW_HEIGHT);
	}

	
	if (r < 0 || r >= DataAppCreater->nrows){
		return(FALSE);
	}
	// reverse the order
	r = DataAppCreater->nrows - r - 1;

	if (r == 0){
		// user selected Title row, so ignore
		return(TRUE);
	}
	vorow = r - 1 + DataAppCreater->iVscrollPos;
	// see which column was selected
	c = MGLCalcCol(DataAppCreater, x);


	//columns 3,5,7 are the ones in use. c==2(column 3) corresponds to "Add New", c==5 (column 6) corresponds to "Source", c==7 (column 8) corresponds to "Create" 
	if (c == 3 && vorow == 0){
		int t = 0;
		AppName = VP(DataAppCreater->vo, vorow, "AppName", char);
		if (!strcmp(AppName, "  Add New")){//Spaces in the string is to adjust the look in the GUI
			// user wants to create a new app, so set the text input box and allow him to enter the name of the app
			// popup white text box in place of the "New"
			// clear out current string
			AppName[0] = '\0';
			DataAppCreater->textrow = vorow;
			DataAppCreater->parent_w->TextActive = TRUE;
			DataAppCreater->character_limit = 1;
			ActiveTextGrid = DataAppCreater;
			strcpy_s(DataAppCreater->textcolname, sizeof(DataAppCreater->textcolname), "AppName");
			DataAppCreater->vo->row_index[vorow].fg = MGLColorHex2Int("Black");
			AppName = VP(DataAppCreater->vo, vorow, "AppName", char);

		}
		
		else if (!strcmp(AppName, "")){
			// user wants to create a new app, so set the text input box and allow him to enter the name of the app
			// popup white text box in place of the "New"
			// clear out current string
			AppName[0] = '\0';
			DataAppCreater->textrow = vorow;
			DataAppCreater->parent_w->TextActive = TRUE;
			DataAppCreater->character_limit = 1;
			ActiveTextGrid = DataAppCreater;
			strcpy_s(DataAppCreater->textcolname, sizeof(DataAppCreater->textcolname), "AppName");
			DataAppCreater->vo->row_index[vorow].fg = MGLColorHex2Int("Black");
			AppName = VP(DataAppCreater->vo, vorow, "AppName", char);

		}
		
	}

	else if (c == 5 && vorow == 0)
	{

		// see if the Choose button was selected
		if (!strcmp("  Source", VP(DataAppCreater->vo, vorow, "DataSource", char))){
			// if user did not pick a name yet, ignore
			AppName = VP(DataAppCreater->vo, vorow, "AppName", char);
			//call the function
			if (!strcmp(AppName, "  Add New") || !strlen(AppName)){
				// user must choose a name first, so skip 
			}
			else {
				// have the user select from FDO, etc
				if (!DataTypeMenuGrid){
					// popup the menu
					DataTypeMenuGrid = CreateDataTypeMenuGrid(id, DataAppCreater);
					id->DataTypeMenuGrid = DataTypeMenuGrid;
				}
				if (DataTypeMenuGrid && DataTypeMenuGrid->parent_w){
					// make sure top of menu is just below the header row of parent grid
					// how big is GridMenu?
					gridmenuheight = DataTypeMenuGrid->gridrect.y2 - DataTypeMenuGrid->gridrect.y1 - 145;
					colstartx = DataAppGrid->gridcells[0][1].cellrect.x1;
					MGLGridMove(id, DataTypeMenuGrid, DataAppCreater->gridrect.x1 + (19 * MGL_GRID_DEF_ROW_HEIGHT), DataAppCreater->gridrect.y1 - gridmenuheight - (7 * MGL_GRID_DEF_ROW_HEIGHT));
					DataTypeMenuGrid->parent_w->visible = TRUE;
				}
			}
		}
		else if (!strcmp("ROI", VP(DataAppCreater->vo, vorow, "DataSource", char))){
			// if user did not pick a name yet, ignore
			AppName = VP(DataAppCreater->vo, vorow, "AppName", char);
			//call the function
			if (!strcmp(AppName, "  Add New") || !strlen(AppName)){
				// user must choose a name first, so skip 
			}
			else {
				// have the user select from FDO, etc
				if (!DataTypeMenuGrid){
					// popup the menu
					DataTypeMenuGrid = CreateDataTypeMenuGrid(id, DataAppCreater);
					id->DataTypeMenuGrid = DataTypeMenuGrid;
				}
				if (DataTypeMenuGrid && DataTypeMenuGrid->parent_w){
					// make sure top of menu is just below the header row of parent grid
					// how big is GridMenu?
					gridmenuheight = DataTypeMenuGrid->gridrect.y2 - DataTypeMenuGrid->gridrect.y1 - 145;
					colstartx = DataAppGrid->gridcells[0][1].cellrect.x1;
					MGLGridMove(id, DataTypeMenuGrid, DataAppCreater->gridrect.x1 + (19 * MGL_GRID_DEF_ROW_HEIGHT), DataAppCreater->gridrect.y1 - gridmenuheight - (7 * MGL_GRID_DEF_ROW_HEIGHT));
					DataTypeMenuGrid->parent_w->visible = TRUE;
				}
			}
		}
		else if (!strcmp("FDO", VP(DataAppCreater->vo, vorow, "DataSource", char))){
			// if user did not pick a name yet, ignore
			AppName = VP(DataAppCreater->vo, vorow, "AppName", char);
			//call the function
			if (!strcmp(AppName, "  Add New") || !strlen(AppName)){
				// user must choose a name first, so skip 
			}
			else {
				// have the user select from FDO, etc
				if (!DataTypeMenuGrid){
					// popup the menu
					DataTypeMenuGrid = CreateDataTypeMenuGrid(id, DataAppCreater);
					id->DataTypeMenuGrid = DataTypeMenuGrid;
				}
				if (DataTypeMenuGrid && DataTypeMenuGrid->parent_w){
					// make sure top of menu is just below the header row of parent grid
					// how big is GridMenu?
					gridmenuheight = DataTypeMenuGrid->gridrect.y2 - DataTypeMenuGrid->gridrect.y1 - 145;
					colstartx = DataAppGrid->gridcells[0][1].cellrect.x1;
					MGLGridMove(id, DataTypeMenuGrid, DataAppCreater->gridrect.x1 + (19 * MGL_GRID_DEF_ROW_HEIGHT), DataAppCreater->gridrect.y1 - gridmenuheight - (7 * MGL_GRID_DEF_ROW_HEIGHT));
					DataTypeMenuGrid->parent_w->visible = TRUE;
				}
			}
		}


		else if (!strcmp("SysAlerts", VP(DataAppCreater->vo, vorow, "DataSource", char))){
			// if user did not pick a name yet, ignore
			AppName = VP(DataAppCreater->vo, vorow, "AppName", char);
			//call the function
			if (!strcmp(AppName, "  Add New") || !strlen(AppName)){
				// user must choose a name first, so skip 
			}
			else {
				// have the user select from FDO, etc
				if (!DataTypeMenuGrid){
					// popup the menu
					DataTypeMenuGrid = CreateDataTypeMenuGrid(id, DataAppCreater);
					id->DataTypeMenuGrid = DataTypeMenuGrid;
				}
				if (DataTypeMenuGrid && DataTypeMenuGrid->parent_w){
					// make sure top of menu is just below the header row of parent grid
					// how big is GridMenu?
					gridmenuheight = DataTypeMenuGrid->gridrect.y2 - DataTypeMenuGrid->gridrect.y1 - 145;
					colstartx = DataAppGrid->gridcells[0][1].cellrect.x1;
					MGLGridMove(id, DataTypeMenuGrid, DataAppCreater->gridrect.x1 + 350, DataAppCreater->gridrect.y1 - gridmenuheight - 125);
					DataTypeMenuGrid->parent_w->visible = TRUE;
				}
			}
		}



	}


	else if (c == 7 && vorow == 0)
	{
		AppName = VP(DataAppCreater->vo, vorow, "AppName", char);
		if (strcmp(AppName, "") != 0)
		{
			DataSource = VP(DataAppCreater->vo, vorow, "DataSource", char);		
			//call the function

			if (strcmp(AppName, "  Add New") != 0){
				if (strcmp(DataSource, "  Source") != 0)
				{
					Grid = AddRowDataAppGrid(id);//the function where grid is built and also the corresponding build grids are built	
					/*if (Grid_List)
						Grid_List->parent_w->visible = FALSE;*/
					ActiveTextGrid = NULL;
					/*if (strcmp(VP(DataAppCreater->vo, vorow, "Create", char), "Copy" )== 0)
					{
						Copy_Grid(id, Grid);
						strcpy_s(VP(DataAppCreater->vo, vorow, "Create", char), 64,"Create");
					}*/
				}
			}

		}
	}

	return(TRUE);
}



MGLGRID *CreateDataAppSelector(struct InstanceData* id, MGLGRID *DataAppGrid)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);

	gui* oglgui = (gui*)id->m_poglgui;
	//	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;

	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "DataAppSelector");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	//w->gregion.x2 = 200;
	//w->gregion.y2 = 570;
	//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;

	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;


	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;



	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "DataAppWidget");
	w->gregion.x1 = 500;
	w->gregion.y1 = 320;
	w->gregion.x2 = 400;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "DataAppSelectorGrid", 10, 9, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->grid->edit = 0;//to display the rules-layout icon
	w->visible = 0;
	w->grid->AllowScrollbar = TRUE;
	w->grid->HasTitleBar = TRUE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	DataAppGrid = w->grid;


	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)white;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey2;
	}


	w->grid->gridrows[0].fg = Text_yellow;



	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "    ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "    ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "Grid                  ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, " Source", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "Active", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, "  Show", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[7], MGL_GRID_LABEL_MAX, "   Save", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[8], MGL_GRID_LABEL_MAX, " Detail", _TRUNCATE);




	//w->grid->gridcols[2].val_callback = Cnv2HideShow_mtm;

	// we also need to fetch initial filter values from user settings DB
	for (int xx = 0; xx < 100; xx++)
	{
		DataAppGrid->show_checkbox[xx] = 0;
	}





	// set row callbacks
	for (i = 0; i < DataAppGrid->nrows; i++){
		DataAppGrid->gridrows[i].row_callback = DataAppCallback;
	}

	strncpy_s(w->grid->gridcols[0].pngfilename, sizeof(w->grid->gridcols[0].pngfilename), "up-reorder-arrow.png", _TRUNCATE);
	w->grid->gridcols[0].texturesize = 16;
	w->grid->gridcols[0].img_displaysize = 10;

	strncpy_s(w->grid->gridcols[1].pngfilename, sizeof(w->grid->gridcols[1].pngfilename), "down-reorder-arrow.png", _TRUNCATE);
	w->grid->gridcols[1].texturesize = 16;
	w->grid->gridcols[1].img_displaysize = 10;

	strncpy_s(w->grid->gridcols[2].pngfilename, sizeof(w->grid->gridcols[2].pngfilename), "alert-red-x.png", _TRUNCATE);
	w->grid->gridcols[2].texturesize = 16;
	w->grid->gridcols[2].img_displaysize = 10;

	strncpy_s(w->grid->gridcols[5].pngfilename, sizeof(w->grid->gridcols[5].pngfilename), "show-circle-off.png", _TRUNCATE);
	w->grid->gridcols[5].texturesize = 16;
	w->grid->gridcols[5].img_displaysize = 10;


	strncpy_s(w->grid->gridcols[6].pngfilename, sizeof(w->grid->gridcols[6].pngfilename), "show-circle-off.png", _TRUNCATE);
	w->grid->gridcols[6].texturesize = 16;
	w->grid->gridcols[6].img_displaysize = 10;

	strncpy_s(w->grid->gridcols[7].pngfilename, sizeof(w->grid->gridcols[7].pngfilename), "save-icon-thick.png", _TRUNCATE);
	w->grid->gridcols[7].texturesize = 16;
	w->grid->gridcols[7].img_displaysize = 10;

	strncpy_s(w->grid->gridcols[8].pngfilename, sizeof(w->grid->gridcols[8].pngfilename), "zoom-arrow-off.png", _TRUNCATE);
	w->grid->gridcols[8].texturesize = 32;
	w->grid->gridcols[8].img_displaysize = 19;



	// set TITLE text widgets
	w = (widgets *)vo_malloc(sizeof(widgets));
	DataAppGrid->title_widgets = w;

	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
	// give location of widget relative to title_rect
	w->gregion.x1 = DataAppGrid->marginx;
	w->gregion.y1 = DataAppGrid->marginy;
	w->gregion.x2 = w->gregion.x1 + 120;
	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - DataAppGrid->marginy;
	w->type = WGT_TEXT;
	w->wgt_mouse_cb = title_view_cb;
	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "GRID Manager", _TRUNCATE);
	w->visible = 0;
	w->grid = DataAppGrid;
	DataAppGrid->fixed_width = 1;

	DataAppGrid->nrows = 2;
	//DataAppGrid->height = (DataAppGrid->nrows+1) * MGL_GRID_DEF_ROW_HEIGHT;
	DataAppGrid->check_firstopen = 1;
	id->m_poglgui = oglgui;
	return(DataAppGrid);
	//id->DataAppGrid = DataAppGrid;
}
//mtm new gui --grid to enter the new grid that we want to create
MGLGRID *CreateDataAppCreater(struct InstanceData* id, MGLGRID *DataAppCreater)
{
	widgets *w, *lastw, *panel_widget;
	int r, i;

	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;

	id->DataAppGrid = DataAppGrid;
	
	// find last widget in list and add it here
	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
		lastw = w;
	}

	// Create panel for holding the two grids plus buttons
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "DataAppCreater");
	w->gregion.x1 = 20;
	w->gregion.y1 = 370;
	w->width = 500;
	w->height = 300;
	
	w->type = WGT_PANEL;
	w->visible = 0;  // don't show panel
	w->bg = (GLfloat*)gridgrey0;
	w->fg = (GLfloat*)white;
	panel_widget = w;


	// add new widget
	if (lastw){
		lastw->np = w;
	}
	else {
		// first widget
		oglgui->widgets = w;
	}
	lastw = w;



	// Create MGLGrid for list of Available columns
	w = (widgets *)vo_malloc(sizeof(widgets));
	strcpy_s(w->name, sizeof(w->name), "DataAppCreaterWidget");
	w->gregion.x1 = DataAppGrid->gridrect.x1;
	w->gregion.y1 = DataAppGrid->gridrect.y1 + 90;
	w->gregion.x2 = 400;
	w->gregion.y2 = id->m_nHeight;
	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel

	w->type = WGT_GRID;
	w->grid = MGLGridCreate(id, NULL, "DataAppCreaterGrid", 3,9, w->gregion.x1, w->gregion.y1);
	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
	w->grid->iVscrollPos = 0;
	w->grid->GScrollWidth = 20;
	w->grid->edit = 0;//to display the rules-layout icon
	w->visible = 0;
	w->grid->AllowScrollbar = FALSE;
	w->grid->HasTitleBar = FALSE;
	// add default colors for grid and the rows
	w->grid->bg = (GLfloat*)gridgrey0;
	w->grid->fg = (GLfloat*)white;
	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
	DataAppCreater = w->grid;

	for (r = 0; r < w->grid->nrows; r += 2){
		if (r == 0){
			w->grid->gridrows[r].fg = (GLfloat*)white;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		else {
			w->grid->gridrows[r].fg = (GLfloat*)black;
			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
		}
		w->grid->gridrows[r].bg = (GLfloat*)Grid_grey2;
		w->grid->gridrows[r + 1].bg = (GLfloat*)Grid_grey2;
	}





	for (i = 0; i < DataAppCreater->nrows; i++){
		DataAppCreater->gridrows[i].row_callback = DataAppCreatorCallback;
	}


	// add new widget
	if (lastw){
		lastw->np = w;
		lastw = w;
	}

	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "    ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "    ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "                      ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[4], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[5], MGL_GRID_LABEL_MAX, "      ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[6], MGL_GRID_LABEL_MAX, "      ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[7], MGL_GRID_LABEL_MAX, "       ", _TRUNCATE);
	strncpy_s(w->grid->ColLabels[8], MGL_GRID_LABEL_MAX, "     ", _TRUNCATE);

	DataAppCreater->gridcells[1][3].bg = (GLfloat*)Grid_grey5;
	DataAppCreater->gridcells[1][3].fg = (GLfloat*)Grid_grey3;
	DataAppCreater->gridcells[1][5].fg = (GLfloat*)white;
	DataAppCreater->gridcells[1][5].bg = (GLfloat*)Grid_grey3;
	DataAppCreater->gridcells[1][7].fg = (GLfloat*)Text_yellow;

	DataAppCreater->fixed_width = 2;	
	id->m_poglgui = oglgui;
	return DataAppCreater;

}




//mtm

//void CreateMainTab(struct InstanceData* id)
//{
//	widgets *w, *lastw, *panel_widget;
//	int r;
//
//	assert(id);
//	gui* oglgui = (gui*)id->m_poglgui;
//	// find last widget in list and add it here
//	for (w = oglgui->widgets, lastw = NULL; w; w = w->np){
//		lastw = w;
//	}
//
//	// Create panel for holding the two grids plus buttons
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "MainTab");
//	w->gregion.x1 = 20;
//	w->gregion.y1 = 370;
//	w->width = 500;
//	w->height = 300;
//	//w->gregion.x2 = 200;
//	//w->gregion.y2 = 570;
//	//	w->gregion_parentw = parentgrid->parent_w; // coordinates relative to parent widget;
//
//	w->type = WGT_PANEL;
//	w->visible = 0;  // don't show panel
//	w->bg = (GLfloat*)gridgrey0;
//	w->fg = (GLfloat*)white;
//	panel_widget = w;
//
//
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//	}
//	else {
//		// first widget
//		oglgui->widgets = w;
//	}
//	lastw = w;
//
//
//
//	// Create MGLGrid for list of Available columns
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	strcpy_s(w->name, sizeof(w->name), "MainTabWidget");
//	w->gregion.x1 = 20;
//	w->gregion.y1 = 100;
//	w->gregion.x2 = 400;
//	w->gregion.y2 = id->m_nHeight + 250;
//	w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
//
//	w->type = WGT_GRID;
//	w->grid = MGLGridCreate(id, NULL, "MainTabGrid", 20, 4, w->gregion.x1, w->gregion.y1);
//	w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
//	w->grid->iVscrollPos = 0;
//	w->grid->GScrollWidth = 20;
//	w->grid->RulesLayout = 0;//to display the rules-layout icon
//	w->visible = 0;
//	w->grid->AllowScrollbar = FALSE;
//	w->grid->HasTitleBar = TRUE;
//	// add default colors for grid and the rows
//	w->grid->bg = (GLfloat*)gridgrey0;
//	w->grid->fg = (GLfloat*)white;
//	w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
//	MainTabGrid = w->grid;
//
//	for (r = 0; r < w->grid->nrows; r += 2){
//		if (r == 0){
//			w->grid->gridrows[r].fg = (GLfloat*)black;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
//		}
//		else {
//			w->grid->gridrows[r].fg = (GLfloat*)black;
//			w->grid->gridrows[r + 1].fg = (GLfloat*)black;
//		}
//		w->grid->gridrows[r].bg = (GLfloat*)grey_Tab_bg;
//		w->grid->gridrows[r + 1].bg = (GLfloat*)grey_Tab_bg;
//	}
//	w->grid->gridrows[0].fg = (GLfloat*)white;
//	// add new widget
//	if (lastw){
//		lastw->np = w;
//		lastw = w;
//	}
//
//	strncpy_s(w->grid->ColLabels[0], MGL_GRID_LABEL_MAX, "Grid List             ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[1], MGL_GRID_LABEL_MAX, "                      ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[2], MGL_GRID_LABEL_MAX, "Grid Builder          ", _TRUNCATE);
//	strncpy_s(w->grid->ColLabels[3], MGL_GRID_LABEL_MAX, "                      ", _TRUNCATE);
//
//	w->grid->gridrows[0].row_callback = MainTabCallback;
//	// we also need to fetch initial filter values from user settings DB
//
//	// set TITLE text widgets
//	w = (widgets *)vo_malloc(sizeof(widgets));
//	MainTabGrid->title_widgets = w;
//
//	strcpy_s(w->name, sizeof(w->name), "TitleWidget");
//	// give location of widget relative to title_rect
//	w->gregion.x1 = MainTabGrid->marginx;
//	w->gregion.y1 = MainTabGrid->marginy;
//	w->gregion.x2 = w->gregion.x1 + 120;
//	w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - MainTabGrid->marginy;
//	w->type = WGT_TEXT;
//	w->wgt_mouse_cb = title_view_cb;
//	strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "GRID Tool", _TRUNCATE);
//	w->visible = 0;
//	w->grid = MainTabGrid;
//	id->m_poglgui = oglgui;
//
//
//}




#ifdef NEED_2_GRIDS
// Create MGLGrid for list of Selected SORT columns
w = (widgets *)vo_malloc(sizeof(widgets));
strcpy_s(w->name, sizeof(w->name), "SelectedSortWidget");
w->gregion.x1 = 20;
w->gregion.y1 = 370;
w->gregion.x2 = 200;
w->gregion.y2 = 570;
w->gregion_parentw = panel_widget; // coordinates relative to parent sort panel
w->type = WGT_GRID;
w->grid = MGLGridCreate(id, parentgrid, "SortSelectedGrid", 10, 2, w->gregion.x1, w->gregion.y1);
w->grid->width = 150;  // make column wide enough to fit all region names, since there is not a vo, it will not auto-adjust
w->grid->iVscrollPos = 0;
w->grid->GScrollWidth = 20;
w->visible = 1;
w->grid->AllowScrollbar = TRUE;
w->grid->HasTitleBar = TRUE;
w->grid->parent_w = w; // path back to parent widget, needed for things like setting visibility of widget
// add default colors for grid and the rows
w->grid->bg = (GLfloat*)gridgrey0;
w->grid->fg = (GLfloat*)white;
for (r = 0; r < w->grid->nrows; r++){
	if (r == 0){
		w->grid->gridrows[r].fg = (GLfloat*)yellow;
	}
	else {
		w->grid->gridrows[r].fg = (GLfloat*)white;
	}
	w->grid->gridrows[r].bg = (GLfloat*)gridgreyelev;
}
// add new widget
if (lastw){
	lastw->np = w;
	lastw = w;
}
SortSelectedGrid = w->grid;

// add SUBMIT widget to linked list
w = (widgets *)vo_malloc(sizeof(widgets));
strcpy_s(w->name, sizeof(w->name), "SubmitSortWidget");
// give location of widget relative to title_rect
w->gregion.x1 = ROIGrid->marginx;
w->gregion.y1 = ROIGrid->marginy;
w->gregion.x2 = w->gregion.x1 + 120;
w->gregion.y2 = w->gregion.y1 + MGL_GRID_DEF_ROW_HEIGHT - ROIGrid->marginy;
w->type = WGT_TEXT;
w->wgt_mouse_cb = sort_submit_cb;
strncpy_s(w->textbuf, MGL_GRID_TEXTBUF_SIZE, "Sort Order", _TRUNCATE);
w->visible = 1;

w->grid = SortSelectedGrid;
SortSelectedGrid->title_widgets = w;
#endif





void CrAlertsColorVO(MGLGRID *parentgrid)
{

	VO *AlertsColorVO;

	assert(parentgrid);

	AlertsColorVO = (VO *)vo_create(0, NULL);
	vo_set(AlertsColorVO, V_NAME, "AlertsColorVO", NULL);
	VOPropAdd(AlertsColorVO, "Column", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(AlertsColorVO, "Settings", NTBSTRINGBIND, 16, VO_NO_ROW);	

	//  THIS LOOKS WRONG   vo_set(AlertsColorVO, 2, "Column", NULL, NULL);

	parentgrid->AlertsColorGrid->vo = AlertsColorVO;
	
}



void CrAlertsColorVO_me(MGLGRID *parentgrid)
{

	VO *AlertsColorVO;

	assert(parentgrid);

	AlertsColorVO = (VO *)vo_create(0, NULL);
	vo_set(AlertsColorVO, V_NAME, "AlertsColorVO", NULL);
	VOPropAdd(AlertsColorVO, "Column", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(AlertsColorVO, "Settings", NTBSTRINGBIND, 16, VO_NO_ROW);
	
	//  BUG??  vo_set(AlertsColorVO, 2, "Column", NULL, NULL);

	parentgrid->GridActions->vo = AlertsColorVO;


}

void CrAlertsColorPaletteVO(MGLGRID *parentgrid)
{

	VO *AlertsColorVO;

	assert(parentgrid);

	AlertsColorVO = (VO *)vo_create(0, NULL);
	vo_set(AlertsColorVO, V_NAME, "AlertsColorPaletteVO", NULL);	

	VOPropAdd(AlertsColorVO, "Yellow1", NTBSTRINGBIND, 16, VO_NO_ROW);  // 
	VOPropAdd(AlertsColorVO, "Yellow2", NTBSTRINGBIND, 16, VO_NO_ROW);  // 
	VOPropAdd(AlertsColorVO, "Yellow3", NTBSTRINGBIND, 16, VO_NO_ROW);  // 
	VOPropAdd(AlertsColorVO, "Yellow4", NTBSTRINGBIND, 16, VO_NO_ROW);  // 
	VOPropAdd(AlertsColorVO, "Yellow5", NTBSTRINGBIND, 16, VO_NO_ROW);  // 

	VOPropAdd(AlertsColorVO, "Maroon", NTBSTRINGBIND, 16, 1);
	VOPropAdd(AlertsColorVO, "Yellow1", NTBSTRINGBIND, 16, 1);  // 
	VOPropAdd(AlertsColorVO, "Yellow2", NTBSTRINGBIND, 16, 1);  // 
	VOPropAdd(AlertsColorVO, "Yellow3", NTBSTRINGBIND, 16, 1);  // 
	VOPropAdd(AlertsColorVO, "Yellow4", NTBSTRINGBIND, 16, 1); // 

	// BUG??  vo_set(AlertsColorVO,5 , "Column", NULL, NULL);

	parentgrid->AlertsColorGrid->vo = AlertsColorVO;


}





void CrDataAppVO(MGLGRID *DataAppGrid)
{

	VO *DataAppVO;

	assert(DataAppGrid);

	DataAppVO = (VO *)vo_create(0, NULL);
	vo_set(DataAppVO, V_NAME, "DataAppVO", NULL);
	VOPropAdd(DataAppVO, "Up", INTBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "Down", INTBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "Del", INTBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "AppName", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "DataSource", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "Show", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "Save", INTBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "Build", INTBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppVO, "gridptr", VOIDBIND, -1, VO_NO_ROW);

	DataAppGrid->vo = DataAppVO;

}

/*Create vo for Calendar App*/

void CrCalendarAppVO(MGLGRID *CalendarAppGrid){

	VO *CalendarAppVO;

	assert(CalendarAppGrid);

	CalendarAppVO = (VO *)vo_create(0, NULL);
	vo_set(CalendarAppVO, V_NAME, "CalendarAppVO", NULL);
	VOPropAdd(CalendarAppVO, "0", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(CalendarAppVO, "1", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(CalendarAppVO, "2", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(CalendarAppVO, "3", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(CalendarAppVO, "4", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(CalendarAppVO, "5", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(CalendarAppVO, "6", NTBSTRINGBIND, 64, VO_NO_ROW);

	CalendarAppGrid->vo = CalendarAppVO;

}

//void CrGridListVO(MGLGRID *GridList)
//{
//
//	VO *GridListVO;
//
//	assert(GridList);
//
//	GridListVO = (VO *)vo_create(0, NULL);
//	vo_set(GridListVO, V_NAME, "GridListVO", NULL);
//	VOPropAdd(GridListVO, "GridNames", NTBSTRINGBIND, 64, VO_NO_ROW);
//	GridList->vo = GridListVO;
//
//}

//mtm
void CrDataAppCreaterVO(MGLGRID *DataAppCreater)
{

	VO *DataAppCreaterVO;

	assert(DataAppCreater);

	DataAppCreaterVO = (VO *)vo_create(0, NULL);
	vo_set(DataAppCreaterVO, V_NAME, "DataAppVO", NULL);
	VOPropAdd(DataAppCreaterVO, "Up Arrow", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "Down Arrow", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "Extra1", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "AppName", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "Extra2", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "DataSource", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "Extra3", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "Create", NTBSTRINGBIND, 64, VO_NO_ROW);
	VOPropAdd(DataAppCreaterVO, "Extra4", NTBSTRINGBIND, 64, VO_NO_ROW);

	DataAppCreater->vo = DataAppCreaterVO;

}


VO *CrTarmacVO(struct InstanceData* id, MGLGRID *topgrid)
{
	assert(id);

	return(id->m_pTDVO);

}




VO *CrAlertsVO(MGLGRID *topgrid)
{

	VO *AlertsVO;

	AlertsVO = (VO *)vo_create(0, NULL);
	vo_set(AlertsVO, V_NAME, "AlertsVO", NULL);
	//    VOPropAdd( AlertsVO, "ID", INTBIND, -1, VO_NO_ROW );
	VOPropAdd(AlertsVO, "KeyStr", NTBSTRINGBIND, 128, VO_NO_ROW);     // must uniquely define a row via a combination of fltnum:origin (JBU2:KJFK),
	VOPropAdd(AlertsVO, "Alert", NTBSTRINGBIND, 128, VO_NO_ROW);    // action name becomes alert name if Alert Box checked
	VOPropAdd(AlertsVO, "Trigger", NTBSTRINGBIND, 128, VO_NO_ROW);
	VOPropAdd(AlertsVO, "Time", INTBIND, -1, VO_NO_ROW);
	VOPropAdd(AlertsVO, "Ack", INTBIND, -1, VO_NO_ROW);  // clicking in this column spawns select list of possible values from main gri
	VOPropAdd(AlertsVO, "Del", INTBIND, -1, VO_NO_ROW);

	// fields below are for tracking alarms. The alarm should show up in the Alert Grid only once, even if the user deletes it from the grid
	VOPropAdd(AlertsVO, "IsVisible", INTBIND, -1, VO_NO_ROW);  // if > 0, then copy from HistAlertsVO to AlertsGrid->vo
	VOPropAdd(AlertsVO, "DataSource", NTBSTRINGBIND, 64, VO_NO_ROW); // FDO, ,ROI, etc


	vo_set(AlertsVO, V_ORDER_COLS, "Time", NULL, NULL);

	return(AlertsVO);

}








//void MGLInitAlertsColorsVO(MGLGRID *topgrid)
//{
//
//	assert(topgrid);
//
//	// fill a vo with all the unique values in this columns for all rows
//	if (!topgrid->AlertsColorGrid->vo){
//		CrAlertsColorVO(topgrid);
//	}
//	else {
//		// already created the SortAvailableGrid->vo 
//	}
//
//	// clear out the current settings
//	if (topgrid->AlertsColorGrid->vo->count){
//		vo_rm_rows(topgrid->AlertsColorGrid->vo, 0, topgrid->AlertsColorGrid->vo->count);
//	}
//	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
//	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Foreground Color"/*"Grid"*/, _TRUNCATE);
//	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
//	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Background Color"/*"Icon"*/, _TRUNCATE);
//	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
//	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Icon color", _TRUNCATE);
//	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
//	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Filter", _TRUNCATE);
//	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
//	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "E-Mail", _TRUNCATE);
//	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
//	
//
//
//}

//mtm added for new grid (foreground color, background color)
void MGLInitAlertsColorsVO_me(MGLGRID *topgrid)
{

	//	assert(topgrid);
	//
	//	// fill a vo with all the unique values in this columns for all rows
	//	if (!topgrid->GridActions->vo){
	//		CrAlertsColorVO_me(topgrid);
	//	}
	//	else {
	//		// already created the SortAvailableGrid->vo 
	//	}
	//
	//	// clear out the current settings
	//	vo_rm_rows(topgrid->GridActions->vo, 0, topgrid->GridActions->vo->count);
	//	vo_alloc_rows(topgrid->GridActions->vo, 1);
	//	strncpy_s(VP(topgrid->GridActions->vo, topgrid->GridActions->vo->count - 1, "Column", char), 64, "ForeGround Color", _TRUNCATE);
	//	vo_alloc_rows(topgrid->GridActions->vo, 1);
	//	strncpy_s(VP(topgrid->GridActions->vo, topgrid->GridActions->vo->count - 1, "Column", char), 64, "BackGround Color", _TRUNCATE);
	//	vo_alloc_rows(topgrid->GridActions->vo, 1);
	//	/*strncpy_s(VP(topgrid->GridActions->vo, topgrid->GridActions->vo->count - 1, "Column", char), 64, "Alert Box", _TRUNCATE);
	//	vo_alloc_rows(topgrid->GridActions->vo, 1);
	//	strncpy_s(VP(topgrid->GridActions->vo, topgrid->GridActions->vo->count - 1, "Column", char), 64, "Filter", _TRUNCATE);
	//	vo_alloc_rows(topgrid->GridActions->vo, 1);
	//#if 0
	//	strncpy_s(VP(topgrid->AlertsColorGrid->vo, topgrid->AlertsColorGrid->vo->count - 1, "Column", char), 64, "Sounds", _TRUNCATE);
	//	vo_alloc_rows(topgrid->AlertsColorGrid->vo, 1);
	//#endif
	//	strncpy_s(VP(topgrid->GridActions->vo, topgrid->GridActions->vo->count - 1, "Column", char), 64, "Default Color", _TRUNCATE);*/


}





void FillDefaultTarmacVO(struct InstanceData* id, MGLGRID *TarmacGrid)
{
	assert(id);
	assert(TarmacGrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!TarmacGrid->vo){
		TarmacGrid->vo = CrTarmacVO(id, TarmacGrid);
		// 		HistTarmacVO = CrTarmacVO( TarmacGrid ); // master list of Tarmac to keep forever so we don't dup Tarmac
	}
	else {
		// already created the TarmacGrid->vo
		return;
	}


}




void FillDefaultAlertsVO(struct InstanceData* id, MGLGRID *AlertsGrid)
{
	assert(id);
	assert(AlertsGrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!AlertsGrid->vo){
		AlertsGrid->vo = CrAlertsVO(AlertsGrid);
		HistAlertsVO = CrAlertsVO(AlertsGrid); // master list of alerts to keep forever so we don't dup alerts
	}
	else {
		// already created the AlertsGrid->vo
		return;
	}
	// fill in first alert with time stamp data

	vo_rm_rows(AlertsGrid->vo, 0, AlertsGrid->vo->count);
	vo_alloc_rows(AlertsGrid->vo, 1);
	strncpy_s(VP(AlertsGrid->vo, AlertsGrid->vo->count - 1, "Trigger", char), 64, "No Active Alerts", _TRUNCATE);

	VV(AlertsGrid->vo, AlertsGrid->vo->count - 1, "Time", int) = time(0);


}


//mtm
void FillDataAppCreaterVO(struct InstanceData* id, MGLGRID *DataAppCreater)
{

	assert(id);
	assert(DataAppCreater);

	// fill a vo with all the unique values in this columns for all rows
	if (!DataAppCreater->vo){
		CrDataAppCreaterVO(DataAppCreater);
	}
	else {
		// already created the DataAppGrid->vo
		return;
	}

	vo_rm_rows(DataAppCreater->vo, 0, DataAppCreater->vo->count);
	CalcColWidths(id, DataAppCreater);
}



void FillDataAppVO(struct InstanceData* id, MGLGRID *DataAppGrid)
{

	assert(id);
	assert(DataAppGrid);

	// fill a vo with all the unique values in this columns for all rows
	if (!DataAppGrid->vo){
		CrDataAppVO(DataAppGrid);
	}
	else {
		// already created the DataAppGrid->vo
		return;
	}

	vo_alloc_rows(DataAppGrid->vo, 1);
	CalcColWidths(id, DataAppGrid);
}

void FillCalendarAppVO(struct InstanceData *id, MGLGRID *CalendarAppGrid){

	assert(id);
	assert(CalendarAppGrid);

	//fill a vo with all the unique values  in this colums for all rows
	if (!CalendarAppGrid->vo){
		CrCalendarAppVO(CalendarAppGrid);
	}
	else{
			//already created the CalendarAppGrid->vo
		return;
	}

	vo_alloc_rows(CalendarAppGrid->vo, 9);

	CalcColWidths(id, CalendarAppGrid);

}


//added mtm

//void FillMainTabVO(struct InstanceData* id, MGLGRID *MainTabGrid)
//{
//
//	assert(id);
//	assert(MainTabGrid);
//
//	// fill a vo with all the unique values in this columns for all rows
//	if (!MainTabGrid->vo){
//		CrMainTabVO(MainTabGrid);
//	}
//	else {
//		// already created the DataAppGrid->vo
//		return;
//	}
//
//	vo_rm_rows(MainTabGrid->vo, 0, MainTabGrid->vo->count);
//	CalcColWidths(id, MainTabGrid);
//}




char *GetUserXgridSettings(struct InstanceData *id, char *user, char *arpt, char *app_name)
{
	char	tmpstr[1024];
	char *outstr;
	int trycount = 0;
	char encodebuf[2048];

	struct	MemoryStruct chunk;
	extern int GetCurlFormGetResults_timeout(struct InstanceData* id, struct MemoryStruct *chunk, char *url, char *postfields, enum SecureMode eMode, int logflag, long curltimeout);

	assert(id);

	if (user == NULL || strlen(user) == 0) {
		return (NULL);
	}

	if (app_name == NULL || strlen(app_name) == 0) {
		return (NULL);
	}

	if (!arpt){
		arpt = "";
	}
	chunk.memory = NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	urlencode(encodebuf, app_name, 0);
	memset(tmpstr, 0, sizeof(tmpstr));
	_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "Action=GetXgridConfig&app_name=%s&user_name=%s&arpt=%s", encodebuf, user, arpt);



	if (!GetCurlFormGetResults_timeout(id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE, 45L)){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load XGRID config, url %s%s\n", GetCurrentThreadId(), 
			id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr );
		GetNxtSurfSrvr(id);
	}


#ifdef OLDWAY
	while (!GetCurlFormPostResults(id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE) && trycount++ < 4)
	{
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load XGRID config\n", GetCurrentThreadId());
		GetNxtSurfSrvr(id);
	}

#endif

	// Check for valid data from server
	if (chunk.memory) {
		if (strncmp(chunk.memory, "FAILED", 6)) {
			outstr = str_falloc(chunk.memory);
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "retrieved %d bytes in Xgrid config\n", chunk.size);
			free_chunk_memory(&chunk);
			return (outstr);
		}
		else {
			free_chunk_memory(&chunk);
			return (NULL);
		}
	}
	return (NULL);
}




int GetFieldValStr(char *str, char *outbuf, int outsize)
{
	char *endstr, *curstr;
	int len;

	if (!str || !strlen(str)) return(FALSE);

	if (!(str = strchr(str, '"'))){  // all values must have double quotes on both ends
		return(FALSE);
	}

	curstr = str;
	curstr++; // skip over first '"' 
	if (!(endstr = strchr(curstr, '"'))){  // all values must have double quotes on both ends
		return(FALSE);
	}

	len = endstr - curstr;
	if (len < outsize){
		strncpy_s(outbuf, outsize, curstr, len);
		outbuf[len] = '\0';
		return(TRUE);
	}
	return(FALSE);
}

char *LoadXGridStr(struct InstanceData *id, char *AppName, char *arpt)
{

	assert(id);

	return(NULL);

}


int GetConfigDataSource(char *configstr, char *valstrp, int vlen)
{
	char *dstr;

	assert(configstr);
	assert(valstrp);

	if (!(dstr = strstr(configstr, "datasource:"))){
		// not found
		return(FALSE);
	}
	dstr += 11; // skip datasource:
	return(GetFieldValStr(dstr, valstrp, vlen));

}


MGLGRID *ProcessXGridAppNames(struct InstanceData *id, char *srcstr, VO *DataAppVO, MGLGRID *DataAppGrid,MGLGRID *DataAppCreater)
{
	char *str;
	int len, i, vorow;
	char *AppName, *endstr, *curstr, *configstr;
	char DataSource[32];
	int check;
	MGLGRID *FDOGrid, *ROIGrid,*SysAlertsGrid;
	char count[128];
	char linebuf[128];
	char appname[64], PrevAppName[64];
	extern MGLGRID *CreateFDOGrid(struct InstanceData* id);
	extern void FillFDOVO(struct InstanceData* id, MGLGRID *FDOGrid);
	extern int LoadXGrid(struct InstanceData *id, MGLGRID *grid, char *AppName, char *Arpt, char *outstr, MGLGRID *DataAppGrid);
	extern MGLGRID *CreateROIGrid(struct InstanceData* id);
	char test[64];
	assert(id);
	extern void UpdateROIGridVO(struct InstanceData* id);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	str = srcstr;
	check = 0;

	
	PrevAppName[0] = '\0';
	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			endstr = strchr(str, '\"');
			if (endstr)
			{
				int i = 0;
				len = 0;
				strcpy_s(count,128,endstr);
				while (count[i] != '\0')
				{
					len++;
					i++;
				}
				goto t1;
			}
			else
				break;
		}
		len = endstr - str;
	t1:		if (len < sizeof(linebuf)){
		strncpy_s(linebuf, sizeof(linebuf), str, len);
		linebuf[len] = '\0';
		str += len + 1; /* add one to length to skip over newline character */

		curstr = linebuf;
		curstr++; // skip over first '"'
	}
				// get rid of double quotes
				if (!(endstr = strchr(curstr, '"'))){
					break;
				}
				len = endstr - curstr;
				strncpy_s(appname, sizeof(appname), curstr, len);

				if (!strcmp(appname, PrevAppName)){
					// dup name
					continue;
				}
				strncpy_s(PrevAppName, sizeof(PrevAppName), curstr, len);
				// see if we already have this app loaded  into our DataAppVO
				for (i = 0; DataAppVO && i < DataAppVO->count; i++){
					AppName = VP(DataAppVO, i, "AppName", char);
					if (!strcmp(AppName, appname)){
						// the app is already in the DataAppVO
						break;
					}
				}
				if (i < DataAppVO->count){
					// we already have this app loaded
					continue; // get next app from DB
				}
				if (DataAppGrid->nrows <= 9)
					DataAppGrid->nrows++;
				MGLGridMove(id, DataAppGrid, DataAppGrid->gridrect.x1, DataAppGrid->gridrect.y1);
				if (DataAppGrid->nrows <= 9)
					MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1 - DataAppGrid->gridrows[0].height);
				else if (DataAppGrid->nrows > 9)
					MGLGridMove(id, DataAppCreater, DataAppCreater->gridrect.x1, DataAppCreater->gridrect.y1);
				vo_alloc_rows(DataAppVO, 1); // add it to our appvo	
				vorow = DataAppVO->count - 2;

				if (!(configstr = GetUserXgridSettings(id, id->m_strUserName, NULL, appname))){
					vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "could not load app %s for user %s\n", appname, id->m_strUserName);
					vo_rm_row(DataAppVO, DataAppVO->count - 1);
					//	continue;
				}

				if (configstr){
					if (!GetConfigDataSource(configstr, DataSource, sizeof(DataSource))){
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "could not load configdatasource %s for user %s\n", DataSource, id->m_strUserName);
						vo_rm_row(DataAppVO, DataAppVO->count - 1);
						//	continue;
					}
				}

				if (!strcmp(DataSource, "FDO")){
					FDOGrid = CreateFDOGrid(id);
					FillFDOFromGFDO(id, FDOGrid);
					MGLSetlabelNamesFromVO(FDOGrid);
					// fill in the ptr to the grid 
					VV(DataAppVO, vorow, "gridptr", void *) = FDOGrid;
					strncpy_s(VP(DataAppVO, vorow, "DataSource", char), 64, "FDO", _TRUNCATE);
					strncpy_s(VP(DataAppVO, vorow, "AppName", char), 64, appname, _TRUNCATE);		
					DataAppGrid->active_checkbox[vorow] = 20;
					strcpy_s(FDOGrid->name, MGL_GRID_NAME_MAX, appname);

					// fill in any prior settings from the user that are stored in the DB


					// move this row before the "New" row
					vo_move_row(DataAppVO, vorow, vorow);
					MGLSetTitle(FDOGrid, appname);
					CalcColWidths(id, FDOGrid);

					SecondTabGrid->childgrid = FDOGrid;
					CreateBuildSummary(id, FDOGrid);
					FillBuildSummaryVO(id, SecondTabGrid->childgrid->BuildSummaryGrid);

					CreateActionsGrid(id, FDOGrid);
					FDOGrid->ActionsGrid->parent_w->visible = FALSE;
					FillActionsGrid(id, FDOGrid, FALSE);



					CreateParametersGrid1(id, FDOGrid, 0);

					MGLFillParametersColsVO(FDOGrid);

					FDOGrid->ParametersGrid->parent_w->visible = FALSE;


					CreateAlertsColorGrid(id, FDOGrid, 0);
					MGLInitAlertsColorsVO(FDOGrid);
					FDOGrid->AlertsColorGrid->parent_w->visible = FALSE;


					CreateGridrowcolumns(id, FDOGrid, 0);
					MGLInitGridofRowsColumnsVO(FDOGrid);


					CreateSortPanel(id, FDOGrid);
					FDOGrid->RowColumn->parent_w->visible = FALSE;

					FillAvailableColsVO(id, FDOGrid);
					FillActionsGrid1(id, FDOGrid, FALSE);
					FillActionsGrid2(id, FDOGrid, FALSE);
					//	MGLFillParameters(id, ROIGrid->ParametersGrid);
					MGLFillParameters(id, FDOGrid->ParametersGrid);
					MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum,id);
					MGLFillParametersColsVO(SecondTabGrid->childgrid);

					//MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum);
					//MGLFillParametersColsVO(SecondTabGrid->childgrid);
					FillAvailableColsVO(id, SecondTabGrid->childgrid);
					if (!SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig){
						SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
						SecondTabGrid->childgrid->nActions = 1; // add only one for now if user did not have any stored
						SecondTabGrid->childgrid->Actions[0].IsActive = TRUE;
						SecondTabGrid->childgrid->Actions[0].ActionType = ACTION_FILTER;
					}
					SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 1;
					SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].filter_active = 1;


					CreateTitleGrid(id, FDOGrid->name, FDOGrid);
					SecondTabGrid->childgrid->TitleGrid->parent_w->visible = 0;
					CreateColorMatrix_icon(id, SecondTabGrid->childgrid->AlertsColorGrid, 0);
					SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
					CreateEMailGrid(id, SecondTabGrid->childgrid);
					FillEMailVO(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid);
					CreateSubjectGrid(id, SecondTabGrid->childgrid);
					FillSubjectVO(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid);
					SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = 0;
					SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 0;
					for (i = 0; i < 3; i++)
					{
						FDOGrid->DisplayCol[i] = i;
						VV(FDOGrid->SortAvailableGrid->vo, i, "Visible", int) = 1;
					}
					for (i = 3; i < FDOGrid->ncols; i++)
					{
						VV(FDOGrid->SortAvailableGrid->vo, i, "Visible", int) = 0;
						FDOGrid->DisplayCol[i] = -1;
					}

					VV(FDOGrid->SortAvailableGrid->vo, 6, "Visible", int) = 1;
					VV(FDOGrid->SortAvailableGrid->vo, 9, "Visible", int) = 1;
					LoadXGrid(id, FDOGrid, appname, NULL, configstr,DataAppGrid);					
					MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum,id);
					color_actions(id);					
				}
				else if (!strcmp(DataSource, "ROI")){
					ROIGrid = CreateROIGrid(id);
					UpdateROIGridVO(id);
					ROIGrid->vo = CreateRegionVO(id, "OpenglROI");
					MGLSetlabelNamesFromVO(ROIGrid);

					//  filled from UpdateROIGridVO
					// fill in the ptr to the grid 
					VV(DataAppVO, vorow, "gridptr", void *) = ROIGrid;
					strncpy_s(VP(DataAppVO, vorow, "DataSource", char), 64, "ROI", _TRUNCATE);
					strncpy_s(VP(DataAppVO, vorow, "AppName", char), 64, appname, _TRUNCATE);					
					strcpy_s(ROIGrid->name,MGL_GRID_NAME_MAX, appname);
					DataAppGrid->active_checkbox[vorow] = 20;
					// fill in any prior settings from the user that are stored in the DB

					// move this row before the "New" row
					vo_move_row(DataAppVO, vorow, vorow);
					MGLSetTitle(ROIGrid, appname);
					CalcColWidths(id, ROIGrid);

					SecondTabGrid->childgrid = ROIGrid;
					CreateBuildSummary(id, ROIGrid);
					FillBuildSummaryVO(id, SecondTabGrid->childgrid->BuildSummaryGrid);

					CreateActionsGrid(id, ROIGrid);
					ROIGrid->ActionsGrid->parent_w->visible = FALSE;
					FillActionsGrid(id, ROIGrid, FALSE);



					CreateParametersGrid1(id, ROIGrid, 0);

					MGLFillParametersColsVO(ROIGrid);

					ROIGrid->ParametersGrid->parent_w->visible = FALSE;


					CreateAlertsColorGrid(id, ROIGrid, 0);
					MGLInitAlertsColorsVO(ROIGrid);
					ROIGrid->AlertsColorGrid->parent_w->visible = FALSE;


					CreateGridrowcolumns(id, ROIGrid, 0);
					MGLInitGridofRowsColumnsVO(ROIGrid);


					CreateSortPanel(id, ROIGrid);
					ROIGrid->RowColumn->parent_w->visible = FALSE;

					FillAvailableColsVO(id, ROIGrid);
					FillActionsGrid1(id, ROIGrid, FALSE);
					FillActionsGrid2(id, ROIGrid, FALSE);
					//	MGLFillParameters(id, ROIGrid->ParametersGrid);
					//	MGLFillParameters(id, ROIGrid->ParametersGrid);
					MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum,id);
					//MGLFillParametersColsVO(SecondTabGrid->childgrid);

					//MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum);
					//MGLFillParametersColsVO(SecondTabGrid->childgrid);
					FillAvailableColsVO(id, SecondTabGrid->childgrid);
					if (!SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig){
						SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
						SecondTabGrid->childgrid->nActions = 1; // add only one for now if user did not have any stored
						SecondTabGrid->childgrid->Actions[0].IsActive = TRUE;
						SecondTabGrid->childgrid->Actions[0].ActionType = ACTION_FILTER;
					}
					SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 1;
					SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].filter_active = 1;
					CreateEMailGrid(id, SecondTabGrid->childgrid);
					FillEMailVO(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid);
					SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = 0;
					CreateSubjectGrid(id, SecondTabGrid->childgrid);
					FillSubjectVO(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid);
					SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 0;					
					CreateTitleGrid(id, ROIGrid->name, ROIGrid);
					SecondTabGrid->childgrid->TitleGrid->parent_w->visible = 0;
					//UpdateROIGridVO(id);
					CreateColorMatrix_icon(id, SecondTabGrid->childgrid->AlertsColorGrid, 0);
					SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
					strncpy_s(test, 64, VP(SecondTabGrid->childgrid->SortAvailableGrid->vo, 1, "Column", char), _TRUNCATE);
					LoadXGrid(id, ROIGrid, appname, NULL, configstr,DataAppGrid);					
					strncpy_s(test, 64, VP(SecondTabGrid->childgrid->SortAvailableGrid->vo, 1, "Column", char), _TRUNCATE);
					MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum,id);
					strncpy_s(test, 64, VP(SecondTabGrid->childgrid->SortAvailableGrid->vo, 1, "Column", char), _TRUNCATE);
					color_actions(id);
					
				}
				//count = count + 1;
				//t = 0;

				/*if (!strcmp(DataSource, "SysAlerts")){
					SysAlertsGrid = CreateSysAlertsGrid(id);
					FillSysAlertsFromGSA(id, SysAlertsGrid);
					MGLSetlabelNamesFromVO(SysAlertsGrid);
					// fill in the ptr to the grid 
					VV(DataAppVO, vorow, "gridptr", void *) = SysAlertsGrid;
					strncpy_s(VP(DataAppVO, vorow, "DataSource", char), 64, "SysAlerts", _TRUNCATE);
					strncpy_s(VP(DataAppVO, vorow, "AppName", char), 64, appname, _TRUNCATE);
					DataAppGrid->active_checkbox[vorow] = 20;
					strcpy_s(FDOGrid->name, MGL_GRID_NAME_MAX, appname);

					// fill in any prior settings from the user that are stored in the DB


					// move this row before the "New" row
					vo_move_row(DataAppVO, vorow, vorow);
					MGLSetTitle(SysAlertsGrid, appname);
					CalcColWidths(id, SysAlertsGrid);

					SecondTabGrid->childgrid = SysAlertsGrid;
					CreateBuildSummary(id, SysAlertsGrid);
					FillBuildSummaryVO(id, SecondTabGrid->childgrid->BuildSummaryGrid);

					CreateActionsGrid(id, SysAlertsGrid);
					SysAlertsGrid->ActionsGrid->parent_w->visible = FALSE;
					FillActionsGrid(id, SysAlertsGrid, FALSE);



					CreateParametersGrid1(id, SysAlertsGrid, 0);

					MGLFillParametersColsVO(SysAlertsGrid);

					SysAlertsGrid->ParametersGrid->parent_w->visible = FALSE;


					CreateAlertsColorGrid(id, SysAlertsGrid, 0);
					MGLInitAlertsColorsVO(SysAlertsGrid);
					SysAlertsGrid->AlertsColorGrid->parent_w->visible = FALSE;


					CreateGridrowcolumns(id, SysAlertsGrid, 0);
					MGLInitGridofRowsColumnsVO(SysAlertsGrid);


					CreateSortPanel(id, SysAlertsGrid);
					FDOGrid->RowColumn->parent_w->visible = FALSE;

					FillAvailableColsVO(id, SysAlertsGrid);
					FillActionsGrid1(id, SysAlertsGrid, FALSE);
					FillActionsGrid2(id, SysAlertsGrid, FALSE);
					//	MGLFillParameters(id, ROIGrid->ParametersGrid);
					MGLFillParameters(id, SysAlertsGrid->ParametersGrid);
					MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum, id);
					MGLFillParametersColsVO(SecondTabGrid->childgrid);

					//MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum);
					//MGLFillParametersColsVO(SecondTabGrid->childgrid);
					FillAvailableColsVO(id, SecondTabGrid->childgrid);
					if (!SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig){
						SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
						SecondTabGrid->childgrid->nActions = 1; // add only one for now if user did not have any stored
						SecondTabGrid->childgrid->Actions[0].IsActive = TRUE;
						SecondTabGrid->childgrid->Actions[0].ActionType = ACTION_FILTER;
					}
					SecondTabGrid->childgrid->AlertsColorGrid->show_checkbox[3] = 1;
					SecondTabGrid->childgrid->Actions[SecondTabGrid->childgrid->CurActionNum].filter_active = 1;


					CreateTitleGrid(id, SysAlertsGrid->name, FDOGrid);
					SecondTabGrid->childgrid->TitleGrid->parent_w->visible = 0;
					CreateColorMatrix_icon(id, SecondTabGrid->childgrid->AlertsColorGrid, 0);
					SecondTabGrid->childgrid->AlertsColorGrid->IconColorGrid->parent_w->visible = 0;
					CreateEMailGrid(id, SecondTabGrid->childgrid);
					FillEMailVO(id, SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid);
					CreateSubjectGrid(id, SecondTabGrid->childgrid);
					FillSubjectVO(id, SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid);
					SecondTabGrid->childgrid->AlertsColorGrid->EMailGrid->parent_w->visible = 0;
					SecondTabGrid->childgrid->AlertsColorGrid->SubjectGrid->parent_w->visible = 0;
					for (i = 0; i < 3; i++)
					{
						SysAlertsGrid->DisplayCol[i] = i;
						VV(SysAlertsGrid->SortAvailableGrid->vo, i, "Visible", int) = 1;
					}
					for (i = 3; i < SysAlertsGrid->ncols; i++)
					{
						VV(SysAlertsGrid->SortAvailableGrid->vo, i, "Visible", int) = 0;
						SysAlertsGrid->DisplayCol[i] = -1;
					}

					VV(SysAlertsGrid->SortAvailableGrid->vo, 6, "Visible", int) = 1;
					VV(SysAlertsGrid->SortAvailableGrid->vo, 9, "Visible", int) = 1;
					LoadXGrid(id, SysAlertsGrid, appname, NULL, configstr, DataAppGrid);
					MGLFillParametersGrid(SecondTabGrid->childgrid->ActionsGrid, SecondTabGrid->childgrid->CurActionNum, id);
					color_actions(id);
				}*/
	}
	return(DataAppGrid);
	//next:t = 0;
}




int ListUserXgridAppnames(struct InstanceData *id, char *user, char *arpt, VO *DataAppVO,MGLGRID *DataAppGrid,MGLGRID *DataAppCreater)
{
	VO	* namesVO = NULL;
	char username[64];
	char tmpstr[4096];
	struct MemoryStruct chunk;

	assert(id);

	if (user == NULL || strlen(user) == 0) {
		return (FALSE);
	}

	if (!arpt){
		arpt = "";
	}

	strncpy_s(username, sizeof(username), user, _TRUNCATE);
	//  StrUpper(username);

	// Query the server to get the list of ROI Configurations for this user/arpt
	chunk.size = 0;
	chunk.memory = NULL;

	_snprintf_s(tmpstr, sizeof(tmpstr), _TRUNCATE, "Action=GetXgridAppnameList&user_name=%s&arpt=%s", username, arpt);

	if (!GetCurlFormPostResults(id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE)){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to save layout\n", GetCurrentThreadId());
		GetNxtSurfSrvr(id);
	}
	else {
		// Check for valid data from server
		if (chunk.memory){
			if (!strstr(chunk.memory, "404 Not Found")) {
				DataAppGrid=ProcessXGridAppNames(id, chunk.memory, DataAppVO,DataAppGrid,DataAppCreater);
			}

			free_chunk_memory(&chunk);

			return (TRUE);
		}
	}

	return (FALSE);
}





char *GetFieldName(char *str)
{
	static char buf[1024];
	char *endstr;
	int len;

	if (!str || !strlen(str)) return(NULL);

	if (!(endstr = strchr(str, ':'))){
		return(NULL);
	}

	len = endstr - str;
	strncpy_s(buf, sizeof(buf), str, len);
	buf[len] = '\0';

	return(buf);
}



int GetSelectFieldValStr(char *str, char *outbuf, int outsize)
{
	char *endstr, *curstr;
	int len;

	if (!str || !strlen(str)) return(FALSE);

	do {  // process all fields found 
		if (!(str = strchr(str, '"'))){  // all values must have double quotes on both ends
			return(FALSE);
		}

		curstr = str;
		curstr++; // skip over first '"' 
		if (!(endstr = strchr(curstr, '"'))){  // all values must have double quotes on both ends
			return(FALSE);
		}

		len = endstr - curstr;
		if (len < outsize){
			strncpy_s(outbuf, outsize, curstr, len);
			outbuf[len] = '\0';
			return(TRUE);
		}
	} while (str);

	return(FALSE);
}




int LoadXGridDefaults(struct InstanceData *id, MGLGRID *grid, char **strptr)
{
	char *curstr, *fieldname;
	char *str, *endstr;
	int len;
	char valstr[64];
	char linebuf[2048];
	int f = 0;

	/*
	Tablename: "xgriddefaults"
	minx: "0"
	miny: "0"
	maxx: "638"
	maxy: "305"
	*/

	str = *strptr;

	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';
			curstr = linebuf;

			if (!strncmp(linebuf, "Tablename:", 6)){
				// next table, so no more processing here
				break;
			}
			else {

				str += len + 1; /* add one to length to skip over newline character */

				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "minx1")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmin.x1 = atoi(valstr);
				}
				else if (!strcmp(fieldname, "miny1")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmin.y1 = atoi(valstr);
				}
				else if (!strcmp(fieldname, "minx2")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmin.x2 = atoi(valstr);
				}
				else if (!strcmp(fieldname, "miny2")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmin.y2 = atoi(valstr);
				}
				else if (!strcmp(fieldname, "maxx1")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmax.x1 = atoi(valstr);

				}
				else if (!strcmp(fieldname, "maxy1")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmax.y1 = atoi(valstr);
				}
				else if (!strcmp(fieldname, "maxx2")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmax.x2 = atoi(valstr);
				}
				else if (!strcmp(fieldname, "maxy2")){
					GetFieldValStr(curstr, valstr, 64);
					grid->gridrectmax.y2 = atoi(valstr);
				}
			}
		}
	}
	// grids start maximized??? may change later
	if (grid->IsMinimized){
		grid->gridrect = grid->gridrectmin;
	}
	else {
		grid->gridrect = grid->gridrectmax;
	}


	*strptr = str;

	return(TRUE);
}


int LoadXGridActions(struct InstanceData *id, MGLGRID *grid, char **strptr)
{
	char *curstr, *fieldname;
	char actionname[64], actiontype[64], isactive[64], row_fg[64], row_bg[64], soundname[64], filteron[64];
	int  red_ival, green_ival, blue_ival, fg_ival, bg_ival;
	char *str, *endstr;
	int len, maxlen;
	char linebuf[2048];
	int f = 0;
	char *cptr1, *cptr2;
	int x;
	
	/*
	Tablename: "xgridactions"
	actionname: "JFK"
	actiontype: "1"
	isactive: "1"
	row_fg: ""
	row_bg: ""
	soundname: ""
	*/
	assert(id);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	str = *strptr;

	grid->nActions = SecondTabGrid->childgrid->ActionsGrid->vo->count;
	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				// next table, so no more processing here
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "actionname")){
					GetFieldValStr(curstr, actionname, 64);
					if (strcmp(actionname, ""))
						strncpy_s(VP(SecondTabGrid->childgrid->ActionsGrid->vo, grid->nActions - 1, "New", char), 64, actionname, _TRUNCATE);					
				}
				else if (!strcmp(fieldname, "actiontype")){
					GetFieldValStr(curstr, actiontype, 64);
				}
				else if (!strcmp(fieldname, "isactive")){
					GetFieldValStr(curstr, isactive, 64);
				}
				else if (!strcmp(fieldname, "row_fg")){
					GetFieldValStr(curstr, row_fg, 64);
				}
				else if (!strcmp(fieldname, "row_bg")){
					GetFieldValStr(curstr, row_bg, 64);
				}
				else if (!strcmp(fieldname, "soundname")){
					GetFieldValStr(curstr, soundname, 64);
				}
				else if (!strcmp(fieldname, "Filter On")){
					GetFieldValStr(curstr, filteron, 64);
				}
			}
		}
	}
	// put fieldname, op, etc into grid
	// create action and gridconfig
	//(grid->nActions)++;
	grid->CurActionNum = grid->nActions - 1;  // the latest action is the current action
	for (x = 0; x < SecondTabGrid->childgrid->ActionsGrid->vo->count; x++)
	{
		SecondTabGrid->childgrid->ActionsGrid->vo->row_index[x].fg = MGLColorHex2Int("000000");
	}

	if (strcmp(VP(SecondTabGrid->childgrid->ActionsGrid->vo, SecondTabGrid->childgrid->ActionsGrid->vo->count - 1, "Name", char),"New")!=0)
	{
		SecondTabGrid->childgrid->ActionsGrid->vo->row_index[grid->CurActionNum].fg = MGLColorHex2Int("529EFD");
		SecondTabGrid->childgrid->Actions[grid->CurActionNum].IsActive = 1;
		vo_alloc_rows(SecondTabGrid->childgrid->ActionsGrid->vo, 1);
		strncpy_s(VP(SecondTabGrid->childgrid->ActionsGrid->vo, SecondTabGrid->childgrid->ActionsGrid->vo->count - 1, "Name", char), 64, "New", _TRUNCATE);
		strncpy_s(VP(SecondTabGrid->childgrid->ActionsGrid->vo, SecondTabGrid->childgrid->ActionsGrid->vo->count - 1, "OnOff", char), 16, "On", _TRUNCATE);
	}
	grid->Actions[grid->CurActionNum].IsActive = atoi(isactive);
	grid->Actions[grid->CurActionNum].ActionType = atoi(actiontype);
	grid->Actions[grid->CurActionNum].filter_active =atoi(filteron);
	
	if (grid->Actions[grid->CurActionNum].filter_active == 1)
		grid->AlertsColorGrid->show_checkbox[3] = 1;
	else if (grid->Actions[grid->CurActionNum].filter_active == 0)
		grid->AlertsColorGrid->show_checkbox[3] = 0;

	maxlen = sizeof(grid->Actions[grid->CurActionNum].ActionName);
	strcpy_s(grid->Actions[grid->CurActionNum].ActionName, maxlen, actionname);
	maxlen = sizeof(grid->Actions[grid->CurActionNum].row_fg);
	strcpy_s(grid->Actions[grid->CurActionNum].row_fg, maxlen, row_fg);
	maxlen = sizeof(grid->Actions[grid->CurActionNum].row_bg);
	strcpy_s(grid->Actions[grid->CurActionNum].row_bg, maxlen, row_bg);

	//display the saved foreground and background upon restarting application



	cptr1 = grid->Actions[grid->CurActionNum].row_fg;
	cptr2 = grid->Actions[grid->CurActionNum].row_bg;
	if (strcmp(grid->Actions[grid->CurActionNum].row_fg, "") || strcmp(grid->Actions[grid->CurActionNum].row_bg, ""))
	{
		if (!grid->Actions[grid->CurActionNum].gridconfig){
			grid->Actions[grid->CurActionNum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
		}

		if (strcmp(grid->Actions[grid->CurActionNum].row_fg, ""))
		{

			red_ival = HexStrtod(&cptr1[0], 2);//converts the string which has the color name in it to an integer
			green_ival = HexStrtod(&cptr1[2], 2);
			blue_ival = HexStrtod(&cptr1[4], 2);

			fg_ival = red_ival << 16 | green_ival << 8 | blue_ival;
			grid->AlertsColorGrid->vo->row_index[0].fg = fg_ival;
		}

		if (strcmp(grid->Actions[grid->CurActionNum].row_bg, ""))
		{
			red_ival = HexStrtod(&cptr2[0], 2);//converts the string which has the color name in it to an integer
			green_ival = HexStrtod(&cptr2[2], 2);
			blue_ival = HexStrtod(&cptr2[4], 2);

			bg_ival = red_ival << 16 | green_ival << 8 | blue_ival;

			grid->AlertsColorGrid->vo->row_index[1].bg = bg_ival;

		}


	}






	*strptr = str;

	return(TRUE);
}



int LoadXGridColumns(struct InstanceData *id, MGLGRID *grid, char **strptr)
{
	char *curstr, *fieldname;
	char colname[64], IsVisible[64], SortType[64], SortPriority[64], col_order[64];
	char *str, *endstr;
	int len, col, isvis;
	char linebuf[2048];
	VO *vcol;	
	char full[64];
	int x = 0;
	int y = 0;

	assert(id);
	assert(grid);

	/*
	Tablename: "xgridcolumns"
	colname: "OutToOff"
	IsVisible: "1"
	SortPriority: "(null)"
	col_order: "0"
	SortType: "(null)"
	colname: "FltNum"
	IsVisible: "1"
	SortPriority: "(null)"
	col_order: "1"
	SortType: "(null)"
	colname: "Origin"
	IsVisible: "1"
	SortPriority: "(null)"
	col_order: "2"
	SortType: "(null)"
	*/

	// clear out the AvailableVO
	if (grid->SortAvailableGrid && grid->SortAvailableGrid->vo && grid->SortAvailableGrid->vo->count){
	}
	else {
		// create the grid
		CreateSortPanel(id, grid);
		FillAvailableColsVO(id, grid);
	}
	vo_rm_rows(grid->SortAvailableGrid->vo, 0, grid->SortAvailableGrid->vo->count);

	str = *strptr;

	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';
			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "colname")){
					GetFieldValStr(curstr, colname, 64);
				}
				else if (!strcmp(fieldname, "IsVisible")){
					GetFieldValStr(curstr, IsVisible, 64);
				}
				else if (!strcmp(fieldname, "SortPriority")){
					GetFieldValStr(curstr, SortPriority, 64);
				}
				else if (!strcmp(fieldname, "col_order")){
					GetFieldValStr(curstr, col_order, 64);
				}
				else if (!strcmp(fieldname, "SortType")){
					GetFieldValStr(curstr, SortType, 64);
					// this is the last value, so add the column data to DisplayCols and ColrOrderNames

					if (vcol = vcol_find_label(grid->vo, colname)){
						col = atoi(col_order);
						// SortType?? put that into AvailableVO???
						vo_alloc_rows(grid->SortAvailableGrid->vo, 1);
						strncpy_s(VP(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Column", char), 64, vcol->label, _TRUNCATE);
						isvis = atoi(IsVisible);
						VV(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "Visible", int) = isvis;
						VV(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "SortVal", int) = atoi(SortType);
						VV(grid->SortAvailableGrid->vo, grid->SortAvailableGrid->vo->count - 1, "SortPriority", int) = atoi(SortPriority);
						if (isvis){
							grid->DisplayCol[col] = MGLNextVisCol(grid, grid->SortAvailableGrid->vo->count - 1);
							strncpy_s(grid->ColLabels[col], MGL_GRID_LABEL_MAX, colname, _TRUNCATE);
							y++;
							if (grid->roi_fdo == 1){
								write_layout_column_roi(full, col);//to write into build summary if grid is roi
							}
							else if (grid->roi_fdo == 2){
								write_layout_column_fdo(full, col, grid);//to write into build summary if grid is fdo
							}
							if (grid->BuildSummaryGrid->vo->count <= col)
								vo_alloc_rows(grid->BuildSummaryGrid->vo, 1);
							strncpy_s(VP(grid->BuildSummaryGrid->vo, x, "Layout", char), 64, full, _TRUNCATE);
							x++;
						}
						else {
							grid->DisplayCol[col] = -1;
							//if (grid->roi_fdo == 1){
							//	write_layout_column_roi(full, col);//to write into build summary if grid is roi
							//}
							//else if (grid->roi_fdo == 2){
							//	write_layout_column_fdo(full, col,grid);//to write into build summary if grid is fdo
							//}

							/*if (grid->BuildSummaryGrid->vo->count <= col)
								vo_alloc_rows(grid->BuildSummaryGrid->vo, 1);
							strncpy_s(VP(grid->BuildSummaryGrid->vo, x, "Layout", char), 64, full, _TRUNCATE);
							x++;*/
						}

					}
				}
			}
		}
	}
	*strptr = str;

	return(TRUE);
}




VO *MGLCrSearchVOFromDB(char *str)
{
	// differs from MGLCrSearchVO because of double quotes around field values
	char *endstr, *curstr;
	int len;
	char fieldbuf[256];
	VO *searchvo;

	/*
	Tablename: "xgridparameters"
	fieldname: "Region"
	select_value: ""Rwy 14 32","Rwy 15L 33R","Rwy 15R 33L","Rwy 15R 33L","Rwy 4L 22R","Rwy 4R 22L","Rwy 9 27","
	*/
	if (!str || !strlen(str)) return(NULL);

	searchvo = vo_create(0, NULL);
	vo_set(searchvo, V_NAME, "searchvo", NULL);
	VOPropAdd(searchvo, "listval", NTBSTRINGBIND, 256, VO_NO_ROW);
	vo_rm_rows(searchvo, 0, searchvo->count);

	// skip over field name select_value:
	if (!(str = strchr(str, '"'))){  // all values must have double quotes on both ends
		return(NULL);
	}

	str++; // skip over first double quote  ie ""Rwy 14 32","Rwy 15L 33R","Rwy 
	// get rid of last double quote at end of line
	do {
		if (!(str = strchr(str, '"')) || strlen(str) < 2){  // all values must have double quotes on both ends
			break;  // done with this set of values
		}

		curstr = str;
		curstr++; // skip over first '"' 
		if (!(endstr = strchr(curstr, '"'))){  // all values must have double quotes on both ends
			return(NULL);
		}

		len = endstr - curstr;
		if (len < sizeof(fieldbuf)){
			// add this string to list

			strncpy_s(fieldbuf, sizeof(fieldbuf), curstr, _TRUNCATE);
			fieldbuf[len] = '\0';

			vo_alloc_rows(searchvo, 1);
			if (strlen(curstr) < 255){
				strcpy_s(VP(searchvo, searchvo->count - 1, "listval", char), 255, fieldbuf);
			}
		}
		str = endstr + 2; // /skip over double quote and comma separator

	} while (str);

	return(searchvo);

}



void ProcessParameters(struct InstanceData *id, MGLGRID *grid, int f, int actionnum, char *gridfieldname, char *op,
	char *cmp_value, char *field_type, char *filter_type, char *exclude_selected)
{
	VO *vcol;
	int maxlen;

	// put fieldname, op, etc into grid
	grid->Actions[actionnum].gridconfig->filters[f].field_type = atoi(field_type);
	if (!grid->Actions[actionnum].gridconfig->filters[f].field_type){
		// default 
		if (grid->vo && (vcol = vcol_find(grid->vo, gridfieldname))){
			grid->Actions[actionnum].gridconfig->filters[f].field_type = vcol->dbtype;
		}
		else {
			grid->Actions[actionnum].gridconfig->filters[f].field_type = NTBSTRINGBIND;
		}
	}


	maxlen = sizeof(grid->Actions[actionnum].gridconfig->filters[f].field_name);
	strcpy_s(grid->Actions[actionnum].gridconfig->filters[f].field_name, maxlen, gridfieldname);

	grid->Actions[actionnum].gridconfig->filters[f].filter_type = atoi(filter_type);

	switch (grid->Actions[actionnum].gridconfig->filters[f].filter_type)
	{
	case FTYPE_LIST:
		// we set searchvo already
		break;
	case FTYPE_OPERATORS:
		maxlen = sizeof(grid->Actions[actionnum].gridconfig->filters[f].op);
		strcpy_s(grid->Actions[actionnum].gridconfig->filters[f].op, maxlen, op);  // op is 2 letter code, ala NE EQ LE
		maxlen = sizeof(grid->Actions[actionnum].gridconfig->filters[f].cmp_value);
		strcpy_s(grid->Actions[actionnum].gridconfig->filters[f].cmp_value, maxlen, cmp_value);
		grid->Actions[actionnum].gridconfig->filters[f].exclude_selected = atoi(exclude_selected);
		break;
	default:
		break;
	}

	(grid->Actions[actionnum].gridconfig->nFilters)++;

}


int LoadXGridParameters(struct InstanceData *id, int actionnum, MGLGRID *grid, char **strptr)
{
	char *curstr, *fieldname;
	char gridfieldname[64], op[64], cmp_value[64], filter_type[64], field_type[64], exclude_selected[64];
	char *str, *endstr;
	int len;
	char linebuf[4096];
	int f = 0;


	/*
	Tablename: "xgridparameters"
	fieldname: "Origin"
	op: "EQ"
	cmp_value: "JFK"
	filter_type: "1"
	field_type: "13"
	exclude_selected: "0"
	*/

	assert(id);
	assert(grid);

	// create action and gridconfig
	if (!grid->Actions[actionnum].gridconfig){
		grid->Actions[actionnum].gridconfig = (GridConfig *)calloc(1, sizeof(GridConfig));
	}

	f = grid->Actions[actionnum].gridconfig->nFilters;

	f = grid->Actions[grid->CurActionNum].gridconfig->nFilters;

	str = *strptr;

	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "fieldname")){
					GetFieldValStr(curstr, gridfieldname, 64);
				}
				else if (!strcmp(fieldname, "op")){
					GetFieldValStr(curstr, op, 64);
				}
				else if (!strcmp(fieldname, "cmp_value")){
					GetFieldValStr(curstr, cmp_value, 64);
				}
				else if (!strcmp(fieldname, "filter_type")){
					GetFieldValStr(curstr, filter_type, 64);
				}
				else if (!strcmp(fieldname, "field_type")){
					GetFieldValStr(curstr, field_type, 64);
				}
				else if (!strcmp(fieldname, "exclude_selected")){
					GetFieldValStr(curstr, exclude_selected, 64);
					ProcessParameters(id, grid, f, actionnum, gridfieldname, op, cmp_value, field_type, filter_type, exclude_selected);
					f = grid->Actions[actionnum].gridconfig->nFilters;
				}
				else if (!strcmp(fieldname, "select_value")){
					grid->Actions[actionnum].gridconfig->filters[f].searchvo = MGLCrSearchVOFromDB(curstr);
					strcpy_s(filter_type, sizeof(filter_type), "0"); // FTYPE_LIST;
				}
			}
		}
	}



	*strptr = str;

	return(TRUE);
}

//mtm- retrieve stored icon color
void SaveXIconColor(struct InstanceData* id, MGLGRID *grid, char **strptr)
{




	char *curstr, *fieldname;
	char color[64];
	char *str, *endstr;
	int len;
	char linebuf[4096];


	assert(id);
	assert(grid);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	str = *strptr;
	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "Color")){
					GetFieldValStr(curstr, color, 64);
				}

			}
		}
	}
	if (strcmp(color, "100"))
	{
		SecondTabGrid->childgrid->icon_set = 1;
		SecondTabGrid->icon_set = 1;
		if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count == 0)
			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
		strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Actions", char), 64, "Icon:", _TRUNCATE);
		grid->AlertsColorGrid->IconColorGrid->icon_color = atoi(color);
		grid->AlertsColorGrid->show_checkbox[2] = 1;
	}




}

void Subject_Grid(struct InstanceData* id, MGLGRID *grid, char **strptr)
{
	char *str, *endstr;
	int len;
	char value[64];

	assert(id);
	assert(grid);

	str = *strptr;
	char linebuf[4096];
	char *curstr, *fieldname;

	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "Subject")){
					GetFieldValStr(curstr, value, 64);
				}

			}
		}
	}
	strcpy_s(VP(grid->AlertsColorGrid->SubjectGrid->vo, 0, "Subject Line", char), 64, value);
}

void EMail_Alerts(struct InstanceData* id, MGLGRID *grid, char **strptr)
{
	char *str, *endstr;
	int len;
	char value[64];

	assert(id);
	assert(grid);

	str = *strptr;
	char linebuf[4096];
	char *curstr, *fieldname;

	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "email_enabled")){
					GetFieldValStr(curstr, value, 64);
				}

			}
		}
	}
	grid->AlertsColorGrid->EMailGrid->email_enabled = atoi(value);
	grid->AlertsColorGrid->show_checkbox[4] = atoi(value);
}
void EMailGrid_ID(struct InstanceData* id, MGLGRID *grid, char **strptr)
{
	char *str, *endstr;
	int len;
	char value[64];

	assert(id);
	assert(grid);

	str = *strptr;
	char linebuf[4096];
	char *curstr, *fieldname;
	int count = 0;

	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "EMailID")){
					GetFieldValStr(curstr, value, 64);
					if (grid->AlertsColorGrid->EMailGrid->vo->count - 1 < count)
						vo_alloc_row(grid->AlertsColorGrid->EMailGrid->vo, 1);
					strcpy_s(VP(grid->AlertsColorGrid->EMailGrid->vo, count, "EMail Notifications", char), 64, value);
					if (grid->BuildSummaryGrid->vo->count < count)
						vo_alloc_row(grid->BuildSummaryGrid->vo, 1);
					if (strcmp(VP(grid->AlertsColorGrid->EMailGrid->vo, count, "EMail Notifications", char),"Enter EMailID")!=0)
						strcpy_s(VP(grid->BuildSummaryGrid->vo, count + 1, "Actions", char), 64, value);
					count++;
				}

			}
		}
	}
	
}
void ShowGrid(struct InstanceData* id, MGLGRID *grid, char **strptr)
{
	int vorow;
	VO *DataAppVO;
	MGLGRID *buffer;
	char *DataSource;
	char *curstr, *fieldname;
	char value[64];
	char *str, *endstr;
	int len;
	char linebuf[4096];


	assert(id);
	assert(grid);

	str = *strptr;

	//Setting void pointers to MGLGRID struct
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	id->DataAppGrid = DataAppGrid;

	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "Show")){
					GetFieldValStr(curstr, value, 64);
				}

			}
		}
	}
	
	if (atoi(value) == 1)
	{
		DataAppVO = DataAppGrid->vo;
		for (vorow = 0; vorow < DataAppVO->count; vorow++){
			if (!strcmp(grid->name, VP(DataAppVO, vorow, "AppName", char))){
				// found match
				DataSource = VP(DataAppVO, vorow, "DataSource", char);
				buffer = VV(DataAppGrid->vo, vorow, "gridptr", void *);
				DataAppGrid->show_checkbox[vorow] = 20;
				grid->parent_w->visible = TRUE;
				break;
			}
		}
		DataAppGrid->show_checkbox[vorow] = 20;
	}

}
//mtm- retrieve stored Order number
void OrderGrid(struct InstanceData* id, MGLGRID *grid, char **strptr)
{




	char *curstr, *fieldname;
	char value[64];
	char *str, *endstr;
	int len,order;
	char linebuf[4096];


	assert(id);
	assert(grid);

	str = *strptr;
	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "Order")){
					GetFieldValStr(curstr, value, 64);
				}

			}
		}
	}
	order = atoi(value);
	grid->order_number = order;
	
}


//mtm- retrieve stored icon color
void ActiveGrid(struct InstanceData* id, MGLGRID *grid, char **strptr,MGLGRID *DataAppGrid)
{




	char *curstr, *fieldname;
	char value[64];
	char *str, *endstr;
	int len;
	char linebuf[4096];


	assert(id);
	assert(grid);

	str = *strptr;
	
	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "Active")){
					GetFieldValStr(curstr, value, 64);
				}

			}
		}
	}

	if (strcmp(value, "1") == 0)
	{
		for (int x = 0; x < grid->nActions; x++)
			grid->Actions[x].IsActive = 1;
		grid->active_icon = 1;
		DataAppGrid->active_checkbox[DataAppGrid->vo->count - 2] = 20;
		
	}
	else if (strcmp(value, "0") == 0)
	{
		for (int x = 0; x < grid->nActions; x++)
			grid->Actions[x].IsActive = 0;
		grid->active_icon = 0;
		DataAppGrid->active_checkbox[DataAppGrid->vo->count - 2] = 0;
	}
}

//mtm -row size after save
void SaveXrowsize(struct InstanceData* id, MGLGRID *grid, char **strptr)
{

	int rows;
	char full[64];

	char *curstr, *fieldname;
	char gridrowsize[64];
	char *str, *endstr;
	int len;
	char linebuf[4096];


	assert(id);
	assert(grid);
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;

	str = *strptr;
	while (*str != '\0'){
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';


			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				break;
			}
			else {
				str += len + 1; /* add one to length to skip over newline character */
				fieldname = GetFieldName(linebuf);
				if (!strcmp(fieldname, "Rows")){
					GetFieldValStr(curstr, gridrowsize, 64);
				}

			}
		}
	}


	rows = atoi(gridrowsize);
	if (strcmp(gridrowsize, "0") == 0)
	{
		if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 0)
			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
		strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, "", _TRUNCATE);
		strncpy_s(VP(SecondTabGrid->childgrid->RowColumn->vo, 0, "Settings", char), 16, "", _TRUNCATE);
	}
	else
	{
		strcpy_s(full, 64, "rows:");
		strcat_s(full, 64, gridrowsize);
		if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 0)
			vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
		strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 0, "Visiblerows/columns", char), 64, full, _TRUNCATE);
		strncpy_s(VP(SecondTabGrid->childgrid->RowColumn->vo, 0, "Settings", char), 16, gridrowsize, _TRUNCATE);
		if ((rows > 0) && (rows < 25)){
			SecondTabGrid->childgrid->nrows = rows + 1;
			SecondTabGrid->childgrid->height = (rows + 1)* MGL_GRID_DEF_ROW_HEIGHT;
		}
		else
		{
			SecondTabGrid->childgrid->nrows = 15;
			SecondTabGrid->childgrid->height = 15 * MGL_GRID_DEF_ROW_HEIGHT;
		}
	}


}

//mtm col size after save
void SaveXcolsize(struct InstanceData* id, MGLGRID *grid, char **strptr)
{

	//int cols,i;
	//char full[64];

	//char *curstr, *fieldname;
	//char gridcolsize[64];
	//char *str, *endstr;
	//int len;
	//char linebuf[4096];


	//assert(id);
	//assert(grid);

	//str = *strptr;
	//while (*str != '\0'){
	//	if (!(endstr = strchr(str, '\n'))){
	//		break;
	//	}
	//	len = endstr - str;
	//	if (len < sizeof(linebuf)){
	//		strncpy_s(linebuf, sizeof(linebuf), str, len);
	//		linebuf[len] = '\0';


	//		curstr = linebuf;
	//		if (!strncmp(linebuf, "Tablename:", 6)){
	//			break;
	//		}
	//		else {
	//			str += len + 1; /* add one to length to skip over newline character */
	//			fieldname = GetFieldName(linebuf);
	//			if (!strcmp(fieldname, "Cols")){
	//				GetFieldValStr(curstr, gridcolsize, 64);
	//			}

	//		}
	//	}
	//}


	//cols = atoi(gridcolsize);
	//if (strcmp(gridcolsize, "") == 0)
	//{

	//}
	//else
	//{
	//	strcpy_s(full, 64, "cols:");
	//	strcat_s(full, 64, gridcolsize);
	//	if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <0)
	//		vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
	//	if (SecondTabGrid->childgrid->BuildSummaryGrid->vo->count <= 1)
	//	{
	//		vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
	//		vo_alloc_rows(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1);
	//	}
	//	strncpy_s(VP(SecondTabGrid->childgrid->BuildSummaryGrid->vo, 1, "Visiblerows/columns", char), 64, full, _TRUNCATE);
	//	strncpy_s(VP(SecondTabGrid->childgrid->RowColumn->vo, 1, "Settings", char), 16, gridcolsize, _TRUNCATE);
	//	if (cols > 0 && cols < 9)
	//	{
	//		if (cols == 8)
	//		{
	//			for (i = 0; i < 8; i++)
	//			{
	//				SecondTabGrid->childgrid->DisplayCol[i] = i;
	//			}

	//		}
	//		if (cols == 7)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[7] = -1;
	//			for (i = 0; i < 7; i++)
	//			{
	//				SecondTabGrid->childgrid->DisplayCol[i] = i;
	//			}
	//		}
	//		else if (cols == 6)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[7] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[6] = -1;
	//			for (i = 0; i < 6; i++)
	//			{
	//				SecondTabGrid->childgrid->DisplayCol[i] = i;
	//			}
	//		}
	//		else if (cols == 5)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[7] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[6] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[5] = -1;
	//			for (i = 0; i < 5; i++)
	//			{
	//				SecondTabGrid->childgrid->DisplayCol[i] = i;
	//			}

	//		}
	//		else if (cols == 4)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[7] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[6] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[5] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[4] = -1;
	//			for (i = 0; i < 4; i++)
	//			{
	//				SecondTabGrid->childgrid->DisplayCol[i] = i;
	//			}

	//		}
	//		else if (cols == 3)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[7] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[6] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[5] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[4] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[3] = -1;
	//			for (i = 0; i < 3; i++)
	//			{
	//				SecondTabGrid->childgrid->DisplayCol[i] = i;
	//			}

	//		}
	//		else if (cols == 2)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[7] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[6] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[5] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[4] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[3] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[2] = -1;
	//			for (i = 0; i < 2; i++)
	//			{
	//				SecondTabGrid->childgrid->DisplayCol[i] = i;
	//			}

	//		}
	//		else if (cols == 1)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[7] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[6] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[5] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[4] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[3] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[2] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[1] = -1;
	//			SecondTabGrid->childgrid->DisplayCol[0] = 0;

	//		}



	//	}

	//	else
	//		for (i = 0; i < 8; i++)
	//		{
	//			SecondTabGrid->childgrid->DisplayCol[i] = i;
	//		}
	//}


}



int LoadXGrid(struct InstanceData *id, MGLGRID *grid, char *AppName, char *Arpt, char *outstr, MGLGRID *DataAppGrid)
{
	char linebuf[2048];
	char *str, *endstr, *fieldname, *curstr;
	char tablename[64], valbuf[64];
	int len;
	int actionnum = 0;


	assert(id);
	assert(grid);

	// return(SUCCEED); // test 

	strcpy_s(grid->name,32,AppName);
	// outstr = LoadDataFromDB( id, AppName, Arpt );

	// outstr = LoadTESTDataFromDB( id, AppName, Arpt );

	if (!outstr && !(outstr = GetUserXgridSettings(id, id->m_strUserName, Arpt, AppName))){
		return(FALSE);
	}
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__,
		"saved xgrid to DB str='%s'\n", outstr);
	str = outstr;

	

	do {
		if (!(endstr = strchr(str, '\n'))){
			break;
		}
		len = endstr - str;
		if (len < sizeof(linebuf)){
			strncpy_s(linebuf, sizeof(linebuf), str, len);
			linebuf[len] = '\0';
			str += len + 1; /* add one to length to skip over newline character */

			curstr = linebuf;
			if (!strncmp(linebuf, "Tablename:", 6)){
				curstr = &linebuf[6];
				GetFieldValStr(curstr, tablename, 64);
				if (!strcmp(tablename, "xgridactions")){
					LoadXGridActions(id, grid, &str);  // sets grid->nActions
					continue;
				}
				if (!strcmp(tablename, "xgridparameters")){

					LoadXGridParameters(id, grid->CurActionNum, grid, &str);
					int test = grid->Actions[grid->CurActionNum].gridconfig->nFilters;
					continue;
				}
				if (!strcmp(tablename, "xgridcolumns")){
					LoadXGridColumns(id, grid, &str);
					continue;
				}
				if (!strcmp(tablename, "xgriddefaults")){
					LoadXGridDefaults(id, grid, &str);
					continue;
				}

				if (!strcmp(tablename, "xrowsize")){
					SaveXrowsize(id, grid, &str);  // this appears to be the problem function xxxxx
					continue;
				}

				if (!strcmp(tablename, "xcolsize")){
					SaveXcolsize(id, grid, &str);
					continue;
				}

				if (!strcmp(tablename, "IconColor")){
					SaveXIconColor(id, grid, &str);
					continue;
				}
				if (!strcmp(tablename, "ActiveGrid")){
					ActiveGrid(id, grid, &str,DataAppGrid);
					continue;
				}
				if (!strcmp(tablename, "OrderOfGrid")){
					OrderGrid(id, grid, &str);
					continue;
				}
				if (!strcmp(tablename, "ShowGrid")){
					ShowGrid(id, grid, &str);
					continue;
				}
				if (!strcmp(tablename, "EMailGrid_ID")){
					EMailGrid_ID(id, grid, &str);
					continue;
				}
				if (!strcmp(tablename, "EmailAlerts")){
					EMail_Alerts(id, grid, &str);
					continue;
				}
				if (!strcmp(tablename, "EMailGrid_Subject")){
					Subject_Grid(id, grid, &str);
					continue;
				}
				}
				
			else {
				fieldname = GetFieldName(linebuf);
				GetFieldValStr(linebuf, valbuf, 64);
			}
		}
		else {
			break;
		}


	} while (*str != '\0');
	s_free(outstr);
	return(SUCCEED);
}



gui *CreateGUI(struct InstanceData* id)
{
	VO *DataAppVO;	
	gui *ggui;
	int i;
	static int drawopenglGUI = TRUE;
	char *DataSource;
	MGLGRID *FDOGrid, *ROIGrid, *AlertsGrid,*SysAlertsGrid, *TarmacGrid,*Grid,*a,*key;
	static int skipnewgui = 1;
	int j, k;
	static int draw_cal = 1;


	assert(id);

	//Setting void pointers to MGLGRID struct
	gui* oglgui = (gui*)id->m_poglgui;
	MGLGRID* DataAppGrid = (MGLGRID*)id->DataAppGrid;
	MGLGRID* DataAppCreater = (MGLGRID*)id->DataAppCreater;
	MGLGRID* SecondTabGrid = (MGLGRID*)id->SecondTabGrid;
	MGLGRID* CalendarAppGrid = (MGLGRID*)id->CalendarAppGrid;

	// create the gui
	ggui = (gui *)vo_malloc(sizeof(gui));
	ggui->gregion.x1 = 0;
	ggui->gregion.y1 = id->m_nHeight - 460;
	ggui->gregion.x2 = 220;
	ggui->gregion.y2 = id->m_nHeight - 30;
	ggui->bg = (GLfloat*)grey;
	ggui->active = 1;
	ggui->visible = 1;

	oglgui = ggui;
	id->m_poglgui = oglgui;
	if (!skipnewgui){
		if (drawopenglGUI) {
			CreateTopGUI(id);
			if (!ApplicationsBarW) {
				CreateApplicationsSubNav(id);
				InActivateAllSubNavs(id);

			}
			if (!OverlaysBarW) {
				CreateOverlaysSubNav(id);
				InActivateAllSubNavs(id);

			}
			if (!SettingsBarW) {
				CreateSettingsSubNav(id);
				InActivateAllSubNavs(id);

			}
			if (!AirportsBarW) {
				CreateAirportsSubNav(id);
				InActivateAllSubNavs(id);

			}

			CreateRightBar(id);
		}
	}


	//CreateMainTab(id);
	//FillMainTabVO(id, MainTabGrid);
	DataAppGrid=CreateDataAppSelector(id,DataAppGrid);//It is the grid with the grid list(Main grid with ROI's and FDO's)
	id->DataAppGrid = DataAppGrid;
	//CreateGridManagerGrid(id);//Title grid for the main grid(Has "Grid Manager" on it)
	DataAppCreater=CreateDataAppCreater(id,DataAppCreater);//Creates new grid.(Grid where we enter the name, type of grid and hit create)
	id->DataAppCreater = DataAppCreater;
	FillDataAppVO(id, DataAppGrid);	// top level widget for all data grids
	FillDataAppCreaterVO(id, DataAppCreater);
	SecondTabGrid = CreateSecondTab(id,SecondTabGrid);
	id->SecondTabGrid = SecondTabGrid;
	FillSecondTabVO(id, SecondTabGrid);

	/*Functions to generate Calendar widget*/
	CalendarAppGrid= CreateCalendar(id, CalendarAppGrid);
	id->CalendarAppGrid = CalendarAppGrid;
	FillCalendarAppVO(id, CalendarAppGrid);
	display_calendar(draw_cal, 's', id, CalendarAppGrid);
	draw_cal = 0;






	DataAppVO = DataAppGrid->vo;

	for (i = 0; DataAppVO && i < DataAppVO->count; i++){
		/*if (DataAppGrid->nrows <= 9)
			DataAppGrid->nrows++;*/
		DataSource = VP(DataAppVO, i, "DataSource", char);
		//	DataSource = VP(DataAppVO, 1, "DataSource", char);
		if (!strcmp(DataSource, "FDO")){
			FDOGrid = CreateFDOGrid(id); // top level widget for all data grids
			FillFDOFromGFDO(id, FDOGrid);
			// fill in the ptr to the grid 
			VV(DataAppVO, i, "gridptr", void *) = FDOGrid;
			// fill in any prior settings from the user that are stored in the DB
			LoadXGrid(id, FDOGrid, VP(DataAppVO, i, "AppName", char), NULL, NULL,DataAppGrid);
			MGLSetTitle(FDOGrid, VP(DataAppVO, i, "AppName", char));
		}
		else if (!strcmp(DataSource, "ROI")){
			ROIGrid = CreateROIGrid(id); // top level widget for all data grids
			// fill in the ptr to the grid
			VV(DataAppVO, i, "gridptr", void *) = ROIGrid;
			LoadXGrid(id, ROIGrid, VP(DataAppVO, i, "AppName", char), NULL, NULL,DataAppGrid);
			MGLSetTitle(ROIGrid, VP(DataAppVO, i, "AppName", char));
		}
		else if (!strcmp(DataSource, "Alerts")){
			AlertsGrid = CreateAlertsGrid(id); // top level widget for all data grids
			// fill in the ptr to the grid 
			FillDefaultAlertsVO(id, AlertsGrid);
			CalcColWidths(id, AlertsGrid);
			VV(DataAppVO, i, "gridptr", void *) = AlertsGrid;
		}
		else if (!strcmp(DataSource, "Tarmac")){
			TarmacGrid = CreateTarmacGrid(id); // top level widget for all data grids
			// fill in the ptr to the grid 
			FillDefaultTarmacVO(id, TarmacGrid);
			MGLSetColNamesFromVO(TarmacGrid);
			VV(DataAppVO, i, "gridptr", void *) = TarmacGrid;
			LoadXGrid(id, TarmacGrid, VP(DataAppVO, i, "AppName", char), NULL, NULL,DataAppGrid);
			MGLSetTitle(TarmacGrid, VP(DataAppVO, i, "AppName", char));
		}
		/*else if (!strcmp(DataSource, "SysAlerts")){
			SysAlertsGrid = CreateSysAlertsGrid(id); // top level widget for all data grids
			FillSysAlertsFromGSA(id, SysAlertsGrid);
			// fill in the ptr to the grid 
			VV(DataAppVO, i, "gridptr", void *) = SysAlertsGrid;
			// fill in any prior settings from the user that are stored in the DB
			LoadXGrid(id, SysAlertsGrid, VP(DataAppVO, i, "AppName", char), NULL, NULL, DataAppGrid);
			MGLSetTitle(SysAlertsGrid, VP(DataAppVO, i, "AppName", char));
		}*/
	}

	ListUserXgridAppnames(id, id->m_strUserName, "", DataAppVO, DataAppGrid,DataAppCreater);

	//for (i = 0; DataAppGrid && (DataAppVO = DataAppGrid->vo) && i < DataAppVO->count - 1; i++){
	//	DataSource = VP(DataAppVO, i, "DataSource", char);
	//	Grid = VV(DataAppGrid->vo, i, "gridptr", void *);
	for (j = 1; j < DataAppVO->count - 1; j++)
	{
		key = VV(DataAppGrid->vo, j, "gridptr", void *);
		k = j - 1;
		a = VV(DataAppGrid->vo, k, "gridptr", void *);
		if (key)
		{
			while (a && (k >= 0) && (key->order_number < a->order_number))
			{
				Grid = VV(DataAppGrid->vo, j, "gridptr", void *);
				MGLGridColumnMoveDown(DataAppGrid, k);
				Grid = VV(DataAppGrid->vo, j, "gridptr", void *);
				k--;
				if (k >= 0)
					a = VV(DataAppGrid->vo, k, "gridptr", void *);
			}
		}
		

	}
	
//	}
	// create "NEW" row, that user will click on to add an action
	vo_alloc_rows(DataAppCreater->vo, 1);
	//strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "Down Arrow", char), 64, "Type", _TRUNCATE);
	//strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "Extra1", char), 64, " Option", _TRUNCATE);
	strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "AppName", char), 64, "  Add New", _TRUNCATE);
	strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "DataSource", char), 16, "  Source", _TRUNCATE);
	strncpy_s(VP(DataAppCreater->vo, DataAppCreater->vo->count - 1, "Create", char), 16, "Create", _TRUNCATE);


	/*vo_alloc_rows(DataAppGrid->vo, 1);
	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "AppName", char), 64, "New", _TRUNCATE);
	strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "DataSource", char), 16, "Choose", _TRUNCATE);*/
	//strncpy_s(VP(DataAppGrid->vo, DataAppGrid->vo->count - 1, "Show", char), 16, "Show", _TRUNCATE);

	

	SetFontIndex(id, id->m_nCustFontIdx);  // reset so the large tags will have a good box
	return(oglgui);
}


int MGLDrawWidgets(struct InstanceData* id, gui *oglgui)
{
	widgets *w;
	MRECT wrld;
	static int fsize = 0;
	static char cmpstr[64];

	assert(id);

	if (!oglgui){
		return(FALSE);
	}
	wrld.x1 = 0;
	wrld.x2 = id->m_nWidth;
	wrld.y1 = 0;
	wrld.y2 = id->m_nHeight;

	for (w = oglgui->widgets; w; w = w->np){
		// draw the image 

		if (!strcmp(cmpstr, w->name)){
			// test
			fsize = 0; // dummy for breakpoint
		}
		if (!strcmp("LeftSliderEdgeW", w->name)){
			// test
			fsize = 0; // dummy for breakpoint
		}

		if (!w->visible)
			continue;

		switch (w->type)
		{
		case WGT_BUTTON:
			// MGLDrawButton( id, grid, w );
			MGLDrawPanelImage(id, w);
			break;
		case WGT_TEXT:
			MGLDrawText(id, w);
			break;
		case WGT_PANEL:
			MGLDrawPanel(id, w);
			break;
		case WGT_PANEL_IMAGE:
			MGLDrawPanelImage(id, w);
			break;
		case WGT_TEXT_BUTTON:
			MGLDrawTextButton(id, w);
			break;
		case WGT_SLIDER:
			MGLDrawSlider(id, w);
			break;
		default:
			break;
		}


		if (w->grid)
		{
			MGLGridDraw(id, w->grid);
		}

	}

	//SetGUIFontIndex(id, 15, 0);
	//DrawGLGUIText(  id, 81, id->m_nHeight - 40, "ORD", (GLfloat *) yellow );

	//SetGUIFontIndex(id, 5, 0);
	//DrawGLGUIText(  id, 201, 731, "PASSUR", (GLfloat *) yellow );

	// DrawGLText( id, xpixel, ypixel, mystr, (GLfloat *) color);
	//DrawGLGUIText(  id, 400, 400, "My Test Verdana Text", (GLfloat *) white );

	return(TRUE);
}


void UpdateGUIValues(struct InstanceData* id)
{
	char *arpt;
	struct row_index *row_index;
	static char prevarpt[5];

	assert(id);

	if (AirportTextW && id->m_pAirportsVO){
		arpt = "N/A";
		if (id->m_sCurLayout.m_bShowPassurAircraft) {
			arpt = id->m_strPassurArpts[0];
		}
		else if (id->m_sCurLayout.m_bShowASDEXAircraft){
			if (IsValidAsdexAirport(id, id->m_strAsdexArpt)){
				arpt = id->m_strAsdexArpt;
			}
		}
		if (!strcmp(arpt, prevarpt)){
			return;
		}
		strcpy_s(prevarpt, sizeof(prevarpt), arpt);
		if ((row_index = vo_search((VO*)id->m_pAirportsVO, "passur_code", arpt, NULL))){
			arpt = VP((VO *)id->m_pAirportsVO, row_index->rownum, "icao_code", char);
			strcpy_s(AirportTextW->textbuf, sizeof(AirportTextW->textbuf), arpt);
			StrUpper(AirportTextW->textbuf);
		}

		strncpy_s(AirportText2W->textbuf, sizeof(AirportText2W->textbuf), AirportTextW->textbuf, _TRUNCATE);
		strncpy_s(AirportText3W->textbuf, sizeof(AirportText3W->textbuf), AirportTextW->textbuf, _TRUNCATE);
	}



}



void MGLDrawGUI(struct InstanceData* id)
{
	// gui* panel;

	assert(id);
	gui* oglgui = (gui*)id->m_poglgui;
	if (!oglgui){
		oglgui = CreateGUI(id);
		id->m_poglgui = oglgui;
	}

	UpdateGUIValues(id);
	// draw the widgets
	MGLDrawWidgets(id, oglgui);
	id->resize = 0;


}




