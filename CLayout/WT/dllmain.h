// dllmain.h : Declaration of module class.

class CSMModule : public CAtlDllModuleT< CSMModule >
{
public :
	DECLARE_LIBID(LIBID_SMLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SM, "{D42093A1-23F5-4D10-9DFB-3E72C0F45677}")
};

extern class CSMModule _AtlModule;
