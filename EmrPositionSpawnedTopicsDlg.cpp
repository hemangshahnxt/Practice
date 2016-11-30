// EmrPositionSpawnedTopicsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrPositionSpawnedTopicsDlg.h"
#include "InternationalUtils.h"

//TES 9/9/2009 - PLID 35495 - Created
// CEmrPositionSpawnedTopicsDlg dialog

IMPLEMENT_DYNAMIC(CEmrPositionSpawnedTopicsDlg, CNxDialog)

CEmrPositionSpawnedTopicsDlg::CEmrPositionSpawnedTopicsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrPositionSpawnedTopicsDlg::IDD, pParent)
{
	m_nTemplateID = -1;
	m_nIconSize = 32;
	m_lpDraggingRow = NULL;
}

CEmrPositionSpawnedTopicsDlg::~CEmrPositionSpawnedTopicsDlg()
{
}

void CEmrPositionSpawnedTopicsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CEmrPositionSpawnedTopicsDlg, CNxDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CEmrPositionSpawnedTopicsDlg::OnOK)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//TES 9/9/2009 - PLID 35495 - The columns in our version of the topic tree.
#define POSITION_TREE_COLUMN_ID			0
#define POSITION_TREE_COLUMN_ROW_TYPE	1
#define POSITION_TREE_COLUMN_ICON		2
#define POSITION_TREE_COLUMN_NAME		3
#define POSITION_TREE_COLUMN_SOURCE_ACTION_ID	4
#define POSITION_TREE_COLUMN_SOURCE_NAME	5	//TES 9/18/2009 - PLID 35590
#define POSITION_TREE_COLUMN_ORIGINAL_NAME	6	//TES 9/18/2009 - PLID 35590
#define POSITION_TREE_COLUMN_MODIFIED	7	//TES 9/18/2009 - PLID 35589

//TES 9/9/2009 - PLID 35495 - Our own personal define of this ID (the value doesn't really matter much).
#define POSITION_MORE_INFO_NODE_ID	-2
//TES 2/12/2014 - PLID 60748 - New <Codes> topic
#define POSITION_CODES_NODE_ID	-3

