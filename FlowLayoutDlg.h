// (r.gonet 08/22/2011) - Placeholder - Initial Commit

#pragma once

#include "FlowLayoutEngine.h"
#include "PracticeRc.h"

class CFlowLayoutRow;
class CFlowLayoutCell
{
private:
	CFlowLayoutRow &m_parentRow;
	CRect m_rcBoundingBox;
	CRect m_rcPadding;
	long m_nSpacing;
	enum EVAlign
	{
		evTop,
		evBottom,
	};
	EVAlign m_evVAlign;
	CArray<CWnd *, CWnd *> m_aryControls;

	// Move these over to the cpp file before marking Test
	inline void UpdateControls()
	{
		m_rcBoundingBox.SetRect(m_rcBoundingBox.left, m_rcBoundingBox.top, 0, 0);
		CPoint ptOrigin = m_rcBoundingBox.TopLeft();
		CPoint ptNextControlPosition;
		if(m_evVAlign == evTop) {
			ptNextControlPosition = ptOrigin + m_rcPadding.TopLeft();
			for(int i = 0; i < m_aryControls.GetSize(); i++) {
				CWnd *pWndControl = m_aryControls[i];
				if(pWndControl->IsWindowVisible()) {
					CRect rcWindowRect;
					pWndControl->GetWindowRect(&rcWindowRect);
					pWndControl->GetParent()->ScreenToClient(&rcWindowRect);
					// Append the control onto the bottom of the others
					rcWindowRect.SetRect(ptNextControlPosition.x, ptNextControlPosition.y, 
						ptNextControlPosition.x + rcWindowRect.Width(), ptNextControlPosition.y + rcWindowRect.Height());
					pWndControl->MoveWindow(&rcWindowRect);
					m_rcBoundingBox.UnionRect(&m_rcBoundingBox, &rcWindowRect);
					ptNextControlPosition.Offset(0, rcWindowRect.Height() + m_nSpacing);
				}
			}
		} else {
			CSize szCellSize = CSize(0, 0);
			for(int i = 0; i < m_aryControls.GetSize(); i++) {
				CWnd *pWndControl = m_aryControls[i];
				if(pWndControl->IsWindowVisible()) {
					CRect rcWindowRect;
					pWndControl->GetWindowRect(&rcWindowRect);
					// Append the control onto the bottom of the others
					if(szCellSize.cx < rcWindowRect.Width() + m_rcPadding.left + m_rcPadding.right) {
						szCellSize.cx = rcWindowRect.Width() + m_rcPadding.left + m_rcPadding.right;
					}
					szCellSize.cy += rcWindowRect.Height() + m_nSpacing; 
				}
			}
			ptNextControlPosition = CPoint(m_rcPadding.left, szCellSize.cy);
			
			for(int i = m_aryControls.GetSize() - 1; i >= 0; i--) {
				CWnd *pWndControl = m_aryControls[i];
				if(pWndControl->IsWindowVisible()) {
					CRect rcWindowRect;
					pWndControl->GetWindowRect(&rcWindowRect);
					pWndControl->GetParent()->ScreenToClient(&rcWindowRect);
					// Append the control onto the top of the others
					ptNextControlPosition.Offset(0, -(rcWindowRect.Height() + m_nSpacing));
					rcWindowRect.SetRect(ptNextControlPosition.x, ptNextControlPosition.y, 
						ptNextControlPosition.x + rcWindowRect.Width(), ptNextControlPosition.y + rcWindowRect.Height());
					pWndControl->MoveWindow(&rcWindowRect);
					m_rcBoundingBox.UnionRect(&m_rcBoundingBox, &rcWindowRect);
				}
			}
			m_rcBoundingBox.left -= m_rcPadding.left;
			m_rcBoundingBox.top -= m_rcPadding.top;
		}
		m_rcBoundingBox.right += m_rcPadding.right;
		m_rcBoundingBox.bottom += m_rcPadding.bottom;
	};
public:
	inline CFlowLayoutCell(CFlowLayoutRow &parentRow)
		: m_parentRow(parentRow)
	{
		m_rcBoundingBox = CRect(0,0,0,0);
		m_rcPadding = CRect(0,0,0,0);
		m_nSpacing = 0;
		m_evVAlign = evBottom;
	};

	inline void SetPadding(CRect rcPadding)
	{
		rcPadding.NormalizeRect();
		m_rcPadding = rcPadding;
		UpdateControls();
	};

	inline void SetSpacing(long nSpacing)
	{
		if(nSpacing < 0) {
			nSpacing = 0;
		}

		m_nSpacing = nSpacing;
		UpdateControls();
	};
	
