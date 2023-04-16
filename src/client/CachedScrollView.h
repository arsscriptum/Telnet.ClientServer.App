
//==============================================================================
//
//   log.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#ifndef __CACHED_SCROLL_VIEW_H__
#define __CACHED_SCROLL_VIEW_H__



class CCachedScrollView : public CScrollView
{
	DECLARE_DYNCREATE(CCachedScrollView)

protected:
	CCachedScrollView();   
	virtual ~CCachedScrollView();
	virtual void OnDraw(CDC* pDC) { }
private:
	CBitmap CacheBitmap;
public:
	BOOL CacheValid;
	CRect CacheRect;
	void SetCacheSize(SIZE sz) {
		if (CacheRect.IsRectNull()) 
			CacheRect=CRect(CPoint(0,0),sz);
	}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};


#endif //__CACHED_SCROLL_VIEW_H__