using namespace NXDATALIST2Lib;
using namespace ADODB;
// CEmrPositionSpawnedTopicsDlg message handlers
BOOL CEmrPositionSpawnedTopicsDlg::OnInitDialog()
{
	try {
		//TES 9/9/2009 - PLID 35495 - Initialize our controls.
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		m_pTree = BindNxDataList2Ctrl(this, IDC_POSITION_TOPICS_TREE, NULL, false);

		//TES 9/9/2009 - PLID 35495 - Set the title to indicate what template we're editing.
		SetWindowText(m_strTemplateName + " - Positioning Topics");

		//TES 9/9/2009 - PLID 35495 - Load our desired icon width.
		//m_nIconSize = GetRemotePropertyInt("EMRTreeIconSize", 32, 0, GetCurrentUserName(), true);
		//TES 9/16/2009 - PLID 35495 - Per Christina, hardcode the size to 24x24.
		m_nIconSize = 24;
		m_pTree->GetColumn(POSITION_TREE_COLUMN_ICON)->PutStoredWidth(m_nIconSize);

		//TES 9/9/2009 - PLID 35495 - Now, go through and fill the tree.
		//Add the top-level node
		IRowSettingsPtr pEmnRow = m_pTree->GetNewRow();
		pEmnRow->PutValue(POSITION_TREE_COLUMN_ID, m_nTemplateID);
		pEmnRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)0);
		pEmnRow->PutValue(POSITION_TREE_COLUMN_NAME, _bstr_t(FormatString("[%s]\r\n%s", FormatDateTimeForInterface(m_dtTemplateDate, NULL, dtoDate), m_strTemplateName)));
		pEmnRow->PutValue(POSITION_TREE_COLUMN_ROW_TYPE, (long)etrtEmn);
		pEmnRow->PutValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID, g_cvarNull);
		pEmnRow->PutValue(POSITION_TREE_COLUMN_SOURCE_NAME, g_cvarNull);
		pEmnRow->PutValue(POSITION_TREE_COLUMN_ORIGINAL_NAME, g_cvarNull);
		//TES 9/18/2009 - PLID 35589 - Initialize to unmodified
		pEmnRow->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)0);
		m_pTree->AddRowAtEnd(pEmnRow, NULL);
		EnsureRowIcon(pEmnRow, m_nIconSize);

		CArray<TopicInfo,TopicInfo&> arTopics;
		//TES 9/10/2009 - PLID 35495 - Load the topics.  This query is so complicated because there may be topics
		// in data which can no longer actually be spawned, because the actions that spawned them have been deleted
		// or branched.  So we join with the active versions of each action, and if the topic has a SourceActionID
		// which doesn't map to an action, we filter it out.  This is analogous to what NexEMRExporter does.
		//TES 9/18/2009 - PLID 35590 - Added SourceName, the name of the object (if any) that spawned this topic.
		//TES 10/5/2009 - PLID 35495 - Filter out topics which are spawned by details which aren't actually on
		// this template.
		//TES 3/18/2010 - PLID 37530 - Added support for topics spawned from Smart Stamps.
		_RecordsetPtr rsTopics = CreateParamRecordset(FormatString("SELECT "
			"EmrTemplateTopicsT.ID, EmrTemplateTopicsT.Name, EMRParentTemplateTopicID, SourceActionID, "
			"CASE WHEN EmrActionsT.SourceType = %d THEN EmrInfoT.Name "
			"WHEN EmrActionsT.SourceType = %d THEN EmrDataT.Data WHEN EmrActionsT.SourceType = %d THEN ProcedureT.Name "
			"WHEN EmrActionsT.SourceType = %d THEN EmrImageStampsT.StampText + ' - ' + convert(nvarchar(50),EmrTemplateTopicsT.SourceStampIndex) "
			"ELSE '' END AS SourceName "
			"FROM EmrTemplateTopicsT "
			"INNER JOIN ("
			"	SELECT EmrTemplateTopicsT.ID, "
			"	CASE WHEN EmrActionsT.SourceType = 4 THEN DataActionMap.MappedActionID "
			"	WHEN EmrActionsT.SourceType = 13 THEN DropdownActionMap.MappedActionID "
			"	WHEN EmrActionsT.SourceType = 10 THEN HotSpotActionMap.MappedActionID "
			"	WHEN EmrActionsT.SourceType = 14 THEN SmartStampActionMap.MappedActionID ELSE NULL END AS MappedActionID "
			"	FROM EmrTemplateTopicsT LEFT JOIN EmrActionsT ON EmrTemplateTopicsT.SourceActionID = EmrActionsT.ID "
			"	LEFT JOIN ( "
			"		SELECT EmrActionsT.ID, CurrentActions.ID AS MappedActionID "
			"		FROM (EmrActionsT INNER JOIN EmrDataT ON EmrActionsT.SourceID = EmrDataT.ID) LEFT JOIN ("
			"			SELECT EmrActionsT.*, EmrDataT.EmrDataGroupID "
			"			FROM EmrActionsT INNER JOIN EmrDataT ON EmrActionsT.SourceID = EmrDataT.ID "
			"			INNER JOIN EmrInfoMasterT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID "
			"			WHERE EmrActionsT.SourceType = 4 AND EmrActionsT.Deleted = 0 AND EmrActionsT.DestType IN (6,9)"
			"			AND EmrInfoMasterT.ID IN (SELECT EmrInfoMasterID FROM EmrTemplateDetailsT WHERE TemplateID = {INT}) "
			"		) AS CurrentActions "
			"		ON EmrActionsT.DestType = CurrentActions.DestType AND EmrActionsT.DestID = CurrentActions.DestID "
			"		AND EmrDataT.EmrDataGroupID = CurrentActions.EmrDataGroupID "
			"		WHERE EmrActionsT.SourceType = 4 AND EmrActionsT.Deleted = 0 AND EmrActionsT.DestType IN (6,9) "
			"	) AS DataActionMap ON EmrTemplateTopicsT.SourceActionID = DataActionMap.ID "
			"	LEFT JOIN "
			"	( "
			"		SELECT EmrActionsT.ID, CurrentActions.ID AS MappedActionID "
			"		FROM (EmrActionsT INNER JOIN EmrTableDropdownInfoT ON EmrActionsT.SourceID = EmrTableDropdownInfoT.ID) "
			"		LEFT JOIN ("
			"			SELECT EmrActionsT.*, EmrTableDropdownInfoT.DropdownGroupID "
			"			FROM EmrActionsT INNER JOIN EmrTableDropdownInfoT ON EmrActionsT.SourceID = EmrTableDropdownInfoT.ID "
			"			INNER JOIN EmrDataT ON EmrTableDropdownInfoT.EmrDataID = EmrDataT.ID "
			"			INNER JOIN EmrInfoMasterT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID "
			"			WHERE EmrActionsT.SourceType = 13 AND EmrActionsT.Deleted = 0 AND EmrActionsT.DestType IN (6,9)"
			"			AND EmrInfoMasterT.ID IN (SELECT EmrInfoMasterID FROM EmrTemplateDetailsT WHERE TemplateID = {INT}) "
			"		) AS CurrentActions "
			"		ON EmrActionsT.DestType = CurrentActions.DestType AND EmrActionsT.DestID = CurrentActions.DestID "
			"		AND EmrTableDropdownInfoT.DropdownGroupID = CurrentActions.DropdownGroupID "
			"		WHERE EmrActionsT.SourceType = 13 AND EmrActionsT.Deleted = 0 AND EmrActionsT.DestType IN (6,9) "
			"	) AS DropdownActionMap ON EmrTemplateTopicsT.SourceActionID = DropdownActionMap.ID "
			"	LEFT JOIN "
			"	( "
			"		SELECT EmrActionsT.ID, CurrentActions.ID AS MappedActionID "
			"		FROM (EmrActionsT INNER JOIN EMRImageHotSpotsT ON EmrActionsT.SourceID = EMRImageHotSpotsT.ID) "
			"		LEFT JOIN ("
			"			SELECT EmrActionsT.*, EmrImageHotSpotsT.EmrSpotGroupID "
			"			FROM EmrActionsT INNER JOIN EmrImageHotSpotsT ON EmrActionsT.SourceID = EmrImageHotSpotsT.ID "
			"			INNER JOIN EmrInfoMasterT ON EmrImageHotSpotsT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID "
			"			WHERE EmrActionsT.SourceType = 10 AND EmrActionsT.Deleted = 0 AND EmrActionsT.DestType IN (6,9)"
			"			AND EmrInfoMasterT.ID IN (SELECT EmrInfoMasterID FROM EmrTemplateDetailsT WHERE TemplateID = {INT}) "
			"		) AS CurrentActions "
			"		ON EmrActionsT.DestType = CurrentActions.DestType AND EmrActionsT.DestID = CurrentActions.DestID "
			"		AND EmrImageHotSpotsT.EmrSpotGroupID = CurrentActions.EmrSpotGroupID "
			"		WHERE EmrActionsT.SourceType = 10 AND EmrActionsT.Deleted = 0 AND EmrActionsT.DestType IN (6,9) "
			"	) AS HotSpotActionMap ON EmrTemplateTopicsT.SourceActionID = HotSpotActionMap.ID "
			"	LEFT JOIN "
			"	( "
			"		SELECT EmrActionsT.ID, EmrActionsT.ID AS MappedActionID "
			"		FROM (EmrActionsT INNER JOIN EmrImageStampsT ON EmrActionsT.SourceID = EmrImageStampsT.ID) "
			"		WHERE EmrActionsT.SourceType = 14 AND EmrActionsT.Deleted = 0 AND EmrActionsT.DestType IN (6,9) "
			"	) AS SmartStampActionMap ON EmrTemplateTopicsT.SourceActionID = SmartStampActionMap.ID "
			") AS TopicActionMap ON EmrTemplateTopicsT.ID = TopicActionMap.ID "
			"LEFT JOIN EmrActionsT ON EmrTemplateTopicsT.SourceActionID = EmrActionsT.ID "
			"LEFT JOIN EmrInfoT ON EmrActionsT.SourceID = EmrInfoT.ID "
			// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
			"LEFT JOIN EMRDataT ON EMRActionsT.SourceID = EMRDataT.ID AND EmrActionsT.SourceType = {INT} "
			"LEFT JOIN ProcedureT ON EmrActionsT.SourceID = ProcedureT.ID "
			"LEFT JOIN EmrImageStampsT ON EmrActionsT.SourceID = EmrImageStampsT.ID "
			"WHERE TemplateID = {INT} AND (SourceActionID Is Null OR MappedActionID Is Not Null) "
			"ORDER BY OrderIndex ASC", eaoEmrItem, eaoEmrDataItem, eaoProcedure, eaoSmartStamp), 
			m_nTemplateID, m_nTemplateID, m_nTemplateID,
			eaoEmrDataItem, m_nTemplateID);
		while(!rsTopics->eof) {
			TopicInfo ti;
			ti.varID = rsTopics->Fields->GetItem("ID")->Value;
			ti.varName = rsTopics->Fields->GetItem("Name")->Value;
			ti.varParentID = rsTopics->Fields->GetItem("EMRParentTemplateTopicID")->Value;
			ti.varSourceActionID = rsTopics->Fields->GetItem("SourceActionID")->Value;
			ti.varSourceName = rsTopics->Fields->GetItem("SourceName")->Value;
			arTopics.Add(ti);
			rsTopics->MoveNext();
		}

		//TES 9/10/2009 - PLID 35495 - Add all top-level rows.
		for(int i = 0; i < arTopics.GetSize(); i++) {
			TopicInfo ti = arTopics[i];
			if(ti.varParentID.vt == VT_NULL) {
				IRowSettingsPtr pChildRow = m_pTree->GetNewRow();
				pChildRow->PutValue(POSITION_TREE_COLUMN_ID, ti.varID);
				pChildRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)0);
				pChildRow->PutValue(POSITION_TREE_COLUMN_NAME, ti.varName);
				pChildRow->PutValue(POSITION_TREE_COLUMN_ROW_TYPE, (long)etrtTopic);
				pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID, ti.varSourceActionID);
				if(ti.varSourceActionID.vt != VT_NULL) {
					pChildRow->PutForeColor(RGB(127,127,127));
				}
				pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_NAME, ti.varSourceName);
				pChildRow->PutValue(POSITION_TREE_COLUMN_ORIGINAL_NAME, ti.varName);
				//TES 9/18/2009 - PLID 35589 - Initialize to unmodified
				pChildRow->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)0);
				m_pTree->AddRowAtEnd(pChildRow, pEmnRow);
				EnsureRowIcon(pChildRow, m_nIconSize);
			}
		}
		//TES 9/10/2009 - PLID 35495 - Now go through each of those rows and add any children (recursively)
		IRowSettingsPtr pParentRow = pEmnRow->GetFirstChildRow();
		while(pParentRow) {
			AddChildRows(pParentRow, &arTopics);
			pParentRow = pParentRow->GetNextRow();
		}

		
		//TES 2/12/2014 - PLID 60748 - Add the Codes child node that we know is always there.
		IRowSettingsPtr pChildRow = m_pTree->GetNewRow();
		pChildRow->PutValue(POSITION_TREE_COLUMN_ID, (long)POSITION_CODES_NODE_ID);
		pChildRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)0);
		pChildRow->PutValue(POSITION_TREE_COLUMN_NAME, _bstr_t("<Codes>"));
		pChildRow->PutValue(POSITION_TREE_COLUMN_ROW_TYPE, (long)etrtCodes);
		pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID, g_cvarNull);
		pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_NAME, g_cvarNull);
		pChildRow->PutValue(POSITION_TREE_COLUMN_ORIGINAL_NAME, g_cvarNull);
		//TES 2/12/2014 - PLID 60748 - The Codes topic will always be unmodified
		pChildRow->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)0);
		m_pTree->AddRowAtEnd(pChildRow, pEmnRow);
		EnsureRowIcon(pChildRow, m_nIconSize);

		//TES 9/10/2009 - PLID 35495 - Add the More Info child node that we know is always there.
		pChildRow = m_pTree->GetNewRow();
		pChildRow->PutValue(POSITION_TREE_COLUMN_ID, (long)POSITION_MORE_INFO_NODE_ID);
		pChildRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)0);
		pChildRow->PutValue(POSITION_TREE_COLUMN_NAME, _bstr_t("<More Info>"));
		pChildRow->PutValue(POSITION_TREE_COLUMN_ROW_TYPE, (long)etrtMoreInfo);
		pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID, g_cvarNull);
		pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_NAME, g_cvarNull);
		pChildRow->PutValue(POSITION_TREE_COLUMN_ORIGINAL_NAME, g_cvarNull);
		//TES 9/18/2009 - PLID 35589 - The More Info topic will always be unmodified
		pChildRow->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)0);
		m_pTree->AddRowAtEnd(pChildRow, pEmnRow);
		EnsureRowIcon(pChildRow, m_nIconSize);

		//TES 9/10/2009 - PLID 35495 - Expand the EMN so they can see the topics.
		pEmnRow->Expanded = TRUE;

		//TES 9/18/2009 - PLID 35590 - Now see if there are any duplicated topic names, and rename them if possible.
		CheckDuplicateTopics(pEmnRow);

	}NxCatchAll("Error in CEmrPositionSpawnedTopicsDlg::OnInitDialog()");

	return CNxDialog::OnInitDialog();
}

