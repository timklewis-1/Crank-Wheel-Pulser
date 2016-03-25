// ---------------------------------------------------------
//   CrankWheelPulser
//
//   (C) 2005 - B. A. Bowling And A. C. Grippo
//
//   This header must appear on all derivatives of this code.
//
// ----------------------------------------------------------
//
//   V1.001 - Modified to add general M-N tooth pattern and variable rpm
//            by L. Gardiner, April 2006

#include "stdafx.h"
#include "CrankWheelPulser.h"
#include "CrankWheelPulserDlg.h"
#include "stdafx.h"
#include "stdio.h"
#include "memory.h"
#include "stdlib.h"
#include "mmsystem.h"
#include ".\crankwheelpulserdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CComboBox * wheeldef;
CComboBox * camdef;
CComboBox * sampleratepull;
CEdit * upperrpmbox;
CEdit * lowrpmbox;
CEdit * timebox;
CEdit * camsyncbox;
CEdit * mteethbox;
CEdit * nteethbox;

#define WAVE_FORMAT_PCM 1

typedef struct {
  char riff[4];		    // 4 bytes
  long filesize;	    // 4 bytes
  char wave[4];		    // 4 bytes
  char fmt[4];		    // 4 bytes
  long chunkSize;       // 4 bytes
  short wFormatTag;     // 2 bytes
  short nChannels;      // 2 bytes
  long nSamplesPerSec;  // 4 bytes
  long nAvgBytesPerSec; // 4 bytes
  short nBlockAlign;    // 2 bytes
  short wBitsPerSample; // 2 bytes
  char data[4];		    // 4 bytes
  long length;			// 4 bytes
} MyWaveFormat;

char fullfilename[256];	// WAV filename

double tooth[361], antitooth1[361], antitooth2[361];
int mtooth;        // total teeth (incl. missing)
int ntooth;        // missing teeth
double angle;
double cycletime;  // average time per cycle
long totalsamp;    // Total number of samples
long tempsamp;     // tentative number of samples
long samplerat;	   // sample rate
double rpm;        // instantaneous rpm
long progress;     // file writing progress (0 to totalsamp)
int lowrpm;        // lower rpm of cycle
int upperrpm;      // upper rpm of cycle
double angledelta; // change in angle per smaple
double da;         // angle for one tooth (not including space)
int temp;          // temporary swap variable
double wavtime;    // file time

int icamsync;	//1 = no cam (crank on both channels, 2 = cam sync on right channel

void createHeader(char *fileName,   // name of the .wav file
		  short channels,   // number of channels
		  long samplerate,  // sample rate
		  short datasize,   // size of samples (bit res.)
		  long totalsa)     // length of the data 
{
                FILE *fp;
	MyWaveFormat *wf = (MyWaveFormat*)malloc(sizeof(MyWaveFormat));
	// set the fields of the header:
	memcpy(wf->riff , (const void *)"RIFF", 4);
	wf->filesize = sizeof(MyWaveFormat) - 8;
	memcpy(wf->wave, (const void *)"WAVE", 4);
	memcpy(wf->fmt, (const void *)"fmt ", 4);
	wf->chunkSize = 16;
	wf->wFormatTag = WAVE_FORMAT_PCM;
	wf->nChannels = channels;
	wf->nSamplesPerSec = samplerate;
	wf->nAvgBytesPerSec = samplerate*((datasize*channels)/8);
	wf->nBlockAlign = (datasize*channels)/8;
	wf->wBitsPerSample = datasize;
	memcpy(wf->data, (const void *)"data", 4);
	wf->length = totalsa;	

	// write the header to the file:
	fp = fopen(fileName,"wb"); rewind(fp);
	fwrite(wf,sizeof(*wf),1,fp);
	fclose(fp);	

}


