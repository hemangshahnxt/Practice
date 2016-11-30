#pragma once

//TES 8/29/2011 - PLID 44207 - Created
// CHL7CustomSegmentsDlg dialog
enum HL7MessageType;
enum HL7CustomSegmentScope;
enum HL7Segment;
enum CustomSegmentFieldType;
struct CustomSegment;
struct CustomSegmentField;
	
class CHL7CustomSegmentsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7CustomSegmentsDlg)

public:
	CHL7CustomSegmentsDlg(CWnd* pParent);   // standard constructor
	virtual ~CHL7CustomSegmentsDlg();

	//TES 8/29/2011 - PLID 44207 - Set by our caller, tells us what group this is for, and what type of message (which tells us what
	// hard-coded (non-custom) segments there will be).
	long m_nHL7GroupID;
	HL7MessageType m_MessageType;

// Dialog Data
	enum { IDD = IDD_HL7_CUSTOM_SEGMENTS_DLG };

protected:
	CNxIconButton m_nxbOK, m_nxbCancel, m_nxbNewSegment, m_nxbSegmentUp, m_nxbSegmentDown, m_nxbNewField, m_nxbFieldUp, m_nxbFieldDown;
	NxButton m_nxbScopeMessage, m_nxbScopeSpecimen, m_nxbFieldText, m_nxbFieldData;
	CNxEdit m_nxeSegmentName, m_nxeFieldValue, m_nxeMaxFieldLength;
	NXDATALIST2Lib::_DNxDataListPtr m_pSegmentList, m_pSegmentFields, m_pDataFieldList, m_pSegmentCriteriaField, m_pFieldCriteriaField;

	//TES 9/13/2011 - PLID 45465 - Moved CustomSegment and CustomSegmentField to HL7ParseUtils.h
	CMap<HL7CustomSegmentScope,HL7CustomSegmentScope,CArray<CustomSegment*,CustomSegment*>*,CArray<CustomSegment*,CustomSegment*>*> m_mapCustomSegments;
	bool m_bCustomSegmentsLoaded;

	//TES 8/29/2011 - PLID 44207 - Call to make sure we've loaded our segments from data.
	void EnsureCustomSegments();

	//TES 8/29/2011 - PLID 44207 - Adds the given non-custom segment, plus any associated custom segments, to our list.
	void AddSegments(HL7Segment hsSegment, HL7CustomSegmentScope hcss);

	//TES 8/29/2011 - PLID 44207 - Enable/Fill the controls based on the currently selected row.
	void ReflectCurrentSegment();

	//TES 8/30/2011 - PLID 44207 - Enable/Fill the controls based on the currently selected value in the Fields dropdown.
	void ReflectCurrentField();

	//TES 8/29/2011 - PLID 44207 - Used to know when to re-write our memory variables with the data on screen.
	HL7CustomSegmentScope m_hcssCurrentScope;
	bool m_bCurrentSegmentsChanged;
	void StoreCurrentSegments();

	//TES 8/29/2011 - PLID 44207 - Save our map to data
	void SaveCustomSegments(HL7CustomSegmentScope hcssScope, CString &strSql, CNxParamSqlArray &aryParams);

	//TES 9/8/2011 - PLID 45248 - Returns a query that can be used as the FROM clause in a datalist, which will list all the fields 
	// (either custom or hard-coded) which can be output in custom segments.
	CString GetFieldListQuery();

	//TES 9/8/2011 - PLID 45248 - Either the plaintext, or the name of the field, depending on the type
	CString GetDisplayName(CustomSegmentField csf);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnScopeMessage();
	afx_msg void OnScopeSpecimen();
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	void OnSelChangedHl7SegmentsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnNewSegment();
	afx_msg void OnSegmentUp();
	afx_msg void OnSegmentDown();
	void OnRButtonUpHl7SegmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnSelChangedSegmentFields(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnNewField();
	void OnRButtonUpSegmentFields(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnFieldUp();
	afx_msg void OnFieldDown();
	afx_msg void OnFieldText();
	afx_msg void OnFieldData();
	void OnSelChosenDataFieldList(LPDISPATCH lpRow);
	void OnSelChosenSegmentCriteriaField(LPDISPATCH lpRow);
	void OnSelChosenFieldCriteriaField(LPDISPATCH lpRow);
	afx_msg void OnEnChangeFieldValue();
	afx_msg void OnEnChangeSegmentName();
	afx_msg void OnEnChangeMaxFieldLength();
};