void CEmrPositionSpawnedTopicsDlg::AddChildRows(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<TopicInfo,TopicInfo&> *parTopics)
{
	long nID = VarLong(pRow->GetValue(POSITION_TREE_COLUMN_ID));
	ASSERT(nID != -1);
	for(int i = 0; i < parTopics->GetSize(); i++) {
		TopicInfo ti = parTopics->GetAt(i);
		if(VarLong(ti.varParentID,-1) == nID) {
			IRowSettingsPtr pChildRow = m_pTree->GetNewRow();
			pChildRow->PutValue(POSITION_TREE_COLUMN_ID, ti.varID);
			pChildRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)0);
			pChildRow->PutValue(POSITION_TREE_COLUMN_NAME, ti.varName);
			pChildRow->PutValue(POSITION_TREE_COLUMN_ROW_TYPE, (long)etrtTopic);
			pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID, ti.varSourceActionID);
			if(ti.varSourceActionID.vt != VT_NULL) {
				pChildRow->PutForeColor(RGB(127,127,127));
			}
			pChildRow->PutValue(POSITION_TREE_COLUMN_SOURCE_NAME, ti.varSourceName);
			pChildRow->PutValue(POSITION_TREE_COLUMN_ORIGINAL_NAME, ti.varName);
			//TES 9/18/2009 - PLID 35589 - Initialize to unmodified
			pChildRow->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)0);
			m_pTree->AddRowAtEnd(pChildRow, pRow);
			EnsureRowIcon(pChildRow, m_nIconSize);

			//TES 9/19/2009 - PLID 35495 - If this is the child of a spawned topic, then color it gray, as it cannot
			// be repositioned.
			if(VarLong(pRow->GetValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID),-1) != -1) {
				pChildRow->PutBackColor(RGB(200,200,200));
			}

			//TES 9/10/2009 - PLID 35495 - Recurse
			AddChildRows(pChildRow, parTopics);
		}
	}
}
int CEmrPositionSpawnedTopicsDlg::Open(long nTemplateID, const CString &strTemplateName, const COleDateTime &dtTemplateDate)
{
	m_nTemplateID = nTemplateID;
	m_strTemplateName = strTemplateName;
	m_dtTemplateDate = dtTemplateDate;

	return DoModal();
}

