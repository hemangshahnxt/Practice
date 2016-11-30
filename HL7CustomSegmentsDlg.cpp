//TES 8/29/2011 - PLID 44207 - Created
// HL7CustomSegmentsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7CustomSegmentsDlg.h"
#include <GlobalParamUtils.h>
#include <HL7ParseUtils.h>
#include <HL7SettingsCache.h>
#include "HL7Utils.h"

// CHL7CustomSegmentsDlg dialog

IMPLEMENT_DYNAMIC(CHL7CustomSegmentsDlg, CNxDialog)

CHL7CustomSegmentsDlg::CHL7CustomSegmentsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7CustomSegmentsDlg::IDD, pParent)
{
	m_bCustomSegmentsLoaded = false;
	m_bCurrentSegmentsChanged = false;
	m_hcssCurrentScope = hcssMessage;
}

CHL7CustomSegmentsDlg::~CHL7CustomSegmentsDlg()
{
}

void CHL7CustomSegmentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_SCOPE_MESSAGE, m_nxbScopeMessage);
	DDX_Control(pDX, IDC_SCOPE_SPECIMEN, m_nxbScopeSpecimen);
	DDX_Control(pDX, IDC_NEW_SEGMENT, m_nxbNewSegment);
	DDX_Control(pDX, IDC_SEGMENT_NAME, m_nxeSegmentName);
	DDX_Control(pDX, IDC_SEGMENT_UP, m_nxbSegmentUp);
	DDX_Control(pDX, IDC_SEGMENT_DOWN, m_nxbSegmentDown);
	DDX_Control(pDX, IDC_FIELD_VALUE, m_nxeFieldValue);
	DDX_Control(pDX, IDC_FIELD_TEXT, m_nxbFieldText);
	DDX_Control(pDX, IDC_FIELD_DATA, m_nxbFieldData);
	DDX_Control(pDX, IDC_NEW_FIELD, m_nxbNewField);
	DDX_Control(pDX, IDC_FIELD_UP, m_nxbFieldUp);
	DDX_Control(pDX, IDC_FIELD_DOWN, m_nxbFieldDown);
	DDX_Control(pDX, IDC_MAX_FIELD_LENGTH, m_nxeMaxFieldLength);
}


BEGIN_MESSAGE_MAP(CHL7CustomSegmentsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SCOPE_MESSAGE, &CHL7CustomSegmentsDlg::OnScopeMessage)
	ON_BN_CLICKED(IDC_SCOPE_SPECIMEN, &CHL7CustomSegmentsDlg::OnScopeSpecimen)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_NEW_SEGMENT, &CHL7CustomSegmentsDlg::OnNewSegment)
	ON_BN_CLICKED(IDC_SEGMENT_UP, &CHL7CustomSegmentsDlg::OnSegmentUp)
	ON_BN_CLICKED(IDC_SEGMENT_DOWN, &CHL7CustomSegmentsDlg::OnSegmentDown)
	ON_BN_CLICKED(IDC_NEW_FIELD, &CHL7CustomSegmentsDlg::OnNewField)
	ON_BN_CLICKED(IDC_FIELD_UP, &CHL7CustomSegmentsDlg::OnFieldUp)
	ON_BN_CLICKED(IDC_FIELD_DOWN, &CHL7CustomSegmentsDlg::OnFieldDown)
	ON_BN_CLICKED(IDC_FIELD_TEXT, &CHL7CustomSegmentsDlg::OnFieldText)
	ON_BN_CLICKED(IDC_FIELD_DATA, &CHL7CustomSegmentsDlg::OnFieldData)
	ON_EN_CHANGE(IDC_FIELD_VALUE, &CHL7CustomSegmentsDlg::OnEnChangeFieldValue)
	ON_EN_CHANGE(IDC_SEGMENT_NAME, &CHL7CustomSegmentsDlg::OnEnChangeSegmentName)
	ON_EN_CHANGE(IDC_MAX_FIELD_LENGTH, &CHL7CustomSegmentsDlg::OnEnChangeMaxFieldLength)
END_MESSAGE_MAP()

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CHL7CustomSegmentsDlg message handlers
BOOL CHL7CustomSegmentsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//TES 8/29/2011 - PLID 44207 - NxIconify
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);
		m_nxbNewSegment.AutoSet(NXB_NEW);
		m_nxbSegmentUp.AutoSet(NXB_UP);
		m_nxbSegmentDown.AutoSet(NXB_DOWN);
		m_nxbNewField.AutoSet(NXB_NEW);
		m_nxbFieldUp.AutoSet(NXB_UP);
		m_nxbFieldDown.AutoSet(NXB_DOWN);
		m_nxeSegmentName.SetLimitText(3);
		m_nxeFieldValue.SetLimitText(2000);
		m_pSegmentList = BindNxDataList2Ctrl(IDC_HL7_SEGMENTS_LIST, false);
		m_pSegmentFields = BindNxDataList2Ctrl(IDC_SEGMENT_FIELDS, false);
		//TES 9/8/2011 - PLID 45248 - Fill the list of database fields that can be output
		m_pDataFieldList = BindNxDataList2Ctrl(IDC_DATA_FIELD_LIST, false);
		m_pDataFieldList->FromClause = _bstr_t(GetFieldListQuery());
		m_pDataFieldList->Requery();
		//TES 9/12/2011 - PLID 45405 - Added lists of database fields that can be used as criteria for whether to output other fields.
		m_pSegmentCriteriaField = BindNxDataList2Ctrl(IDC_SEGMENT_CRITERIA_FIELD, false);
		m_pSegmentCriteriaField->FromClause = _bstr_t(GetFieldListQuery());
		m_pSegmentCriteriaField->Requery();
		m_pFieldCriteriaField = BindNxDataList2Ctrl(IDC_FIELD_CRITERIA_FIELD, false);
		m_pFieldCriteriaField->FromClause = _bstr_t(GetFieldListQuery());
		m_pFieldCriteriaField->Requery();

		//TES 8/29/2011 - PLID 44207 - Default to per-message scope
		CheckRadioButton(IDC_SCOPE_MESSAGE, IDC_SCOPE_SPECIMEN, IDC_SCOPE_MESSAGE);
		OnScopeMessage();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
enum SegmentListColumns
{
	slcID = 0,
	slcIsCustom = 1,
	slcName = 2,
	slcPointer = 3,
};

enum SegmentFieldColumns
{
	sfcDisplayName = 0, //TES 9/8/2011 - PLID 45248
	sfcTextValue = 1,
	sfcFieldType = 2, //TES 9/8/2011 - PLID 45248
	sfcFieldID = 3, //TES 9/8/2011 - PLID 45248
	sfcCriteriaFieldID = 4, //TES 9/12/2011 - PLID 45405
	sfcTextLimit = 5, //TES 9/12/2011 - PLID 45405
};

//TES 9/8/2011 - PLID 45248
enum DataFieldListColumns
{
	dflcID = 0,
	dflcName = 1,
};

