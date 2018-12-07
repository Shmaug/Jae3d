#include "FileTree.hpp"

#include <shlwapi.h>

FileTree::FileTree(UDim2 pos, UDim2 size, std::function<void(jwstring)> clickFile) : UIControl(pos, size), mOnClickFile(clickFile) {}
FileTree::~FileTree() {}

void FileTree::Draw(HDC hdc, RECT &window, bool force) {
	if (!mDirty && !force) return;
	mDirty = false;

	RECT r = CalcRect(window);

	FillRect(hdc, &r, Brushes::bgDarkBrush);

	int pBg = SetBkMode(hdc, TRANSPARENT);
	HGDIOBJ pObj = SelectObject(hdc, Fonts::font11);
	int pTc = SetTextColor(hdc, RGB(241, 241, 241));

	TEXTMETRIC metric;
	GetTextMetrics(hdc, &metric);
	RECT textRect{ r.left + 15, r.top + 20 - metric.tmHeight - 4, r.right, r.top + 20 };

	for (int i = 0; i < mFiles.size(); i++) {
		if (mSelectedIndex == i) {
			RECT sr{ r.left, r.top + 20 * i, r.right, r.top + 20 * (i + 1) };
			FillRect(hdc, &sr, Brushes::selectedBrush);
		} else if (mHoveredIndex == i) {
			RECT sr{ r.left, r.top + 20 * i, r.right, r.top + 20 * (i + 1) };
			FillRect(hdc, &sr, Brushes::hvrBrush);
		}
		DrawTextW(hdc, mFiles[i].mName.c_str(), (int)mFiles[i].mName.length(), &textRect, DT_LEFT);
		textRect.top += 20;
		textRect.bottom += 20;
	}

	SetTextColor(hdc, pTc);
	SelectObject(hdc, pObj);
	SetBkMode(hdc, pBg);
}
void FileTree::Update(WPARAM wParam, RECT &window, InputState input) {
	bool dirty = mDirty;
	UIControl::Update(wParam, window, input);
	mDirty = dirty;

	size_t hi = mHoveredIndex;
	size_t si = mSelectedIndex;

	if (!mHovered) {
		mHoveredIndex = -1;
		if (input.lmb) mSelectedIndex = -1;
	} else {
		RECT r = CalcRect(window);
		UIControl::Update(wParam, window, input);
		for (int i = 0; i < mFiles.size(); i++) {
			if (input.cursor.y > r.top + 20 * i && input.cursor.y < r.top + 20 * (i + 1)) {
				mHoveredIndex = i;
				if (input.lmb)
					mSelectedIndex = i;
				if (input.clickCount == 2)
					mOnClickFile(mFiles[i].mPath);
				break;
			}
		}
	}

	if (si != mSelectedIndex || hi != mHoveredIndex)
		mDirty = true;
}

void FileTree::AddFolder(jwstring folder) {
	AddFolderRecursive(folder);
}
void FileTree::AddFile(jwstring file) {
	if (PathFileExistsW(file.c_str()) != 1) {
		wprintf(L"Could not find %s\n", file.c_str());
		return;
	}

	jwstring fullPath = GetFullPathW(file.c_str());
	bool e = false;
	for (int i = 0; i < mFiles.size(); i++)
		if (mFiles[i].mPath == fullPath) {
			e = true;
			break;
		}
	if (!e) {
		mFiles.push_back(File(fullPath));
		mDirty = true;
	}
	std::qsort(mFiles.data(), mFiles.size(), sizeof(File), [](const void* a, const void* b) {
		bool g = *(File*)a > *(File*)b;
		if (g) return 1;
		if (*(File*)a < *(File*)b)
			return -1;
		return 0;
	});
}

void FileTree::AddFolderRecursive(jwstring &path) {
	jwstring d = path + L"\\*";

	WIN32_FIND_DATAW ffd;
	HANDLE hFind = FindFirstFileW(d.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		wprintf(L"Failed to find directory %s\n", path.c_str());
		return;
	}

	wprintf(L"Adding folder %s\n", path.c_str());

	do {
		if (ffd.cFileName[0] == L'.') continue;

		jwstring c = path + L"\\" + jwstring(ffd.cFileName);
		if (GetExtW(c) == L"meta") continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			AddFolderRecursive(c);
		else
			AddFile(jwstring(c));
	} while (FindNextFileW(hFind, &ffd) != 0);

	FindClose(hFind);
}