	inline void AddControl(CWnd *pWndControl)
	{
		m_aryControls.Add(pWndControl);
		UpdateControls();	
	};

	inline void RemoveControl(CWnd *pWndControl)
	{
		for(int i = 0; i < m_aryControls.GetSize(); i++) {
			if(m_aryControls[i] == pWndControl) {
				m_aryControls.RemoveAt(i);
				break;
			}
		}

		UpdateControls();
	};

	inline void SetPosition(CPoint ptOrigin)
	{
		m_rcBoundingBox.SetRect(ptOrigin, ptOrigin);
		UpdateControls();
	};

	inline CSize GetSize() const
	{
		return m_rcBoundingBox.Size();
	};

	inline CFlowLayoutRow & GetRow()
	{
		return m_parentRow;
	};
};

class CFlowLayoutRow
{
private:
	CRect m_rcBoundingBox;
	CArray<CFlowLayoutCell *, CFlowLayoutCell *> m_aryCells;

	inline void UpdateCells()
	{
		m_rcBoundingBox.SetRect(m_rcBoundingBox.left, m_rcBoundingBox.top, 0, 0);
		// First go through and get the size of this row's bounding box
		for(int i = 0; i < m_aryCells.GetSize(); i++) {
			CFlowLayoutCell *pCell = m_aryCells[i];
			CSize szCellSize = pCell->GetSize();
			m_rcBoundingBox.right += szCellSize.cx;
			if(szCellSize.cy > m_rcBoundingBox.Height()) {
				m_rcBoundingBox.bottom = m_rcBoundingBox.top + szCellSize.cy;
			}
		}

		CPoint ptOrigin = m_rcBoundingBox.TopLeft();
		CPoint ptNextCellPosition = ptOrigin;
		for(int i = 0; i < m_aryCells.GetSize(); i++) {
			CFlowLayoutCell *pCell = m_aryCells[i];
			pCell->SetPosition(CPoint(ptNextCellPosition.x, m_rcBoundingBox.Height() - pCell->GetSize().cy));
			ptNextCellPosition.Offset(pCell->GetSize().cx, 0);
		}
	};
public:
	inline CFlowLayoutRow()
	{
		m_rcBoundingBox = CRect(0,0,0,0);
	};

	inline void AddCell(CFlowLayoutCell *pCell)
	{
		if(pCell) {
			m_aryCells.Add(pCell);
			UpdateCells();
		}
	};

	inline void SetPosition(CPoint ptOrigin)
	{
		m_rcBoundingBox.SetRect(ptOrigin, ptOrigin);
		UpdateCells();
	};

	inline CSize GetSize() const
	{
		return m_rcBoundingBox.Size();
	};
};

class CFlowLayout
{
private:
	CWnd *m_pParentWindow;
	CRect m_rcBoundingBox;
	CArray<CFlowLayoutRow *, CFlowLayoutRow *> m_aryRows;

	inline void UpdateRows()
	{
		m_rcBoundingBox.SetRect(m_rcBoundingBox.left, m_rcBoundingBox.top, 0, 0);
		CPoint ptOrigin = m_rcBoundingBox.TopLeft();
		CPoint ptNextRowPosition = ptOrigin;
		for(int i = 0; i < m_aryRows.GetSize(); i++) {
			CFlowLayoutRow *pRow = m_aryRows[i];
			pRow->SetPosition(CPoint(ptNextRowPosition.x, ptNextRowPosition.y));
			ptNextRowPosition.Offset(0, pRow->GetSize().cy);
			if(m_rcBoundingBox.Width() < pRow->GetSize().cx) {
				m_rcBoundingBox.right = m_rcBoundingBox.left + pRow->GetSize().cx;
			}
			m_rcBoundingBox.bottom += pRow->GetSize().cy;
		}
	};
public:
	inline CFlowLayout(CWnd *pParentWindow)
		: m_pParentWindow(pParentWindow)
	{
		m_rcBoundingBox = CRect(0,0,0,0);
	};

	inline void AddRow(CFlowLayoutRow *pRow)
	{
		if(pRow) {
			m_aryRows.Add(pRow);
		}
	};

	inline void SetPosition(CPoint ptOrigin)
	{
		m_rcBoundingBox.SetRect(ptOrigin, ptOrigin);
		UpdateRows();
	};
};

// CFlowLayoutDlg dialog

class CFlowLayoutDlg : public CDialog
{
	DECLARE_DYNAMIC(CFlowLayoutDlg)

public:
	CFlowLayoutDlg(CWnd* pParent);   // standard constructor
	virtual ~CFlowLayoutDlg();

// Dialog Data
	enum { IDD = IDD_FLOW_LAYOUT_DLG };

protected:
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};