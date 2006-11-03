// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewResources.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "ViewResources.h"
#include "Events.h"
#include <wx/arrimpl.cpp>
#include "res/usage.xpm"

WX_DEFINE_OBJARRAY(wxArrayColour);

IMPLEMENT_DYNAMIC_CLASS(CViewResources, CBOINCBaseView)

BEGIN_EVENT_TABLE (CViewResources, CBOINCBaseView)
    EVT_LIST_ITEM_SELECTED(ID_LIST_RESOURCEUTILIZATIONVIEW, CViewResources::OnListSelected)
    EVT_LIST_ITEM_DESELECTED(ID_LIST_RESOURCEUTILIZATIONVIEW, CViewResources::OnListDeselected)
END_EVENT_TABLE ()


CViewResources::CViewResources()
{}


CViewResources::CViewResources(wxNotebook* pNotebook) :  
	CBOINCBaseView(pNotebook)
{
	//add 14 colors for boinc projects (anyone who have more projects attached ?)
	// m_aProjectColours.Add(wxColour(0,139,69));
	m_aProjectColours.Add(wxColour(135,206,235));
	m_aProjectColours.Add(wxColour(65,105,225));
	m_aProjectColours.Add(wxColour(255,165,0));
	m_aProjectColours.Add(wxColour(238,130,238));
	m_aProjectColours.Add(wxColour(205,197,191));
	m_aProjectColours.Add(wxColour(255,127,80));
	m_aProjectColours.Add(wxColour(250,128,114));
	m_aProjectColours.Add(wxColour(0,255,127));
	m_aProjectColours.Add(wxColour(205,79,57));
	m_aProjectColours.Add(wxColour(143,188,143));
	m_aProjectColours.Add(wxColour(153,50,204));
	m_aProjectColours.Add(wxColour(30,144,255));
	m_aProjectColours.Add(wxColour(0,100,0));
	m_aProjectColours.Add(wxColour(127,255,0));
	m_aProjectColours.Add(wxColour(205,173,0));
	m_aProjectColours.Add(wxColour(140,34,34));
	m_aProjectColours.Add(wxColour(152,245,255));
	m_aProjectColours.Add(wxColour(250,240,230));
	m_aProjectColours.Add(wxColour(144,238,144));
	m_aProjectColours.Add(wxColour(255,105,180));

	wxFlexGridSizer* itemFlexGridSizer = new wxFlexGridSizer(3, 0, 0);
    wxASSERT(itemFlexGridSizer);

	// one row
    itemFlexGridSizer->AddGrowableRow(0);
	// two resizable columns for the pie charts
    itemFlexGridSizer->AddGrowableCol(1);
	itemFlexGridSizer->AddGrowableCol(2);

	//create a default task pane 
    m_pTaskPane = new CBOINCTaskCtrl(this, ID_TASK_RESOURCEUTILIZATIONVIEW, DEFAULT_TASK_FLAGS);
    wxASSERT(m_pTaskPane);

	// create pie chart ctrl for total disk usage
	m_pieCtrlTotal = new wxPieCtrl(this, ID_LIST_RESOURCEUTILIZATIONVIEWTOTAL, wxDefaultPosition, wxSize(-1,-1));
	wxASSERT(m_pieCtrlTotal);
	// setup the legend
	m_pieCtrlTotal->GetLegend()->SetTransparent(true);
	m_pieCtrlTotal->GetLegend()->SetHorBorder(10);
#ifndef __WXMAC__
	m_pieCtrlTotal->GetLegend()->SetWindowStyle(wxSTATIC_BORDER);
#endif
	m_pieCtrlTotal->GetLegend()->SetLabelFont(*wxSWISS_FONT);
	m_pieCtrlTotal->GetLegend()->SetLabelColour(wxColour(0,0,127));
	m_pieCtrlTotal->GetLegend()->SetLabelColour(wxColour(0,0,127));
	//TODO: respect title localization
	m_pieCtrlTotal->GetLegend()->SetTitle(wxT("total disk usage"));
	//set the angle above PI/2 to prevent tilt
	m_pieCtrlTotal->SetAngle(4);	
	//disable 3D drawing
	m_pieCtrlTotal->SetPaint3D(false);
	//disable elliptic drawing 
	m_pieCtrlTotal->SetDrawCircle(true);

	// create pie chart ctrl for BOINC disk usage
	m_pieCtrlBOINC = new wxPieCtrl(this, ID_LIST_RESOURCEUTILIZATIONVIEW, wxDefaultPosition, wxSize(-1,-1));
	wxASSERT(m_pieCtrlBOINC);
	//setup the legend
	m_pieCtrlBOINC->GetLegend()->SetTransparent(true);
	m_pieCtrlBOINC->GetLegend()->SetHorBorder(10);
#ifndef __WXMAC__
	m_pieCtrlBOINC->GetLegend()->SetWindowStyle(wxSTATIC_BORDER);
#endif
	m_pieCtrlTotal->GetLegend()->SetLabelFont(*wxSWISS_FONT);
	m_pieCtrlBOINC->GetLegend()->SetLabelFont(*wxSWISS_FONT);
	m_pieCtrlBOINC->GetLegend()->SetLabelColour(wxColour(0,0,127));
	m_pieCtrlBOINC->GetLegend()->SetLabelColour(wxColour(0,0,127));
	m_pieCtrlBOINC->GetLegend()->SetTitle(wxT("disk usage by BOINC projects"));
	m_pieCtrlBOINC->SetAngle(4);
	m_pieCtrlBOINC->SetPaint3D(false);
	m_pieCtrlBOINC->SetDrawCircle(true);	

	//init the flexGrid
    itemFlexGridSizer->Add(m_pTaskPane, 1, wxGROW|wxALL, 1);
	itemFlexGridSizer->Add(m_pieCtrlTotal, 1, wxGROW|wxALL, 1);
    itemFlexGridSizer->Add(m_pieCtrlBOINC, 1, wxGROW|wxALL, 1);	
	//force same size for both piectrls
	itemFlexGridSizer->SetFlexibleDirection(wxVERTICAL);
	itemFlexGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    SetSizer(itemFlexGridSizer);

    Layout();

	m_pTaskPane->UpdateControls();

    UpdateSelection();
}


