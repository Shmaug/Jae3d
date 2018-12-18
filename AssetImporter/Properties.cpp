#include "Properties.hpp"

#include <Mesh.hpp>
#include <Texture.hpp>
#include <Shader.hpp>
#include <Font.hpp>
#include <AssetFile.hpp>

#include "ImportCommon.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

size_t mHoveredIndex;
size_t mSelectedIndex;

Properties::Properties(UDim2 pos, UDim2 size, std::function<void()> reload) : UIControl(pos, size), reload(reload) {}
Properties::~Properties() {}

template<class Tup, class Func, std::size_t ...Is>
constexpr void static_for_impl(Tup&& t, Func &&f, std::index_sequence<Is...>) {
	(f(std::integral_constant<std::size_t, Is>{}, std::get<Is>(t)), ...);
}
template<class ... T, class Func >
constexpr void static_for(std::tuple<T...>&t, Func &&f) {
	static_for_impl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)>{});
}

template<typename T>
class UpdateArgs {
public:
	union Args {
		HDC hdc;
		InputState input;
		Args(HDC hdc) : hdc(hdc) {}
		Args(InputState input) : input(input) {}
	};

	jwstring name;
	T* field;
	RECT &rect;
	Args args;
	bool draw;
	bool* dirty;

	UpdateArgs(bool* dirty, jwstring name, T* field, RECT &rect, HDC hdc) : dirty(dirty), name(name), field(field), rect(rect), args(hdc), draw(true) {}
	UpdateArgs(bool* dirty, jwstring name, T* field, RECT &rect, InputState input) : dirty(dirty), name(name), field(field), rect(rect), args(input), draw(false) {}
};

template<typename T>
void UpdateField(UpdateArgs<T> &args) {
	if (args.draw)
		DrawTextW(args.hdc, name.c_str(), (int)name.length(), &args.rect, DT_LEFT);
}
template<>
void UpdateField(UpdateArgs<bool> &args) {
	if (args.draw) DrawTextW(args.args.hdc, args.name.c_str(), (int)args.name.length(), &args.rect, DT_LEFT);

	RECT r = { args.rect.right - 24, args.rect.top + 1, args.rect.right - 10, args.rect.bottom - 1 };
	if (args.draw) {
		FillRect(args.args.hdc, &r, Brushes::bgBrush);
		if (*args.field) DrawTextW(args.args.hdc, L"x", 1, &r, DT_CENTER | DT_VCENTER);
	} else {
		if (args.args.input.lmbFirst) {
			*args.field = !*args.field;
			*args.dirty = true;
		}
	}
}
template<>
void UpdateField(UpdateArgs<unsigned int> &args) {
	if (args.draw)
		DrawTextW(args.args.hdc, args.name.c_str(), (int)args.name.length(), &args.rect, DT_LEFT);

	RECT r = { args.rect.right - 64, args.rect.top + 1, args.rect.right - 10, args.rect.bottom - 1 };

	if (args.draw) {
		FillRect(args.args.hdc, &r, Brushes::bgBrush);

		r.left += 2;
		r.right -= 2;
		r.top++;
		r.bottom--;

		wchar_t buf[64];
		_itow_s(*args.field, buf, 64, 10);
		DrawTextW(args.args.hdc, buf, (int)wcsnlen_s(buf, 64), &r, DT_RIGHT | DT_VCENTER);
	}
}
template<>
void UpdateField(UpdateArgs<DXGI_FORMAT> &args) {
	DrawTextW(args.args.hdc, args.name.c_str(), (int)args.name.length(), &args.rect, DT_LEFT);

	RECT r = { args.rect.right - 150, args.rect.top + 1, args.rect.right - 10, args.rect.bottom - 1 };

	if (args.draw) {
		FillRect(args.args.hdc, &r, Brushes::bgBrush);

		r.left += 2;
		r.right -= 2;
		r.top++;
		r.bottom--;

		jwstring fmt = FormatToString.at(*args.field).upper();
		DrawTextW(args.args.hdc, fmt.c_str(), (int)fmt.length(), &r, DT_CENTER | DT_VCENTER);
	} else {
		if (args.args.input.lmbFirst) {
			// TODO enum picker
		}
	}
}
template<>
void UpdateField(UpdateArgs<ALPHA_MODE> &args) {
	if (args.draw) DrawTextW(args.args.hdc, args.name.c_str(), (int)args.name.length(), &args.rect, DT_LEFT);

	RECT r = { args.rect.right - 150, args.rect.top + 1, args.rect.right - 10, args.rect.bottom - 1 };

	if (args.draw) {
		FillRect(args.args.hdc, &r, Brushes::bgBrush);
		r.left += 2;
		r.right -= 2;
		r.top++;
		r.bottom--;

		jwstring fmt = AlphaModeToString.at(*args.field).upper();
		DrawTextW(args.args.hdc, fmt.c_str(), (int)fmt.length(), &r, DT_CENTER | DT_VCENTER);
	} else {
		if (args.args.input.lmbFirst) {
			// TODO enum picker
		}
	}
}

void Properties::Draw(HDC hdc, RECT &window, bool force) {
	if (!mDirty && !force) return;
	mDirty = false;

	RECT r = CalcRect(window);

	FillRect(hdc, &r, Brushes::bgDarkBrush);

	if (shownAsset.asset) {
		int pBg = SetBkMode(hdc, TRANSPARENT);
		HGDIOBJ pObj = SelectObject(hdc, Fonts::font11);
		int pTc = SetTextColor(hdc, RGB(241, 241, 241));

		TEXTMETRIC metric;
		GetTextMetrics(hdc, &metric);
		RECT textRect{ r.left + 15, r.top + 35 - metric.tmHeight - 4, r.right, r.top + 35 };

		auto fields = shownAsset.GetFields();
		static_for(fields, [&](auto i, auto w) {
			if (w.Value()) {
				UpdateArgs ua(&mDirty, w.Name(), w.Value(), textRect, hdc);
				UpdateField(ua);

				textRect.top += 20;
				textRect.bottom += 20;
			}
		});

		SetTextColor(hdc, pTc);
		SelectObject(hdc, pObj);
		SetBkMode(hdc, pBg);
	}
}
void Properties::Update(WPARAM wParam, RECT &window, InputState input) {
	UIControl::Update(wParam, window, input);

	RECT r = CalcRect(window);
	r.top += 10;

	if (mHovered) {
		auto fields = shownAsset.GetFields();
		static_for(fields, [&](auto i, auto w) {
			if (w.Value()) {
				if (input.cursor.y > r.top + 20 * i && input.cursor.y < r.top + 20 * (i + 1)) {
					UpdateArgs ua(&mDirty, w.Name(), w.Value(), r, input);
					UpdateField(ua);
				}
			}
		});
	}
}

void Properties::Show(AssetMetadata &asset) {
	shownAsset = asset;
}