void CEmrPositionSpawnedTopicsDlg::EnsureRowIcon(NXDATALIST2Lib::IRowSettingsPtr pRow, long nIconSize)
{
	//TES 9/16/2009 - PLID 35495 - GetEmrTreeIcon() now takes a single boolean parameter, for whether we're using
	// the 24x24 or 32x32 icons.
	bool bUseSmallIcons = (nIconSize == 24) ? true : false;
	if(!bUseSmallIcons) {
		ASSERT(nIconSize == 32);
	}
	
	//TES 9/18/2009 - PLID 35589 - We need to know if this row has been modified.
	bool bModified = (VarLong(pRow->GetValue(POSITION_TREE_COLUMN_MODIFIED)) == 1) ? true : false;
	//TES 9/9/2009 - PLID 35495 - Sort of copied out of CEmrTreeWnd, but because this is a simple dialog we just
	// use the basic OpenSaved icons for each type.
	EmrTreeRowType etrt = (EmrTreeRowType)VarLong(pRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE));
	switch(etrt) {
	case etrtEmn:
		{
			//TES 9/18/2009 - PLID 35589 - We now display a different icon if it's been modified.
			pRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bModified?etiOpenUnsavedEmn:etiOpenSavedEmn, bUseSmallIcons));
		}
		break;
	case etrtTopic:
		{
			//TES 9/18/2009 - PLID 35589 - We now display a different icon if it's been modified.
			pRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bModified?etiOpenUnsavedTopic:etiOpenSavedTopic, bUseSmallIcons));
		}
		break;
	case etrtMoreInfo:
		{
			//TES 9/18/2009 - PLID 35589 - It shouldn't ever be possible for the More Info topic to be modified
			// (on this screen, anyway).
			ASSERT(!bModified);
			pRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenSavedMoreInfo, bUseSmallIcons));
		}
		break;
	case etrtCodes:
		{
			//TES 2/12/2014 - PLID 60748 - It shouldn't ever be possible for the Codes topic to be modified
			// (on this screen, anyway).
			ASSERT(!bModified);
			pRow->PutValue(POSITION_TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenSavedCodes, bUseSmallIcons));
		}
		break;
	default:
		//This isn't valid to have an icon!
		ASSERT(FALSE);
		break;
	}
}BEGIN_EVENTSINK_MAP(CEmrPositionSpawnedTopicsDlg, CNxDialog)
ON_EVENT(CEmrPositionSpawnedTopicsDlg, IDC_POSITION_TOPICS_TREE, 12, CEmrPositionSpawnedTopicsDlg::OnDragBeginPositionTopicsTree, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
ON_EVENT(CEmrPositionSpawnedTopicsDlg, IDC_POSITION_TOPICS_TREE, 13, CEmrPositionSpawnedTopicsDlg::OnDragOverCellPositionTopicsTree, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
ON_EVENT(CEmrPositionSpawnedTopicsDlg, IDC_POSITION_TOPICS_TREE, 1, CEmrPositionSpawnedTopicsDlg::OnSelChangingPositionTopicsTree, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CEmrPositionSpawnedTopicsDlg, IDC_POSITION_TOPICS_TREE, 14, CEmrPositionSpawnedTopicsDlg::OnDragEndPositionTopicsTree, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
END_EVENTSINK_MAP()

void CEmrPositionSpawnedTopicsDlg::OnDragBeginPositionTopicsTree(BOOL* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags)
{
	try {
		//TES 9/10/2009 - PLID 35495 - Simplified version of CEmrTreeWnd::OnDragBeginTree()
		IRowSettingsPtr pRow(lpRow);

		EmrTreeRowType etrt = (EmrTreeRowType)VarLong(pRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE));
		if(etrt == etrtTopic) {
			*pbShowDrag = TRUE;
			m_pTree->CurSel = pRow;
		}
		else {
			*pbShowDrag = FALSE;
		}
		m_lpDraggingRow = lpRow;

	}NxCatchAll("Error in CEmrPositionSpawnedTopicsDlg::OnDragBeginPositionTopicsTree()");
}

#define IDT_DRAG_HOVER	1000

void CEmrPositionSpawnedTopicsDlg::OnDragOverCellPositionTopicsTree(BOOL* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
{
	try {
		//TES 9/10/2009 - PLID 35495 - Simplified version of CEmrTreeWnd::OnDragOverCellTree()
		KillTimer(IDT_DRAG_HOVER);
		IRowSettingsPtr pToRow(lpRow);
		IRowSettingsPtr pFromRow(lpFromRow);
		ClearDragPlaceholders(pToRow);

		*pbShowDrop = IsValidDrag(pFromRow, pToRow);

		if(*pbShowDrop) {
			EmrTreeRowType etrtFrom = (EmrTreeRowType)VarLong(pFromRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE));
			EmrTreeRowType etrtTo = (EmrTreeRowType)VarLong(pToRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE));
			if(etrtFrom == etrtTopic) {
				if(etrtTo == etrtTopic || etrtTo == etrtEmn) {
					BOOL bNoVisibleChildren = TRUE;
					IRowSettingsPtr pChildRow = pToRow->GetFirstChildRow();
					while (bNoVisibleChildren && pChildRow != NULL) {
						if (pChildRow->Visible)
							bNoVisibleChildren = FALSE;
						pChildRow = pChildRow->GetNextRow();
					}
					if(!bNoVisibleChildren && pToRow->Expanded == FALSE) {
						pToRow->Expanded = TRUE;
					}
					else {
						if(etrtTo == etrtTopic) {
							//This is a leaf node.  But maybe they want to make the row they're dragging a child.  Start a timer, if
							//they hover long enough, we'll make this node into a parent.
							SetTimer(IDT_DRAG_HOVER, 1000, NULL);
						}
					}
				}
			}
		}
	} NxCatchAll("CEmrPositionSpawnedTopicsDlg::OnDragOverCellPositionTopicsTree()");
}

BOOL CEmrPositionSpawnedTopicsDlg::IsValidDrag(NXDATALIST2Lib::IRowSettings *pFromRow, NXDATALIST2Lib::IRowSettings *pToRow)
{
	if(!pFromRow || !pToRow) return FALSE;
	
	//TES 9/10/2009 - PLID 35495 - Copied, and then drastically simplified, from CEmrTreeWnd::IsValidDrag()
	EmrTreeRowType etrtFrom = (EmrTreeRowType)VarLong(pFromRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE));
	EmrTreeRowType etrtTo = (EmrTreeRowType)VarLong(pToRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE));
	if(etrtFrom == etrtTopic) {

		// (a.wetta 2006-11-16 11:13) - PLID 19474 - Also, don't allow rearranging on subtopics of spawned topics on templates
		if(pFromRow->GetParentRow() != NULL && VarLong(pFromRow->GetParentRow()->GetValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID),-1) != -1) {
			return FALSE;
		}
		// (a.wetta 2006-11-16 11:47) - PLID 19474 - They are trying to drag onto a spawned topic of a template, which is illegal
		if(pToRow->GetParentRow() != NULL && VarLong(pToRow->GetParentRow()->GetValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID),-1) != -1) {
			return FALSE;
		}

		if(etrtTo == etrtTopic || etrtTo == etrtPlaceholder) {
			IRowSettingsPtr pTopToParent = pToRow;
			while(pTopToParent->GetParentRow()) {
				if(pTopToParent == pFromRow) {
					//They are attempting to drag a topic onto one of its subtopics, which is illegal.
					return FALSE;
				}
				pTopToParent = pTopToParent->GetParentRow();
			}
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}
void CEmrPositionSpawnedTopicsDlg::OnSelChangingPositionTopicsTree(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 9/10/2009 - PLID 35495 - Copied from CEmrTreeWnd::OnSelChangingTree()
		if(m_lpDraggingRow){
			IRowSettingsPtr pDraggingRow(m_lpDraggingRow);
			IRowSettingsPtr pNewRow(*lppNewSel);
			if(!IsValidDrag(pDraggingRow, pNewRow)) {
				// (b.cardillo 2006-11-16 15:53) - PLID 23265 - Need to do reference counting properly
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}
		} else if (*lppNewSel == NULL) {
			// (b.cardillo 2006-02-07 10:49) - PLID 19168 - Just made it so when the user clicks in 
			// the empty space in the tree (i.e. not on any node) it just leaves the selection as is.
			// (b.cardillo 2006-11-16 15:53) - PLID 23265 - Need to do reference counting properly
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll("Error in CEmrPositionSpawnedTopicsDlg::OnSelChangingPositionTopicsTree()");
}

void CEmrPositionSpawnedTopicsDlg::OnDragEndPositionTopicsTree(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
{
	try {
		//TES 9/10/2009 - PLID 35495 - Copied, and then simplified, from CEmrTreeWnd::OnDragEndTree()
		KillTimer(IDT_DRAG_HOVER);
		m_lpDraggingRow = NULL;
		IRowSettingsPtr pRow(lpRow);
		IRowSettingsPtr pFromRow(lpFromRow);
		if(IsValidDrag(pFromRow, pRow)) {
			//If the from and to rows are siblings, and the row is moving down, then insert after, because the target row will
			//shift up when the source row is removed.
			IRowSettingsPtr pDestRow;
			if(pFromRow->GetParentRow() == pRow->GetParentRow() && m_pTree->IsRowEarlierInList(pFromRow, pRow)) {
				pDestRow = pRow->GetNextRow();
			}
			else {
				pDestRow = pRow;
			}

			IRowSettingsPtr pRowAdded;
			if(pDestRow) {
				pRowAdded = m_pTree->AddRowBefore(pFromRow, pDestRow);
			}
			else {
				pRowAdded = m_pTree->AddRowAtEnd(pFromRow, pRow->GetParentRow());
			}

			m_pTree->CurSel = pRowAdded;

			//TES 9/18/2009 - PLID 35589 - We need to set the row being dragged, its old parent (recursively), and its
			// new parent (recursively) as being modified.
			pRowAdded->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)1);
			EnsureRowIcon(pRowAdded, m_nIconSize);
			IRowSettingsPtr pParentRow = pRowAdded->GetParentRow();
			while(pParentRow) {
				pParentRow->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)1);
				EnsureRowIcon(pParentRow, m_nIconSize);
				pParentRow = pParentRow->GetParentRow();
			}
			pParentRow = pFromRow->GetParentRow();
			while(pParentRow) {
				pParentRow->PutValue(POSITION_TREE_COLUMN_MODIFIED, (long)1);
				EnsureRowIcon(pParentRow, m_nIconSize);
				pParentRow = pParentRow->GetParentRow();
			}
			
			m_pTree->RemoveRow(pFromRow);

			//TES 9/18/2009 - PLID 35590 - Since there's been a change, go through all the topics and make sure any
			// duplicated topic names are renamed if possible.
			CheckDuplicateTopics(m_pTree->GetFirstRow());			
		}
		else {
			//Set the CurSel back to the original row.
			m_pTree->CurSel = pFromRow;
		}
		ClearDragPlaceholders();
	}NxCatchAll("Error in CEmrPositionSpawnedTopicsDlg::OnDragEndPositionTopicsTree()");
}