double RampGen(double rangle)
{
	// Genrate a ramp file
	int j;
	double m, amp;

	// Reduce angle to less than 360 degrees
	angle = rangle;
	while(angle>=360) angle = angle - 360.;;

	j=0; // Tooth counter

	while(!(angle >= tooth[j] && angle < tooth[j+1]) && j != mtooth) j++;
		
	if(j == mtooth)
	{
		printf("error in finding array position\n");
		while(1);
	}

	if(angle < antitooth1[j])
	{
		// case 1: between tooth[j] and antitooth1[j]...
		m = 0.5/(antitooth1[j] - tooth[j]);
		amp = m * (angle - antitooth1[j]) + 1.;

		printf(" case 1: angle=%f m=%f amp=%f\n",angle,m,amp);	
	}

	if(angle >= antitooth1[j] && angle < antitooth2[j])
	{
		// case 2: betweee antitooth1[j] and antitooth2[j]...
		m = -1.0/(antitooth2[j] - antitooth1[j]);
		amp = m * (angle - antitooth2[j]);

		printf("   case 2: angle=%f m=%f amp=%f\n",angle,m,amp);	

	}
	if(angle >= antitooth2[j])
	{
		// case 3: antitooth2[j] and tooth[j+1]...
		m = 0.5/(tooth[j+1] - antitooth2[j]);
		amp = m * (angle - antitooth2[j]);

		printf("     case 3: angle=%f m=%f amp=%f\n",angle,m,amp);	
	
	}

	return(amp);

}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	// {{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	// }}AFX_VIRTUAL

// Implementation
protected:
//	{{AFX_MSG(CAboutDlg)
//	}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
//	afx_msg void OnBnClickedOk2();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
//	{{AFX_DATA_INIT(CAboutDlg)
//	}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
//	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrankWheelPulserDlg dialog


/////////////////////////////////////////////////////////////////////////////
// CCrankWheelPulserDlg dialog

CCrankWheelPulserDlg::CCrankWheelPulserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCrankWheelPulserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCrankWheelPulserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCrankWheelPulserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCrankWheelPulserDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCrankWheelPulserDlg, CDialog)
	//{{AFX_MSG_MAP(CCrankWheelPulserDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO_WHEEL, OnSelchangeComboWheel)
	ON_BN_CLICKED(IDC_BUTTON1, OnGenerateWAV)
	ON_BN_CLICKED(IDC_PLAYONCE, OnPlayonce)
	ON_BN_CLICKED(IDC_PLAYLOOP, OnPlayloop)
	ON_BN_CLICKED(IDC_STOPLOOP, OnStoploop)
	ON_CBN_SELCHANGE(IDC_COMBO_CAM, OnSelchangeComboCam)
	ON_BN_CLICKED(IDOK2, OnBnDoubleclickedOk2)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_CAM, OnEnChangeCam)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrankWheelPulserDlg message handlers

BOOL CCrankWheelPulserDlg::OnInitDialog()
{
	CString hhh;
	LPTSTR pp;

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);	    // Set big icon
	SetIcon(m_hIcon, TRUE);		// Set small icon (was false - LG)
	
	camdef = (CComboBox *) GetDlgItem(IDC_COMBO_CAM);
	camdef->AddString("No Cam Sync");
	camdef->AddString("Use Cam Sync");
	camdef->SetCurSel(0);

	sampleratepull = (CComboBox *) GetDlgItem(IDC_COMBO_SAMPLERATE);
	sampleratepull->AddString(" 8000");
	sampleratepull->AddString("11025");
	sampleratepull->AddString("16000");
	sampleratepull->AddString("22050");
	sampleratepull->AddString("44100");
	sampleratepull->AddString("48000");
	sampleratepull->SetCurSel(4);

	// rpmbox = (CEdit *) GetDlgItem(IDC_RPM); Original single rpm box

	// Get Upper and Lower RPM values from Dialog
	lowrpmbox   = (CEdit *) GetDlgItem(IDC_RPM);
	upperrpmbox = (CEdit *) GetDlgItem(IDC_RPM2);

	// RPM Input Validation

	// Force to integers
	lowrpm   = int(lowrpmbox);
	upperrpm = int(upperrpmbox);

	// If hirpm < lorpm, swap them
	if (lowrpm > upperrpm) {
		temp = lowrpm;
		lowrpm = upperrpm;
		upperrpm = temp;
	}

	// End RPM validation

	// Get M and N teeth from Dialog
	mtooth =  int((CEdit *) GetDlgItemInt(IDC_MTEETH)); //needs to be integer
	ntooth =  int((CEdit *) GetDlgItemInt(IDC_NTEETH)); //needs to be integer

		// Tooth Input Validation

