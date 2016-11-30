// (r.gonet 09/21/2011) - PLID 45555 - Added

#include "stdafx.h"
#include "LabCustomField.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomFieldControlManager.h"

// (r.gonet 09/21/2011) - PLID 45555 - Creates a new layout cell that contains all of a field's controls. AnchorRight means that input fields will expand horizontally with the screen size.
CLabCustomFieldControlManager::CFieldCell::CFieldCell(CLabCustomFieldPtr pField, CLabCustomFieldInstance *pFieldInstance, bool bAnchorRight/*= true*/)
{
	if(pField == NULL) {
		ThrowNxException("CLabCustomFieldControlManager::CFieldCell::CFieldCell : Attempted to create a field cell with a NULL field.");
	}
	m_pField = pField;
	m_pFieldInstance = pFieldInstance;
	m_bAnchorRight = bAnchorRight;
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets rid of all of the controls, destroying them.
CLabCustomFieldControlManager::CFieldCell::~CFieldCell()
{
	RemoveAllControls();
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the associated field for this control cell.
CLabCustomFieldPtr CLabCustomFieldControlManager::CFieldCell::GetField() const
{
	return m_pField;
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the associated field instance, may be null, for this control cell.
CLabCustomFieldInstance *CLabCustomFieldControlManager::CFieldCell::GetFieldInstance() const
{
	return m_pFieldInstance;
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the indexed input control for the associated field. This control will store the value of the field or field instance
//  and must be synced back eventually.
// (r.gonet 10/30/2011) - PLID 45583 - Added a way to specify which input control to get, since there can now be multiple
CWnd * CLabCustomFieldControlManager::CFieldCell::GetInputControl(long nIndex) const
{
	if(nIndex < 0 || nIndex >= (long)m_vecInputControls.size()) {
		ThrowNxException("CLabCustomFieldControlManager::CFieldCell::GetInputControl : Index out of range.");
	}

	return m_vecInputControls[nIndex].second;
}

// (r.gonet 10/30/2011) - PLID 45583 - Gets the value associated with the indexed input control for the associated field. This control will store the value of the field or field instance
//  and must be synced back eventually.
_variant_t CLabCustomFieldControlManager::CFieldCell::GetInputControlTag(long nIndex) const
{
	if(nIndex < 0 || nIndex >= (long)m_vecInputControls.size()) {
		ThrowNxException("CLabCustomFieldControlManager::CFieldCell::GetInputControlTag : Index out of range.");
	}

	return m_vecInputControls[nIndex].first;
}

// (r.gonet 10/30/2011) - PLID 45583 - Gets the number of input controls for this field.
long CLabCustomFieldControlManager::CFieldCell::GetInputControlCount() const
{
	return m_vecInputControls.size();
}

// (r.gonet 10/30/2011) - PLID 45555 - Calculate the screen real estate this cell is taking up.
void CLabCustomFieldControlManager::CFieldCell::GetRectangle(CRect &rcCellRect) const
{
	rcCellRect = CRect(0,0,0,0);
	std::set<CWnd *>::const_iterator controlIter;
	for(controlIter = m_sControls.begin(); controlIter != m_sControls.end(); controlIter++) {
		CWnd *pWnd = *controlIter;
		if(pWnd != NULL) {
			CRect rcControlRect;
			pWnd->GetWindowRect(&rcControlRect);
			pWnd->GetParent()->ScreenToClient(&rcControlRect);
			if(rcCellRect.IsRectEmpty()) {
				rcCellRect = rcControlRect;
			} else {
				rcCellRect.UnionRect(&rcCellRect, &rcControlRect);
			}
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45555 - Adds a basic control, like a label, to the cell. These controls are all associated with the same field (and field instance if it exists).
void CLabCustomFieldControlManager::CFieldCell::AddControl(CWnd *pWnd)
{
	if(pWnd == NULL) {
		ThrowNxException("CLabCustomFieldControlManager::CFieldCell::AddControl : Attempted to add a NULL control.");
	}
	bool bAlreadyExisted = m_sControls.insert(pWnd).second == false;
	if(bAlreadyExisted) {
		ThrowNxException("CLabCustomFieldControlManager::CFieldCell::AddControl : Attempted to add a control that was already added.");
	}
}

// (r.gonet 09/21/2011) - PLID 45555 - Adds an input control, which will store the value of the field or field instance. There must always be one defined per cell.
void CLabCustomFieldControlManager::CFieldCell::AddInputControl(CWnd *pWnd)
{
	this->AddControl(pWnd);

	std::pair<_variant_t, CWnd *> pair;
	pair.first = g_cvarNull;
	pair.second = pWnd;
	m_vecInputControls.push_back(pair);
}

// (r.gonet 09/21/2011) - PLID 45555 - Adds an input control, which will store the value of the field or field instance. There must always be one defined per cell.
void CLabCustomFieldControlManager::CFieldCell::AddInputControl(CWnd *pWnd, _variant_t varValue)
{
	this->AddControl(pWnd);

	std::pair<_variant_t, CWnd *> pair;
	pair.first = varValue;
	pair.second = pWnd;
	m_vecInputControls.push_back(pair);
}

// (r.gonet 09/21/2011) - PLID 45555 - Removes all of the controls in this cell, destroying them all.
void CLabCustomFieldControlManager::CFieldCell::RemoveAllControls()
{
	std::set<CWnd *>::iterator controlIter;
	for(controlIter = m_sControls.begin(); controlIter != m_sControls.end(); controlIter++) {
		CWnd *pWnd = *controlIter;
		if(pWnd != NULL) {
			pWnd->DestroyWindow();
			delete pWnd;
		}
	}
	m_sControls.clear();
	m_vecInputControls.clear();
}

void CLabCustomFieldControlManager::CFieldCell::RedrawControls(CLabCustomFieldControlManager &cmControlManager)
{
	CDC dc;
	dc.Attach(::GetDC(cmControlManager.GetParentWindow()->GetSafeHwnd()));

	std::set<CWnd *>::iterator controlIter;
	for(controlIter = m_sControls.begin(); controlIter != m_sControls.end(); controlIter++) {
		CWnd *pWnd = *controlIter;
		if(pWnd != NULL) {
			bool bResize = true;
			if(!m_bAnchorRight) {
				for(unsigned int j = 0; j < m_vecInputControls.size(); j++) {
					if(pWnd == m_vecInputControls[j].second) {
						bResize = false;
						break;
					}
				}
			}
			if(bResize) {
				CRect rcControl;
				pWnd->GetWindowRect(&rcControl);
				cmControlManager.GetParentWindow()->ScreenToClient(&rcControl);
				pWnd->MoveWindow(rcControl.left, rcControl.top, cmControlManager.GetDrawingSize().cx - 20, rcControl.Height());
			}
		}
	}
	dc.Detach();
}

// (r.gonet 09/21/2011) - PLID 45555 - Creates a new management class to store field controls and allow and facilitate their construction. Basically a middle man between the fields and the window.
// - pParentWindow is the window that the controls are drawn to and will contain them.
// - nMinControlID is the dialog contrtol ID to start at when we dynamically create controls.
// - nMaxControlID is the dialog control ID that we must not surpass when we dynamically create controls.
// - pTemplateInstance is the template instance all of these controls and their associated fields are going to be for.
// - emEditMode determines how or if the control values are synced back to their respective fields.
CLabCustomFieldControlManager::CLabCustomFieldControlManager(CWnd *pParentWindow, UINT nMinControlID, UINT nMaxControlID, CCFTemplateInstance *pTemplateInstance/*=NULL*/, EEditMode emEditMode/*=emNone*/)
{
	m_pParentWindow = pParentWindow;
	m_nMinControlID = nMinControlID;
	m_nMaxControlID = nMaxControlID;
	m_nNextControlID = m_nMinControlID;
	m_rcDrawingRect = CRect(0,0,0,0);
	m_rcBoundingBox = CRect(0,0,0,0);
	m_pTemplateInstance = pTemplateInstance;
	m_emEditMode = emEditMode;
	m_pCurrentCell = NULL;
}

// (r.gonet 09/21/2011) - PLID 45555 - Destroys all contained controls.
CLabCustomFieldControlManager::~CLabCustomFieldControlManager()
{
	RemoveAllControls();
}

// (r.gonet 09/21/2011) - PLID 45555 - Prepares things to add a new field. Behind the scenes, associates a new cell with this field and field instance. 
//  Both AddInputControl and EndField must be called afterward.
// - pField is the field to start adding controls for.
// - pFieldInstance is the field instance for pField. May be null if we are editing the field itself and not its instance value.
void CLabCustomFieldControlManager::BeginField(CLabCustomFieldPtr pField, CLabCustomFieldInstance *pFieldInstance/*= NULL*/, bool bAnchorRight/*=true*/)
{
	CFieldCell *pCell = new CFieldCell(pField, pFieldInstance, bAnchorRight);
	m_mapFieldGUIDToCell[pField->GetGUID()] = pCell;
	m_pCurrentCell = pCell;
}

// (r.gonet 09/21/2011) - PLID 45555 - Adds a new control for a field. BeginField must have been called first.
void CLabCustomFieldControlManager::AddControl(CWnd *pWndControl)
{
	m_pCurrentCell->AddControl(pWndControl);
	
	// We have enlarged our bounding box
	CRect rcWindowRect;
	pWndControl->GetWindowRect(&rcWindowRect);
	m_pParentWindow->ScreenToClient(&rcWindowRect);
	m_rcBoundingBox.UnionRect(&m_rcBoundingBox, &rcWindowRect);
}

// (r.gonet 09/21/2011) - PLID 45555 - Adds a new control for the current field that will be the one to sync back input from the user. BeginField must have been called first.
void CLabCustomFieldControlManager::AddInputControl(CWnd *pWndControl, _variant_t varTag/*= g_cvarNull*/)
{
	m_pCurrentCell->AddInputControl(pWndControl, varTag);

	// We have enlarged our bounding box
	CRect rcWindowRect;
	pWndControl->GetWindowRect(&rcWindowRect);
	m_pParentWindow->ScreenToClient(&rcWindowRect);
	m_rcBoundingBox.UnionRect(&m_rcBoundingBox, &rcWindowRect);
}

// (r.gonet 09/21/2011) - PLID 45555 - Completes the field control definition. BeginField and AddInputControl must have been called first.
void CLabCustomFieldControlManager::EndField()
{
	// Add some padding
	m_rcBoundingBox.bottom += 10;
	// And await the next field
	m_pCurrentCell = NULL;
}

// (r.gonet 09/21/2011) - PLID 45555 - Cleans up all controls defined with this manager. Destroys them all. Resets the drawing and bounding boxes.
void CLabCustomFieldControlManager::RemoveAllControls()
{
	std::map<CString, CFieldCell *>::iterator cellIter;
	for(cellIter = m_mapFieldGUIDToCell.begin(); cellIter != m_mapFieldGUIDToCell.end(); cellIter++) {
		CFieldCell *pCell = cellIter->second;
		delete pCell;
	}
	m_mapFieldGUIDToCell.clear();
	m_rcBoundingBox = CRect(GetDrawingOrigin(), CSize(0,0));
	m_nNextControlID = m_nMinControlID;
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets a new dialog control ID for a new dynamic control
UINT CLabCustomFieldControlManager::GetNewControlID()
{
	if(m_nNextControlID > m_nMaxControlID) {
		ThrowNxException("CLabCustomFieldControlManager::GetNewControlID : Surpassed maximum control ID, no more dynamic controls can be drawn.");
	}
	return m_nNextControlID++;
}

// (r.gonet 09/21/2011) - PLID 45555 - Sets the rectangle that we will be able to draw in. Does not factor in clipping though.
void CLabCustomFieldControlManager::SetDrawingRect(CRect rcDrawingRect)
{
	m_rcDrawingRect = rcDrawingRect;
	m_rcDrawingRect.NormalizeRect();
	m_rcBoundingBox = CRect(GetDrawingOrigin(), CSize(0,0));
	this->RedrawControls();
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the drawing rectangle we are abiding to.
CRect CLabCustomFieldControlManager::GetDrawingRect()
{
	return m_rcDrawingRect;
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the origin that the first control will be drawn from.
CPoint CLabCustomFieldControlManager::GetDrawingOrigin()
{
	return m_rcDrawingRect.TopLeft();
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the size of the drawing area.
CSize CLabCustomFieldControlManager::GetDrawingSize()
{
	return m_rcDrawingRect.Size();
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the bounding box around all existing controls.
CRect CLabCustomFieldControlManager::GetBoundingBox()
{
	return m_rcBoundingBox;
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the bottom left of the bounding box, which is useful for the next control position.
CPoint CLabCustomFieldControlManager::GetBoundingBoxBottomLeft()
{
	return CPoint(m_rcBoundingBox.left, m_rcBoundingBox.bottom);
}

// (r.gonet 09/21/2011) - PLID 45555 - Gets the window that actually owns all of these controls.
CWnd *CLabCustomFieldControlManager::GetParentWindow()
{
	return m_pParentWindow;
}

// (r.gonet 09/21/2011) - PLID 45555 - Sets a good size for a given control, calculating it based on its font and text content.
void CLabCustomFieldControlManager::SetIdealSize(CWnd *pControl, bool bAllowLineBreaks/*= false*/)
{
	if(!pControl) {
		ASSERT(FALSE);
		ThrowNxException("CLabCustomFieldControlManager::SetIdealSize : Passed a NULL window.");
	}
	CRect rcControl;
	pControl->GetWindowRect(&rcControl);
	GetParentWindow()->ScreenToClient(&rcControl);
	// First see if we need to use multiple lines
	CClientDC dc(m_pParentWindow);

	CSize szControl;
	if(!bAllowLineBreaks) {
		CalcControlIdealDimensions(&dc, pControl, szControl, FALSE);
	}
	if(bAllowLineBreaks || rcControl.left + szControl.cx > (m_rcDrawingRect.Width() - 20)) {
		// Now we know that we need multiple lines
		szControl = CSize(m_rcDrawingRect.Width() - 20, 0);
		CalcControlIdealDimensions(&dc, pControl, szControl, TRUE);
	}
	pControl->MoveWindow(rcControl.left, rcControl.top, szControl.cx, szControl.cy);
}

// (r.gonet 09/21/2011) - PLID 45555 - Syncs back all control values to their associated fields or field instances, depending on the edit mode.
bool CLabCustomFieldControlManager::SyncControlsToFields()
{
	CString strErrorMessages = "";
	std::map<CString, CFieldCell *>::iterator cellIter;
	for(cellIter = m_mapFieldGUIDToCell.begin(); cellIter != m_mapFieldGUIDToCell.end(); cellIter++) {
		CFieldCell *pCell = cellIter->second;
		ASSERT(pCell != NULL);
		CLabCustomFieldPtr pField = pCell->GetField();
		CLabCustomFieldInstance *pInstance = pCell->GetFieldInstance();

		// If we are allowed to edit the values of the fields, then we need to have an instance, ensure it exists.
		if(m_emEditMode == emValues && pInstance == NULL) {
			ThrowNxException("CLabCustomFieldControlManager::Save : Attempted to set a custom field's value without a corresponding field instance.");
		}
		
		// If we are allowed to commit input then pass the value on to the field so it can save it.
		if(m_emEditMode == emDefaultValues || m_emEditMode == emValues) {
			CString strErrorMessage;
			bool bValueValid = pField->SetValueFromControl(pCell, strErrorMessage, pInstance);
			if(!bValueValid) {
				strErrorMessages += "- " + strErrorMessage + "\r\n";
			}
		}
		
	}
	
	if(strErrorMessages.GetLength() > 0 && this->GetParentWindow() != NULL) {
		MessageBox(this->GetParentWindow()->GetSafeHwnd(), FormatString("The following error(s) occurred while checking field values. You must correct these before saving.\r\n%s\r\n", 
			strErrorMessages), "Field Validation Errors", MB_ICONERROR|MB_OK);
		return false;
	}

	return true;
}

// (r.gonet 10/30/2011) - PLID 45555 - Calculate the screen real estate a lab custom field is taking up.
void CLabCustomFieldControlManager::GetFieldRectangle(CLabCustomFieldPtr pField, CRect &rcFieldRect)
{
	if(pField == NULL) {
		return;
	}

	CFieldCell *pFieldCell;
	std::map<CString, CFieldCell *>::iterator iter;
	if((iter = m_mapFieldGUIDToCell.find(pField->GetGUID())) != m_mapFieldGUIDToCell.end()) {
		pFieldCell = iter->second;
	} else {
		pFieldCell = NULL;
	}

	if(pFieldCell) {
		pFieldCell->GetRectangle(rcFieldRect);
	}
}

// (r.gonet 10/30/2011) - PLID 45555 - Reposition the controls to account for changed window size.
void CLabCustomFieldControlManager::RedrawControls()
{
	std::map<CString, CFieldCell *>::iterator cellIter;
	for(cellIter = m_mapFieldGUIDToCell.begin(); cellIter != m_mapFieldGUIDToCell.end(); cellIter++) {
		CFieldCell *pCell = cellIter->second;
		ASSERT(pCell != NULL);
		pCell->RedrawControls(*this);
		CRect rcCell;
		pCell->GetRectangle(rcCell);
		m_rcBoundingBox.UnionRect(&m_rcBoundingBox, &rcCell);
	}
	m_rcBoundingBox.bottom += 10;
}