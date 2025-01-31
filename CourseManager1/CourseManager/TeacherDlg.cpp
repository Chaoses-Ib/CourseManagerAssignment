﻿// TeacherDlg.cpp : implementation file
//

#include "pch.h"
#include "CourseManager.h"
#include "TeacherDlg.h"
#include "afxdialogex.h"

#include "Data.hpp"
#include <set>
#include <map>


// CTeacherDlg dialog

IMPLEMENT_DYNAMIC(CTeacherDlg, CDialogEx)

CTeacherDlg::CTeacherDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEACHER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_iEditList = -1;
}

CTeacherDlg::~CTeacherDlg()
{
}

void CTeacherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_COMBO_CLASS, m_comboClass);
	DDX_Control(pDX, IDC_EDIT_FIND, m_editFind);
	DDX_Control(pDX, IDC_LIST_STATISTICS, m_listStatistics);
	DDX_Control(pDX, IDC_EDIT_LIST, m_editList);
	DDX_Control(pDX, IDC_EDIT_SCORE, m_editScore);
}


BEGIN_MESSAGE_MAP(CTeacherDlg, CDialogEx)
ON_BN_CLICKED(IDC_IMPORT, &CTeacherDlg::OnBnClickedImport)
ON_CBN_SELCHANGE(IDC_COMBO_CLASS, &CTeacherDlg::OnCbnSelchangeComboClass)
ON_EN_KILLFOCUS(IDC_EDIT_SCORE, &CTeacherDlg::OnEnKillfocusEditScore)
ON_BN_CLICKED(IDCANCEL, &CTeacherDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDC_BUTTON_FIND, &CTeacherDlg::OnBnClickedButtonFind)
ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CTeacherDlg::OnNMDblclkList)
ON_EN_KILLFOCUS(IDC_EDIT_LIST, &CTeacherDlg::OnEnKillfocusEditList)
END_MESSAGE_MAP()

// CTeacherDlg message handlers

static int __stdcall CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM sort) {
	int result;
	switch (sort & ~0x80000000) {
	case 0: result = g_scores[lParam1].sid.Compare(g_scores[lParam2].sid); break;
	case 1: result = g_scores[lParam1].name.Compare(g_scores[lParam2].name); break;
	case 2: result = g_scores[lParam1].course.Compare(g_scores[lParam2].course); break;
	case 3: result = g_scores[lParam1].score - g_scores[lParam2].score; break;
	case 4: result = g_scores[lParam1].evaluation.score - g_scores[lParam2].evaluation.score; break;
	case 5: result = g_scores[lParam1].evaluation.comment.Compare(g_scores[lParam2].evaluation.comment); break;
	default:
		assert(false);
	}
	return sort & 0x80000000 ? -result : result;  //down : up
}

BOOL CTeacherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_List.CompareFunc = CompareFunc;

	m_List.InsertColumn(0, L"学号", LVCFMT_LEFT, 90);
	m_List.InsertColumn(1, L"姓名", LVCFMT_LEFT, 90);
	m_List.InsertColumn(2, L"课程名称", LVCFMT_LEFT, 200);
	m_List.InsertColumn(3, L"成绩", LVCFMT_LEFT, 90);
	m_List.InsertColumn(4, L"评分", LVCFMT_LEFT, 40);
	m_List.InsertColumn(5, L"评语", LVCFMT_LEFT, 200);
	m_editList.SetParent(&m_List);


	m_listStatistics.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	m_listStatistics.InsertColumn(0, L"课程名称", LVCFMT_LEFT, 200);
	m_listStatistics.InsertColumn(1, L"平均分", LVCFMT_LEFT, 60);
	m_listStatistics.InsertColumn(2, L"标准差", LVCFMT_LEFT, 60);
	m_listStatistics.InsertColumn(3, L"及格率", LVCFMT_LEFT, 60);


	RefreshScores(true);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CTeacherDlg::OnBnClickedImport()
{
	ScoreDataFile file;
	vector<Score> import_scores = file.OpenDlg(this);
	if (import_scores.empty())
		return;
	for (Score& score : import_scores)
		g_scores.v.push_back(std::move(score));
	m_modified = true;

	RefreshScores(true);
}

