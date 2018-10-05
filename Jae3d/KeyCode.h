#pragma once

namespace KeyCode {
	// Key code values for Windows
	enum Key {
		None = 0x00, // No key was pressed
		LButton = 0x01, // Left mouse button
		RButton = 0x02, // Right mouse button
		Cancel = 0x03, // Cancel key
		MButton = 0x04, // Middle mouse button
		XButton1 = 0x05, // X1 mouse button
		XButton2 = 0x06, // X2 mouse button
		// 0x07 is undefined
		Back = 0x08,
		Tab = 0x09,
		// 0x0A-0B are reserved
		Clear = 0x0c, // The CLEAR key
		Enter = 0x0d, // The Enter key
		// 0x0E-0F are undefined
		ShiftKey = 0x10, // The Shift key
		ControlKey = 0x11, // The Ctrl key
		AltKey = 0x12, // The Alt key
		Pause = 0x13, // The Pause key
		Capital = 0x14, // The Caps Lock key
		CapsLock = 0x14, // The Caps Lock key
		KanaMode = 0x15, // IMI Kana mode
		HanguelMode = 0x15, // IMI Hanguel mode (Use HangulMode)
		HangulMode = 0x15, // IMI Hangul mode
		// 0x16 is undefined
		JunjaMode = 0x17, // IMI Janja mode
		FinalMode = 0x18, // IMI Final mode
		HanjaMode = 0x19, // IMI Hanja mode
		KanjiMode = 0x19, // IMI Kanji mode
		// 0x1A is undefined
		Escape = 0x1b, // The ESC key
		IMEConvert = 0x1c, // IMI convert key
		IMINoConvert = 0x1d, // IMI noconvert key
		IMEAccept = 0x1e, // IMI accept key
		IMIModeChange = 0x1f, // IMI mode change key
		Space = 0x20, // The Space key
		Prior = 0x21, // The Page Up key
		PageUp = 0x21, // The Page Up key
		Next = 0x22, // The Page Down key
		PageDown = 0x22, // The Page Down key
		End = 0x23, // The End key
		Home = 0x24, // The Home key
		Left = 0x25, // The Left arrow key
		Up = 0x26, // The Up arrow key
		Right = 0x27, // The Right arrow key
		Down = 0x28, // The Down arrow key
		Select = 0x29, // The Select key
		Print = 0x2a, // The Print key
		Execute = 0x2b, // The Execute key
		PrintScreen = 0x2c, // The Print Screen key
		Snapshot = 0x2c, // The Print Screen key
		Insert = 0x2d, // The Insert key
		Delete = 0x2e, // The Delete key
		Help = 0x2f, // The Help key
		D0 = 0x30, // 0
		D1 = 0x31, // 1
		D2 = 0x32, // 2
		D3 = 0x33, // 3
		D4 = 0x34, // 4
		D5 = 0x35, // 5
		D6 = 0x36, // 6
		D7 = 0x37, // 7
		D8 = 0x38, // 8
		D9 = 0x39, // 9
		// 0x3A - 40 are undefined
		A = 0x41, // A
		B = 0x42, // B
		C = 0x43, // C
		D = 0x44, // D
		E = 0x45, // E
		F = 0x46, // F
		G = 0x47, // G
		H = 0x48, // H
		I = 0x49, // I
		J = 0x4a, // J
		K = 0x4b, // K
		L = 0x4c, // L
		M = 0x4d, // M
		N = 0x4e, // N
		O = 0x4f, // O
		P = 0x50, // P
		Q = 0x51, // Q
		R = 0x52, // R
		S = 0x53, // S
		T = 0x54, // T
		U = 0x55, // U
		V = 0x56, // V
		W = 0x57, // W
		X = 0x58, // X
		Y = 0x59, // Y
		Z = 0x5a, // Z
		LWin = 0x5b, // Left Windows key
		RWin = 0x5c, // Right Windows key
		Apps = 0x5d, // Apps key
		// 0x5E is reserved
		Sleep = 0x5f, // The Sleep key
		NumPad0 = 0x60, // The Numeric keypad 0 key
		NumPad1 = 0x61, // The Numeric keypad 1 key
		NumPad2 = 0x62, // The Numeric keypad 2 key
		NumPad3 = 0x63, // The Numeric keypad 3 key
		NumPad4 = 0x64, // The Numeric keypad 4 key
		NumPad5 = 0x65, // The Numeric keypad 5 key
		NumPad6 = 0x66, // The Numeric keypad 6 key
		NumPad7 = 0x67, // The Numeric keypad 7 key
		NumPad8 = 0x68, // The Numeric keypad 8 key
		NumPad9 = 0x69, // The Numeric keypad 9 key
		Multiply = 0x6a, // The Multiply key
		Add = 0x6b, // The Add key
		Separator = 0x6c, // The Separator key
		Subtract = 0x6d, // The Subtract key
		Decimal = 0x6e, // The Decimal key
		Divide = 0x6f, // The Divide key
		F1 = 0x70, // The F1 key
		F2 = 0x71, // The F2 key
		F3 = 0x72, // The F3 key
		F4 = 0x73, // The F4 key
		F5 = 0x74, // The F5 key
		F6 = 0x75, // The F6 key
		F7 = 0x76, // The F7 key
		F8 = 0x77, // The F8 key
		F9 = 0x78, // The F9 key
		F10 = 0x79, // The F10 key
		F11 = 0x7a, // The F11 key
		F12 = 0x7b, // The F12 key
		F13 = 0x7c, // The F13 key
		F14 = 0x7d, // The F14 key
		F15 = 0x7e, // The F15 key
		F16 = 0x7f, // The F16 key
		F17 = 0x80, // The F17 key
		F18 = 0x81, // The F18 key
		F19 = 0x82, // The F19 key
		F20 = 0x83, // The F20 key
		F21 = 0x84, // The F21 key
		F22 = 0x85, // The F22 key
		F23 = 0x86, // The F23 key
		F24 = 0x87, // The F24 key
		// 0x88 - 0x8f are unassigned
		NumLock = 0x90, // The Num Lock key
		ScrollLock = 0x91, // The Scroll Lock key
		// 0x92 - 96 are OEM specific
		// 0x97 - 9f are unassigned
		LShiftKey = 0xa0, // The Left Shift key
		RShiftKey = 0xa1, // The Right Shift key
		LControlKey = 0xa2, // The Left Control key
		RControlKey = 0xa3, // The Right Control key
		LMenu = 0xa4, // The Left Alt key
		RMenu = 0xa5, // The Right Alt key
		BrowserBack = 0xa6, // The Browser Back key
		BrowserForward = 0xa7, // The Browser Forward key
		BrowserRefresh = 0xa8, // The Browser Refresh key
		BrowserStop = 0xa9, // The Browser Stop key
		BrowserSearch = 0xaa, // The Browser Search key
		BrowserFavorites = 0xab, // The Browser Favorites key
		BrowserHome = 0xac, // The Browser Home key
		VolumeMute = 0xad, // The Volume Mute key
		VolumeDown = 0xae, // The Volume Down key
		VolumeUp = 0xaf, // The Volume Up key
		MediaNextTrack = 0xb0, // The Next Track key
		MediaPreviousTrack = 0xb1, // The Previous Track key
		MediaStop = 0xb2, // The Stop Media key
		MediaPlayPause = 0xb3, // The Play/Pause Media key
		LaunchMail = 0xb4, // The Start Mail key
		SelectMedia = 0xb5, // The Select Media key
		LaunchApplication1 = 0xb6, // The Launch Application 1 key.
		LaunchApplication2 = 0xb7, // The Launch Application 2 key.
		// 0xB8 - B9 are reserved
		OemSemicolon = 0xba, // Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
		Oem1 = 0xba, // Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
		OemPlus = 0xbb, // For any country/region, the '+' key
		OemComma = 0xbc, // For any country/region, the ',' key
		OemMinus = 0xbd, // For any country/region, the '-' key
		OemPeriod = 0xbe, // For any country/region, the '.' key
		OemQuestion = 0xbf, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
		Oem2 = 0xbf, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
		OemTilde = 0xc0, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
		Oem3 = 0xc0, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
		// 0xC1 - D7 are reserved
		// 0xD8 - DA are unassigned
		OemOpenBrackets = 0xdb, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
		Oem4 = 0xdb, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
		OemPipe = 0xdc, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
		Oem5 = 0xdc, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
		OemCloseBrackets = 0xdd, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
		Oem6 = 0xdd, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
		OemQuotes = 0xde, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
		Oem7 = 0xde, // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
		Oem8 = 0xdf, // Used for miscellaneous characters; it can vary by keyboard.
		// 0xE0 is reserved
		// 0xE1 is OEM specific
		OemBackslash = 0xe2, // Either the angle bracket key or the backslash key on the RT 102-key keyboard
		Oem102 = 0xe2, // Either the angle bracket key or the backslash key on the RT 102-key keyboard
		// 0xE3 - E4 OEM specific
		ProcessKey = 0xe5, // IME Process key
		// 0xE6 is OEM specific
		Packet = 0xe7, // Used to pass Unicode characters as if they were keystrokes. The Packet key value is the low word of a 32-bit virtual-key value used for non-keyboard input methods.
		// 0xE8 is unassigned
		// 0xE9 - F5 OEM specific
		Attn = 0xf6, // The Attn key
		CrSel = 0xf7, // The CrSel key
		ExSel = 0xf8, // The ExSel key
		EraseEof = 0xf9, // The Erase EOF key
		Play = 0xfa, // The Play key
		Zoom = 0xfb, // The Zoom key
		NoName = 0xfc, // Reserved
		Pa1 = 0xfd, // The PA1 key
		OemClear = 0xfe, // The Clear key
	};