void CEmrPositionSpawnedTopicsDlg::OnTimer(UINT nIDEvent)
{
	try {
		switch(nIDEvent) {
		case IDT_DRAG_HOVER:
			//TES 9/10/2009 - PLID 35495 - Copied from CEmrTreeWnd
			KillTimer(IDT_DRAG_HOVER);
			ASSERT(m_lpDraggingRow);
			if(m_lpDraggingRow) {
				IRowSettingsPtr pRow = m_pTree->CurSel;
				//We need to make sure that a placeholder only appears if the topic has no visible children
				if(pRow->GetFirstChildRow() == NULL) {
					//We need to insert a placeholder child row.
					if(VarLong(pRow->GetValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID),-1) == -1) {
						m_arDragPlaceholders.Add(InsertPlaceholder(pRow));
						pRow->Expanded = TRUE;
					}
				}
			}
			break;
		}
	}NxCatchAll("Error in CEmrPositionSpawnedTopicsDlg::OnTimer()");
}

void CEmrPositionSpawnedTopicsDlg::ClearDragPlaceholders(NXDATALIST2Lib::IRowSettings *pRowToPreserve /*= NULL*/)
{
	//TES 9/10/2009 - PLID 35495 - Copied from CEmrTreeWnd
	for(int i = m_arDragPlaceholders.GetSize()-1; i >= 0; i--) {
		if(m_arDragPlaceholders[i] != pRowToPreserve) {
			m_pTree->RemoveRow(m_arDragPlaceholders[i]);
			m_arDragPlaceholders.RemoveAt(i);
		}
	}
}