bool CTeacherDlg::ForeachFilteredScore(std::function<void(size_t i, Score& score, bool& stop)> f) {
	//[min, max)
	int score_min = 0;
	int score_max = 0x7FFFFFFF;

	CString buf;
	m_editScore.GetWindowTextW(buf);
	if (!buf.IsEmpty()) {
		std::wstringstream ss(buf.GetString());
		if (!(ss >> score_min && ss.ignore() && ss >> score_max)) {
			//#TODO
		}
	}

	bool stop = false;
	int sel = m_comboClass.GetCurSel();
	if (sel == 0 || sel == -1) {
		for (size_t i = 0; i < g_scores.v.size(); i++) {
			Score& score = g_scores[i];
			if (score_min <= score.score && score.score < score_max) {
				f(i, score, stop);
				if (stop)
					break;
			}
		}
	}
	else {
		CString classid;
		m_comboClass.GetLBText(sel, classid);
		for (size_t i = 0; i < g_scores.v.size(); i++) {
			if (g_students.find_sid(g_scores[i].sid)->classid == classid) {
				Score& score = g_scores[i];
				if (score_min <= score.score && score.score < score_max) {
					f(i, score, stop);
					if (stop)
						break;
				}
			}
		}
	}
	return stop;
}

void CTeacherDlg::OnCbnSelchangeComboClass()
{
	RefreshScores(false);
}

void CTeacherDlg::OnEnKillfocusEditScore()
{
	RefreshScores(false);
}

void CTeacherDlg::RefreshScores(bool refresh_classes)
{
	m_List.DeleteAllItems();

	static std::set<std::wstring> classes;
	int item = 0;
	ForeachFilteredScore([this, &item, refresh_classes](size_t i, Score& score, bool& stop) {
		m_List.InsertItem(LVIF_TEXT | LVIF_PARAM, item, score.sid, 0, 0, 0, i);  //nItem can't be -1
		m_List.SetItem(item, 1, LVIF_TEXT, score.name, 0, 0, 0, 0);
		m_List.SetItem(item, 2, LVIF_TEXT, score.course, 0, 0, 0, 0);
		CString text;
		text.Format(L"%d", score.score);  //#shit
		m_List.SetItem(item, 3, LVIF_TEXT, text, 0, 0, 0, 0);
		if (score.evaluation.evaluated) {
			text.Format(L"%d", score.evaluation.score);
			m_List.SetItem(item, 4, LVIF_TEXT, text, 0, 0, 0, 0);
			m_List.SetItem(item, 5, LVIF_TEXT, score.evaluation.comment, 0, 0, 0, 0);
		}
		item++;

		if (refresh_classes)
			classes.emplace(g_students.find_sid(score.sid)->classid);  //#todo
	});
	m_List.RefreshSort();

	if (refresh_classes) {
		m_comboClass.ResetContent();  //#shit name
		m_comboClass.AddString(L"全部");
		for (std::wstring classid : classes) {
			m_comboClass.AddString(classid.c_str());
		}
		m_comboClass.SetCurSel(0);
	}

	RefreshStatistics();
}


void CTeacherDlg::OnBnClickedCancel()
{
	if (m_modified) {
		int button = MessageBoxW(L"是否保存更改？", L"询问", MB_YESNOCANCEL | MB_ICONQUESTION);
		switch (button) {
		default:
			assert(false); [[fallthrough]];
		case IDCANCEL:
			return;
			break;
		case IDYES:
			g_scores.file.SaveFile(LR"(.\data\score.txt)", g_scores.v); [[fallthrough]];
		case IDNO: CDialogEx::OnCancel();
		}
	}
	else {
		CDialogEx::OnCancel();
	}
}

