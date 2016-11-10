
#ifndef __MGL_H__
#define __MGL_H__



// defines
#define MGL_GRID_DEF_ROW_HEIGHT  18
#define MGL_GRID_DEF_COL_WIDTH  75

#define MGL_GRID_MAX_ROWS 200
#define MGL_GRID_MAX_COLS 60


#define MAX_GRID_CELL_STRING_SIZE   256
#define MGL_DEFAULT -1
#define MGL_GRID_MARGIN_X 6
#define MGL_GRID_MARGIN_Y 4

#define MGL_GRID_SCROLL_WIDTH 20


// standart alpha mode
#define ALPHA_MODE_NONE		 0
// since gd lib reads alpha encoding incorrectly in our version, ALPHA_MODE_BLACK assume all black pixels are transparent
#define ALPHA_MODE_BLACK   1 


#ifdef OLDWAY
typedef struct guiregion {
	int x1, y1, x2, y2;
} gregion;
#endif


typedef struct zrect_struct
{
	int x1, y1;
	int width, height;
} ZRECT;


typedef struct mrect_struct
{
	int x1, y1, x2, y2;
} MRECT;


typedef struct guiwindow {
	struct guiwindow *np;
	MRECT gregion;
	HWND hwnd;
	int visible;	// Show(1)/Hide(0) Window
} gwindow;


// structs and typedefs
typedef enum {
	MGL_MENU = 1,
	MGL_PANEL,
	MGL_SELECTLIST,
	MGL_LABEL,
} mgl_types;

typedef struct g_cells {
	// int x, y;
	MRECT cellrect;
	GLfloat *fg;
	GLfloat *bg;
	int width, height;
	//mtm//
	char * (*Text_callback)(char *text_fill);
	//
	char *val;
	GLuint texture;
	int(*cell_callback)(struct InstanceData* id, int b, int m, int x, int y);

	//mtm
	char pngfilename[32];

} MGLGRIDCELLS;

typedef struct g_rows {
	
	int x, y;
	GLfloat *fg;
	GLfloat *bg;
	int width, height;
	GLuint texture;
	MRECT rowrect;
	int HasCellCallbacks;  // if any cells in this row have callbacks, this is true. used to speed up cell searches
	int(*row_callback)(struct InstanceData* id, struct grid_struct *parentgrid, int b, int m, int x, int y);
	int(*row_callback_np)(struct InstanceData* id, struct grid_struct *parentgrid, int b, int m, int x, int y,int test);
	GLfloat *(*bg_callback)(struct grid_struct *grid, int rownum);
	GLfloat *(*fg_callback)(struct grid_struct *grid, int rownum);
} MGLGRIDROW;

typedef struct g_cols {
	// int x, y;
	GLfloat *fg;
	GLfloat *bg;
	int width, height;
	// MRECT colrect;
	char * (*val_callback)(struct InstanceData* id, char *valstr);
	void(*filter_callback)(struct InstanceData* id, struct grid_struct *grid, int colnum, char *colname);
	int display_data_type;  // DATETIMEBIND, etc
	char display_fmt[32];  //  i.e., %Y%m%d etc   -- not yet implemented
	char pngfilename[32];  // if an image is displayed instead of a value
	int texturesize;
	int img_displaysize;
} MGLGRIDCOL;

//MGLGRIDROW gridrows[ MGL_GRID_MAX_ROWS ];

#define MGL_GRID_NAME_MAX 50
#define MGL_GRID_LABEL_MAX 32
#define MGL_GRID_TEXTBUF_SIZE	32
#define MGL_GRID_MAX_SORT_COLS  10   // can sort by up to 10 columns out of all the columns in a grid
#define MAX_GRID_ACTIONS 10
#define MIN_ELEVATOR_HEIGHT 5


typedef enum {
	WGT_BUTTON = 1,
	WGT_TEXT_BUTTON,  // button made of generated text using DrawGLGUIText
	WGT_PANEL,
	WGT_PANEL_IMAGE,
	WGT_SLIDER,
	WGT_GRID,
	WGT_TAB,
	WGT_TEXT,

	WGT_LAST  /* never use */
} WGTcodes;


