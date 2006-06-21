#include "sg_SkinClass.h" 


SkinClass::SkinClass() 
{ 
}
SkinClass* SkinClass::Instance() 
{ 
	static SkinClass inst;
    return &inst;
}
wxColour SkinClass::GetColorFromStr(wxString col){
	// create color
	int delPos = col.Find(":");
	wxString rcol = col.Mid(0,delPos);
	col = col.Mid(delPos+1, col.Length() - delPos);
	delPos = col.Find(":");
    wxString gcol = col.Mid(0,delPos);
	wxString bcol = col.Mid(delPos+1, col.Length()- delPos);

	unsigned char r_ch = atoi(rcol.c_str());
	unsigned char g_ch = atoi(gcol.c_str());
	unsigned char b_ch = atoi(bcol.c_str());

    return wxColour(r_ch,g_ch,b_ch);
}
