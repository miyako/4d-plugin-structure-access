/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.c
 #	source generated by 4D Plugin Wizard
 #	Project : Structure Access
 #	author : miyako
 #	2014/06/23
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(int32_t selector, PA_PluginParameters params)
{
	try
	{
		int32_t pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (int32_t pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
// --- Structure Access

		case 1 :
			CREATE_ELEMENTS_FROM_XML(pResult, pParams);
			break;

	}
}

// ------------------------------- Structure Access -------------------------------


void CREATE_ELEMENTS_FROM_XML(sLONG_PTR *pResult, PackagePtr pParams)
{	
	PA_CreateElementsFromXMLDefinition((PA_Unistring *)(pParams[0]));
}