// make ACTIONCodes power of 2 so that the actions can me or'd together. Each action can be any combination of all the actions
typedef enum {
	ACTION_FILTER = 1 << 0,
	ACTION_ALERT_LIST = 1 << 1,
	ACTION_COLOR_FG = 1 << 2,
	ACTION_COLOR_BG = 1 << 3,
	ACTION_EMAIL = 1 << 4,
	ACTION_SOUND = 1 << 5,

	ACTION_LAST = 1 << 6 // never use
} ACTIONCodes;


typedef enum {
	FTYPE_LIST = 0,
	FTYPE_OPERATORS,

	FTYPE_LAST  // never use
} FilterTypes;


// Grid Configuration Structures

typedef struct _GridFilterConfig
{
	int		filter_type;		// 0=FTYPE_LIST, 1=FTYPE_OPERATORS  (Criteria-View)
	char	field_name[40];
	int		field_type;			// INTBIND, NTBSTRINGBIND, as stored in VO data structure
	int		exclude_selected;	// 1=true
	char	select_value[2048];	// Comma separated list of values for list-view
	char	op[4];				// LT, LE, EQ, GE, GT, CO(ntains) for criteria-view
	char	cmp_value[64];		// Comparison value for criteria-view	
	VO *searchvo;  // select_values placed into a vo for fast search  -- should probably replace with hash someday
} GridFilterConfig;

typedef struct _GridSortConfig
{
	int	enabled;		// 1=true
	int	active;			// 1=true
	int	order;			// sort-order
	int	sort_type;		// 0=Ascending, 1=Descending
} GridSortConfig;


typedef struct _GridColumnConfig
{
	char	name[16];		// Column Header name
	char	field_name[16];	// DB object field name
	int		order;			// column order
	int		data_type;		// INTBIND, NTBSTRINGBIND,  As displayed, so if field_type = int, and data_type = DATETIMEBIND, then convert to time
	int		display;		// 1=true
	int		filter_enabled;	// 1=true
	int		list_src;
	char	value_list[4096]; // Comma separated list of values for filters
	char	display_fmt[32];  /// user can select format on screen, like %H%M%S for dates, or %2d for integers  (enter in sort grid?????)
	GridSortConfig			sortConfig;
} GridColumnConfig;


typedef struct _GridConfig
{
	int		outline;			// 1=on, 0=off
	unsigned	outline_color;	// Draw outline color of region
	int		nFilters;			// Number of filters defined in filters
	GridFilterConfig	filters[10];
	// GridColumnConfig	configcolumns[10];  // no longer needed use AvailableVO
} GridConfig;


typedef struct _GridActions
{
	char ActionName[32];
	int IsActive;  // user has turned it on in Alerts Grid settings
	int ActionType;   // enum ACTIONCodes (ACTION_FILTER, ACTION_ALERT_LIST, etc)
	GridConfig *gridconfig;  // used to store and retrieve user defined grid parameters from database
	char row_fg[7];  // in rgb hex  RRGGBB, FF00FE 
	char row_bg[7];
	char soundname[32];  // tbd, see BD's sound's in DM
	int  filter_active;
	int  action_delete;
} GridActions;

typedef struct _BuildSummary
{
	int Criteria_rownumber;
	int Parameter_rownumber[10];
	int IconColor_rownumber;
	int Layout_rownumber;
	int Visiblerowscolumns_rownumber;

	
}BuildSummary;

typedef struct widgetstruct {
	int flag;
	struct widgetstruct *np;
	struct widgetstruct *siblings[5];
	struct widget_ll *radio_ll;
	int nsiblings;
	char name[MGL_GRID_NAME_MAX];
	MRECT gregion; //  size of bounding box for this region, may be absolute or relative coodinates offset to parent widget
	MRECT aregion;  // a is for the actual location of bounding region, calculated using offset from parent's coordinates 
	int width, height;
	struct widgetstruct *gregion_parentw;  // parent widget to use for relative coordinates
	int texsize;  // // size of texture holding the image (power of 2 required by opengl)
	int type; // button, slider, etc
	int IsActive;
	char *active_image; // image when activated ( ON )
	char *inactive_image; // image when inactivated ( OFF )
	struct grid_struct *grid;
	int visible;	// Show(1)/Hide(0) widget
	int(*wgt_mouse_cb)(struct InstanceData* id, struct widgetstruct *w, int x, int y, struct guistruct* g);
	int(*OnClickCB)(struct InstanceData* id, struct widgetstruct *w, int x, int y);
	char textbuf[64];  // for text widgets
	int(*wgt_text_cb)(struct InstanceData* id, struct widgetstruct *w);
	GLfloat *bg;
	GLfloat *fg;
	int TextActive;
	int img_alpha_mode;
	int img_blend;
	int filtertype;  // GL_LINEAR or GL_NEAREST
	GLfloat ImgAngle;  // rotation angle for widget
	int fontindex;  // 0 to GUI_FONTCOUNT
	int textheight;  // for multiline text buttons
	int fontbase;  // gl base list 
	GLfloat minval, maxval, curval; // slider values
	//mtm//
	int visible_memory;
	int maintab_visible;
} widgets;

