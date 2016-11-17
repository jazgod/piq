#ifndef _GL_UTIL_H
#define _GL_UTIL_H

#ifdef __cplusplus 
extern "C"{
#endif //__cplusplus

#ifdef _DEBUG
extern const char gl_errors[6][30];
#define GL_ASSERT(a)\
	a;{GLint e = GL_NO_ERROR; while(GL_NO_ERROR != (e = glGetError()))	vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)[%d] OpenGL %s error <0x%x:%s>\n", GetCurrentThreadId(), __LINE__, #a, e, gl_errors[e - GL_INVALID_ENUM]); }
#define GL_ASSERT_WITH_LOGSTATE(a,b,c)\
	a;{GLint e = GL_NO_ERROR; while(GL_NO_ERROR != (e = glGetError())){\
		GLint m = 0, d = 0; glGetIntegerv(b,&m); glGetIntegerv(c,&d);\
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)[%d] OpenGL %s error <0x%x:%s>, %s<%d>, %s<%d>\n", GetCurrentThreadId(), __LINE__, #a, e, gl_errors[e - GL_INVALID_ENUM], #b, m, #c, d); \
}}
#define GL_LOGSTATE(a)\
{GLint s = 0; glGetIntegerv(a,&s); vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)[%d] OpenGL %s<%d>\n", GetCurrentThreadId(), __LINE__, #a, s);}
#else
#define GL_ASSERT(a)\
	a;
#define GL_ASSERT_WITH_LOGSTATE(a,b,c)\
	a;
#endif//_DEBUG

#ifdef __cplusplus 
}
#endif//__cplusplus

#endif//_GL_UTIL_H