CViewResources::~CViewResources() {
    EmptyTasks();
}


wxString& CViewResources::GetViewName() {
    static wxString strViewName(_("Disk"));
    return strViewName;
}


const char** CViewResources::GetViewIcon() {
    return usage_xpm;
}

void CViewResources::UpdateSelection() {
	//TODO: is this needed ? no list ctrl at this view
    CBOINCBaseView::PreUpdateSelection();
    CBOINCBaseView::PostUpdateSelection();
}


wxInt32 CViewResources::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    CMainDocument* doc = wxGetApp().GetDocument();
    PROJECT* project = doc->DiskUsageProject(item);
    PROJECT* state_project = NULL;
    std::string project_name;

    wxASSERT(doc);
    wxASSERT(wxDynamicCast(doc, CMainDocument));

    if (project) {
        state_project = doc->state.lookup_project(project->master_url);
        if (state_project) {
            state_project->get_name(project_name);
            strBuffer = wxString(project_name.c_str(), wxConvUTF8);
        }
    }

    return 0;
}


bool CViewResources::OnSaveState(wxConfigBase* pConfig) {
    bool bReturnValue = true;

    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnSaveState(pConfig)) {
        bReturnValue = false;
    }

    return bReturnValue;
}

bool CViewResources::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pTaskPane);

    if (!m_pTaskPane->OnRestoreState(pConfig)) {
        return false;
    }

    return true;
}

