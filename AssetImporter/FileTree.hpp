#pragma once

#include "UIControl.hpp"

#include <jstring.hpp>
#include <jvector.hpp>

#include <IOUtil.hpp>

class FileTree : public UIControl {
public:
	FileTree(UDim2 pos, UDim2 size, std::function<void(jwstring)> clickFile);
	~FileTree();

	void Draw(HDC hdc, RECT &window, bool force = false);
	void Update(WPARAM wParam, RECT &window, InputState input);

	void AddFile(jwstring file);
	void AddFolder(jwstring folder);

private:
	struct File {
		jwstring mFilter;
		jwstring mPath;

		jwstring mName;
		jwstring mExt;
		
		File() {}
		File(const jwstring path) : mPath(path) { mExt = GetExtW(path); mName = GetNameExtW(path); }
		File(const File &f) : mPath(f.mPath), mFilter(f.mFilter), mName(f.mName), mExt(f.mExt) {}
		~File() {}

		const File& operator=(const File& rhs) { mPath = rhs.mPath; mFilter = rhs.mFilter; mName = rhs.mName; mExt = rhs.mExt; return *this; }
		bool operator==(const File& rhs) const { return mPath == rhs.mPath; }
		bool operator!=(const File& rhs) const { return mPath != rhs.mPath; }

		friend bool operator >(const File& lhs, const File& rhs) {
			if (lhs.mExt == rhs.mExt)
				return lhs.mName > rhs.mName; 
			return lhs.mExt > rhs.mExt;
		}
		friend bool operator >=(const File& lhs, const File& rhs) {
			if (lhs.mExt == rhs.mExt)
				return lhs.mName >= rhs.mName;
			return lhs.mExt >= rhs.mExt;
		}

		friend bool operator <(const File& lhs, const File& rhs) { return rhs > lhs; }
		friend bool operator <=(const File& lhs, const File& rhs) { return rhs >= lhs; }
	};
	jvector<File> mFiles;

	size_t mHoveredIndex;
	size_t mSelectedIndex;

	std::function<void(jwstring)> mOnClickFile;

	void AddFolderRecursive(jwstring &folder);
};


