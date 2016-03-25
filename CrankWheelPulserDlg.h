// CrankWheelPulserDlg.h : header file
//

#if !defined(AFX_CRANKWHEELPULSERDLG_H__87852EB7_E462_46F6_B658_B0291854A299__INCLUDED_)
#define AFX_CRANKWHEELPULSERDLG_H__87852EB7_E462_46F6_B658_B0291854A299__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCrankWheelPulserDlg dialog

class CCrankWheelPulserDlg : public CDialog
{
// Construction
public:
	CCrankWheelPulserDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCrankWheelPulserDlg)
	enum { IDD = IDD_CRANKWHEELPULSER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrankWheelPulserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCrankWheelPulserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeComboWheel();
	afx_msg void OnGenerateWAV();
	afx_msg void OnPlayonce();
	afx_msg void OnPlayloop();
	afx_msg void OnStoploop();
	afx_msg void OnSelchangeComboCam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeRpm();
	afx_msg void OnCbnSelchangeComboSamplerate();
	afx_msg void OnEnChangeTime2();
	afx_msg void OnEnChangeRpm2();
	afx_msg void OnEnChangeTime();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeNteeth();
	afx_msg void OnEnChangeMteeth();
	afx_msg void OnBnDoubleclickedOk2();
	afx_msg void OnEnChangeCam();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRANKWHEELPULSERDLG_H__87852EB7_E462_46F6_B658_B0291854A299__INCLUDED_)