void CViewResources::OnListRender( wxTimerEvent& WXUNUSED(event) ) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString diskspace;
	double boinctotal=0.0;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

	//clear former data
	m_pieCtrlBOINC->m_Series.Clear();
	m_pieCtrlTotal->m_Series.Clear();

	//get data for BOINC projects disk usage
    pDoc->CachedDiskUsageUpdate();
    pDoc->CachedStateUpdate();
	if (disk_usage.projects.size()>0) {
        for (i=0; i<disk_usage.projects.size(); i++) {
            //update data for boinc projects pie chart 
			PROJECT* project = pDoc->DiskUsageProject(count);
			wxString projectname;			
			FormatProjectName(project, projectname);
			FormatDiskSpace(project, diskspace);
			double usage = project->disk_usage;
            boinctotal += usage;
			wxPiePart part;
			part.SetLabel(projectname + wxT(" - ") + diskspace);
			part.SetValue(usage);
			part.SetColour(m_aProjectColours[count>MAX_PROJECTCOLORINDEX ? count % MAX_PROJECTCOLORINDEX : count]);
			m_pieCtrlBOINC->m_Series.Add(part);
		}
		m_pieCtrlBOINC->Refresh();
	} else {
		//paint an empty black pie
		wxPiePart part;
		part.SetLabel(_("not attached to any BOINC project - 0 bytes"));
		part.SetValue(boinctotal);
		part.SetColour(wxColour(0,0,0));
		m_pieCtrlBOINC->m_Series.Add(part);		
		m_pieCtrlBOINC->Refresh();
	}
	//data for pie chart 2 (total disk usage)
	wxPiePart part;		
	double free = pDoc->host.d_free;
	double total = pDoc->host.d_total;			
	//free disk space
	FormatDiskSpace2(free,diskspace);		
	part.SetLabel(_("free disk space - ") + diskspace);
	part.SetValue(free);
	part.SetColour(m_aProjectColours[4]);
	m_pieCtrlTotal->m_Series.Add(part);
	//used by boinc projects
	FormatDiskSpace2(boinctotal,diskspace);		
	part.SetLabel(_("used by BOINC projects - ") + diskspace);
	part.SetValue(boinctotal);
	part.SetColour(m_aProjectColours[2]);
	m_pieCtrlTotal->m_Series.Add(part);
	//used by others
	FormatDiskSpace2(total-boinctotal-free,diskspace);
	part.SetLabel(_("used by others - ") + diskspace);
	part.SetValue(total-boinctotal-free);
	part.SetColour(m_aProjectColours[3]);
	m_pieCtrlTotal->m_Series.Add(part);
	m_pieCtrlTotal->Refresh();
}

wxInt32 CViewResources::FormatDiskSpace(wxInt32 item, wxString& strBuffer) const {
	double fBuffer = 0.0;
	PROJECT* resource = wxGetApp().GetDocument()->resource(item);
    if (resource) {
        fBuffer = resource->disk_usage;
    }
	return FormatDiskSpace2(fBuffer,strBuffer);
}

wxInt32 CViewResources::FormatDiskSpace2(double bytes, wxString& strBuffer) const {
    float          fBuffer = bytes;
    double         xTera = 1099511627776.0;
    double         xGiga = 1073741824.0;
    double         xMega = 1048576.0;
    double         xKilo = 1024.0;

    if (fBuffer >= xTera) {
        strBuffer.Printf(wxT("%0.2f TB"), fBuffer/xTera);
    } else if (fBuffer >= xGiga) {
        strBuffer.Printf(wxT("%0.2f GB"), fBuffer/xGiga);
    } else if (fBuffer >= xMega) {
        strBuffer.Printf(wxT("%0.2f MB"), fBuffer/xMega);
    } else if (fBuffer >= xKilo) {
        strBuffer.Printf(wxT("%0.2f KB"), fBuffer/xKilo);
    } else {
        strBuffer.Printf(wxT("%0.0f bytes"), fBuffer);
    }

    return 0;
}

const char *BOINC_RCSID_5a37b46a6e = "$Id$";