// use this struct to group a set of widgets in a group, such as radio buttons, or groups of button images forming one button
typedef struct widget_ll {
	struct widget_ll *np;
	widgets *w;
} widgets_ll;

typedef struct row_names {
	int total_rows;
	char col[5][25];
	char row_name[32];

}row_names;


#ifndef FLTNUM_SIZE
#define FLTNUM_SIZE 8
#endif

#ifndef ORIG_DES_SIZE
#define ORIG_DES_SIZE 5
#endif

typedef struct flightstruct  {
	char flightid[FLTNUM_SIZE];
	char destination[ORIG_DES_SIZE];
	char flightidRegion[40];
	int filteredrownum;  // filteredvo rownum
} FilteredLUT;



typedef struct grid_struct {
	char name[MGL_GRID_NAME_MAX];
	struct grid_struct *parentgrid; // for settings and filter grids, need path to parent grid
	struct grid_struct *childgrid;//mtm
	GLfloat *bg;			// background color of main panel
	GLfloat *fg;    
	
	//madhuri//
	row_names row[5];
	int row_number;
	int flag1,flag2;
	int move_flag;
	int callback;
	int row_checkbox[100];
	int active_checkbox[100];
	int show_checkbox[100];
	int dataapp_confirmdelete[100];
	int criteria_confirmdelete[100];

	//sy 
	int showflights_check;

	int mm;
	int del;
	////

	 // of main panel 
	
	
	int nrows, ncols;		// number of rows and columns in grid
	//int x, y;				// location of grid lower, left
	int width, height;	// width and height of entire grid including non-scrolling areas
	int winwidth, winheight; // actual size of scroll window
	int nBox;  // flag for dragging elevator button
	int MoveBox; // flag for dragging grid window
	int textrow;  // row number that is currently accepting input
	char textcolname[64];
	struct widgetstruct *parent_w;  // for reference to parent widget
	MGLGRIDROW gridrows[MGL_GRID_MAX_ROWS];
	MGLGRIDCOL gridcols[MGL_GRID_MAX_COLS];
	MGLGRIDCELLS gridcells[MGL_GRID_MAX_ROWS][MGL_GRID_MAX_COLS];
	char ColLabels[MGL_GRID_MAX_COLS][MGL_GRID_LABEL_MAX];   // name of each column to appear on display
	char ColOrderNames[MGL_GRID_MAX_COLS][MGL_GRID_LABEL_MAX];  // list of VO column names in sort order. apps can change column order at any time
	char KeyFields[MGL_GRID_MAX_COLS][MGL_GRID_LABEL_MAX];   // list of columns in vo for identifying unique rows for alert dups 
	char DisplayCol[MGL_GRID_MAX_COLS];  // since user can change display column location, this value maps to default column value
	char Flightnum[20];
	int marginx, marginy; // margins in cells for displaying text
	int iVscrollPos;
	int GScrollWidth;
	int AllowScrollbar;  // TRUE to display scrollbar
	int NeedsScrollbar; // if FALSE, the scrollbar will not be drawn since not enough rows
	int HasTitleBar;
	int edit;
	MRECT gridrect;  //  used for both open and closed states
	MRECT gridrectmin; // rect for open state - mimimized
	MRECT gridrectmax; // rect for open state - maximized
	MRECT scrollbar_rect;
	MRECT scrolltop_rect;
	MRECT scrollbot_rect;
	MRECT elev_rect;
	MRECT mouse_rect; // needed so we can ignore small mouse movements and treat them as a single click
	MRECT drag_start_rect;  // for keeping original value of grid during drag operations 
	MRECT title_rect;
	MRECT xclose_rect;  // rectangle containing X close button
	MRECT minimize_rect;  // rectangle grid minimize button
	MRECT maximize_rect;  // rectangle grid maximize button
	MRECT rules;//mtm-rules button
	MRECT cal_prev;//calendar_< arrow members
	MRECT cal_next;//calendar_> arrow members
	MRECT layout;//mtm-layout button
	int IsMinimized;     // Normal operations - FALSE, if user minimizes grid to just title row, the IsMinimized = 1
	// int *RowIsFiltered;   // to decide if a row is filtered, use if ( RowIsFiltered[i] )
	// int RowIsFilteredCount; // if source vo rows no longer equals RowIsFilteredCount, then we need to recreate it to match
	// int NVisColumns;   // number of visible columns for this grid, used with int *VisColumnNames
	// int *VisColumnNames;  // list of column names in display order, ie colname = VisColumnNames[2] gives name of 3rd column that is visible in grid
	VO *vo;  // if data source is a vo
	GridActions Actions[MAX_GRID_ACTIONS];
	BuildSummary Summary[10];
	int nActions;
	int CurActionNum;
	// GridConfig *gridconfig;  // used to store and retrieve user defined grid parameters from database
	VO *filteredvo;  // copy of vo, but only for rows that match filter
	widgets *title_widgets;  // for submit, cancel buttons, title text field,  or menu items that appear in title bar
	//VO *FilterVO;
	//VO *AvailableVO;
	//VO *AlertsVO;
	//VO *AlertsColorVO;
	//VO *ActionsVO;
	int checkifopen;

	///added by mtm to revert color back in dataapp grid//
	int rownumbindataapp;
	int check_newgrid;
	int roi_fdo;//for roi set flag to 1 for fdo set flag to 2 and for system alerts to 3;
	////

	struct grid_struct *ParametersGrid;   // only parent  grid will have this
	struct grid_struct *OperatorGrid;   // only parent  grid will have this
	struct grid_struct *AlertsColorGrid; // only parent  grid will have this
	struct grid_struct *ActionsGrid; // only parent  grid will have this
	struct grid_struct *BackgroundColorGrid;//Color palette for background color
	struct grid_struct *ForegroundColorGrid;//Color palette for foreground color
	struct grid_struct *IconColorGrid;//Color palette for icon color
	struct grid_struct *FilterGrid;
	struct grid_struct *SortAvailableGrid;
	struct grid_struct *GridActions;
	struct grid_struct *IconGrid;
	struct grid_struct *Databookgrid;
	struct grid_struct *Notifygrid;
	struct grid_struct *BackgroundPanel;
	struct grid_struct *RowColumn;
	struct grid_struct *BuildSummaryGrid;
	struct grid_struct *TitleGrid;
	struct grid_struct *EMailGrid;
	struct grid_struct *SubjectGrid;
	
	FilteredLUT *Flut;
	int order_number;

	int icon_set;//mtm
	int active_icon;//mtm
	int icon_color;//mtm
	char *grid_type;//mtm
	int to_open_build;//mtm
	int to_open_details;//mtm
	int character_limit;//mtm
	int shift_limit;//mtm
	int fixed_width;//mtm
	int aircraftcolor_change_MLAT;//mtm
	int aircraftcolor_change_ASDEX;//mtm
	int aircraftcolor_change_Noise;//mtm
	int aircraftcolor_change_Gate;//mtm
	int aircraftcolor_change_ASD;//mtm
	int aircraftcolor_change;//mtm
	int updateroi;//mtm
	int check_firstopen;//mtm
	int email_enabled;
	char copy_create[64];
	int vonumber;

	char *icon_name;
	// struct grid_struct *SortSelectedGrid;
	struct grid_struct *GridSettingsMenu;
	int(*UpdateCB)(struct InstanceData* id, struct grid_struct *grid);
	int(*UpdateSACB)(struct InstanceData* id, struct grid_struct *grid);
	int lastcalccol;  // since calccolwidths is a bit expensive, limit calls to it
} MGLGRID;