void CHL7CustomSegmentsDlg::AddSegments(HL7Segment hs, HL7CustomSegmentScope hcss)
{
	//TES 8/29/2011 - PLID 44207 - There's a special "scope beginning" segment which isn't a real segment, but is used to track custom
	// segments that are meant to show up before any non-custom segments.  Don't show it in the list, but if we were given any other segment,
	// add a row for it.
	if(hs == hsScopeBeginning) {
		ASSERT(m_pSegmentList->GetRowCount() == 0);
	}
	else {
		IRowSettingsPtr pRow = m_pSegmentList->GetNewRow();
		pRow->PutValue(slcID, (long)hs);
		pRow->PutValue(slcIsCustom, g_cvarFalse);
		pRow->PutValue(slcName, _bstr_t(GetSegmentName(hs)));
		pRow->PutValue(slcPointer, g_cvarNull);
		pRow->PutBackColor(RGB(200,200,200));
		m_pSegmentList->AddRowAtEnd(pRow, NULL);
	}
	
	//TES 8/29/2011 - PLID 44207 - Now, any associated custom segments
	CArray<CustomSegment*,CustomSegment*> *parSegments = NULL;
	m_mapCustomSegments.Lookup(hcss, parSegments);
	if(parSegments) {
		for(int i = 0; i < parSegments->GetSize(); i++) {
			//TES 8/30/2011 - PLID 44207 - Get the pointer, and store it in the datalist.
			CustomSegment *pCs = parSegments->GetAt(i);
			if(pCs->hsOffsetFrom == hs) {
				IRowSettingsPtr pRow = m_pSegmentList->GetNewRow();
				pRow->PutValue(slcID, pCs->nID);
				pRow->PutValue(slcIsCustom, g_cvarTrue);
				pRow->PutValue(slcName, _bstr_t(pCs->strName));
				pRow->PutValue(slcPointer, (long)pCs);
				m_pSegmentList->AddRowAtEnd(pRow, NULL);
			}
		}
	}
}
		
