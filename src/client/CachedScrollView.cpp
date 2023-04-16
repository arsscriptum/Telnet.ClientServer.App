

#include "stdafx.h"
#include "CachedScrollView.h"


// CCachedScrollView

IMPLEMENT_DYNCREATE(CCachedScrollView, CScrollView)

CCachedScrollView::CCachedScrollView()
{
		CacheValid=FALSE;
		CacheRect.SetRectEmpty();
}

CCachedScrollView::~CCachedScrollView()
{
}


BEGIN_MESSAGE_MAP(CCachedScrollView, CScrollView)
	ON_WM_PAINT()
END_MESSAGE_MAP()




void CCachedScrollView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	OnPrepareDC(&dc);

	if (CacheRect.IsRectNull()) 
	{
		OnDraw(&dc);
		return;
	}
	

	if (CacheBitmap.m_hObject==NULL) 
		CacheBitmap.CreateCompatibleBitmap(&dc,CacheRect.Width(),CacheRect.Height()*5);
	
	CRect ClientRect;
	GetClientRect(ClientRect);
	
	CPoint org=dc.GetViewportOrg();
	ClientRect.OffsetRect(-org.x,-org.y);

	if (!CacheRect.PtInRect(ClientRect.TopLeft()) ||
		!CacheRect.PtInRect(ClientRect.BottomRight()))
	{
		CacheRect.OffsetRect(
			ClientRect.CenterPoint()-CacheRect.CenterPoint());
		CacheValid=FALSE;
	}

	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	mdc.SelectObject(&CacheBitmap);
	
	if (!CacheValid) {
		mdc.SelectStockObject(SYSTEM_FIXED_FONT);
		mdc.SelectStockObject(NULL_PEN);
		mdc.Rectangle(0,0,CacheRect.Width(),CacheRect.Height());
		mdc.SelectStockObject(BLACK_PEN);
		mdc.SetViewportOrg(-CacheRect.left,-CacheRect.top);
		OnDraw(&mdc);
		mdc.SetViewportOrg(0,0);
		CacheValid=TRUE;
	}
	
	dc.SelectClipRgn(NULL);
	dc.BitBlt(CacheRect.left,CacheRect.top,
		CacheRect.Width(),CacheRect.Height(),&mdc,0,0,SRCCOPY);


}
