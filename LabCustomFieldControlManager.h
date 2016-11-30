// (r.gonet 09/21/2011) - PLID 45555 - Added

#pragma once

#include "LabCustomField.h"
#include "LabCustomFieldTemplate.h"
#include <set>

class CLabCustomFieldInstance;
class CCFTemplateInstance;
typedef boost::shared_ptr<class CLabProcCFTemplate> CLabProcCFTemplatePtr;
typedef boost::shared_ptr<class CLabCustomField> CLabCustomFieldPtr;

// (r.gonet 09/21/2011) - PLID 45555 - CLabCustomFieldControlManager is a go middle man between lab custom fields and the UI.
//  Lab custom fields write their dynamic controls here in order to have them tracked and freed eventually.
//  This manager allows the sync back of values the user enters when a caller calls Save.
class CLabCustomFieldControlManager
{
public:
	// Defines various modes that the manager can be in that allow more choice over how the control values are synced back.
	enum EEditMode
	{
		emNone = 0, // Do not sync control values back to the fields or field instances
		emValues, // Sync control values back to the field instances
		emDefaultValues, // Sync control values back to the fields themselves
	};

	// (r.gonet 09/21/2011) - PLID 45555 - CFieldCell is a helper component that is defines a collection of controls specific to an individual 
	//  custom field. 
	class CFieldCell
	{
	private:
		// (r.gonet 09/21/2011) - PLID 45555 - The field this cell is associated with.
		CLabCustomFieldPtr m_pField;
		// (r.gonet 09/21/2011) - PLID 45555 - The field instance the field is associated with.
		CLabCustomFieldInstance *m_pFieldInstance;
		// (r.gonet 09/21/2011) - PLID 45555 - A collection of controls created by the associated field.
		std::set<CWnd *> m_sControls;
		// (r.gonet 09/21/2011) - PLID 45555 - The control that serves as the user input control for the associated field. There must be one.
		//CWnd *m_pInputControl;
		std::vector<std::pair<_variant_t, CWnd *> > m_vecInputControls;
		// Anchors this field to the right side of the parent window. Left anchor is a given for all fields.
		bool m_bAnchorRight;
	public:
		CFieldCell(CLabCustomFieldPtr pField, CLabCustomFieldInstance *pFieldInstance, bool bAnchorRight = true);
		~CFieldCell();

		CLabCustomFieldPtr GetField() const;
		CLabCustomFieldInstance *GetFieldInstance() const;
		CWnd * GetInputControl(long nIndex) const;
		_variant_t GetInputControlTag(long nIndex) const;
		long GetInputControlCount() const;
		void GetRectangle(CRect &rcCellRect) const;

		void AddControl(CWnd *pWnd);
		void AddInputControl(CWnd *pWnd);
		void AddInputControl(CWnd *pWnd, _variant_t varValue);
		void RemoveAllControls();
		void RedrawControls(CLabCustomFieldControlManager &cmControlManager);
	};

private:
	// (r.gonet 09/21/2011) - PLID 45555 - The window we are drawing on
	CWnd *m_pParentWindow;
	// (r.gonet 09/21/2011) - PLID 45555 - The minimum control ID that dynamic controls use when they need an ID.
	UINT m_nMinControlID;
	// (r.gonet 09/21/2011) - PLID 45555 - The maximum control ID that dynamic controls use when they need an ID.
	UINT m_nMaxControlID;
	// (r.gonet 09/21/2011) - PLID 45555 - The current control ID that dynamic controls use when they need an ID.
	UINT m_nNextControlID;
	// (r.gonet 09/21/2011) - PLID 45555 - Where on the screen we can draw controls at
	CRect m_rcDrawingRect;
	// (r.gonet 09/21/2011) - PLID 45555 - The rectangle around all drawn controls.
	CRect m_rcBoundingBox;
	// (r.gonet 09/21/2011) - PLID 45555 - Maps a field's GUID to its associated cell.
	std::map<CString, CFieldCell *> m_mapFieldGUIDToCell;
	// (r.gonet 09/21/2011) - PLID 45555 - The template instance that contains the controls we are drawing.
	CCFTemplateInstance *m_pTemplateInstance; 
	// (r.gonet 09/21/2011) - PLID 45555 - The current mode of editing (control-field syncing) allowed.
	EEditMode m_emEditMode;
	// (r.gonet 09/21/2011) - PLID 45555 - The cell that is currently being worked on, added to.
	CFieldCell *m_pCurrentCell;
public:
	CLabCustomFieldControlManager(CWnd *pParent, UINT nMinControlID, UINT nMaxControlID, CCFTemplateInstance *pTemplateInstance = NULL, EEditMode emEditMode = emNone);
	~CLabCustomFieldControlManager();

	void BeginField(CLabCustomFieldPtr pField, CLabCustomFieldInstance *pFieldInstance = NULL, bool bAnchorRight = true);
	void AddControl(CWnd *pWndControl);
	void AddInputControl(CWnd *pWndControl, _variant_t varTag = g_cvarNull);
	void EndField();
	void RemoveAllControls();
	// Message mapping at run time is not easy in C++. 
	UINT GetNewControlID();

	void SetDrawingRect(CRect rcDrawingRect);
	CRect GetDrawingRect();
	CPoint GetDrawingOrigin();
	CSize GetDrawingSize();
	CRect GetBoundingBox();
	CPoint GetBoundingBoxBottomLeft();

	CWnd *GetParentWindow();

	void SetIdealSize(CWnd *pControl, bool bAllowLineBreaks = false);

	bool SyncControlsToFields();
	void GetFieldRectangle(CLabCustomFieldPtr pField, CRect &rcFieldRect);
	void RedrawControls();
};