typedef struct guistruct {
	struct guistruct *np; // more than one panel, or GUI
	MRECT gregion;
	widgets *widgets;
	gwindow *windows;
	GLfloat *bg;  // background color
	int visible;	// Show(1)/Hide(0) Panel
	int active;		// Show background (1)
} gui;



typedef struct menuitem_struct {
	struct menuitem_struct *np;
	char *label;
	int IsActive;
	GLfloat *bg;
	GLfloat *fg;
} MGLMenuItem;


typedef struct panellist_struct {
	struct panellist_struct *np;
	struct panellist_struct *lastp;
	int itemtype;
	void *panelitem;
} panellist;


typedef struct  panel_struct {
	int x;
	int y;
	int width;
	int height;
	GLfloat *bg;
	GLfloat *fg;
	panellist *plist;
} MGLPanel;



typedef struct  menu_struct {
	MGLPanel *panel;
	char *label;
	int xoffset;  /* offset from panel's x,y origin */
	int yoffset;
	GLfloat *bg;
	GLfloat *fg;
	int IsActive;
	int labelx1, labelx2, labely1, labely2; /* label extents for GUI mouse picks */
	MGLMenuItem *itemlist;
	void(*callback)(struct  menu_struct *menu);
} MGLMenu;



typedef struct calcll_struct {
	struct calcll_struct *np;
	struct calcll_struct *pp;
	int type; // operator or value
	char val[512];
} CALC_LL;

