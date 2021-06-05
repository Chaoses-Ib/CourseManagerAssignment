﻿// TeacherDlg.cpp : implementation file
//

#include "pch.h"
#include "CourseManager.h"
#include "TeacherDlg.h"
#include "afxdialogex.h"

#include "csv-parser.hpp"


// CTeacherDlg dialog

IMPLEMENT_DYNAMIC(CTeacherDlg, CDialogEx)

CTeacherDlg::CTeacherDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEACHER_DIALOG, pParent)
{
}

CTeacherDlg::~CTeacherDlg()
{
}

void CTeacherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
}


BEGIN_MESSAGE_MAP(CTeacherDlg, CDialogEx)
ON_BN_CLICKED(IDC_IMPORT, &CTeacherDlg::OnBnClickedImport)
END_MESSAGE_MAP()


// CTeacherDlg message handlers

BOOL CTeacherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_List.InsertColumn(0, L"学号", LVCFMT_LEFT, 90);
	m_List.InsertColumn(1, L"姓名", LVCFMT_LEFT, 90);
	m_List.InsertColumn(2, L"课程名称", LVCFMT_LEFT, 90);
	m_List.InsertColumn(3, L"成绩", LVCFMT_LEFT, 90);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CTeacherDlg::OnBnClickedImport()
{
	CFileDialog dlg(TRUE, L"csv", NULL, OFN_FILEMUSTEXIST, L"Text files (*.txt)|*.txt|CSV Files (*.csv)|*.csv", this);
	if (dlg.DoModal() == IDOK) {
		CString path = dlg.GetPathName();


	}
}