void CHL7CustomSegmentsDlg::OnScopeMessage()
{
	try {
		//TES 8/29/2011 - PLID 44207 - Store any current information back to our memory variable.
		StoreCurrentSegments();

		//TES 8/29/2011 - PLID 44207 - Make sure we've got our list of segments.
		EnsureCustomSegments();

		//TES 8/29/2011 - PLID 44207 - Now clear out the list, and add each built-in segment, followed by each associated custom segment.
		m_pSegmentList->Clear();
		
		//TES 8/29/2011 - PLID 44207 - This is all valid for lab orders, which is the only currently supported message.
		ASSERT(m_MessageType == hmtLabOrder);

		//MSH (always first)
		AddSegments(hsMSH, hcssMessage);
		//PID
		AddSegments(hsPID, hcssMessage);
		//PV1
		AddSegments(hsPV1, hcssMessage);
		//IN1
		AddSegments(hsIN1, hcssMessage);
		//GT1
		AddSegments(hsGT1, hcssMessage);

		//TES 8/29/2011 - PLID 44207 - Make sure our controls are up to date
		ReflectCurrentSegment();

		//TES 8/29/2011 - PLID 44207 - We're now showing per-message scope
		m_hcssCurrentScope = hcssMessage;

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnScopeSpecimen()
{
	try {
		CHL7SettingsCache *pCache = GetHL7SettingsCache();

		//TES 8/29/2011 - PLID 44207 - Store any current information back to our memory variable.
		StoreCurrentSegments();

		//TES 8/29/2011 - PLID 44207 - Make sure we've got our list of segments.
		EnsureCustomSegments();

		//TES 8/29/2011 - PLID 44207 - Now clear out the list, and add each built-in segment, followed by each associated custom segment.
		m_pSegmentList->Clear();

		//TES 8/29/2011 - PLID 44207 - This is all valid for lab orders, which is the only currently supported message.
		ASSERT(m_MessageType == hmtLabOrder);
		//First, anything that comes before the first segment in this scope.
		AddSegments(hsScopeBeginning, hcssSpecimen);
		//ORC
		AddSegments(hsORC, hcssSpecimen);
		//OBR
		AddSegments(hsOBR, hcssSpecimen);
		if(pCache->GetSettingBit(m_nHL7GroupID, "SendNTEBeforeDG1") == TRUE){
			// (r.gonet 02/28/2012) - PLID 48044 - This is correct according to the HL7 standard.
			//NTE
			AddSegments(hsNTE, hcssSpecimen);
			//DG1
			AddSegments(hsDG1, hcssSpecimen);
		} else {
			// (r.gonet 02/28/2012) - PLID 48044 - This is old legacy way which is wrong
			//DG1
			AddSegments(hsDG1, hcssSpecimen);
			//NTE
			AddSegments(hsNTE, hcssSpecimen);		
		}

		//TES 8/29/2011 - PLID 44207 - Make sure our controls are up to date
		ReflectCurrentSegment();

		//TES 8/29/2011 - PLID 44207 - We're now showing per-specimen scope
		m_hcssCurrentScope = hcssSpecimen;

	}NxCatchAll(__FUNCTION__);
}


void CHL7CustomSegmentsDlg::EnsureCustomSegments()
{
	//TES 8/29/2011 - PLID 44207 - Have we already loaded them?
	if(m_bCustomSegmentsLoaded) {
		return;
	}
	//TES 8/29/2011 - PLID 44207 - OK, pull all the segments from data that are for our HL7 Settings group and message type
	//TES 8/30/2011 - PLID 44207 - Also pull all the fields for each segment.
	//TES 9/8/2011 - PLID 45248 - Added FieldType and DataFieldID
	//TES 9/12/2011 - PLID 45405 - Added SegmentCriteriaField, FieldCriteriaField, and TextLimit
	_RecordsetPtr rsCustomSegments = CreateParamRecordset("SELECT HL7CustomSegmentsT.ID, HL7CustomSegmentsT.Scope, HL7CustomSegmentsT.SegmentName, "
		"HL7CustomSegmentsT.OffsetFromSegment, HL7CustomSegmentsT.OffsetAmount, HL7CustomSegmentFieldsT.FieldValue, "
		"HL7CustomSegmentFieldsT.FieldType, HL7CustomSegmentFieldsT.DataFieldID, "
		"HL7CustomSegmentsT.CriteriaFieldID AS SegmentCriteriaField, HL7CustomSegmentFieldsT.CriteriaFieldID AS FieldCriteriaField, "
		"HL7CustomSegmentFieldsT.TextLimit "
		"FROM HL7CustomSegmentsT LEFT JOIN HL7CustomSegmentFieldsT ON HL7CustomSegmentsT.ID = HL7CustomSegmentFieldsT.HL7CustomSegmentID "
		"WHERE HL7CustomSegmentsT.HL7GroupID = {INT} AND HL7CustomSegmentsT.MessageType = {INT} "
		"ORDER BY HL7CustomSegmentsT.OffsetFromSegment, HL7CustomSegmentsT.OffsetAmount, HL7CustomSegmentFieldsT.OrderIndex ", 
		m_nHL7GroupID, m_MessageType);
	long nCurrentSegmentID = -1;
	HL7CustomSegmentScope hcssCurrent;
	CustomSegment *pcsCurrent = NULL;
	while(!rsCustomSegments->eof) {
		long nSegmentID = AdoFldLong(rsCustomSegments, "ID");
		HL7CustomSegmentScope scope = (HL7CustomSegmentScope)AdoFldLong(rsCustomSegments,"Scope");
		//TES 8/30/2011 - PLID 44207 - Are we on a new segment?
		if(nSegmentID != nCurrentSegmentID) {
			//TES 8/30/2011 - PLID 44207 - If we had an existing segment, commit it to the map.
			if(nCurrentSegmentID != -1) {
				//TES 8/29/2011 - PLID 44207 - See if there's already an array in our map
				CArray<CustomSegment*,CustomSegment*> *parSegments = NULL;
				m_mapCustomSegments.Lookup(hcssCurrent,parSegments);
				if(parSegments == NULL) {
					//TES 8/29/2011 - PLID 44207 - There wasn't, so initialize an array for this scope.
					parSegments = new CArray<CustomSegment*,CustomSegment*>;
				}
				//TES 8/29/2011 - PLID 44207 - Add our struct to the array, and update the map
				parSegments->Add(pcsCurrent);
				m_mapCustomSegments.SetAt(hcssCurrent, parSegments);
			}
			nCurrentSegmentID = nSegmentID;
			hcssCurrent = scope;
			//TES 8/29/2011 - PLID 44207 - Create the struct.
			pcsCurrent = new CustomSegment;
			pcsCurrent->nID = nSegmentID;
			pcsCurrent->strName = AdoFldString(rsCustomSegments, "SegmentName");
			pcsCurrent->hsOffsetFrom = (HL7Segment)AdoFldLong(rsCustomSegments, "OffsetFromSegment");
			pcsCurrent->nOffsetBy = AdoFldLong(rsCustomSegments, "OffsetAmount");
			//TES 9/12/2011 - PLID 45405 - Added CriteriaFieldID
			pcsCurrent->nCriteriaFieldID = AdoFldLong(rsCustomSegments, "SegmentCriteriaField", 0);
			pcsCurrent->parFields = new CArray<CustomSegmentField,CustomSegmentField&>;
		}
		//TES 8/30/2011 - PLID 44207 - Now add this field to the current object.
		//TES 9/8/2011 - PLID 45248 - This is an object now, not just a string
		CustomSegmentField csf;
		csf.type = (CustomSegmentFieldType)AdoFldLong(rsCustomSegments, "FieldType");
		csf.nFieldID = AdoFldLong(rsCustomSegments, "DataFieldID", 0);
		csf.strValue = AdoFldString(rsCustomSegments, "FieldValue", pcsCurrent->strName + "|");
		//TES 9/12/2011 - PLID 45405 - Added CriteriaFieldID and TextLimit
		csf.nCriteriaFieldID = AdoFldLong(rsCustomSegments, "FieldCriteriaField", 0);
		csf.nTextLimit = AdoFldLong(rsCustomSegments, "TextLimit", -1);
		pcsCurrent->parFields->Add(csf);
		
		
		rsCustomSegments->MoveNext();
	}
	//TES 8/30/2011 - PLID 44207 - Now commit the last segment we were filling (if any) to the map.
	if(nCurrentSegmentID != -1) {
		//TES 8/29/2011 - PLID 44207 - See if there's already an array in our map
		CArray<CustomSegment*,CustomSegment*> *parSegments = NULL;
		m_mapCustomSegments.Lookup(hcssCurrent,parSegments);
		if(parSegments == NULL) {
			//TES 8/29/2011 - PLID 44207 - There wasn't, so initialize an array for this scope.
			parSegments = new CArray<CustomSegment*,CustomSegment*>;
		}
		//TES 8/29/2011 - PLID 44207 - Add our struct to the array, and update the map
		parSegments->Add(pcsCurrent);
		m_mapCustomSegments.SetAt(hcssCurrent, parSegments);
	}

	m_bCustomSegmentsLoaded = true;
}

void CHL7CustomSegmentsDlg::OnDestroy()
{
	try {
		//TES 8/29/2011 - PLID 44207 - Clean up any memory used by our map
		POSITION pos = m_mapCustomSegments.GetStartPosition();
		HL7CustomSegmentScope scope;
		CArray<CustomSegment*,CustomSegment*> *parCustomSegments;
		while (pos != NULL) {
			m_mapCustomSegments.GetNextAssoc(pos, scope, parCustomSegments);
			if(parCustomSegments) {
				for(int i = 0; i < parCustomSegments->GetSize(); i++) {
					CustomSegment *pCs = parCustomSegments->GetAt(i);
					if(pCs->parFields) {
						delete pCs->parFields;
					}
					delete pCs;
				}
				delete parCustomSegments;
			}
		}
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

BEGIN_EVENTSINK_MAP(CHL7CustomSegmentsDlg, CNxDialog)
ON_EVENT(CHL7CustomSegmentsDlg, IDC_HL7_SEGMENTS_LIST, 2, CHL7CustomSegmentsDlg::OnSelChangedHl7SegmentsList, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CHL7CustomSegmentsDlg, IDC_HL7_SEGMENTS_LIST, 7, CHL7CustomSegmentsDlg::OnRButtonUpHl7SegmentsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CHL7CustomSegmentsDlg, IDC_SEGMENT_FIELDS, 2, CHL7CustomSegmentsDlg::OnSelChangedSegmentFields, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CHL7CustomSegmentsDlg, IDC_SEGMENT_FIELDS, 7, CHL7CustomSegmentsDlg::OnRButtonUpSegmentFields, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CHL7CustomSegmentsDlg, IDC_DATA_FIELD_LIST, 16, CHL7CustomSegmentsDlg::OnSelChosenDataFieldList, VTS_DISPATCH)
ON_EVENT(CHL7CustomSegmentsDlg, IDC_SEGMENT_CRITERIA_FIELD, 16, CHL7CustomSegmentsDlg::OnSelChosenSegmentCriteriaField, VTS_DISPATCH)
ON_EVENT(CHL7CustomSegmentsDlg, IDC_FIELD_CRITERIA_FIELD, 16, CHL7CustomSegmentsDlg::OnSelChosenFieldCriteriaField, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CHL7CustomSegmentsDlg::OnSelChangedHl7SegmentsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//TES 8/29/2011 - PLID 44207 - Update the screen.
		ReflectCurrentSegment();
	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::ReflectCurrentSegment()
{
	//TES 8/29/2011 - PLID 44207 - Do we have a row?
	IRowSettingsPtr pRow = m_pSegmentList->CurSel;
	if(pRow == NULL) {
		//TES 8/29/2011 - PLID 44207 - Disable everything
		SetDlgItemText(IDC_SEGMENT_NAME, "");
		m_nxeSegmentName.SetReadOnly(TRUE);
		m_nxbSegmentUp.EnableWindow(FALSE);
		m_nxbSegmentDown.EnableWindow(FALSE);

		//TES 9/12/2011 - PLID 45405 - Added Criteria Field
		m_pSegmentCriteriaField->CurSel = NULL;
		GetDlgItem(IDC_SEGMENT_CRITERIA_FIELD)->EnableWindow(FALSE);
		
		//TES 8/30/2011 - PLID 44207 - Disable the field controls.
		GetDlgItem(IDC_SEGMENT_FIELDS)->EnableWindow(FALSE);
		GetDlgItem(IDC_FIELD_VALUE)->EnableWindow(FALSE);
		m_nxbNewField.EnableWindow(FALSE);
		m_nxbFieldUp.EnableWindow(FALSE);
		m_nxbFieldDown.EnableWindow(FALSE);
		//TES 9/12/2011 - PLID 45405 - Added Criteria Field and Text Limit
		m_pFieldCriteriaField->CurSel = NULL;
		GetDlgItem(IDC_FIELD_CRITERIA_FIELD)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_FIELD_LENGTH)->EnableWindow(FALSE);

		m_pSegmentFields->Clear();
		ReflectCurrentField();
	}
	else {
		//TES 8/29/2011 - PLID 44207 - We do, update the name.
		SetDlgItemText(IDC_SEGMENT_NAME, VarString(pRow->GetValue(slcName),""));
		//TES 8/29/2011 - PLID 44207 - Is this a custom item?
		BOOL bIsCustomItem = VarBool(pRow->GetValue(slcIsCustom),FALSE);
		m_nxeSegmentName.SetReadOnly(!bIsCustomItem);
		if(bIsCustomItem) {
			//TES 8/30/2011 - PLID 44207 - Enable the field controls.
			GetDlgItem(IDC_SEGMENT_FIELDS)->EnableWindow(TRUE);
			GetDlgItem(IDC_FIELD_VALUE)->EnableWindow(TRUE);
			m_nxbNewField.EnableWindow(TRUE);

			//TES 8/30/2011 - PLID 44207 - Load the fields from our object, and update the controls appropriately
			CustomSegment *pCs = (CustomSegment*)VarLong(pRow->GetValue(slcPointer));
			
			//TES 9/12/2011 - PLID 45405 - Display the Criteria Field
			if(pCs->nCriteriaFieldID == 0) {
				m_pSegmentCriteriaField->CurSel = NULL;
			}
			else {
				m_pSegmentCriteriaField->SetSelByColumn(dflcID, pCs->nCriteriaFieldID);
			}
			GetDlgItem(IDC_SEGMENT_CRITERIA_FIELD)->EnableWindow(TRUE);
			
			m_pSegmentFields->Clear();
			for(int i = 0; i < pCs->parFields->GetSize(); i++) {
				IRowSettingsPtr pRow = m_pSegmentFields->GetNewRow();
				CustomSegmentField csf = pCs->parFields->GetAt(i);
				pRow->PutValue(sfcTextValue, _bstr_t(csf.strValue));
				pRow->PutValue(sfcDisplayName, _bstr_t(GetDisplayName(csf)));
				//TES 9/8/2011 - PLID 45248 - Added FieldType and FieldID
				pRow->PutValue(sfcFieldType, (long)csf.type);
				pRow->PutValue(sfcFieldID, (csf.nFieldID == 0 ? g_cvarNull : csf.nFieldID));
				//TES 9/12/2011 - PLID 45405 - Added Criteria Field and Text Limit
				pRow->PutValue(sfcCriteriaFieldID, (csf.nCriteriaFieldID == 0 ? g_cvarNull : csf.nCriteriaFieldID));
				pRow->PutValue(sfcTextLimit, (csf.nTextLimit == -1 ? g_cvarNull : csf.nTextLimit));
				m_pSegmentFields->AddRowAtEnd(pRow, NULL);
			}
			m_pSegmentFields->CurSel = m_pSegmentFields->GetFirstRow();
			ReflectCurrentField();

			//TES 8/29/2011 - PLID 44207 - Enable the up/down arrows appropriately.
			if(pRow->GetNextRow()) {
				m_nxbSegmentDown.EnableWindow(TRUE);
			}
			else {
				m_nxbSegmentDown.EnableWindow(FALSE);
			}

			IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
			if(pPrevRow) {
				//TES 8/29/2011 - PLID 44207 - We can't move a custom segment above the MSH segment.
				if(!VarBool(pPrevRow->GetValue(slcIsCustom)) && VarLong(pPrevRow->GetValue(slcID)) == (long)hsMSH) {
					m_nxbSegmentUp.EnableWindow(FALSE);
				}
				else {
					m_nxbSegmentUp.EnableWindow(TRUE);
				}
			}
			else {
				m_nxbSegmentUp.EnableWindow(FALSE);
			}
		}
		else {
			//TES 8/29/2011 - PLID 44207 - We can't move non-custom segments.
			m_nxbSegmentUp.EnableWindow(FALSE);
			m_nxbSegmentDown.EnableWindow(FALSE);

			//TES 9/12/2011 - PLID 45405 - Added Criteria Field
			m_pSegmentCriteriaField->CurSel = NULL;
			GetDlgItem(IDC_SEGMENT_CRITERIA_FIELD)->EnableWindow(FALSE);

			//TES 8/30/2011 - PLID 44207 - Disable the field controls
			GetDlgItem(IDC_SEGMENT_FIELDS)->EnableWindow(FALSE);
			GetDlgItem(IDC_FIELD_VALUE)->EnableWindow(FALSE);
			m_nxbNewField.EnableWindow(FALSE);
			m_nxbFieldUp.EnableWindow(FALSE);
			m_nxbFieldDown.EnableWindow(FALSE);
			//TES 9/12/2011 - PLID 45405 - Added Criteria Field and Text Limit
			m_pFieldCriteriaField->CurSel = NULL;
			GetDlgItem(IDC_FIELD_CRITERIA_FIELD)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAX_FIELD_LENGTH)->EnableWindow(FALSE);

			m_pSegmentFields->Clear();
			ReflectCurrentField();
		}
	}
}

void CHL7CustomSegmentsDlg::OnNewSegment()
{
	try {
		//TES 8/29/2011 - PLID 44207 - Prompt for a name (it can only be 3 characters)
		CString strSegmentName ;
		if(IDOK == InputBoxLimited(this, "Please enter the name of the new Custom Segment.", strSegmentName, "", 3, false, false, "Cancel")) {
			//TES 8/29/2011 - PLID 44207 - Add it to our list, and select it.
			IRowSettingsPtr pNewRow = m_pSegmentList->GetNewRow();
			pNewRow->PutValue(slcID, g_cvarNull);
			pNewRow->PutValue(slcIsCustom, g_cvarTrue);
			pNewRow->PutValue(slcName, _bstr_t(strSegmentName));
			
			//TES 8/30/2011 - PLID 44207 - Create an object, and store the pointer in the datalist.
			CustomSegment *pCs = new CustomSegment;
			pCs->nID = -1;
			IRowSettingsPtr pLastRow = m_pSegmentList->GetLastRow();
			long nOffset = 1;
			while(VarBool(pLastRow->GetValue(slcIsCustom))) {
				nOffset++;
				pLastRow = pLastRow->GetPreviousRow();
			}
			pCs->hsOffsetFrom = (HL7Segment)VarLong(pLastRow->GetValue(slcID));
			pCs->nOffsetBy = nOffset;
			pCs->strName = strSegmentName;
			//TES 9/12/2011 - PLID 45405 - Added Criteria Field
			pCs->nCriteriaFieldID = 0;
			pCs->parFields = new CArray<CustomSegmentField,CustomSegmentField&>;
			//TES 8/30/2011 - PLID 44207 - The first field of any segment (even non-custom ones, for that matter) is the name of the segment, 
			// plus the field separator.
			CustomSegmentField csf;
			//TES 9/8/2011 - PLID 45248 - Added FieldType and FieldID
			csf.type = csftText;
			csf.strValue = strSegmentName + "|";
			csf.nFieldID = 0;
			//TES 9/12/2011 - PLID 45405 - Added Criteria Field and Text Limit
			csf.nCriteriaFieldID = 0;
			csf.nTextLimit = -1;
			pCs->parFields->Add(csf);
			pNewRow->PutValue(slcPointer, (long)pCs);

			m_pSegmentList->AddRowAtEnd(pNewRow, NULL);
			m_pSegmentList->CurSel = pNewRow;
			ReflectCurrentSegment();
			//TES 8/29/2011 - PLID 44207 - Remember that something's changed.
			m_bCurrentSegmentsChanged = true;
		}
			
	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::StoreCurrentSegments()
{
	//TES 8/29/2011 - PLID 44207 - If anything changed, update the map based on what's currently stored in the datalist.
	if(m_bCurrentSegmentsChanged) {
		CArray<CustomSegment*,CustomSegment*> *parSegments = NULL;
		m_mapCustomSegments.Lookup(m_hcssCurrentScope, parSegments);
		if(parSegments == NULL) {
			parSegments = new CArray<CustomSegment*,CustomSegment*>;
		}
		else {
			parSegments->RemoveAll();
		}

		IRowSettingsPtr pRow = m_pSegmentList->GetFirstRow();
		HL7Segment hsCurrent = hsScopeBeginning;
		long nCurrentOffset = 1;
		while(pRow) {
			BOOL bIsCustom = VarBool(pRow->GetValue(slcIsCustom));
			if(bIsCustom) {
				_variant_t varPtr = pRow->GetValue(slcPointer);
				//TES 8/30/2011 - PLID 44207 - Get the pointer from the list, and allocate one if we haven't yet.
				CustomSegment *pCs = NULL;
				if(varPtr.vt == VT_NULL) {
					pCs = new CustomSegment;
				}
				else {
					pCs = (CustomSegment*)VarLong(varPtr);
				}
				pCs->nID = VarLong(pRow->GetValue(slcID),-1);
				pCs->hsOffsetFrom = hsCurrent;
				pCs->nOffsetBy = nCurrentOffset++;
				pCs->strName = VarString(pRow->GetValue(slcName));
				pRow->PutValue(slcPointer, (long)pCs);
				parSegments->Add(pCs);
			}
			else {
				hsCurrent = (HL7Segment)VarLong(pRow->GetValue(slcID));
				nCurrentOffset = 1;
			}
			pRow = pRow->GetNextRow();
		}
		m_mapCustomSegments.SetAt(m_hcssCurrentScope, parSegments);

	}
	m_bCurrentSegmentsChanged = false;
}

void CHL7CustomSegmentsDlg::OnSegmentUp()
{
	try {
		//TES 8/29/2011 - PLID 44207 - Swap with the row above.
		IRowSettingsPtr pRow = m_pSegmentList->CurSel;
		if(pRow && VarBool(pRow->GetValue(slcIsCustom))) {
			IRowSettingsPtr pPrev = pRow->GetPreviousRow();
			if(pPrev && (VarBool(pRow->GetValue(slcIsCustom)) || VarLong(pRow->GetValue(slcID)) != (long)hsMSH)) {
				m_pSegmentList->RemoveRow(pRow);
				m_pSegmentList->AddRowBefore(pRow, pPrev);
				m_pSegmentList->CurSel = pRow;
				ReflectCurrentSegment();
				//TES 8/29/2011 - PLID 44207 - Remember that something's changed
				m_bCurrentSegmentsChanged = true;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnSegmentDown()
{
	try {
		//TES 8/29/2011 - PLID 44207 - Swap with the row below
		IRowSettingsPtr pRow = m_pSegmentList->CurSel;
		if(pRow && VarBool(pRow->GetValue(slcIsCustom))) {
			IRowSettingsPtr pNext = pRow->GetNextRow();
			if(pNext) {
				m_pSegmentList->RemoveRow(pNext);
				m_pSegmentList->AddRowBefore(pNext, pRow);
				m_pSegmentList->CurSel = pRow;
				ReflectCurrentSegment();
				//TES 8/29/2011 - PLID 44207 - Remember that something's changed.
				m_bCurrentSegmentsChanged = true;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnRButtonUpHl7SegmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		//TES 8/29/2011 - PLID 44207 - If they're on a custom segment row, give them an option to remove the row.
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		if(!VarBool(pRow->GetValue(slcIsCustom))) {
			return;
		}

		m_pSegmentList->CurSel = pRow;

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "Remove");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == 1) {
			//TES 8/29/2011 - PLID 44207 - Remove it, and update the screen.
			//TES 9/13/2011 - PLID 44207 - Free the memory
			CustomSegment *pCs = (CustomSegment*)VarLong(pRow->GetValue(slcPointer));
			if(pCs) {
				if(pCs->parFields) {
					delete pCs->parFields;
				}
				delete pCs;
			}
			m_pSegmentList->RemoveRow(pRow);
			ReflectCurrentSegment();
			//TES 8/29/2011 - PLID 44207 - Remember that something's changed.
			m_bCurrentSegmentsChanged = true;
		}

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnOK()
{
	try {
		//TES 8/29/2011 - PLID 44207 - Update our member variable.
		StoreCurrentSegments();

		//TES 8/29/2011 - PLID 44207 - Clear out all the current custom segments, and re-add them.
		CString strSql;
		CNxParamSqlArray aryParams;
		//TES 8/30/2011 - PLID 44207 - Also clear out the fields, and declare a variable that we'll use to tie the tables together.
		AddParamStatementToSqlBatch(strSql, aryParams, "DELETE FROM HL7CustomSegmentFieldsT WHERE HL7CustomSegmentID IN "
			"(SELECT ID FROM HL7CustomSegmentsT WHERE HL7GroupID = {INT} AND MessageType = {INT})", m_nHL7GroupID, m_MessageType);
		AddParamStatementToSqlBatch(strSql, aryParams, "DELETE FROM HL7CustomSegmentsT WHERE HL7GroupID = {INT} AND MessageType = {INT}", m_nHL7GroupID, m_MessageType);
		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @HL7CustomSegmentID INT;");

		//TES 8/29/2011 - PLID 44207 - The valid scopes for lab order messages (which is all we currently support) are per-message and per-specimen.
		ASSERT(m_MessageType == hmtLabOrder);

		SaveCustomSegments(hcssMessage, strSql, aryParams);
		SaveCustomSegments(hcssSpecimen, strSql, aryParams);

		//TES 8/29/2011 - PLID 44207 - Commit
		ExecuteParamSqlBatch(GetRemoteData(), strSql, aryParams);

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::SaveCustomSegments(HL7CustomSegmentScope hcssScope, CString &strSql, CNxParamSqlArray &aryParams)
{
	//TES 8/29/2011 - PLID 44207 - Load the array, and generate queries for each entry.
	CArray<CustomSegment*,CustomSegment*> *parSegments = NULL;
	m_mapCustomSegments.Lookup(hcssScope, parSegments);
	if(parSegments) {
		for(int i = 0; i < parSegments->GetSize(); i++) {
			CustomSegment *pCs = parSegments->GetAt(i);
			//TES 9/12/2011 - PLID 45405 - Added CriteriaFieldID
			_variant_t varCriteriaFieldID = g_cvarNull;
			if(pCs->nCriteriaFieldID != 0) {
				varCriteriaFieldID = pCs->nCriteriaFieldID;
			}
			AddParamStatementToSqlBatch(strSql, aryParams, "INSERT INTO HL7CustomSegmentsT "
				"(HL7GroupID, MessageType, Scope, OffsetFromSegment, OffsetAmount, SegmentName, CriteriaFieldID) "
				"VALUES ({INT}, {INT}, {INT}, {INT}, {INT}, {STRING}, {VT_I4})",
				m_nHL7GroupID, m_MessageType, hcssScope, pCs->hsOffsetFrom, pCs->nOffsetBy, pCs->strName, varCriteriaFieldID);
			//TES 8/30/2011 - PLID 44207 - Remember the segment ID, and go through and add all the fields.
			AddParamStatementToSqlBatch(strSql, aryParams, "SET @HL7CustomSegmentID = (SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID); \r\n");
			for(int j = 0; j < pCs->parFields->GetSize(); j++) {
				CustomSegmentField csf = pCs->parFields->GetAt(j);
				_variant_t varFieldID = g_cvarNull;
				if(csf.nFieldID != 0) {
					varFieldID = csf.nFieldID;
				}
				//TES 9/12/2011 - PLID 45405 - Added CriteriaFieldID and TextLimit
				_variant_t varCriteriaFieldID = g_cvarNull;
				if(csf.nCriteriaFieldID != 0) {
					varCriteriaFieldID = csf.nCriteriaFieldID;
				}
				_variant_t varTextLimit = g_cvarNull;
				if(csf.nTextLimit != -1) {
					varTextLimit = csf.nTextLimit;
				}
				//TES 9/8/2011 - PLID 45248 - Added FieldType and FieldID
				AddParamStatementToSqlBatch(strSql, aryParams, "INSERT INTO HL7CustomSegmentFieldsT "
					"(HL7CustomSegmentID, OrderIndex, FieldValue, FieldType, DataFieldID, CriteriaFieldID, TextLimit) "
					"VALUES (@Hl7CustomSegmentID, {INT}, {STRING}, {INT}, {VT_I4}, {VT_I4}, {VT_I4})",
					j, csf.strValue, csf.type, varFieldID, varCriteriaFieldID, varTextLimit);
			}
		}
	}
}

void CHL7CustomSegmentsDlg::ReflectCurrentField()
{
	//TES 8/30/2011 - PLID 44207 - Check if a field is selected.
	IRowSettingsPtr pRow = m_pSegmentFields->CurSel;
	if(pRow == NULL) {
		//TES 8/30/2011 - PLID 44207 - Nope, clear and disable the field value.
		SetDlgItemText(IDC_FIELD_VALUE, "");
		GetDlgItem(IDC_FIELD_VALUE)->EnableWindow(FALSE);
		m_nxbFieldUp.EnableWindow(FALSE);
		m_nxbFieldDown.EnableWindow(FALSE);
		m_nxbFieldText.EnableWindow(FALSE);
		m_nxbFieldData.EnableWindow(FALSE);
		m_pDataFieldList->CurSel = NULL;
		GetDlgItem(IDC_DATA_FIELD_LIST)->EnableWindow(FALSE);

		//TES 9/12/2011 - PLID 45405 - Added Criteria Field and Text Limit
		m_pFieldCriteriaField->CurSel = NULL;
		GetDlgItem(IDC_FIELD_CRITERIA_FIELD)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_FIELD_LENGTH)->EnableWindow(FALSE);
	}
	else {
		//TES 8/30/2011 - PLID 44207 - There is, fill the field value box.
		GetDlgItem(IDC_FIELD_VALUE)->EnableWindow(TRUE);
		SetDlgItemText(IDC_FIELD_VALUE, VarString(pRow->GetValue(sfcTextValue)));

		//TES 9/12/2011 - PLID 45405 - Display the Criteria Field
		GetDlgItem(IDC_FIELD_CRITERIA_FIELD)->EnableWindow(TRUE);
		m_pFieldCriteriaField->SetSelByColumn(dflcID, pRow->GetValue(sfcCriteriaFieldID));

		//TES 9/12/2011 - PLID 45405 - Display the Text Limit
		_variant_t varTextLimit = pRow->GetValue(sfcTextLimit);
		if(varTextLimit.vt == VT_NULL) {
			SetDlgItemText(IDC_MAX_FIELD_LENGTH, "");
		}
		else {
			SetDlgItemInt(IDC_MAX_FIELD_LENGTH, VarLong(varTextLimit));
		}
		GetDlgItem(IDC_MAX_FIELD_LENGTH)->EnableWindow(TRUE);

		//TES 9/8/2011 - PLID 45248 - Check the FieldType, and check the appropriate radio button
		CustomSegmentFieldType csft = (CustomSegmentFieldType)VarLong(pRow->GetValue(sfcFieldType));
		if(csft == csftText) {
			CheckRadioButton(IDC_FIELD_TEXT, IDC_FIELD_DATA, IDC_FIELD_TEXT);
			GetDlgItem(IDC_DATA_FIELD_LIST)->ShowWindow(SW_HIDE);
			//TES 9/12/2011 - PLID 45405 - Hide the Text Limit (it's only for data fields)
			GetDlgItem(IDC_FIELD_LENGTH_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MAX_FIELD_LENGTH)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FIELD_VALUE)->ShowWindow(SW_SHOW);
		}
		else {
			ASSERT(csft == csftDataField);
			CheckRadioButton(IDC_FIELD_TEXT, IDC_FIELD_DATA, IDC_FIELD_DATA);
			GetDlgItem(IDC_DATA_FIELD_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DATA_FIELD_LIST)->EnableWindow(TRUE);
			//TES 9/12/2011 - PLID 45405 - Show the Text Limit
			GetDlgItem(IDC_FIELD_LENGTH_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MAX_FIELD_LENGTH)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_FIELD_VALUE)->ShowWindow(SW_HIDE);
			//TES 9/8/2011 - PLID 45248 - Load the FieldID
			_variant_t varFieldID = pRow->GetValue(sfcFieldID);
			if(varFieldID.vt == VT_I4) {
				m_pDataFieldList->SetSelByColumn(dflcID, varFieldID);
			}
			else {
				m_pDataFieldList->CurSel = NULL;
			}
		}
		//TES 8/30/2011 - PLID 44207 - They can't change the first field (it's always the segment name).
		if(pRow->GetPreviousRow() == NULL) {
			m_nxeFieldValue.SetReadOnly(TRUE);
			m_nxbFieldUp.EnableWindow(FALSE);
			m_nxbFieldDown.EnableWindow(FALSE);
			m_nxbFieldText.EnableWindow(FALSE);
			m_nxbFieldData.EnableWindow(FALSE);
			//TES 9/12/2011 - PLID 45405 - Disable the Criteria Field and Text Limit
			m_pFieldCriteriaField->CurSel = NULL;
			GetDlgItem(IDC_FIELD_CRITERIA_FIELD)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAX_FIELD_LENGTH)->EnableWindow(FALSE);
		}
		else {
			m_nxeFieldValue.SetReadOnly(FALSE);
			m_nxbFieldText.EnableWindow(TRUE);
			m_nxbFieldData.EnableWindow(TRUE);
			if(pRow->GetNextRow() == NULL) {
				m_nxbFieldDown.EnableWindow(FALSE);
			}
			else {
				m_nxbFieldDown.EnableWindow(TRUE);
			}
			if(pRow->GetPreviousRow()->GetPreviousRow() == NULL) {
				m_nxbFieldUp.EnableWindow(FALSE);
			}
			else {
				m_nxbFieldUp.EnableWindow(TRUE);
			}
		}
	}
}

void CHL7CustomSegmentsDlg::OnSelChangedSegmentFields(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//TES 8/30/2011 - PLID 44207 - Update the field value box.
		ReflectCurrentField();
	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnNewField()
{
	try {
		//TES 9/7/2011 - PLID 44207 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 9/7/2011 - PLID 44207 - OK, get our object.
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 9/7/2011 - PLID 44207 - Add a field that's a blank string to our datalist and object
		IRowSettingsPtr pNewFieldRow = m_pSegmentFields->GetNewRow();
		pNewFieldRow->PutValue(sfcTextValue, _bstr_t(""));
		pNewFieldRow->PutValue(sfcDisplayName, _bstr_t(""));
		//TES 9/8/2011 - PLID 45248 - Added FieldType and FieldID
		pNewFieldRow->PutValue(sfcFieldType, (long)csftText);
		pNewFieldRow->PutValue(sfcFieldID, g_cvarNull);
		//TES 9/12/2011 - PLID 45405 - Added Criteria Field and Text Limit
		pNewFieldRow->PutValue(sfcCriteriaFieldID, g_cvarNull);
		pNewFieldRow->PutValue(sfcTextLimit, g_cvarNull);
		m_pSegmentFields->AddRowAtEnd(pNewFieldRow, NULL);
		m_pSegmentFields->CurSel = pNewFieldRow;
		CustomSegmentField csf;
		csf.type = csftText;
		csf.strValue = "";
		csf.nFieldID = 0;
		csf.nCriteriaFieldID = 0;
		csf.nTextLimit = -1;
		pCs->parFields->Add(csf);
		ReflectCurrentField();

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnRButtonUpSegmentFields(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		//TES 9/7/2011 - PLID 44207 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 9/7/2011 - PLID 44207 - OK, get our object.
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 9/7/2011 - PLID 44207 - Now, if they're on any row but the first one, let them remove it
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		if(pRow->GetPreviousRow() == NULL) {
			return;
		}

		m_pSegmentFields->CurSel = pRow;

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "Remove");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == 1) {
			//TES 9/7/2011 - PLID 44207 - Remove it from the datalist and array.
			long nIndex = pRow->CalcRowNumber();
			pCs->parFields->RemoveAt(nIndex);
			m_pSegmentFields->RemoveRow(pRow);
			ReflectCurrentField();
		}

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnFieldUp()
{
	try {
		//TES 9/7/2011 - PLID 44207 - Make sure we're on a custom segment
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow && VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			//TES 9/7/2011 - PLID 44207 - Get the object
			CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

			IRowSettingsPtr pRow = m_pSegmentFields->CurSel;
			if(pRow == NULL) {
				return;
			}
			IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
			if(pPrevRow == NULL) {
				return;
			}
			if(pPrevRow->GetPreviousRow() == NULL) {
				//TES 9/7/2011 - PLID 44207 - We can't swap with the first row
				return;
			}

			//TES 9/7/2011 - PLID 44207 - OK, swap in the datalist and in the array
			long nOrder = pRow->CalcRowNumber();
			CustomSegmentField csfCurrent = pCs->parFields->GetAt(nOrder);
			CustomSegmentField csfPrev = pCs->parFields->GetAt(nOrder-1);
			pCs->parFields->SetAt(nOrder-1, csfCurrent);
			pCs->parFields->SetAt(nOrder, csfPrev);
			m_pSegmentFields->RemoveRow(pRow);
			m_pSegmentFields->AddRowBefore(pRow, pPrevRow);
			m_pSegmentFields->CurSel = pRow;
			ReflectCurrentField();
		}

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnFieldDown()
{
	try {
		//TES 9/7/2011 - PLID 44207 - Make sure we're on a custom segment
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow && VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			//TES 9/7/2011 - PLID 44207 - Get the object
			CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

			IRowSettingsPtr pRow = m_pSegmentFields->CurSel;
			if(pRow == NULL) {
				return;
			}
			IRowSettingsPtr pNextRow = pRow->GetNextRow();
			if(pNextRow == NULL) {
				return;
			}
			if(pRow->GetPreviousRow() == NULL) {
				//TES 9/7/2011 - PLID 44207 - We can't move the first row
				return;
			}

			//TES 9/7/2011 - PLID 44207 - OK, swap in the datalist and in the array
			long nOrder = pRow->CalcRowNumber();
			CustomSegmentField csfCurrent = pCs->parFields->GetAt(nOrder);
			CustomSegmentField csfNext = pCs->parFields->GetAt(nOrder+1);
			pCs->parFields->SetAt(nOrder+1, csfCurrent);
			pCs->parFields->SetAt(nOrder, csfNext);
			m_pSegmentFields->RemoveRow(pNextRow);
			m_pSegmentFields->AddRowBefore(pNextRow, pRow);
			m_pSegmentFields->CurSel = pRow;
			ReflectCurrentField();
		}

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnFieldText()
{
	try {
		//TES 9/8/2011 - PLID 45248 - Update our array and datalist.
		//TES 9/8/2011 - PLID 45248 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 9/8/2011 - PLID 45248 - OK, get our object.
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 9/8/2011 - PLID 45248 - Are we on a valid field?
		IRowSettingsPtr pFieldRow = m_pSegmentFields->CurSel;
		if(pFieldRow == NULL) {
			return;
		}
		if(pFieldRow->GetPreviousRow() == NULL) {
			//TES 9/8/2011 - PLID 45248 - The first field is hard-coded to the segment name (and thus Text).
			return;
		}
		//TES 9/8/2011 - PLID 45248 - OK, update the array in our segment object.
		long nOrderIndex = pFieldRow->CalcRowNumber();
		CustomSegmentField csf = pCs->parFields->GetAt(nOrderIndex);
		if(csf.type != csftText) {
			csf.type = csftText;
			pCs->parFields->SetAt(nOrderIndex, csf);
			pFieldRow->PutValue(sfcFieldType, (long)csftText);
			pFieldRow->PutValue(sfcDisplayName, _bstr_t(GetDisplayName(csf)));
			m_bCurrentSegmentsChanged = true;
			ReflectCurrentField();
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnFieldData()
{
	try {
		//TES 9/8/2011 - PLID 45248 - Update our array and datalist.
		//TES 9/8/2011 - PLID 45248 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 9/8/2011 - PLID 45248 - OK, get our object.
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 9/8/2011 - PLID 45248 - Are we on a valid field?
		IRowSettingsPtr pFieldRow = m_pSegmentFields->CurSel;
		if(pFieldRow == NULL) {
			return;
		}
		if(pFieldRow->GetPreviousRow() == NULL) {
			//TES 9/8/2011 - PLID 45248 - The first field is hard-coded to the segment name (and thus Text).
			return;
		}
		//TES 9/8/2011 - PLID 45248 - OK, update the array in our segment object.
		long nOrderIndex = pFieldRow->CalcRowNumber();
		CustomSegmentField csf = pCs->parFields->GetAt(nOrderIndex);
		if(csf.type != csftDataField) {
			csf.type = csftDataField;
			pCs->parFields->SetAt(nOrderIndex, csf);
			pFieldRow->PutValue(sfcFieldType, (long)csftDataField);
			pFieldRow->PutValue(sfcDisplayName, _bstr_t(GetDisplayName(csf)));
			m_bCurrentSegmentsChanged = true;
			ReflectCurrentField();
		}
	}NxCatchAll(__FUNCTION__);
}


CString CHL7CustomSegmentsDlg::GetFieldListQuery()
{
	//TES 9/8/2011 - PLID 45248 - First, pull all the custom fields.
	CString strSql = "(SELECT ID, Name FROM LabCustomFieldsT ";
	//TES 9/8/2011 - PLID 45248 - Now, pull all the hard-coded database fields that we support.
	for(HL7MessageDataField hmdf = (HL7MessageDataField)-1; hmdf > hmdf_LastFieldEnum; hmdf = (HL7MessageDataField)((long)hmdf-1)) {
		strSql += FormatString(" UNION SELECT %i, '%s' ", hmdf, GetHL7MessageDataFieldName(hmdf));
	}
	return strSql + ") AS FieldsQ";
}

void CHL7CustomSegmentsDlg::OnSelChosenDataFieldList(LPDISPATCH lpRow)
{
	try {
		//TES 9/8/2011 - PLID 45248 - Update our array and datalist.
		//TES 9/8/2011 - PLID 45248 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 9/8/2011 - PLID 45248 - OK, get our object.
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 9/8/2011 - PLID 45248 - Are we on a valid field?
		IRowSettingsPtr pFieldRow = m_pSegmentFields->CurSel;
		if(pFieldRow == NULL) {
			return;
		}
		if(pFieldRow->GetPreviousRow() == NULL) {
			//TES 9/8/2011 - PLID 45248 - The first field is hard-coded to the segment name (and thus Text).
			return;
		}
		//TES 9/8/2011 - PLID 45248 - OK, update the array in our segment object.
		long nOrderIndex = pFieldRow->CalcRowNumber();
		CustomSegmentField csf = pCs->parFields->GetAt(nOrderIndex);
		long nNewFieldID = 0;
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL) {
			nNewFieldID = VarLong(pRow->GetValue(dflcID));
		}
		if(csf.nFieldID != nNewFieldID) {
			csf.nFieldID = nNewFieldID;
			pCs->parFields->SetAt(nOrderIndex, csf);
			pFieldRow->PutValue(sfcFieldID, nNewFieldID);
			pFieldRow->PutValue(sfcDisplayName, _bstr_t(GetDisplayName(csf)));
			m_bCurrentSegmentsChanged = true;
			ReflectCurrentField();
		}

	}NxCatchAll(__FUNCTION__);
}

//TES 9/8/2011 - PLID 45248 - Either the plaintext, or the name of the field, depending on the type
CString CHL7CustomSegmentsDlg::GetDisplayName(CustomSegmentField csf)
{
	if(csf.type == csftText) {
		return csf.strValue;
	}
	else {
		ASSERT(csf.type == csftDataField);
		IRowSettingsPtr pRow = m_pDataFieldList->FindByColumn(dflcID, csf.nFieldID, NULL, g_cvarFalse);
		if(pRow) {
			return VarString(pRow->GetValue(dflcName),"");
		}
		else {
			return "";
		}
	}
}
void CHL7CustomSegmentsDlg::OnSelChosenSegmentCriteriaField(LPDISPATCH lpRow)
{
	try {
		//TES 9/12/2011 - PLID 45405 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 9/12/2011 - PLID 45405 - OK, get the new Field ID, and store it to our object
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));
		long nCurrentCriteriaFieldID = pCs->nCriteriaFieldID;
		long nNewCriteriaFieldID = 0;
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL) {
			nNewCriteriaFieldID = VarLong(pRow->GetValue(dflcID),0);
		}
		if(nCurrentCriteriaFieldID != nNewCriteriaFieldID) {
			pCs->nCriteriaFieldID = nNewCriteriaFieldID;
		}

	}NxCatchAll(__FUNCTION__);
}

void CHL7CustomSegmentsDlg::OnSelChosenFieldCriteriaField(LPDISPATCH lpRow)
{
	try {
		//TES 9/12/2011 - PLID 45405 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 9/12/2011 - PLID 45405 - OK, are we on a field (that isn't the first field)?
		IRowSettingsPtr pFieldRow = m_pSegmentFields->CurSel;
		if(pFieldRow == NULL) {
			return;
		}
		if(pFieldRow->GetPreviousRow() == NULL) {
			return;
		}
		
		//TES 9/12/2011 - PLID 45405 - Get the new Field ID, and store it to our object, and to the datalist.
		long nOrderIndex = pFieldRow->CalcRowNumber();
		CustomSegmentField csf = pCs->parFields->GetAt(nOrderIndex);
		long nNewFieldID = 0;
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL) {
			nNewFieldID = VarLong(pRow->GetValue(dflcID));
		}
		if(csf.nCriteriaFieldID != nNewFieldID) {
			csf.nCriteriaFieldID = nNewFieldID;
			pCs->parFields->SetAt(nOrderIndex, csf);
			pFieldRow->PutValue(sfcCriteriaFieldID, nNewFieldID);
			ReflectCurrentField();
		}

	}NxCatchAll(__FUNCTION__);
}

//TES 11/16/2011 - PLID 44207 - Moved code from OnEnKillFocus to OnEnChange
void CHL7CustomSegmentsDlg::OnEnChangeFieldValue()
{
	try {
		//TES 8/30/2011 - PLID 44207 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 8/30/2011 - PLID 44207 - OK, get our object.
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 8/30/2011 - PLID 44207 - Are we on a valid field?
		IRowSettingsPtr pFieldRow = m_pSegmentFields->CurSel;
		if(pFieldRow == NULL) {
			return;
		}
		if(pFieldRow->GetPreviousRow() == NULL) {
			//TES 8/30/2011 - PLID 44207 - The first field is hard-coded to the segment name.
			return;
		}
		//TES 8/30/2011 - PLID 44207 - OK, if they've changed this value, update the array in our segment object.
		long nOrderIndex = pFieldRow->CalcRowNumber();
		//TES 9/8/2011 - PLID 45248 - This is an object now, not just a string
		CustomSegmentField csf = pCs->parFields->GetAt(nOrderIndex);
		CString strNewValue;
		GetDlgItemText(IDC_FIELD_VALUE, strNewValue);
		if(strNewValue != csf.strValue) {
			csf.strValue = strNewValue;
			pCs->parFields->SetAt(nOrderIndex, csf);
			pFieldRow->PutValue(sfcTextValue, _bstr_t(strNewValue));
			pFieldRow->PutValue(sfcDisplayName, _bstr_t(GetDisplayName(csf)));
			m_bCurrentSegmentsChanged = true;
		}
	}NxCatchAll(__FUNCTION__);
}

//TES 11/16/2011 - PLID 44207 - Moved code from OnEnKillFocus to OnEnChange
void CHL7CustomSegmentsDlg::OnEnChangeSegmentName()
{
	try {
		//TES 8/30/2011 - PLID 44207 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		//TES 8/30/2011 - PLID 44207 - OK, load our stored field name, and the new one they're trying to set.
		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));
		CString strCurrentName = pCs->strName;
		CString strNewName;
		GetDlgItemText(IDC_SEGMENT_NAME, strNewName);
		if(strCurrentName != strNewName) {
			//TES 8/30/2011 - PLID 44207 - They changed it, we need to update the object, the datalist, and the field array
			pCs->strName = strNewName;
			pSegmentRow->PutValue(slcName, _bstr_t(strNewName));
			//TES 9/8/2011 - PLID 45248 - This is an object now, not just a string
			CustomSegmentField csf = pCs->parFields->GetAt(0);
			csf.strValue = strNewName + "|";
			pCs->parFields->SetAt(0, csf);
			IRowSettingsPtr pFieldRow = m_pSegmentFields->GetFirstRow();
			pFieldRow->PutValue(sfcTextValue, _bstr_t(strNewName + "|"));
			pFieldRow->PutValue(sfcDisplayName, _bstr_t(strNewName + "|"));
			ReflectCurrentField();
		}

	}NxCatchAll(__FUNCTION__);
}

//TES 11/16/2011 - PLID 44207 - Moved code from OnEnKillFocus to OnEnChange
void CHL7CustomSegmentsDlg::OnEnChangeMaxFieldLength()
{
	try {
		//TES 9/12/2011 - PLID 45405 - Are we on a custom segment?
		IRowSettingsPtr pSegmentRow = m_pSegmentList->CurSel;
		if(pSegmentRow == NULL) {
			return;
		}
		if(!VarBool(pSegmentRow->GetValue(slcIsCustom))) {
			return;
		}

		CustomSegment *pCs = (CustomSegment*)VarLong(pSegmentRow->GetValue(slcPointer));

		//TES 9/12/2011 - PLID 45405 - Are we on a field (that isn't the first field)?
		IRowSettingsPtr pFieldRow = m_pSegmentFields->CurSel;
		if(pFieldRow == NULL) {
			return;
		}
		if(pFieldRow->GetPreviousRow() == NULL) {
			return;
		}
		//TES 9/12/2011 - PLID 45405 - OK, get the new Text Limit, ensure that it's greater than 0 (otherwise just blank it out), and save it
		// to our object and datalist.
		long nOrderIndex = pFieldRow->CalcRowNumber();
		CustomSegmentField csf = pCs->parFields->GetAt(nOrderIndex);
		long nNewLimit = GetDlgItemInt(IDC_MAX_FIELD_LENGTH);
		if(nNewLimit <= 0) {
			nNewLimit = -1;
			SetDlgItemText(IDC_MAX_FIELD_LENGTH, "");
		}
		if(nNewLimit != csf.nTextLimit) {
			csf.nTextLimit = nNewLimit;
			pCs->parFields->SetAt(nOrderIndex, csf);
			_variant_t varLimit = g_cvarNull;
			if(csf.nTextLimit != -1) {
				varLimit = csf.nTextLimit;
			}
			pFieldRow->PutValue(sfcTextLimit, varLimit);
		}
	}NxCatchAll(__FUNCTION__);
}