	const char* const key_str[] = {
		"None", // 0x00 No key
		"LButton", // 0x01
		"RButton", // 0x02
		"Cancel", // 0x03
		"MButton", // 0x04
		"XButton1", // 0x05
		"XButton2", // 0x06
		"0x07", // undefined
		"Back", // 0x08,
		"Tab", // 0x09,
		"0x0A", // reserved
		"0x0B",// reserved
		"Clear", // 0x0c
		"Enter", // 0x0d
		"0x0E", // undefined
		"0x0F", // undefined
		"ShiftKey", // 0x10
		"ControlKey", // 0x11
		"AltKey", // 0x12,
		"Pause", // 0x13,
		"CapsLock", // 0x14
		"KanaMode", // 0x15 IMI Kana mode / IMI Hangul mode / IMI Hanguel mode (Use HangulMode)
		"0x16", // undefined
		"JunjaMode", // 0x17 IMI Janja mode
		"FinalMode", // 0x18 IMI Final mode
		"HanjaMode", // 0x19 IMI Hanja/Kanji mode
		"0x1A", // undefined
		"Escape", // 0x1b
		"IMEConvert", // 0x1c IMI convert key
		"IMINoConvert", // 0x1d IMI noconvert key
		"IMEAccept", // 0x1e IMI accept key
		"IMIModeChange", // 0x1f IMI mode change key
		"Space", // 0x20
		"PageUp", // 0x21
		"PageDown", // 0x22
		"End", // 0x23
		"Home", // 0x24,
		"Left", // 0x25
		"Up", // 0x26
		"Right", // 0x27
		"Down", // 0x28
		"Select", // 0x29
		"Print", // 0x2a
		"Execute", // 0x2b
		"PrintScreen", // 0x2c
		"Insert", // 0x2d
		"Delete", // 0x2e
		"Help", // 0x2f
		"D0", // 0x30
		"D1", // 0x31
		"D2", // 0x32
		"D3", // 0x33
		"D4", // 0x34
		"D5", // 0x35
		"D6", // 0x36
		"D7", // 0x37
		"D8", // 0x38
		"D9", // 0x39
		"0x3A", // undefined
		"0x3B", // undefined
		"0x3C", // undefined
		"0x3D", // undefined
		"0x3E", // undefined
		"0x3F", // undefined
		"0x40", // undefined
		"A", // 0x41
		"B", // 0x42
		"C", // 0x43
		"D", // 0x44
		"E", // 0x45
		"F", // 0x46
		"G", // 0x47
		"H", // 0x48
		"I", // 0x49
		"J", // 0x4a
		"K", // 0x4b
		"L", // 0x4c
		"M", // 0x4d
		"N", // 0x4e
		"O", // 0x4f
		"P", // 0x50
		"Q", // 0x51
		"R", // 0x52
		"S", // 0x53
		"T", // 0x54
		"U", // 0x55
		"V", // 0x56
		"W", // 0x57
		"X", // 0x58
		"Y", // 0x59
		"Z", // 0x5a
		"LWin", // 0x5b,
		"RWin", // 0x5c,
		"Apps", // 0x5d,
		"0x5E", // 0x5E is reserved
		"Sleep", // 0x5f
		"NumPad0", // 0x60
		"NumPad1", // 0x61
		"NumPad2", // 0x62
		"NumPad3", // 0x63
		"NumPad4", // 0x64
		"NumPad5", // 0x65
		"NumPad6", // 0x66
		"NumPad7", // 0x67
		"NumPad8", // 0x68
		"NumPad9", // 0x69
		"Multiply", // 0x6a
		"Add", // 0x6b,
		"Separator", // 0x6c
		"Subtract", // 0x6d
		"Decimal", // 0x6e
		"Divide", // 0x6f
		"F1", // 0x70
		"F2", // 0x71
		"F3", // 0x72
		"F4", // 0x73
		"F5", // 0x74
		"F6", // 0x75
		"F7", // 0x76
		"F8", // 0x77
		"F9", // 0x78
		"F10", // 0x79
		"F11", // 0x7a
		"F12", // 0x7b
		"F13", // 0x7c
		"F14", // 0x7d
		"F15", // 0x7e
		"F16", // 0x7f
		"F17", // 0x80
		"F18", // 0x81
		"F19", // 0x82
		"F20", // 0x83
		"F21", // 0x84
		"F22", // 0x85
		"F23", // 0x86
		"F24", // 0x87
		"0x88", // unassigned
		"0x89", // unassigned
		"0x8a", // unassigned
		"0x8b", // unassigned
		"0x8c", // unassigned
		"0x8d", // unassigned
		"0x8e", // unassigned
		"0x8f", // unassigned
		"NumLock", // 0x90
		"ScrollLock", // 0x91
		"0x92", // OEM specific
		"0x93", // OEM specific
		"0x94", // OEM specific
		"0x95", // OEM specific
		"0x96", // OEM specific
		"0x97", // unassigned
		"0x98", // unassigned
		"0x99", // unassigned
		"0x9a", // unassigned
		"0x9b", // unassigned
		"0x9c", // unassigned
		"0x9d", // unassigned
		"0x9e", // unassigned
		"0x9f", // unassigned
		"LShiftKey", // 0xa0 The Left Shift key
		"RShiftKey", // 0xa1 The Right Shift key
		"LControlKey", // 0xa2, The Left Control key
		"RControlKey", // 0xa3, The Right Control key
		"LMenu", // 0xa4 The Left Alt key
		"RMenu", // 0xa5 The Right Alt key
		"BrowserBack", // 0xa6 The Browser Back key
		"BrowserForward", // 0xa7 The Browser Forward key
		"BrowserRefresh", // 0xa8 The Browser Refresh key
		"BrowserStop", // 0xa9 The Browser Stop key
		"BrowserSearch", // 0xaa The Browser Search key
		"BrowserFavorites", // 0xab The Browser Favorites key
		"BrowserHome", // 0xac The Browser Home key
		"VolumeMute", // 0xad The Volume Mute key
		"VolumeDown", // 0xae The Volume Down key
		"VolumeUp", // 0xaf The Volume Up key
		"MediaNextTrack", // 0xb0 The Next Track key
		"MediaPreviousTrack", // 0xb1 The Previous Track key
		"MediaStop", // 0xb2 The Stop Media key
		"MediaPlayPause", // 0xb3 The Play/Pause Media key
		"LaunchMail", // 0xb4 The Start Mail key
		"SelectMedia", // 0xb5 The Select Media key
		"LaunchApplication1", // 0xb6 The Launch Application 1 key.
		"LaunchApplication2", // 0xb7 The Launch Application 2 key.
		"0xB8", // reserved
		"0xB9", // reserved
		"OemSemicolon", // 0xba Used for miscellaneous characters; it can vary by keyboard.  For the US standard keyboard, the ';:' key
		"OemPlus", // 0xbb For any country/region, the '+' key
		"OemComma", // 0xbc For any country/region, the ',' key
		"OemMinus", // 0xbd For any country/region, the '-' key
		"OemPeriod", // 0xbe For any country/region, the '.' key
		"OemQuestion", // 0xbf Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
		"OemTilde", // 0xc0 Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
		"0xC1", // reserved
		"0xC2", // reserved
		"0xC3", // reserved
		"0xC4", // reserved
		"0xC5", // reserved
		"0xC6", // reserved
		"0xC7", // reserved
		"0xC8", // reserved
		"0xC9", // reserved
		"0xCa", // reserved
		"0xCb", // reserved
		"0xCc", // reserved
		"0xCd", // reserved
		"0xCe", // reserved
		"0xCf", // reserved
		"0xD0", // reserved
		"0xD1", // reserved
		"0xD2", // reserved
		"0xD3", // reserved
		"0xD4", // reserved
		"0xD5", // reserved
		"0xD6", // reserved
		"0xD7", // reserved
		"0xD8", // unassigned
		"0xD9", // unassigned
		"0xDa", // unassigned
		"OemOpenBrackets", // 0xdb Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
		"OemPipe", // 0xdc Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
		"OemCloseBrackets", // 0xdd Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
		"OemQuotes", // 0xde Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
		"Oem8", // 0xdf Used for miscellaneous characters; it can vary by keyboard.
		"0xE0", // reserved
		"0xE1", // OEM specific
		"OemBackslash", // 0xe2 Either the angle bracket key or the backslash key on the RT 102-key keyboard
		"0xE3", // OEM specific
		"0xE4", // OEM specific
		"ProcessKey", // 0xe5 IME Process key
		"0xE6", // OEM specific
		"Packet", // 0xe7 Used to pass Unicode characters as if they were keystrokes. The Packet key value is the low word of a 32-bit virtual-key value used for non-keyboard input methods.
		"0xE8", // is unassigned
		"0xE9", // OEM specific
		"0xEa", // OEM specific
		"0xEb", // OEM specific
		"0xEc", // OEM specific
		"0xEd", // OEM specific
		"0xEe", // OEM specific
		"0xEf", // OEM specific
		"0xF0", // OEM specific
		"0xF1", // OEM specific
		"0xF2", // OEM specific
		"0xF3", // OEM specific
		"0xF4", // OEM specific
		"0xF5", // OEM specific
		"Attn", // 0xf6
		"CrSel", // 0xf7
		"ExSel", // 0xf8
		"EraseEof", // 0xf9
		"Play", // 0xfa
		"Zoom", // 0xfb
		"NoName", // 0xfc Reserved
		"Pa1", // 0xfd
		"OemClear" // 0xfe
	};
}