void CTeacherDlg::OnBnClickedButtonFind()
{
	CString find;
	m_editFind.GetWindowTextW(find);
	if (find.IsEmpty())
		return;

	int item = -1;
	if (L'0' <= find[0] && find[0] <= '9') {  //sid
		LVFINDINFOW info;
		info.flags = LVFI_STRING;
		info.psz = find;
		item = m_List.FindItem(&info);  //only first column
	}
	else {  //name
		if (ForeachFilteredScore([&find, &item](size_t i, Score& score, bool& stop) {
			if (score.name == find) {
				item = i;
				stop = true;
			}
		})) {
			LVFINDINFOW info;
			info.flags = LVFI_PARAM;
			info.lParam = item;
			item = m_List.FindItem(&info);
		}
	}

	if (item == -1)
		return;
	m_List.SetFocus();  //otherwise the selected item is in grey
	m_List.SetItemState(item, LVIS_SELECTED, LVIS_SELECTED);
	m_List.EnsureVisible(item, false);
}

struct CStringHash
{
	std::size_t operator()(CString const& s) const noexcept
	{
		return std::hash<std::wstring>{}(s.GetString());
	}
};

void CTeacherDlg::RefreshStatistics()
{
	struct Statistics {
		uint32_t sum;
		int pass;
		int n;
		double avg;
		double standard_deviation;
	};

	std::unordered_map<CString, Statistics, CStringHash> map;
	ForeachFilteredScore([&map](size_t i, Score& score, bool& stop) {
		Statistics& sta = map[score.course];
		++sta.n;
		sta.sum += score.score;
		if (score.score >= 60)
			++sta.pass;
	});
	for (auto& [course, sta] : map) {
		sta.avg = (double)sta.sum / sta.n;
	}
	ForeachFilteredScore([&map](size_t i, Score& score, bool& stop) {
		Statistics& sta = map[score.course];
		sta.standard_deviation += pow(score.score - sta.avg, 2);
	});

	m_listStatistics.DeleteAllItems();
	int i = 0;
	for (auto& [course, sta] : map) {
		sta.standard_deviation = sqrt(sta.standard_deviation / sta.n);

		m_listStatistics.InsertItem(LVIF_TEXT, i, course, 0, 0, 0, 0);
		CString buf;
		buf.Format(L"%.1f", sta.avg);
		m_listStatistics.SetItem(i, 1, LVIF_TEXT, buf, 0, 0, 0, 0);
		buf.Format(L"%.1f", sta.standard_deviation);
		m_listStatistics.SetItem(i, 2, LVIF_TEXT, buf, 0, 0, 0, 0);
		buf.Format(L"%.1f%%", 100.0 * sta.pass / sta.n);
		m_listStatistics.SetItem(i, 3, LVIF_TEXT, buf, 0, 0, 0, 0);
		++i;
	}
}

void CTeacherDlg::ShowEditList(int item)
{
	CRect rect;
	m_List.GetSubItemRect(item, 3, LVIR_LABEL, rect);
	m_editList.MoveWindow(&rect);
	m_editList.SetWindowTextW(m_List.GetItemText(item, 3));
	m_editList.SetSel(-1);
	m_editList.ShowWindow(SW_SHOW);
	m_editList.SetFocus();

	m_iEditList = item;

}

void CTeacherDlg::OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NMLISTVIEW* list = (NMLISTVIEW*)pNMHDR;

	if (m_iEditList == -1) {
		if (list->iSubItem == 3) {
			ShowEditList(list->iItem);
		}
	}

	*pResult = 0;
}

void CTeacherDlg::OnEnKillfocusEditList()
{
	if (m_iEditList == -1) {
		//#TODO
		return;
	}
	CString text;
	m_editList.GetWindowTextW(text);

	std::wstringstream ss(text.GetString());
	ss >> g_scores[m_iEditList].score;
	m_modified = true;

	text.Format(L"%d", g_scores[m_iEditList].score);
	m_List.SetItemText(m_iEditList, 3, text);

	m_editList.ShowWindow(SW_HIDE);
	m_iEditList = -1;
}

BOOL CTeacherDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		if (GetFocus() == &m_editList) {
			OnEnKillfocusEditList();
		}
		else if (GetFocus() == &m_List) {
			POSITION pos = m_List.GetFirstSelectedItemPosition();
			if (pos != NULL) {
				ShowEditList(m_List.GetNextSelectedItem(pos));
			}
		}
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}