//	pp = hhh.GetBuffer(10);
//	sprintf(pp,"1000");
//	hhh.ReleaseBuffer();
//	pp->SetWindowText(hhh);

	timebox = (CEdit *) GetDlgItem(IDC_TIME);
	pp = hhh.GetBuffer(10);
	sprintf(pp,"6");
	hhh.ReleaseBuffer();
	timebox->SetWindowText(hhh);

	camsyncbox = (CEdit *) GetDlgItem(IDC_CAM);
	pp = hhh.GetBuffer(10);
	sprintf(pp,"710");
	hhh.ReleaseBuffer();
	camsyncbox->SetWindowText(hhh);

	upperrpmbox = (CEdit *) GetDlgItem(IDC_RPM2);
	pp = hhh.GetBuffer(10);
	sprintf(pp,"5000");
	hhh.ReleaseBuffer();
	upperrpmbox->SetWindowText(hhh);

	lowrpmbox = (CEdit *) GetDlgItem(IDC_RPM);
	pp = hhh.GetBuffer(10);
	sprintf(pp,"800");
	hhh.ReleaseBuffer();
	lowrpmbox->SetWindowText(hhh);

	mteethbox = (CEdit *) GetDlgItem(IDC_MTEETH);
	pp = hhh.GetBuffer(10);
	sprintf(pp,"36");
	hhh.ReleaseBuffer();
	mteethbox->SetWindowText(hhh);

	nteethbox = (CEdit *) GetDlgItem(IDC_NTEETH);
	pp = hhh.GetBuffer(10);
	sprintf(pp,"1");
	hhh.ReleaseBuffer();
	nteethbox->SetWindowText(hhh);

	icamsync = 1;	// Crank sync only

	GetDlgItem(IDC_PLAYONCE)->EnableWindow(FALSE);
	GetDlgItem(IDC_PLAYLOOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOPLOOP)->EnableWindow(FALSE);

	GetDlgItem(IDC_CAM)->EnableWindow(FALSE);


	fullfilename[0] = 0;	// Null out

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCrankWheelPulserDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCrankWheelPulserDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CCrankWheelPulserDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCrankWheelPulserDlg::OnSelchangeComboWheel() 
{
	// TODO: Add your control notification handler code here
	
}