typedef enum {
	CALC_VAL = 1,
	CALC_OP,
	CALC_LAST  /* never use */
} CALCcodes;

//extern gui *oglgui;

extern const GLfloat gridgrey0[3];
extern const GLfloat gridgreyelev[3];
extern const GLfloat gridgrey1[3];
extern const GLfloat color504e4f[3];
extern const GLfloat colorfc4740[3];
extern const GLfloat color454545[3];
extern const GLfloat color868686[3];
extern const GLfloat colorf8cf53[3];
extern const GLfloat color353535[3];
extern const GLfloat color686868[3];
extern const GLfloat colorblue[3];
extern const GLfloat colorbrightblue[3];
//extern void DropDownColorBox(struct InstanceData* id,MGLGRID *parentgrid);
extern MGLMenuItem *MGLCreateMenuItem(MGLMenu *reportsmenu, char *label);
extern MGLMenu *MGLCreateMenu(MGLPanel *panel, char *label, int xoffset, int yoffset,
	void(*callback)(MGLMenu *menu));
extern MGLPanel *MGLCreatePanel(int x, int y, int width, int height,
	GLfloat *fg, GLfloat *bg);
extern void SetColorCol(MGLGRID *AlertsColorGrid);
extern void SetAlertColors(MGLGRID *parentgrid);
extern int FindUserFlight(struct InstanceData* id, char *flightnumtext);
extern void ColorPalette_foreground(MGLGRID *parentgrid);
extern void ColorPalette_background(MGLGRID *parentgrid);
extern void CrAlertsColorVO(MGLGRID *parentgrid);
void CrAlertsColorPaletteVO(MGLGRID *parentgrid);
//extern int AlertsColorCallback_colorpalette(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y);
//extern int AlertsColorCallback_colorpalette(struct InstanceData* id, MGLGRID *AlertsColorGrid, MGLGRID *parentgrid, int b, int m, int x, int y);
extern int AlertsColorCallback_colorpalettefg(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y);
extern int Iconmatrix_callback(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y);
extern int AlertsColorCallback_colorpalettebg(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y);

extern void CreateColorMatrix_foreground(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible);
extern void CreateColorMatrix_background(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible);
extern void GridToDisaplaySave(struct InstanceData* id, MGLGRID *parentgrid, int IsVisible);
extern int DropDownColor(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y);
extern int GridActions_callback(struct InstanceData* id, MGLGRID *AlertsColorGrid, int b, int m, int x, int y);

extern MGLGRID *MGLGridCreate(struct InstanceData* id, MGLGRID *parentgrid, char *name, int nrow, int ncols, int nxpos, int nypos);

extern int MGLGridText(struct InstanceData* id, MGLGRID *grid, int row, int col, char *text);

extern int MGLGridRowColorBG(MGLGRID *grid, int row, GLfloat *bg);

extern int MGLGridDraw(struct InstanceData* id, MGLGRID *grid);

extern int MGLGridMove(struct InstanceData *id, MGLGRID *grid, int nxpos, int nypos);

extern int MGLCalcCol(MGLGRID *grid, int x);

extern int MGLIsGuiMouseEvent(struct InstanceData* id, int b, int m, int x, int y);