IRowSettingsPtr CEmrPositionSpawnedTopicsDlg::InsertPlaceholder(NXDATALIST2Lib::IRowSettings *pParentRow)
{
	//TES 9/10/2009 - PLID 35495 - Copied from CEmrTreeWnd()
	IRowSettingsPtr pPlaceholder = m_pTree->GetNewRow();
	pPlaceholder->PutValue(POSITION_TREE_COLUMN_ID, (long)0);
	pPlaceholder->PutValue(POSITION_TREE_COLUMN_ROW_TYPE, (long)etrtPlaceholder);
	pPlaceholder->PutValue(POSITION_TREE_COLUMN_ICON, (long)0);
	pPlaceholder->PutValue(POSITION_TREE_COLUMN_NAME, _bstr_t(""));
	pPlaceholder->PutValue(POSITION_TREE_COLUMN_SOURCE_ACTION_ID, (long)-1);
	m_pTree->AddRowAtEnd(pPlaceholder, pParentRow);
	return pPlaceholder;
}

void CEmrPositionSpawnedTopicsDlg::OnOK()
{
	try {
		//TES 9/10/2009 - PLID 35495 - Go through and construct a SQL batch to just update all the OrderIndexes and
		// EmrParentTemplateTopicIDs.  Note that we want to update ALL of them, not just the ones that changed, because 
		// part of the purpose of this dialog is to enforce all topics to have unique indexes against their parent, 
		// which CEmrTreeWnd generally fails to do.
		CString strSqlBatch = BeginSqlBatch();
		IRowSettingsPtr pRow = m_pTree->GetFirstRow()->GetFirstChildRow();
		long nIndex = 0;
		while(pRow && (EmrTreeRowType)VarLong(pRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE)) == etrtTopic) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE EmrTemplateTopicsT SET OrderIndex = %li, EmrParentTemplateTopicID = NULL "
				"WHERE ID = %li", nIndex, VarLong(pRow->GetValue(POSITION_TREE_COLUMN_ID)));
			SaveChildRows(pRow, strSqlBatch);
			nIndex++;
			pRow = pRow->GetNextRow();
		}

		ExecuteSqlBatch(strSqlBatch);

		CNxDialog::OnOK();
	}NxCatchAll("Error in CEmrPositionSpawnedTopicsDlg::OnOK()");			
}

