

#define MILES_PER_KM  0.6213712
#define NMILES_PER_KM 0.539957
#define MILES_PER_NMILE 1.15078

#define FILE_HDR_SIZE 30
#define HDR_SIZE 150
#define RAS_HDR_SIZE 8
#define MAX_RADIALS  512
#define MAX_BINS 512
#define N_COLORS 16

/* WX_IMAGE_NMILES is the size of the image which = 2*radius of radar = 124NM * 2 */
#define WX_IMAGE_NMILES 248
#define WX_IMAGE_SIZE  128


extern int WxAlpha;
extern GLfloat WxIntensity;
extern int NActiveRadars;

extern time_t LastGoodWxSecs;

extern int drawWx( struct InstanceData* id, int wx );




