// SkillListBox.cpp : implementation file
//

#include "stdafx.h"
#include "PokemonMemHack.h"
#include "SkillListBox.h"


// CSkillListBox

IMPLEMENT_DYNAMIC(CSkillListBox, CListBox)

CSkillListBox::CSkillListBox()
{

}

CSkillListBox::~CSkillListBox()
{
}


BEGIN_MESSAGE_MAP(CSkillListBox, CListBox)
END_MESSAGE_MAP()



void CSkillListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);
	LPCTSTR	lpszText = _T("あj");

	CSize	Size;
	CDC *	pDC = GetDC();

	Size = pDC->GetTextExtent(lpszText, (int)(_tcslen(lpszText)));
	ReleaseDC(pDC);

	lpMeasureItemStruct->itemHeight = Size.cy << 1;
}

void CSkillListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
	ASSERT(g_MemRom.m_bOpened);

	WORD	wSkill = (WORD)(lpDrawItemStruct->itemData);
	if(wSkill >= SKILL_COUNT)
		return;

	SkillListEntry * pSkill = g_MemRom.GetSkillListEntry(wSkill);
	if(!pSkill)
		return;

	CString	szText;
	LPCTSTR	szFmt3[3] = { _T(""), _T("%-3lu: "), _T("%03lX: ") };

	CRect	rect = lpDrawItemStruct->rcItem;
	CDC		dc;
	COLORREF	clrForeOld, clrBackOld;

	dc.Attach(lpDrawItemStruct->hDC);

	// set text color
	clrForeOld = dc.SetTextColor(g_rgForeClrTable[pSkill->bType]);
	clrBackOld = dc.SetBkColor(g_rgBackClrTable[pSkill->bType]);

	// draw rectangle
	dc.FillSolidRect(&rect, g_rgBackClrTable[pSkill->bType]);
	if(lpDrawItemStruct->itemState & ODS_SELECTED)
		dc.DrawEdge(&rect, EDGE_SUNKEN, BF_ADJUST | BF_RECT);
	else
		dc.DrawEdge(&rect, BDR_RAISEDINNER, BF_ADJUST | BF_RECT);

	// draw text
	szText.Format(szFmt3[cfg.dwCount], wSkill);
	szText += cfg.pSkillNameList[wSkill].rgszText[cfg.dwLang];
	dc.DrawText(szText,
				&(lpDrawItemStruct->rcItem),
				DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	// clean up
	dc.SetTextColor(clrForeOld);
	dc.SetBkColor(clrBackOld);
	dc.Detach();
}

int CSkillListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	ASSERT(lpCompareItemStruct->CtlType == ODT_LISTBOX);
	ASSERT(g_MemRom.m_bOpened);

	WORD	wSkill1 = (WORD)(lpCompareItemStruct->itemData1);
	WORD	wSkill2 = (WORD)(lpCompareItemStruct->itemData2);

	if(wSkill1 == wSkill2)
		return 0;

	SkillListEntry * pSkill1 = g_MemRom.GetSkillListEntry(wSkill1);
	SkillListEntry * pSkill2 = g_MemRom.GetSkillListEntry(wSkill2);
	if(!pSkill1 || !pSkill2)
		return (wSkill1 - wSkill2);

	if(pSkill1->bType != pSkill2->bType)
		return (pSkill1->bType - pSkill2->bType);
	else if(pSkill1->bPower != pSkill2->bPower)
		return (pSkill2->bPower - pSkill1->bPower);
	else
		return (wSkill1 - wSkill2);
}

// CSkillListBox message handlers