extern int MGLIsGuiMotionEvent(struct InstanceData* id, int x, int y);
extern int MGLIsGridKeydown(struct InstanceData* id, WPARAM wParam, int keystate);

extern void MGLGridResizeCol(struct InstanceData *id, MGLGRID *grid, int c, int maxwidth);

extern void SetGridGLMode(struct InstanceData* id);
extern int ConfirmCallback(struct InstanceData* id,  MGLGRID *ConfirmGrid, int b, int m, int x, int y);
extern int CriteriaConfirmCallback(struct InstanceData* id, MGLGRID *CriteriaConfirmGrid, int b, int m, int x, int y);
extern int GridMenuCallback_rules(struct InstanceData* id, MGLGRID *settingsgrid, int b, int m, int x, int y);
extern int GridMenuCallback_layout(struct InstanceData* id, MGLGRID *settingsgrid, int b, int m, int x, int y);
extern void MGLDrawPanel(struct InstanceData* id, widgets *w);
extern void MGLDrawPanel_new(struct InstanceData* id, widgets *w);
extern int MGLMatch(MGLGRID *SourceGrid, int action, char *op, int f, char *cmp_value, char *valstr);
extern int MGLMatch_all(MGLGRID *SourceGrid, int action, char *op, int f, char *cmp_value, char *valstr,char *colname);
extern void MGLFillParameters(struct InstanceData* id, MGLGRID *ParametersGrid);
extern void MGLFillParametersDelete(struct InstanceData* id, MGLGRID *ParametersGrid, int vorow);
extern void MGLFillParametersColsVO(MGLGRID *grid);
extern void MGLClearParametersGrid(MGLGRID *SourceGrid);
extern void MGLFillParametersGrid(MGLGRID *ActionsGrid, int CurActionNum, struct InstanceData* id);
extern void MGLInitAlertsColorsVO(MGLGRID *topgrid);
//added by mtm
extern void MGLInitAlertsColorsVO_me(MGLGRID *topgrid);
//

extern void MGLSetColNamesFromVO(MGLGRID *grid);
extern void MGLSetlabelNamesFromVO(MGLGRID *grid);
extern MGLGRID *CreateSecondTab(struct InstanceData* id, MGLGRID *SecondTabGrid);


extern int MGLHeadersCallback(struct InstanceData* id, MGLGRID *parentgrid, int b, int m, int x, int y);
//mtm// 
extern int MainTabCallback(struct InstanceData* id, MGLGRID *parentgrid, int b, int m, int x, int y);
extern int SecondTabCallback(struct InstanceData* id, MGLGRID *parentgrid, int b, int m, int x, int y);
//
extern int MGLFiltersMatch(struct InstanceData* id, MGLGRID *parentgrid, int action, int i);
extern VO *MGLCrSearchVO(char *select_value);
extern void MGLSortFilteredGrid(MGLGRID *parentgrid);

extern MGLGRID *CreateGridSettingsMenu(struct InstanceData* id, MGLGRID *parentgrid);
extern MGLGRID *CreateGridSettingsMenu_me(struct InstanceData* id, MGLGRID *parentgrid);


extern const GLfloat yellow[3];
extern GLfloat orange[3];
extern const GLfloat green[3];
extern GLfloat silver[3];
extern GLfloat maroon[3];
extern GLfloat yellow1[3];
extern GLfloat yellow2[3];
extern GLfloat yellow3[3];
extern GLfloat yellow4[3];
extern GLfloat yellow5[3];

extern GLfloat red1[3];
extern GLfloat red2[3];
extern GLfloat red3[3];
extern GLfloat red4[3];
extern GLfloat red5[3];

extern GLfloat violet1[3];
extern GLfloat violet2[3];
extern GLfloat violet3[3];
extern GLfloat violet4[3];
extern GLfloat violet5[3];

extern GLfloat blue1[3];
extern GLfloat blue2[3];
extern GLfloat blue3[3];
extern GLfloat blue4[3];
extern GLfloat blue5[3];

extern GLfloat green1[3];
extern GLfloat green2[3];
extern GLfloat green3[3];
extern GLfloat green4[3];
extern GLfloat green5[3];


//mtm new colors of grid tool
extern GLfloat Blue_tab[3];
extern GLfloat Blue_title[3];
extern GLfloat Blue_fg[3];
extern GLfloat grey_Tab_bg[3];