void CCrankWheelPulserDlg::OnGenerateWAV() 
{
	double r, amp;
	double camangle, camchk;
	unsigned char cbuf;
	FILE *fp;
	
	CString hhh;
	LPTSTR pp;
	float tmp1;
	int itmp1;

	PlaySound(NULL,NULL,SND_FILENAME | SND_ASYNC); //turn off sound (even if not playing...)

	GetDlgItem(IDC_PLAYONCE)->EnableWindow(FALSE);
	GetDlgItem(IDC_PLAYLOOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOPLOOP)->EnableWindow(FALSE);

	lowrpmbox->GetWindowText((CString &) hhh);
	pp = hhh.GetBuffer(20);
	sscanf(pp,"%f",&tmp1);
	hhh.ReleaseBuffer();
	rpm = (double) tmp1;
	
	itmp1 = sampleratepull->GetCurSel();
	switch(itmp1)
	{
	case 0:
		samplerat = 8000;
		break;

	case 1:
		samplerat = 11025;
		break;

	case 2:
		samplerat = 16000;
		break;

	case 3:
		samplerat = 22050;
		break;

	case 4:
		samplerat = 44100;
		break;

	case 5:
		samplerat = 48000;
		break;
	}

	timebox->GetWindowText((CString &) hhh);
	pp = hhh.GetBuffer(20);
	sscanf(pp,"%f",&tmp1);
	hhh.ReleaseBuffer();
	wavtime = (double) tmp1;

	// Total number of samples
	tempsamp = (long) (wavtime * samplerat * icamsync);

    // Original angledelta calc
	// angledelta = rpm * 6 / ((double) samplerat);

	camangle = 0.;
	if(icamsync == 2)
	{
		camsyncbox->GetWindowText((CString &) hhh);
		pp = hhh.GetBuffer(20);
		sscanf(pp,"%f",&tmp1);
		hhh.ReleaseBuffer();
		camchk = (double) tmp1;
	}


	CFileDialog dlgr(FALSE, "wav", "crankwheel.wav");
	if(dlgr.DoModal() == IDOK)
	{
		CString pathn = dlgr.GetPathName();
		sprintf(fullfilename,"%s",(LPCTSTR) pathn);
	}
	else
	{

		MessageBox("WAV file not generated.");
		fullfilename[0] = 0;	// Null out
		GetDlgItem(IDC_PLAYONCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_PLAYLOOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_STOPLOOP)->EnableWindow(FALSE);
	
		return;
	}

	// Set parameters from dialog
	lowrpm   = int(GetDlgItemInt(IDC_RPM));    //needs to be integer
	upperrpm = int(GetDlgItemInt(IDC_RPM2));   //needs to be integer
	mtooth   = int(GetDlgItemInt(IDC_MTEETH)); //needs to be integer
	ntooth   = int(GetDlgItemInt(IDC_NTEETH)); //needs to be integer

	// If missing teeth > total teeth, swap them
	if (ntooth > mtooth) {
		temp = mtooth;
		mtooth = ntooth;
		ntooth = temp;
		MessageBox("Using N-M for tooth count.");
	}

	if (mtooth == 0) { 
		mtooth = 1;
		MessageBox("Total teeth set to 1.");
	}
	
	// Set mtooth equal to number of teeth (*not* incl. missing teeth)
	mtooth = mtooth - ntooth;

	// For generalized M-N wheel
	da = 180./(double) (mtooth + ntooth); // the number of degrees per tooth (not space)

	for(itmp1=0; itmp1<(mtooth); itmp1++)
		// tooth[i] = position of tooth(i) in degrees
		tooth[itmp1] = (double)(itmp1 * (da * 2));

	tooth[mtooth] = 360.;
	progress = 0; // no progress yet
	
	// Set the 'antiteeth'
	for(itmp1=0; itmp1<=(mtooth); itmp1++)
	{
		if(itmp1 != 0)	antitooth2[itmp1-1] = (tooth[itmp1] - da/2);
		antitooth1[itmp1] = (tooth[itmp1] + da/2);
	}

	r = 0; // initial angle

	// Set totalsamp to interger number of cycles
	cycletime = 120/((double) (lowrpm + upperrpm)); // average cycle time (seconds)
	wavtime = cycletime * int ((wavtime/cycletime)+0.4);   // integer number of cycles
	totalsamp = (long) (wavtime * samplerat * icamsync);

	// Write WAV file header
	createHeader(fullfilename,icamsync,samplerat,8, totalsamp);
	fp = fopen(fullfilename,"ab");

	// We want this to end on at the completion of any missing teeth
	for(int i=0; i<totalsamp; i++)
	{
		// Each increment, [i], writes another sample

		// Variable rpm angledelta calculation
		if (progress < (0.5*totalsamp))
		rpm = lowrpm  + ((double)(upperrpm-lowrpm)*(2*progress/(double) totalsamp));
		if (progress >= (0.5*totalsamp))
		rpm = lowrpm  + ((double)(upperrpm-lowrpm)*(2*(totalsamp-progress)/(double) totalsamp));
		angledelta = rpm * 6 / ((double) samplerat);
		progress = progress+1; //increment progress
		amp = RampGen(r); // Generate the amplitude
		
		// Write the individual sample
		cbuf = (unsigned char)(amp * 255);
		fputc(cbuf, fp);
		
		// Write the cam sync
		if(icamsync == 2) // Generate CAM channel (right) if selected
		{
			camangle = camangle + angledelta;
			if(camangle >= 720.) camangle = camangle - 720.; // cam pulse width = 10 degrees
			cbuf = 0;// cam no pulse
			if(camangle > camchk && camangle < camchk + 10.) cbuf = 255;// cam pulse
			fputc(cbuf, fp);// cam
		}
		r = r + angledelta;
	}

	fclose(fp); // close the file

	GetDlgItem(IDC_PLAYONCE)->EnableWindow(TRUE);
	GetDlgItem(IDC_PLAYLOOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOPLOOP)->EnableWindow(TRUE);

}