void CEmrPositionSpawnedTopicsDlg::SaveChildRows(NXDATALIST2Lib::IRowSettingsPtr pParentRow, CString &strSqlBatch)
{
	IRowSettingsPtr pRow = pParentRow->GetFirstChildRow();
	long nIndex = 0;
	while(pRow) {
		AddStatementToSqlBatch(strSqlBatch, "UPDATE EmrTemplateTopicsT SET OrderIndex = %li, EmrParentTemplateTopicID = %li "
			"WHERE ID = %li", nIndex, VarLong(pParentRow->GetValue(POSITION_TREE_COLUMN_ID)),
			VarLong(pRow->GetValue(POSITION_TREE_COLUMN_ID)));
		SaveChildRows(pRow, strSqlBatch);
		nIndex++;
		pRow = pRow->GetNextRow();
	}
}

void CEmrPositionSpawnedTopicsDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		SetControlPositions();
	}NxCatchAll("Error in CEmrPositionSpawnedTopicsDlg::OnSize()");
}

//TES 9/18/2009 - PLID 35590 - Goes through all children of the given row, finds any topics that have duplicated
// names, and renames them using their SourceName (if possible).
void CEmrPositionSpawnedTopicsDlg::CheckDuplicateTopics(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	//TES 9/18/2009 - PLID 35590 - Copied and stripped down from CEmrTreeWnd::CheckDuplicateTopics
	//Go through all child rows.
	IRowSettingsPtr pRow = pParentRow->GetFirstChildRow();
	while(pRow) {
		//Only process topics.
		if(etrtTopic == (EmrTreeRowType)VarLong(pRow->GetValue(POSITION_TREE_COLUMN_ROW_TYPE))) {
			BOOL bRenamed = FALSE;
			CString strSourceAction = "";
			//Go through the rest of the topics.
			IRowSettingsPtr pRow2 = pRow->GetNextRow();
			while(pRow2) {
				//Only process topics
				if(etrtTopic == (EmrTreeRowType)VarLong(pRow2->GetValue(POSITION_TREE_COLUMN_ROW_TYPE))) {
					//OK, we've got two non topic rows.  Are they duplicates?
					CString strName = VarString(pRow->GetValue(POSITION_TREE_COLUMN_ORIGINAL_NAME),"");
					CString strName2 = VarString(pRow2->GetValue(POSITION_TREE_COLUMN_ORIGINAL_NAME),"");
					if(strName == strName2) {
						//Yup.  Do they have different source names?
						strSourceAction = VarString(pRow->GetValue(POSITION_TREE_COLUMN_SOURCE_NAME),"");
						CString strSourceAction2 = VarString(pRow2->GetValue(POSITION_TREE_COLUMN_SOURCE_NAME),"");
						if(strSourceAction != strSourceAction2) {
							//Yes.  Rename!
							bRenamed = TRUE; //This will tell pRow to rename itself once it's gone through all other rows.
							//TES 4/4/2006 - PLID 19903 - Only rename it if it was spawned 
							//(if both this and pRow were unspawned, we wouldn't get here, so we know one of them will be
							//renamed).
							if(!strSourceAction2.IsEmpty()) {
								// (a.walling 2007-10-09 13:20) - PLID 25548 - Update the preview as well
								CString strNewName = VarString(pRow2->GetValue(POSITION_TREE_COLUMN_ORIGINAL_NAME),"") + " (" + strSourceAction2 + ")";
								pRow2->PutValue(POSITION_TREE_COLUMN_NAME, _bstr_t(strNewName));
							}
						}
					}
				}
				pRow2 = pRow2->GetNextRow();
			}
			if(bRenamed) {
				//TES 4/4/2006 - PLID 19903 - Only rename it if it was spawned 
				//(if both this and pRow2 were unspawned, we wouldn't get here, so we know one of them will be
				//renamed).
				if(!strSourceAction.IsEmpty()) {
					// (a.walling 2007-10-09 14:14) - PLID 25548 - Update the preview as well
					CString strNewName = VarString(pRow->GetValue(POSITION_TREE_COLUMN_ORIGINAL_NAME),"") + " (" + strSourceAction + ")";
					pRow->PutValue(POSITION_TREE_COLUMN_NAME, _bstr_t(strNewName));
				}
			}
			//TES 9/18/2009 - PLID 35590 - Recurse.
			CheckDuplicateTopics(pRow);
		}
		pRow = pRow->GetNextRow();
	}
}