//larry new gui colors
extern GLfloat Text_yellow[3];
extern GLfloat Grid_grey1[3];
extern GLfloat Grid_grey2[3];
extern GLfloat Grid_grey3[3];
extern GLfloat Grid_grey4[3];
extern GLfloat Grid_grey5[3];
extern GLfloat SubGrid_grey1[3];
extern GLfloat SubGrid_grey2[3];


//mtm icon matrix colors
extern GLfloat Iconcolor0[3];
extern GLfloat Iconcolor1[3];
extern GLfloat Iconcolor2[3];
extern GLfloat Iconcolor3[3];
extern GLfloat Iconcolor4[3];
extern GLfloat Iconcolor5[3];
extern GLfloat Iconcolor6[3];
extern GLfloat Iconcolor7[3];
extern GLfloat Iconcolor8[3];
extern GLfloat Iconcolor9[3];
extern GLfloat Iconcolor10[3];
extern GLfloat Iconcolor11[3];
extern GLfloat Iconcolor12[3];
extern GLfloat Iconcolor13[3];
extern GLfloat Iconcolor14[3];
extern GLfloat Iconcolor15[3];


//Larry BG colors
extern GLfloat GridBg[3];
extern GLfloat YellowFg[3];

extern void MGLSetAlertsColor(struct InstanceData* id, MGLGRID *AlertsColorGrid, char *xcolname, int vorow);

extern int redx_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);


extern int title_airport_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int title_layout_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int title_view_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int up_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int down_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int left_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int right_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int zoom_in_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int zoom_out_cb(struct InstanceData* id, widgets *w, int x, int y, struct guistruct* g);

extern int MGLNextVisCol(MGLGRID *topgrid, int vorow);
extern char *CnvElapsedSecs2HHMMSS(struct InstanceData* id, char *valstr);
extern char *Cnv2YN(struct InstanceData* id, char *valstr);

extern void UniqueFilterCB(struct InstanceData* id, MGLGRID *grid, int c, char *colname);
extern int MGLHexColortoI(char *hexcolorstr);
extern void CreateSortPanel(struct InstanceData* id, MGLGRID *parentgrid);
extern void FillAvailableColsVO(struct InstanceData* id, MGLGRID *grid);
extern MGLGRID *ActiveTextGrid;
extern void MGLSetTitle(MGLGRID *grid, char *appname);
extern int MGLColorHex2Int(char *hexcolorstr);
extern void CalcColWidths(struct InstanceData* id, MGLGRID *grid);
extern void CalcColWidths_condition(struct InstanceData* id, MGLGRID *grid);
extern void MGLDrawPanelImage(struct InstanceData* id, widgets *w);
extern void MGLDrawText(struct InstanceData* id, widgets *w);
extern widgets *MGLAddButton(struct InstanceData* id, widgets *w, char *widgetname, char *activepng, char *inactivepng, widgets *lastw, int x1, int x2, int y1, int y2,
	int alpha_mode, int img_blend, int filtertype);

extern VO *vo_dupnew(VO *src_vo);

extern gui *CreateGUI(struct InstanceData* id);
extern void CrFilteredGrid(struct InstanceData* id, MGLGRID *parentgrid);
extern void MGLDrawGUI(struct InstanceData* id);










//////////////////////////////regex stuff




struct slre_cap {
	const char *ptr;
	int len;
};


int slre_match(const char *regexp, const char *buf, int buf_len,
struct slre_cap *caps, int num_caps, int flags);

/* Possible flags for mre_match() */
enum { SLRE_IGNORE_CASE = 1 };


/* slre_match() failure codes */
#define SLRE_NO_MATCH               -1
#define SLRE_UNEXPECTED_QUANTIFIER  -2
#define SLRE_UNBALANCED_BRACKETS    -3
#define SLRE_INTERNAL_ERROR         -4
#define SLRE_INVALID_CHARACTER_SET  -5
#define SLRE_INVALID_METACHARACTER  -6
#define SLRE_CAPS_ARRAY_TOO_SMALL   -7
#define SLRE_TOO_MANY_BRANCHES      -8
#define SLRE_TOO_MANY_BRACKETS      -9























//////////////////////////////////////////////
#endif  /* _MGL_H__ */