void CCrankWheelPulserDlg::OnPlayonce() 
{

	if(!fullfilename[0])
	{
		CFileDialog dlgr(TRUE, "wav", "crankwheel.wav");
		if(dlgr.DoModal() == IDOK)
		{
			CString pathn = dlgr.GetPathName();
			sprintf(fullfilename,"%s",(LPCTSTR) pathn);
		}
		else
		{
			fullfilename[0] = 0;	// Null out
			return;
		}
	}

	
	PlaySound(fullfilename,NULL,SND_FILENAME | SND_ASYNC);
	
}

void CCrankWheelPulserDlg::OnPlayloop() 
{

	if(!fullfilename[0])
	{
		CFileDialog dlgr(TRUE, "wav", "crankwheel.wav");
		if(dlgr.DoModal() == IDOK)
		{
			CString pathn = dlgr.GetPathName();
			sprintf(fullfilename,"%s",(LPCTSTR) pathn);
		}
		else
		{
			fullfilename[0] = 0;	// Null out
			return;
		}
	}
	
	
	PlaySound(fullfilename,NULL,SND_FILENAME | SND_ASYNC | SND_LOOP);
	
}

void CCrankWheelPulserDlg::OnStoploop() 
{
	PlaySound(NULL,NULL,SND_FILENAME | SND_ASYNC);
	
}

void CCrankWheelPulserDlg::OnSelchangeComboCam() 
{
	// TODO: Add your control notification handler code here

	int i = camdef->GetCurSel();
	if(i == 0)
	{
		icamsync = 1;
		GetDlgItem(IDC_CAM)->EnableWindow(FALSE);
	}

	if(i == 1)
	{
		icamsync = 2;
		GetDlgItem(IDC_CAM)->EnableWindow(TRUE);
	}
	
}

void CCrankWheelPulserDlg::OnEnChangeRpm()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	//
	// This is the lower rpm to which the program cycles
}

void CCrankWheelPulserDlg::OnEnChangeRpm2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	//
	// This is the upper rpm to which the program cycles
}

void CCrankWheelPulserDlg::OnCbnSelchangeComboSamplerate()
{
	// TODO: Add your control notification handler code here
	// Set the sample rate
}

void CCrankWheelPulserDlg::OnEnChangeTime2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CCrankWheelPulserDlg::OnEnChangeTime()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	// Want to ensure time ends on a complete cycle
}


void CCrankWheelPulserDlg::OnBnClickedOk2()
{
	// Open the 'About' dialog
	// MessageBox("Open 'About' box");
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
}

void CCrankWheelPulserDlg::OnBnClickedOk()
{
	// Exit the main dialog
	OnOK();
}


void CCrankWheelPulserDlg::OnEnChangeNteeth()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	// Set N Teeth (In M-N configuration)
}

void CCrankWheelPulserDlg::OnEnChangeMteeth()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	// Set M Teeth (In M-N configuration)

}

void CAboutDlg::OnBnClickedOk()
{
	// Close About Box
	OnOK();
}


void CCrankWheelPulserDlg::OnBnDoubleclickedOk2()
{
	// Open the 'About' dialog
	// MessageBox("Open 'About' box");
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CCrankWheelPulserDlg::OnEnChangeCam()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
